#include "Optimiser.h"

/****************************************************************************
  Create a new Optimiser object

  Be sure to call

  igraph_i_set_attribute_table(&igraph_cattribute_table);

  before using this package, otherwise the attribute handling
  will not be dealt with correctly.

  Parameters:
    consider_comms
                 -- Consider communities in a specific manner:
        ALL_COMMS       -- Consider all communities for improvement.
        ALL_NEIGH_COMMS -- Consider all neighbour communities for
                           improvement.
        RAND_COMM       -- Consider a random commmunity for improvement.
        RAND_NEIGH_COMM -- Consider a random community among the neighbours
                           for improvement.
****************************************************************************/
Optimiser::Optimiser()
{
  this->consider_comms = Optimiser::ALL_NEIGH_COMMS;
}

Optimiser::~Optimiser()
{
  //dtor
}

void Optimiser::print_settings()
{
  cerr << "Consider communities method:\t" << this->consider_comms << endl;
}

/*****************************************************************************
  optimise the provided partition.
*****************************************************************************/
double Optimiser::optimise_partition(MutableVertexPartition* partition)
{
  vector<MutableVertexPartition*> partitions(1, NULL);
  partitions[0] = partition;
  vector<double> layer_weights(1, 1.0);
  return this->optimise_partition(partitions, layer_weights);
}

/*****************************************************************************
  optimise the providede partitions simultaneously. We here use the sum
  of the difference of the moves as the overall quality function, each partition
  weighted by the layer weight.
*****************************************************************************/
/*****************************************************************************
  optimise the provided partition.
*****************************************************************************/
double Optimiser::optimise_partition(vector<MutableVertexPartition*> partitions, vector<double> layer_weights)
{
  #ifdef DEBUG
    cerr << "void Optimiser::optimise_partition(vector<MutableVertexPartition*> partitions, vector<double> layer_weights)" << endl;
  #endif

  double q = 0.0;

  // Number of multiplex layers
  size_t nb_layers = partitions.size();
  if (nb_layers == 0)
    throw Exception("No partitions provided.");

  // Get graphs for all layers
  vector<Graph*> graphs(nb_layers, NULL);
  for (size_t layer = 0; layer < nb_layers; layer++)
    graphs[layer] = partitions[layer]->get_graph();

  // Number of nodes in the graphs. Should be the same across
  // all graphs, so we only take the first one.
  size_t n = graphs[0]->vcount();

  // Make sure that all graphs contain the exact same number of nodes.
  // We assume the index of each vertex in the graph points to the
  // same node (but then in a different layer).
  for (size_t layer = 0; layer < nb_layers; layer++)
    if (graphs[layer]->vcount() != n)
      throw Exception("Number of nodes are not equal for all graphs.");

  // Initialize the vector of the collapsed graphs for all layers
  vector<Graph*> collapsed_graphs(nb_layers, NULL);
  vector<MutableVertexPartition*> collapsed_partitions(nb_layers, NULL);

  // Declare the collapsed_graph variable which will contain the graph
  // collapsed by its communities. We will use this variables at each
  // further iteration, so we don't keep a collapsed graph at each pass.
  for (size_t layer = 0; layer < nb_layers; layer++)
  {
    collapsed_graphs[layer] = graphs[layer];
    collapsed_partitions[layer] = partitions[layer];
  }

  // This reflects the aggregate node, which to start with is simply equal to the graph.
  vector<size_t> aggregate_node_per_individual_node = range(n);
  int aggregate_further = true;
  // As long as there remains improvement iterate
  double total_improv = 0.0;
  double improv = 0.0;
  do
  {

    // Optimise partition for collapsed graph
    #ifdef DEBUG
      q = 0.0;
      for (size_t layer = 0; layer < nb_layers; layer++)
        q += partitions[layer]->quality()*layer_weights[layer];
      cerr << "Quality before moving " <<  q << endl;
    #endif
    
    improv = this->move_nodes(collapsed_partitions, layer_weights);
    total_improv += improv;

    #ifdef DEBUG
      cerr << "Found " << collapsed_partitions[0]->nb_communities() << " communities, improved " << improv << endl;
      q = 0.0;
      for (size_t layer = 0; layer < nb_layers; layer++)
        q += partitions[layer]->quality()*layer_weights[layer];
      cerr << "Quality after moving " <<  q << endl;
    #endif // DEBUG

    // Make sure improvement on coarser scale is reflected on the
    // scale of the graph as a whole.
    for (size_t layer = 0; layer < nb_layers; layer++)
    {
      if (collapsed_partitions[layer] != partitions[layer])
      {
        partitions[layer]->from_coarse_partition(collapsed_partitions[layer]);
      }
    }

    #ifdef DEBUG
      q = 0.0;
      for (size_t layer = 0; layer < nb_layers; layer++)
        q += partitions[layer]->quality()*layer_weights[layer];
      cerr << "Quality on finer partition " << q << endl;
    #endif // DEBUG

    #ifdef DEBUG
        cerr << "Number of communities: " << partitions[0]->nb_communities() << endl;
    #endif

    // Collapse graph (i.e. community graph)
    vector<Graph*> new_collapsed_graphs(nb_layers, NULL);
    vector<MutableVertexPartition*> new_collapsed_partitions(nb_layers, NULL);

    for (size_t layer = 0; layer < nb_layers; layer++)
    {
      new_collapsed_graphs[layer] = collapsed_graphs[layer]->collapse_graph(collapsed_partitions[layer]);
      // Create collapsed partition (i.e. default partition of each node in its own community).
      new_collapsed_partitions[layer] = collapsed_partitions[layer]->create(new_collapsed_graphs[layer]);
      #ifdef DEBUG
        cerr << "Layer " << layer << endl;
        cerr << "Old collapsed graph " << collapsed_graphs[layer] << ", vcount is " << collapsed_graphs[layer]->vcount() << endl;
        cerr << "New collapsed graph " << new_collapsed_graphs[layer] << ", vcount is " << new_collapsed_graphs[layer]->vcount() << endl;
      #endif
    }

    aggregate_further = (new_collapsed_graphs[0]->vcount() < collapsed_graphs[0]->vcount()) &&
                        (collapsed_graphs[0]->vcount() > collapsed_partitions[0]->nb_communities());

    #ifdef DEBUG
      cerr << "Aggregate further " << aggregate_further << endl;
    #endif

    // Delete the previous collapsed partition and graph
    for (size_t layer = 0; layer < nb_layers; layer++)
    {
      if (collapsed_partitions[layer] != partitions[layer])
        delete collapsed_partitions[layer];
      if (collapsed_graphs[layer] != graphs[layer])
        delete collapsed_graphs[layer];
    }

    // and set them to the new one.
    collapsed_partitions = new_collapsed_partitions;
    collapsed_graphs = new_collapsed_graphs;

    #ifdef DEBUG
      for (size_t layer = 0; layer < nb_layers; layer++)
      {
        cerr <<   "Calculate partition " << layer  << " quality." << endl;
        q = partitions[layer]->quality()*layer_weights[layer];
        cerr <<   "Calculate collapsed partition quality." << endl;
        double q_collapsed = 0.0;
        q_collapsed += collapsed_partitions[layer]->quality()*layer_weights[layer];
        if (fabs(q - q_collapsed) > 1e-6)
        {
          cerr << "ERROR: Quality of original partition and collapsed partition are not equal." << endl;
        }
        cerr <<   "partition->quality()=" << q
             << ", collapsed_partition->quality()=" << q_collapsed << endl;
        cerr <<   "graph->total_weight()=" << graphs[layer]->total_weight()
             << ", collapsed_graph->total_weight()=" << collapsed_graphs[layer]->total_weight() << endl;
        cerr <<   "graph->vcount()=" << graphs[layer]->vcount()
             << ", collapsed_graph->vcount()="  << collapsed_graphs[layer]->vcount() << endl;
        cerr <<   "graph->ecount()=" << graphs[layer]->ecount()
             << ", collapsed_graph->ecount()="  << collapsed_graphs[layer]->ecount() << endl;
        cerr <<   "graph->is_directed()=" << graphs[layer]->is_directed()
             << ", collapsed_graph->is_directed()="  << collapsed_graphs[layer]->is_directed() << endl;
        cerr <<   "graph->correct_self_loops()=" << graphs[layer]->correct_self_loops()
             << ", collapsed_graph->correct_self_loops()="  << collapsed_graphs[layer]->correct_self_loops() << endl << endl;
      }
    #endif // DEBUG

  } while (improv > 0);

  // Clean up memory after use.
  for (size_t layer = 0; layer < nb_layers; layer++)
  {
    if (collapsed_partitions[layer] != partitions[layer])
      delete collapsed_partitions[layer];

    if (collapsed_graphs[layer] != graphs[layer])
      delete collapsed_graphs[layer];
  }

  // Make sure the resulting communities are called 0,...,r-1
  // where r is the number of communities.
  q = 0.0;
  vector<size_t> membership = MutableVertexPartition::renumber_communities(partitions);
  // We only renumber the communities for the first graph,
  // since the communities for the other graphs should just be equal
  // to the membership of the first graph.
  for (size_t layer = 0; layer < nb_layers; layer++)
  {
    partitions[layer]->renumber_communities(membership);
    q += partitions[layer]->quality()*layer_weights[layer];
  }
  return improv;
}

/*****************************************************************************
    Move nodes to other communities depending on how other communities are
    considered, see consider_comms parameter of the class.

    Parameters:
      partition -- The partition to optimise.
******************************************************************************/
double Optimiser::move_nodes(MutableVertexPartition* partition)
{
  return this->move_nodes(partition, this->consider_comms);
}

double Optimiser::move_nodes(MutableVertexPartition* partition, int consider_comms)
{
  vector<MutableVertexPartition*> partitions(1, NULL);
  partitions[0] = partition;
  vector<double> layer_weights(1, 1.0);
  return this->move_nodes(partitions, layer_weights, consider_comms, this->consider_empty_community);
}

/*****************************************************************************
  Move nodes to neighbouring communities such that each move improves the
  given quality function maximally (i.e. greedily) for multiple layers,
  i.e. for multiplex networks. Each node will be in the same community in
  each layer, but the method may be different, or the weighting may be
  different for different layers. Notably, this can be used in the case of
  negative links, where you would like to weigh the negative links with a
  negative weight.

  Parameters:
    partitions -- The partitions to optimise.
    layer_weights -- The weights used for the different layers.
******************************************************************************/
double Optimiser::move_nodes(vector<MutableVertexPartition*> partitions, vector<double> layer_weights)
{
  return this->move_nodes(partitions, layer_weights, this->consider_comms, this->consider_empty_community);
}

double Optimiser::move_nodes(vector<MutableVertexPartition*> partitions, vector<double> layer_weights, int consider_comms, int consider_empty_community)
{
  #ifdef DEBUG
    cerr << "double Optimiser::move_nodes_multiplex(vector<MutableVertexPartition*> partitions, vector<double> weights)" << endl;
  #endif
  // Number of multiplex layers
  size_t nb_layers = partitions.size();
  if (nb_layers == 0)
    return -1.0;
  // Get graphs
  vector<Graph*> graphs(nb_layers, NULL);
  for (size_t layer = 0; layer < nb_layers; layer++)
    graphs[layer] = partitions[layer]->get_graph();
  // Number of nodes in the graph
  size_t n = graphs[0]->vcount();

  // Total improvement while moving nodes
  double total_improv = 0.0;

  for (size_t layer = 0; layer < nb_layers; layer++)
    if (graphs[layer]->vcount() != n)
      throw Exception("Number of nodes are not equal for all graphs.");
  // Number of moved nodes during one loop
  size_t nb_moves = 1;

  // We use a random order, we shuffle this order.
  vector<size_t> nodes = range(n);
  shuffle(nodes);

  // Initialize the degree vector
  // If we want to debug the function, we will calculate some additional values.
  // In particular, the following consistencies could be checked:
  // (1) - The difference in the quality function after a move should match
  //       the reported difference when calling diff_move.
  // (2) - The quality function should be exactly the same value after
  //       aggregating/collapsing the graph.

  // As long as there remain changes
  double eps = 1e-10;
  double improv = 0.0;
  while(nb_moves > 0)
  {
    improv = 0.0;
    nb_moves = 0;
    for (vector<size_t>::iterator v_it = nodes.begin();
         v_it!= nodes.end();
         v_it++)
    {
      size_t v = *v_it;

      set<size_t> comms;
      Graph* graph = NULL;
      MutableVertexPartition* partition = NULL;
      // What is the current community of the node (this should be the same for all layers)
      size_t v_comm = partitions[0]->membership(v);

      if (consider_comms == ALL_COMMS)
      {
        for(size_t comm = 0; comm < partitions[0]->nb_communities(); comm++)
        {
          for (size_t layer = 0; layer < nb_layers; layer++)
          {
            if (partitions[layer]->get_community(comm).size() > 0)
            {
              comms.insert(comm);
              break; // Break from for loop in layer
            }
          }

        }
      }
      else if (consider_comms == ALL_NEIGH_COMMS)
      {
        /****************************ALL NEIGH COMMS*****************************/
        for (size_t layer = 0; layer < nb_layers; layer++)
        {
          vector<size_t> const& neigh_comm_layer = partitions[layer]->get_neigh_comms(v, IGRAPH_ALL);
          comms.insert(neigh_comm_layer.begin(), neigh_comm_layer.end());
        }
      }
      else if (consider_comms == RAND_COMM)
      {
        /****************************RAND COMM***********************************/
        comms.insert( partitions[0]->membership(graphs[0]->get_random_node()) );
      }
      else if (consider_comms == RAND_NEIGH_COMM)
      {
        /****************************RAND NEIGH COMM*****************************/
        size_t rand_layer = get_random_int(0, nb_layers - 1);
        if (graphs[rand_layer]->degree(v, IGRAPH_ALL) > 0)
          comms.insert( partitions[0]->membership(graphs[rand_layer]->get_random_neighbour(v, IGRAPH_ALL)) );
      }

      #ifdef DEBUG
        cerr << "Consider " << comms.size() << " communities for moving." << endl;
      #endif

      size_t max_comm = v_comm;
      double max_improv = 0.0;
      for (set<size_t>::iterator comm_it = comms.begin();
           comm_it!= comms.end();
           comm_it++)
      {
        size_t comm = *comm_it;
        double possible_improv = 0.0;

        // Consider the improvement of moving to a community for all layers
        for (size_t layer = 0; layer < nb_layers; layer++)
        {
          graph = graphs[layer];
          partition = partitions[layer];
          // Make sure to multiply it by the weight per layer
          possible_improv += layer_weights[layer]*partition->diff_move(v, comm);
        }

        if (possible_improv > max_improv && possible_improv > eps)
        {
          max_comm = comm;
          max_improv = possible_improv;
        }
      }

      // Check if we should move to an empty community
      if (consider_empty_community)
      {
        for (size_t layer = 0; layer < nb_layers; layer++)
        {
          graph = graphs[layer];
          partition = partitions[layer];
          if ( partition->get_community(v_comm).size() > 1 )  // We should not move a node when it is already in its own empty community (this may otherwise create more empty communities than nodes)
          {
            size_t comm = partition->get_empty_community();
            #ifdef DEBUG
              cerr << "Checking empty community (" << comm << ") for partition " << partition << endl;
            #endif
            if (comm == partition->nb_communities())
            {
              // If the empty community has just been added, we need to make sure
              // that is has also been added to the other layers
              for (size_t layer_2 = 0; layer_2 < nb_layers; layer_2++)
                partitions[layer_2]->add_empty_community();
            }

            double possible_improv = 0.0;
            for (size_t layer_2 = 0; layer_2 < nb_layers; layer_2++)
            {
              possible_improv += layer_weights[layer_2]*partitions[layer_2]->diff_move(v, comm);
            }
            #ifdef DEBUG
              cerr << "Improvement to empty community: " << possible_improv << ", maximum improvement: " << max_improv << endl;
            #endif
            if (possible_improv > max_improv)
            {
              max_improv = possible_improv;
              max_comm = comm;
            }
          }
        }
      }

      // If we actually plan to move the node
      if (max_comm != v_comm)
      {
        // Keep track of improvement
        improv += max_improv;

        #ifdef DEBUG
          // If we are debugging, calculate quality function
          double q_improv = 0;
        #endif

        for (size_t layer = 0; layer < nb_layers; layer++)
        {
          MutableVertexPartition* partition = partitions[layer];

          #ifdef DEBUG
            // If we are debugging, calculate quality function
            double q1 = partition->quality();
          #endif

          // Actually move the node
          partition->move_node(v, max_comm);
          #ifdef DEBUG
            // If we are debugging, calculate quality function
            // and report difference
            double q2 = partition->quality();
            double q_delta = layer_weights[layer]*(q2 - q1);
            q_improv += q_delta;
            cerr << "Move node " << v
            << " from " << v_comm << " to " << max_comm << " for layer " << layer
            << " (diff_move=" << max_improv
            << ", q2 - q1=" << q_delta << ")" << endl;
          #endif
        }
        #ifdef DEBUG
          if (fabs(q_improv - max_improv) > 1e-16)
          {
            cerr << "ERROR: Inconsistency while moving nodes, improvement as measured by quality function did not equal the improvement measured by the diff_move function." << endl
                 << " (diff_move=" << max_improv
                 << ", q2 - q1=" << q_improv << ")" << endl;
          }
        #endif

        // Keep track of number of moves
        nb_moves += 1;
      }
    }
    total_improv += improv;
  }

  partitions[0]->renumber_communities();
  vector<size_t> const& membership = partitions[0]->membership();
  for (size_t layer = 1; layer < nb_layers; layer++)
  {
    partitions[layer]->renumber_communities(membership);
    #ifdef DEBUG
      cerr << "Renumbered communities for layer " << layer << " for " << partitions[layer]->nb_communities() << " communities." << endl;
    #endif DEBUG
  }
  return total_improv;
}

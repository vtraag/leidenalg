#include "Optimiser.h"

/****************************************************************************
  Create a new Optimiser object

  Be sure to call

  igraph_i_set_attribute_table(&igraph_cattribute_table);

  before using this package, otherwise the attribute handling
  will not be dealt with correctly.

  Parameters:
    eps          -- If the improvement falls below this threshold,
                    stop iterating.
    delta        -- If the number of nodes that moves falls below
                    this threshold, stop iterating.
    max_itr      -- Maximum number of iterations to perform.
    random_order
                 -- If True the nodes will be traversed in a random order
                    when optimising a quality function.
    min_diff_resolution
                 -- If the difference in resolution falls below this
                    threshold when bisectioning on a resolution parameter,
                    we won't bisection further.
    min_diff_bisect_value
                 -- If the difference in the bisection value falls below
                    this threshold when bisectioning on a resolution
                    parameter, we won't bisection further.
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
  this->eps = 1e-5;
  this->delta = 1e-2;
  this->max_itr = 10000;
  this->random_order = true;
  this->consider_comms = Optimiser::ALL_NEIGH_COMMS;
  this->smart_local_move = false;
  this->move_individual = false;
  this->consider_empty_community = false;
}

Optimiser::~Optimiser()
{
  //dtor
}

void Optimiser::print_settings()
{
  cerr << "Epsilon:\t" << this->eps << endl;
  cerr << "Delta:\t" << this->delta << endl;
  cerr << "Maximum # iterators:\t" << this->max_itr << endl;
  cerr << "Random node order:\t" << this->random_order << endl;
  cerr << "Consider communities method:\t" << this->consider_comms << endl;
  cerr << "Smart local move:\t" << this->smart_local_move << endl;
  cerr << "Move individual:\t" << this->move_individual << endl;
  cerr << "Consider empty community:\t" << this->consider_empty_community << endl;
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

  // Do one iteration of optimisation
  double improv = this->move_nodes(partitions, layer_weights, this->consider_comms);

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

  // As long as there remains improvement iterate
  do
  {
    // First collapse graph (i.e. community graph)
    // If we do smart local movement, we separate communities in slightly more
    // fine-grained parts for which we collapse the graph.
    vector<MutableVertexPartition*> sub_collapsed_partitions(nb_layers, NULL);

    vector<Graph*> new_collapsed_graphs(nb_layers, NULL);
    vector<MutableVertexPartition*> new_collapsed_partitions(nb_layers, NULL);

    improv = 0.0;

    if (this->smart_local_move)
    {
      // First create a new partition, which should be a sub partition
      // of the collapsed partition, i.e. such that all clusters of
      // the partition are strictly partitioned in the subpartition.

      #ifdef DEBUG
        cerr << "\tBefore SLM " << collapsed_partitions[0]->nb_communities() << " communities." << endl;
      #endif
      for (size_t layer = 0; layer < nb_layers; layer++)
      {
        sub_collapsed_partitions[layer] = collapsed_partitions[layer]->create(collapsed_graphs[layer]);
      }
      // Then move around nodes but restrict movement to within original communities.
      #ifdef DEBUG
        cerr << "\tStarting SLM with " << sub_collapsed_partitions[0]->nb_communities() << " communities." << endl;
      #endif
      this->move_nodes_constrained(sub_collapsed_partitions, layer_weights, collapsed_partitions[0]->membership());
      #ifdef DEBUG
        cerr << "\tAfter applying SLM found " << sub_collapsed_partitions[0]->nb_communities() << " communities." << endl;
      #endif

      // Determine new aggregate node per individual node
      for (size_t v = 0; v < n; v++)
      {
        size_t aggregate_node = aggregate_node_per_individual_node[v];
        aggregate_node_per_individual_node[v] = sub_collapsed_partitions[0]->membership(aggregate_node);
      }

      // Collapse graph based on sub collapsed partition
      for (size_t layer = 0; layer < nb_layers; layer++)
      {
        new_collapsed_graphs[layer] = collapsed_graphs[layer]->collapse_graph(sub_collapsed_partitions[layer]);
      }

      // Determine the membership for the collapsed graph
      vector<size_t> new_collapsed_membership(new_collapsed_graphs[0]->vcount());

      // Every node within the collapsed graph should be assigned
      // to the community of the original partition before the slm.
      // We thus check for each node what the community is in the slm_partition
      // and set the membership equal to the original partition (i.e.
      // even though the aggregation may be slightly different, the
      // membership of the aggregated nodes is as indicated by the original partition.)
      #ifdef DEBUG
        cerr << "SLM\tOrig" << endl;
      #endif // DEBUG
      for (size_t v = 0; v < collapsed_graphs[0]->vcount(); v++)
      {
        size_t new_aggregate_node = sub_collapsed_partitions[0]->membership(v);
        new_collapsed_membership[new_aggregate_node] = collapsed_partitions[0]->membership(v);
        #ifdef DEBUG
          //cerr << sub_collapsed_partition->membership(v) << "\t" << sub_collapsed_partition->membership(v) << endl;
        #endif // DEBUG
      }

      // Create new collapsed partition
      for (size_t layer = 0; layer < nb_layers; layer++)
      {
        delete sub_collapsed_partitions[layer];
        new_collapsed_partitions[layer] = collapsed_partitions[layer]->create(new_collapsed_graphs[layer], new_collapsed_membership);
      }
    }
    else if (this->move_individual)
    {
      #ifdef DEBUG
        q = 0.0;
        for (size_t layer = 0; layer < nb_layers; layer++)
          q += partitions[layer]->quality()*layer_weights[layer];
        cerr << "Quality before moving individual nodes " <<  q << endl;
      #endif
      improv += this->move_nodes(partitions, layer_weights, this->consider_comms);
      #ifdef DEBUG
        cerr << "Found " << partitions[0]->nb_communities() << " communities, improved " << improv << endl;
        q = 0.0;
        for (size_t layer = 0; layer < nb_layers; layer++)
          q += partitions[layer]->quality()*layer_weights[layer];
        cerr << "Quality after moving individual nodes " << q << endl;
      #endif

      for (size_t layer = 0; layer < nb_layers; layer++)
      {
        new_collapsed_graphs[layer] = collapsed_graphs[layer]->collapse_graph(collapsed_partitions[layer]);
        // Create collapsed partition (i.e. default partition of each node in its own community).
        new_collapsed_partitions[layer] = collapsed_partitions[layer]->create(new_collapsed_graphs[layer]);
      }
    }
    else
    {
      for (size_t layer = 0; layer < nb_layers; layer++)
      {
        new_collapsed_graphs[layer] = collapsed_graphs[layer]->collapse_graph(collapsed_partitions[layer]);
        // Create collapsed partition (i.e. default partition of each node in its own community).
        new_collapsed_partitions[layer] = collapsed_partitions[layer]->create(new_collapsed_graphs[layer]);
      }
    }


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
        cerr <<   "graph->ecount()=" << graphs[layer]->ecount()
             << ", collapsed_graph->ecount()="  << collapsed_graphs[layer]->ecount() << endl;
        cerr <<   "graph->is_directed()=" << graphs[layer]->is_directed()
             << ", collapsed_graph->is_directed()="  << collapsed_graphs[layer]->is_directed() << endl;
        cerr <<   "graph->correct_self_loops()=" << graphs[layer]->correct_self_loops()
             << ", collapsed_graph->correct_self_loops()="  << collapsed_graphs[layer]->correct_self_loops() << endl;
      }
    #endif // DEBUG

    // Optimise partition for collapsed graph
    #ifdef DEBUG
      q = 0.0;
      for (size_t layer = 0; layer < nb_layers; layer++)
        q += partitions[layer]->quality()*layer_weights[layer];
      cerr << "Quality before moving " <<  q << endl;
    #endif
    improv += this->move_nodes(collapsed_partitions, layer_weights, this->consider_comms);
    #ifdef DEBUG
      cerr << "Found " << collapsed_partitions[0]->nb_communities() << " communities, improved " << improv << endl;
      q = 0.0;
      for (size_t layer = 0; layer < nb_layers; layer++)
        q += partitions[layer]->quality()*layer_weights[layer];
      cerr << "Quality after moving " <<  q << endl << endl;
    #endif // DEBUG

    // Make sure improvement on coarser scale is reflected on the
    // scale of the graph as a whole.
    for (size_t layer = 0; layer < nb_layers; layer++)
    {
      if (this->smart_local_move)
        partitions[layer]->from_coarser_partition(collapsed_partitions[layer], aggregate_node_per_individual_node);
      else
        partitions[layer]->from_coarser_partition(collapsed_partitions[layer]);
    }

    #ifdef DEBUG
      q = 0.0;
      for (size_t layer = 0; layer < nb_layers; layer++)
        q += partitions[layer]->quality()*layer_weights[layer];
      cerr << "Quality on finer partition " << q << endl << endl;
    #endif // DEBUG

    #ifdef DEBUG
        cerr << "Number of communities: " << partitions[0]->nb_communities() << endl;
    #endif
  } while (improv > this->eps);

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
  partitions[0]->renumber_communities();
  vector<size_t> membership = partitions[0]->membership();
  // We only renumber the communities for the first graph,
  // since the communities for the other graphs should just be equal
  // to the membership of the first graph.
  for (size_t layer = 1; layer < nb_layers; layer++)
  {
    partitions[layer]->renumber_communities(membership);
    q += partitions[layer]->quality()*layer_weights[layer];
  }
  return q;
}

/*****************************************************************************
    Move nodes to other communities depending on how other communities are
    considered, see consider_comms parameter of the class.

    Parameters:
      partition -- The partition to optimise.
******************************************************************************/
double Optimiser::move_nodes(MutableVertexPartition* partition, int consider_comms)
{
  vector<MutableVertexPartition*> partitions(1, NULL);
  partitions[0] = partition;
  vector<double> layer_weights(1, 1.0);
  return this->move_nodes(partitions, layer_weights, consider_comms);
}

double Optimiser::move_nodes_constrained(MutableVertexPartition* partition, vector<size_t> const& constrained_membership)
{
  vector<MutableVertexPartition*> partitions(1, NULL);
  partitions[0] = partition;
  vector<double> layer_weights(1, 1.0);
  return this->move_nodes_constrained(partitions, layer_weights, constrained_membership);
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
double Optimiser::move_nodes(vector<MutableVertexPartition*> partitions, vector<double> layer_weights, int consider_comms)
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

  // Number of iterations
  size_t itr = 0;
  // Total improvement while moving nodes
  double total_improv = 0.0;
  // Improvement for one loop
  double improv = 2*this->eps*nb_layers;
  for (size_t layer = 0; layer < nb_layers; layer++)
    if (graphs[layer]->vcount() != n)
      throw Exception("Number of nodes are not equal for all graphs.");
  // Number of moved nodes during one loop
  size_t nb_moves = 2*n*nb_layers;
  // Initialize the degree vector
  // If we want to debug the function, we will calculate some additional values.
  // In particular, the following consistencies could be checked:
  // (1) - The difference in the quality function after a move should match
  //       the reported difference when calling diff_move.
  // (2) - The quality function should be exactly the same value after
  //       aggregating/collapsing the graph.

  // As long as we keep on improving
  while (improv > this->eps*nb_layers &&
         nb_moves > n*this->delta*nb_layers &&
         itr < this->max_itr)
  {
    itr += 1;
    nb_moves = 0;
    improv = 0.0;
    // Establish vertex order
    vector<size_t> vertex_order = range(n);
    if (this->random_order)
      random_shuffle( vertex_order.begin(), vertex_order.end() );
    // For each node
    for(vector<size_t>::iterator it_vertex = vertex_order.begin();
        it_vertex != vertex_order.end(); ++it_vertex)
    {
      size_t v = *it_vertex; // The actual vertex we will now consider
      unordered_map<size_t, double> comm_improvs;
      size_t neigh_comm;
      unordered_set<size_t>* neigh_comms = NULL;
      Graph* graph = NULL;
      MutableVertexPartition* partition = NULL;
      // What is the current community of the node (this should be the same for all layers)
      size_t v_comm = partitions[0]->membership(v);
      switch (consider_comms)
      {
        /****************************ALL COMMS**********************************/
        case ALL_COMMS:
          for(size_t comm = 0; comm < partitions[0]->nb_communities(); comm++)
          {
            if (partitions[0]->csize(comm) > 0)
            {
              // Consider the improvement of moving to a community for all layers
              for (size_t layer = 0; layer < nb_layers; layer++)
              {
                graph = graphs[layer];
                partition = partitions[layer];
                // Make sure to multiply it by the weight per layer
                comm_improvs[comm] += layer_weights[layer]*partition->diff_move(v, comm);
              }
            }
          }
          break;
        /****************************ALL NEIGH COMMS*****************************/
        case ALL_NEIGH_COMMS:
          neigh_comms = new unordered_set<size_t>();
          for (size_t layer = 0; layer < nb_layers; layer++)
          {
            vector<size_t> const& neigh_comm_layer = partitions[layer]->get_neigh_comms(v, IGRAPH_ALL);
            neigh_comms->insert(neigh_comm_layer.begin(), neigh_comm_layer.end());
          }
          for (unordered_set<size_t>::iterator neigh_comm_it = neigh_comms->begin();
               neigh_comm_it != neigh_comms->end(); ++neigh_comm_it)
          {
            // Consider the improvement of moving to a community for all layers
            neigh_comm = *neigh_comm_it;
            for (size_t layer = 0; layer < nb_layers; layer++)
            {
              graph = graphs[layer];
              partition = partitions[layer];
              // Make sure to multiply it by the weight per layer
              comm_improvs[neigh_comm] += layer_weights[layer]*partition->diff_move(v, neigh_comm);
            }
          }
          delete neigh_comms;
          break;
        /****************************RAND COMM***********************************/
        case RAND_COMM:
          neigh_comm = partitions[0]->membership(graphs[0]->get_random_node());
          for (size_t layer = 0; layer < nb_layers; layer++)
          {
            comm_improvs[neigh_comm] += layer_weights[layer]*partitions[layer]->diff_move(v, neigh_comm);
          }
          break;
        /****************************RAND NEIGH COMM*****************************/
        case RAND_NEIGH_COMM:
          // Community membership should be consistent across layers
          // anyway, so just read it once.
          // First select a random layer
          size_t rand_layer = graphs[0]->get_random_int(0, nb_layers - 1);
          neigh_comm = partitions[0]->membership(graphs[rand_layer]->get_random_neighbour(v, IGRAPH_ALL));
          for (size_t layer = 0; layer < nb_layers; layer++)
          {
            comm_improvs[neigh_comm] += layer_weights[layer]*partitions[layer]->diff_move(v, neigh_comm);
          }
          break;
      }
      size_t max_comm = v_comm;
      double max_improv = 0.0;
      // Determine the maximum improvement
      for (unordered_map<size_t, double>::iterator improv_it = comm_improvs.begin();
           improv_it != comm_improvs.end(); improv_it++)
      {
        size_t comm = improv_it->first;
        double local_improv = improv_it->second;
        if (local_improv > max_improv)
        {
          max_comm = comm;
          max_improv = local_improv;
        }
      }
      // Check if we should move to an empty community
      if (this->consider_empty_community && partitions[0]->csize(v_comm) > graphs[0]->node_size(v))
      {
        neigh_comm = partitions[0]->get_empty_community();
        if (neigh_comm == partitions[0]->nb_communities())
        {
          // If the empty community has just been added, we need to make sure
          // that is has also been added to the other layers
          for (size_t layer = 0; layer < nb_layers; layer++)
            partitions[layer]->add_empty_community();
        }

        double possible_improv = 0.0;
        for (size_t layer = 0; layer < nb_layers; layer++)
        {
          possible_improv += layer_weights[layer]*partitions[layer]->diff_move(v, neigh_comm);
        }

        if (possible_improv > max_improv)
        {
          max_improv = possible_improv;
          max_comm = neigh_comm;
        }
      }

      // If we actually plan to move the nove
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
          if (fabs(q_improv - max_improv) > 1e-6)
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

    // Keep track of total improvement over multiple loops
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

double Optimiser::move_nodes_constrained(vector<MutableVertexPartition*> partitions, vector<double> layer_weights, vector<size_t> const& constrained_membership)
{
  #ifdef DEBUG
    cerr << "double Optimiser::move_nodes_constrained(vector<MutableVertexPartition*> partitions, vector<double> layer_weights, vector<size_t> const& constrained_membership)" << endl;
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

  // Number of iterations
  size_t itr = 0;
  // Total improvement while moving nodes
  double total_improv = 0.0;
  // Improvement for one loop
  double improv = 2*this->eps*nb_layers;
  for (size_t layer = 0; layer < nb_layers; layer++)
    if (graphs[layer]->vcount() != n)
      throw Exception("Number of nodes are not equal for all graphs.");
  // Number of moved nodes during one loop
  size_t nb_moves = 2*n*nb_layers;
  // Initialize the degree vector
  // If we want to debug the function, we will calculate some additional values.
  // In particular, the following consistencies could be checked:
  // (1) - The difference in the quality function after a move should match
  //       the reported difference when calling diff_move.
  // (2) - The quality function should be exactly the same value after
  //       aggregating/collapsing the graph.

  // As long as we keep on improving
  while (improv > this->eps &&
         nb_moves > n*this->delta &&
         itr < this->max_itr)
  {
    // Increase number of iterations
    itr += 1;

    // Initialize number of moves and improvement
    nb_moves = 0;
    improv = 0.0;

    // Establish vertex order
    // We normally initialize the normal vertex order
    // of considering node 0,1,...
    vector<size_t> vertex_order = range(n);
    // But if we use a random order, we shuffle this order.
    if (this->random_order)
      random_shuffle( vertex_order.begin(), vertex_order.end() );

    // For each node
    for(vector<size_t>::iterator it_vertex = vertex_order.begin();
        it_vertex != vertex_order.end(); ++it_vertex)
    {
      size_t v = *it_vertex; // The actual vertex we will now consider
      unordered_map<size_t, double> comm_improvs;
      size_t neigh_comm;
      unordered_set<size_t>* neigh_comms = NULL;
      Graph* graph = NULL;
      MutableVertexPartition* partition = NULL;
      // What is the current community of the node (this should be the same for all layers)
      size_t v_comm = partitions[0]->membership(v);

      neigh_comms = new unordered_set<size_t>();
      for (size_t layer = 0; layer < nb_layers; layer++)
      {
        unordered_set<size_t>* neigh_comm_layer = partitions[layer]->get_neigh_comms(v, IGRAPH_ALL, constrained_membership);
        neigh_comms->insert(neigh_comm_layer->begin(), neigh_comm_layer->end());
        delete neigh_comm_layer;
      }
      for (unordered_set<size_t>::iterator neigh_comm_it = neigh_comms->begin();
           neigh_comm_it != neigh_comms->end(); ++neigh_comm_it)
      {
        // Consider the improvement of moving to a community for all layers
        neigh_comm = *neigh_comm_it;
        for (size_t layer = 0; layer < nb_layers; layer++)
        {
          graph = graphs[layer];
          partition = partitions[layer];
          // Make sure to multiply it by the weight per layer
          comm_improvs[neigh_comm] += layer_weights[layer]*partition->diff_move(v, neigh_comm);
        }
      }
      delete neigh_comms;

      size_t max_comm = v_comm;
      double max_improv = 0.0;
      // TODO: Not implemented yet to consider moving to an empty community for
      // several layers of graphs.
      // Determine the maximum improvement
      for (unordered_map<size_t, double>::iterator improv_it = comm_improvs.begin();
           improv_it != comm_improvs.end(); improv_it++)
      {
        size_t comm = improv_it->first;
        double local_improv = improv_it->second;
        if (local_improv > max_improv)
        {
          max_comm = comm;
          max_improv = local_improv;
        }
      }

      // If we actually plan to move the nove
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
          if (fabs(q_improv - max_improv) > 1e-6)
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
    // Keep track of total improvement over multiple loops
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

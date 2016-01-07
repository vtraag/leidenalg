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
  this->aggregate_smart_local_move = false;
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
  cerr << "Aggregate smart local move:\t" << this->aggregate_smart_local_move << endl;
  cerr << "Move individual:\t" << this->move_individual << endl;
  cerr << "Consider empty community:\t" << this->consider_empty_community << endl;
}

/*****************************************************************************
  optimise the provided partition.
*****************************************************************************/
double Optimiser::optimise_partition(MutableVertexPartition* partition)
{
  #ifdef DEBUG
    cerr << "void Optimiser::optimise_partition(MutableVertexPartition* partition)" << endl;
  #endif

  #ifdef DEBUG
    cerr << "Using partition at address " << partition << endl;
  #endif

  // Get the graph from the partition
  Graph* graph = partition->get_graph();

  #ifdef DEBUG
    cerr << "Using graph at address " << graph << endl;
  #endif

  // Declare the collapsed_graph variable which will contain the graph
  // collapsed by its communities. We will use this variables at each
  // further iteration, so we don't keep a collapsed graph at each pass.
  Graph* collapsed_graph = NULL;
  MutableVertexPartition* collapsed_partition = NULL;

  // Do one iteration of optimisation
  double improv = 2*this->eps;
  // As long as there remains improvement iterate
  while (improv > this->eps)
  {
    // First collapse graph (i.e. community graph)
    // If we do smart local movement, we separate communities in slightly more
    // fine-grained parts for which we collapse the graph.
    MutableVertexPartition* slm_partition = NULL;

    improv = 0.0;

    if (this->smart_local_move)
    {
      // First create a new partition
      slm_partition = collapsed_partition->create(graph);

      // Then move around nodes but restrict movement to within original communities.
      if (this->aggregate_smart_local_move)
        this->move_nodes_constrained(slm_partition, collapsed_partition->membership());
      else
        this->optimise_partition_constrained(slm_partition, collapsed_partition->membership());
      #ifdef DEBUG
        cerr << "\tAfter applying SLM found " << slm_partition->nb_communities() << " communities." << endl;
      #endif

      // Collapse graph based on slm partition
      Graph* new_collapsed_graph = collapse_graph->collapse_graph(slm_partition);

      // Determine the membership for the collapsed graph
      vector< size_t > new_collapsed_membership(new_collapsed_graph->vcount());

      // Every node within the collapsed graph should be assigned
      // to the community of the original partition before the slm.
      // We thus check for each node what the community is in the slm_partition
      // and set the membership equal to the original partition (i.e.
      // even though the aggregation may be slightly different, the
      // membership of the aggregated nodes is as indicated by the original partition.)
      #ifdef DEBUG
        cerr << "SLM\tOrig" << endl;
      #endif // DEBUG
      for (size_t v = 0; v < collapsed_graph->vcount(); v++)
      {
        size_t new_aggregate_node = slm_partition->membership(v);
        new_collapsed_membership[new_aggregate_node] = collapsed_partition->membership(v);
        #ifdef DEBUG
          cerr << slm_partition->membership(v) << "\t" << slm_partition->membership(v) << endl;
        #endif // DEBUG
      }

      // Create collapsed partition. The
      MutableVertexPartition* new_collapsed_partition = collapsed_partition->create(new_collapsed_graph, new_collapsed_membership);

      // Delete the previsou collapsed partition and graph
      delete collapsed_partition;
      delete collapsed_graph;

      // and set them to the new one.
      collapsed_partition = new_collapsed_partition;
      collapsed_graph = new_collapsed_graph;
    }
    else
    {
      Graph* new_collapsed_graph = collapsed_graph->collapse_graph(collapsed_partition);

      // Create collapsed partition (i.e. default partition of each node in its own community).
      MutableVertexPartition* new_collapsed_partition = collapsed_partition->create(collapsed_graph);

      // Delete the previsou collapsed partition and graph
      delete collapsed_partition;
      delete collapsed_graph;

      // and set them to the new one.
      collapsed_partition = new_collapsed_partition;
      collapsed_graph = new_collapsed_graph;
    }
    #ifdef DEBUG
      cerr <<   "Calculate partition quality." << endl;
      double q = partition->quality();
      cerr <<   "Calculate collapsed partition quality." << endl;
      double q_collapsed = collapsed_partition->quality();
      if (fabs(q - q_collapsed) > 1e-6)
      {
        cerr << "ERROR: Quality of original partition and collapsed partition are not equal." << endl;
      }
      cerr <<   "partition->quality()=" << q
           << ", collapsed_partition->quality()=" << q_collapsed << endl;
      cerr <<   "graph->total_weight()=" << graph->total_weight()
           << ", collapsed_graph->total_weight()=" << collapsed_graph->total_weight() << endl;
      cerr <<   "graph->ecount()=" << graph->ecount()
           << ", collapsed_graph->ecount()="  << collapsed_graph->ecount() << endl;
      cerr <<   "graph->is_directed()=" << graph->is_directed()
           << ", collapsed_graph->is_directed()="  << collapsed_graph->is_directed() << endl;
      cerr <<   "graph->correct_self_loops()=" << graph->correct_self_loops()
           << ", collapsed_graph->correct_self_loops()="  << collapsed_graph->correct_self_loops() << endl;
    #endif // DEBUG

    // Optimise partition for collapsed graph
    #ifdef DEBUG
      cerr << "Quality before moving " << collapsed_partition->quality() << endl;
    #endif
    improv += this->move_nodes(collapsed_partition, this->consider_comms);
    #ifdef DEBUG
      cerr << "Found " << partition->nb_communities() << " communities, improved " << improv << endl;
      cerr << "Quality after moving " << collapsed_partition->quality() << endl << endl;
    #endif // DEBUG

    // Make sure improvement on coarser scale is reflected on the
    // scale of the graph as a whole.
    if (this->smart_local_move)
      partition->from_coarser_partition(collapsed_partition, slm_partition->membership());
    else
      partition->from_coarser_partition(collapsed_partition);

    #ifdef DEBUG
      cerr << "Quality on finer partition " << partition->quality() << endl << endl;
    #endif // DEBUG

    // Clean up memory after use.
    delete collapsed_partition;
    delete collapsed_graph;
  }
  // We renumber the communities to make sure we stick in the range
  // 0,1,...,r - 1 for r communities.
  // By default, we number the communities in decreasing order of size,
  // so that 0 is the largest community, 1 the second largest, etc...
  partition->renumber_communities();
  // Return the quality of the current partition.
  return partition->quality();
}

/*****************************************************************************
  optimise the providede partitions simultaneously. We here use the sum
  of the difference of the moves as the overall quality function, each partition
  weighted by the layer weight.
*****************************************************************************/
double Optimiser::optimise_partition(vector<MutableVertexPartition*> partitions, vector<double> layer_weights)
{
  #ifdef DEBUG
    cerr << "vector<MutableVertexPartition*> Optimiser::find_partition(vector<MutableVertexPartition*> partitions)" << endl;
  #endif

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

  // As long as there remains improvement iterate
  while (improv > this->eps)
  {
    // First collapse graphs (i.e. community graph)
    for (size_t layer = 0; layer < nb_layers; layer++)
    {
      // Get graph and partition for current layer
      Graph* graph = graphs[layer];
      MutableVertexPartition* partition = partitions[layer];

      // Create collapsed graph
      Graph* collapsed_graph = graph->collapse_graph(partition);
      collapsed_graphs[layer] = collapsed_graph;

      // Create collapsed partition (i.e. default partition of each node in its own community).
      MutableVertexPartition* collapsed_partition = partitions[layer]->create(collapsed_graph);
      collapsed_partitions[layer] = collapsed_partition;

      // Create partition for collapsed graph
      #ifdef DEBUG
        cerr <<   "Calculate partition quality." << endl;
        double q = partition->quality();
        cerr <<   "Calculate collapsed partition quality." << endl;
        double q_collapsed = collapsed_partition->quality();
        if (fabs(q - q_collapsed) > 1e-6)
        {
          cerr << "ERROR: Quality of original partition and collapsed partition are not equal." << endl;
        }
        cerr <<   "partition->quality()=" << q
             << ", collapsed_partition->quality()=" << q_collapsed << endl;
        cerr <<   "graph->total_weight()=" << graph->total_weight()
             << ", collapsed_graph->total_weight()=" << collapsed_graph->total_weight() << endl;
        cerr <<   "graph->ecount()=" << graph->ecount()
             << ", collapsed_graph->ecount()="  << collapsed_graph->ecount() << endl;
        cerr <<   "graph->is_directed()=" << graph->is_directed()
             << ", collapsed_graph->is_directed()="  << collapsed_graph->is_directed() << endl;
      #endif
    }
    // Optimise partition for all collapsed graphs
    improv = this->move_nodes(collapsed_partitions, layer_weights, this->consider_comms);

    // Make sure improvement on coarser scale is reflected on the
    // scale of the graphs as a whole.
    for (size_t layer = 0; layer < nb_layers; layer++)
    {
      partitions[layer]->from_coarser_partition(collapsed_partitions[layer]);

      delete collapsed_partitions[layer];
      delete collapsed_graphs[layer];
    }

    // Try to move individual nodes again
    if (this->move_individual)
    {
      improv += this->move_nodes(partitions, layer_weights, this->consider_comms);
    }
  }
  // Make sure the resulting communities are called 0,...,r-1
  // where r is the number of communities.
  double q = 0.0;
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
  #ifdef DEBUG
    cerr << "double Optimiser::move_nodes(MutableVertexPartition* partition)" << endl;
  #endif
  // get graph
  Graph* graph = partition->get_graph();
  // Number of iterations
  size_t itr = 0;
  // Total improvement while moving nodes
  double total_improv = 0.0;
  // Improvement for one loop
  double improv = 2*this->eps;
  // Number of nodes in the graph
  size_t n = graph->vcount();
  // Number of moved nodes during one loop
  size_t nb_moves = 2*n;
  // Initialize the degree vector
  // If we want to debug the function, we will calculate some additional values.
  // In particular, the following consistencies could be checked:
  // (1) - The difference in the quality function after a move should match
  //       the reported difference when calling diff_move.
  // (2) - The quality function should be exactly the same value after
  //       aggregating/collapsing the graph.

  // As long as we keep on improving and we don't exceed the
  // maximum number of iterations and number of moves.
  while ( (improv > this->eps || nb_moves > n*this->delta) &&
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
        it_vertex != vertex_order.end(); ++it_vertex) {
      size_t v = *it_vertex; // The actual vertex we will now consider
      // Only take into account nodes of degree higher than zero
      if (graph->degree(v, IGRAPH_ALL) > 0)
      {
        // What is the current community of the node
        size_t v_comm = partition->membership(v);

        // What is the improvement per community if we move the node to one of
        // the other communities, and what is the maximum improvement?
        double max_improv = 0.0;
        double max_comm = v_comm;

        // Keep track of the possible improvements and (neighbouring) communities.
        size_t neigh_comm;
        double possible_improv;
        set<size_t>* neigh_comms = NULL;
        switch (consider_comms)
        {
          /****************************ALL COMMS**********************************/
          case ALL_COMMS:
            #ifdef DEBUG
              cerr << "Consider all communities." << endl;
            #endif
            // Loop through all communities
            for (size_t comm = 0; comm < partition->nb_communities(); comm++)
            {
              // Calculate the possible improvement of the moving the node to that community/
              possible_improv = partition->diff_move(v, comm);
              // We're only interested in the maximum.
              if (possible_improv > max_improv)
              {
                max_improv = possible_improv;
                max_comm = comm;
              }
            }
            break;
          /****************************ALL NEIGH COMMS*****************************/
          case ALL_NEIGH_COMMS:
            #ifdef DEBUG
              cerr << "Consider all neighbour communities." << endl;
            #endif
            // In which communities are its neighbours
            neigh_comms = partition->get_neigh_comms(v, IGRAPH_ALL);
            // Loop through the communities of the neighbours
            for(set<size_t>::iterator it_neigh_comm = neigh_comms->begin();
                it_neigh_comm != neigh_comms->end(); ++it_neigh_comm)
            {
              size_t neigh_comm = *it_neigh_comm;
              // Calculate the possible improvement of the moving the node to that community/
              double possible_improv = partition->diff_move(v, neigh_comm);
              // We're only interested in the maximum.
              if (possible_improv > max_improv)
              {
                max_improv = possible_improv;
                max_comm = neigh_comm;
              }
            }
            delete neigh_comms;
            break;
          /****************************RAND COMM***********************************/
          case RAND_COMM:
            // Select a random community.
            neigh_comm = partition->membership(graph->get_random_node());
            #ifdef DEBUG
              cerr << "Consider random community " << neigh_comm << "." << endl;
            #endif
            // Calculate the possible improvement of the moving the node to that community/
            possible_improv = partition->diff_move(v, neigh_comm);
            if (possible_improv > 0)
            {
              max_improv = possible_improv;
              max_comm = neigh_comm;
            }
            break;
          /****************************RAND NEIGH COMM*****************************/
          case RAND_NEIGH_COMM:
            // Select a random community from the neighbours.
            neigh_comm = partition->membership(graph->get_random_neighbour(v, IGRAPH_ALL));
            #ifdef DEBUG
              cerr << "Consider random neighbour community " << neigh_comm << "." << endl;
            #endif
            // Calculate the possible improvement of the moving the node to that community/
            possible_improv = partition->diff_move(v, neigh_comm);
            if (possible_improv > 0)
            {
              max_improv = possible_improv;
              max_comm = neigh_comm;
            }
          /****************************RAND WEIGH COMM*****************************/
          case RAND_WEIGHT_NEIGH_COMM:
            // Select a random community from the neighbours.
            neigh_comm = partition->membership(graph->get_weighted_random_neighbour(v, IGRAPH_ALL));
            #ifdef DEBUG
              cerr << "Consider weighted random neighbour community " << neigh_comm << "." << endl;
            #endif
            // Calculate the possible improvement of the moving the node to that community/
            possible_improv = partition->diff_move(v, neigh_comm);
            if (possible_improv > 0)
            {
              max_improv = possible_improv;
              max_comm = neigh_comm;
            }
        }
        #ifdef DEBUG
          // If we are debugging, calculate quality function
          double q1 = partition->quality();
        #endif
        // Check if we should move to an empty community
        if (this->consider_empty_community && partition->csize(v_comm) > 1)
        {
          double w = partition->weight_to_comm(v, v_comm) + partition->weight_from_comm(v, v_comm);
          neigh_comm = partition->nb_communities();
          possible_improv = partition->diff_move(v, neigh_comm);
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
          // Actually move the node
          if (max_comm >= partition->nb_communities())
            partition->add_empty_community();
          partition->move_node(v, max_comm);
          // Keep track of number of moves
          nb_moves += 1;
        }
        #ifdef DEBUG
          // If we are debugging, calculate quality function
          // and report difference
          double q2 = partition->quality();

          if (fabs((q2 - q1) - max_improv) > 1e-6)
          {
            cerr << "ERROR: Inconsistency while moving nodes, improvement as measured by quality function did not equal the improvement measured by the diff_move function." << endl;
            //throw Exception("ERROR: Inconsistency while moving nodes, improvement as measured by quality function did not equal the improvement measured by the diff_move function.");
          }
          cerr << "Move node " << v
              << " from " << v_comm << " to " << max_comm
              << " (diff_move=" << max_improv
              << ", q2 - q1=" << q2 - q1 << ")" << endl;
        #endif
      }
    }
    // Keep track of total improvement over multiple loops
    total_improv += improv;
  }
  partition->renumber_communities();

  return total_improv;
}

/*****************************************************************************
  optimise the provided partition.
*****************************************************************************/
double Optimiser::optimise_partition_constrained(MutableVertexPartition* partition, vector<size_t> const & constrained_membership)
{
  #ifdef DEBUG
    cerr << "void Optimiser::optimise_partition_constrained(MutableVertexPartition* partition, vector<size_t> const & constrained_membership)" << endl;
  #endif

  #ifdef DEBUG
    cerr << "Using partition at address " << partition << endl;
  #endif

  // Get the graph from the partition
  Graph* graph = partition->get_graph();

  #ifdef DEBUG
    cerr << "Using graph at address " << graph << endl;
  #endif

  // Declare the collapsed_graph variable which will contain the graph
  // collapsed by its communities. We will use this variables at each
  // further iteration, so we don't keep a collapsed graph at each pass.
  Graph* collapsed_graph = NULL;
  MutableVertexPartition* collapsed_partition = NULL;

  // Do one iteration of optimisation
  double improv = 2*this->eps;
  // As long as there remains improvement iterate
  while (improv > this->eps)
  {
    // First collapse graph (i.e. community graph

    improv = 0.0;
    // Try to move individual nodes again
    // We need to move individual nodes before doing slm, because
    // otherwise, moving individual nodes may possibly disconnected
    // graphs again, which then needs to be corrected for by resorting to slm
    if (this->move_individual)
      improv += this->move_nodes_constrained(partition, constrained_membership);

    collapsed_graph = graph->collapse_graph(partition);

    // Create collapsed partition (i.e. default partition of each node in its own community).
    collapsed_partition = partition->create(collapsed_graph);

    // Determine the membership for the collapsed graph
    vector< size_t > collapsed_constrained_membership(collapsed_graph->vcount());

    // If we collapse the graph, every node of the collapsed graph
    // should still adhere to the same constrained membership.
    // That is, if we collapse community c, then the constrained membership
    // of that community c would be the same for every node v in community c,
    // which is then provided by constrained_membership[v].
    #ifdef DEBUG
      cerr << "SLM\tOrig" << endl;
    #endif // DEBUG
    for (size_t v = 0; v < graph->vcount(); v++)
    {
      collapsed_constrained_membership[partition->membership(v)] = constrained_membership[v];
      #ifdef DEBUG
        cerr << partition->membership(v) << "\t" << constrained_membership[v] << endl;
      #endif // DEBUG
    }

    #ifdef DEBUG
      cerr <<   "Calculate partition quality." << endl;
      double q = partition->quality();
      cerr <<   "Calculate collapsed partition quality." << endl;
      double q_collapsed = collapsed_partition->quality();
      if (fabs(q - q_collapsed) > 1e-6)
      {
        cerr << "ERROR: Quality of original partition and collapsed partition are not equal." << endl;
      }
      cerr <<   "partition->quality()=" << q
           << ", collapsed_partition->quality()=" << q_collapsed << endl;
      cerr <<   "graph->total_weight()=" << graph->total_weight()
           << ", collapsed_graph->total_weight()=" << collapsed_graph->total_weight() << endl;
      cerr <<   "graph->ecount()=" << graph->ecount()
           << ", collapsed_graph->ecount()="  << collapsed_graph->ecount() << endl;
      cerr <<   "graph->is_directed()=" << graph->is_directed()
           << ", collapsed_graph->is_directed()="  << collapsed_graph->is_directed() << endl;
      cerr <<   "graph->correct_self_loops()=" << graph->correct_self_loops()
           << ", collapsed_graph->correct_self_loops()="  << collapsed_graph->correct_self_loops() << endl;
    #endif // DEBUG

    // Optimise partition for collapsed graph
    #ifdef DEBUG
      cerr << "Quality before moving " << collapsed_partition->quality() << endl;
    #endif
    improv += this->move_nodes_constrained(collapsed_partition, collapsed_constrained_membership);
    #ifdef DEBUG
      cerr << "Found " << partition->nb_communities() << " communities, improved " << improv << endl;
      cerr << "Quality after moving " << collapsed_partition->quality() << endl << endl;
    #endif // DEBUG

    // Make sure improvement on coarser scale is reflected on the
    // scale of the graph as a whole.
    partition->from_coarser_partition(collapsed_partition);

    #ifdef DEBUG
      cerr << "Quality on finer partition " << partition->quality() << endl << endl;
    #endif // DEBUG

    // Clean up memory after use.
    delete collapsed_partition;
    delete collapsed_graph;
  }
  // We renumber the communities to make sure we stick in the range
  // 0,1,...,r - 1 for r communities.
  // By default, we number the communities in decreasing order of size,
  // so that 0 is the largest community, 1 the second largest, etc...
  partition->renumber_communities();
  // Return the quality of the current partition.
  return partition->quality();
}

double Optimiser::move_nodes_constrained(MutableVertexPartition* partition, vector<size_t> const& constrained_membership)
{
  #ifdef DEBUG
    cerr << "double Optimiser::move_nodes(MutableVertexPartition* partition)" << endl;
  #endif
  // get graph
  Graph* graph = partition->get_graph();
  // Number of iterations
  size_t itr = 0;
  // Total improvement while moving nodes
  double total_improv = 0.0;
  // Improvement for one loop
  double improv = 2*this->eps;
  // Number of nodes in the graph
  size_t n = graph->vcount();
  // Number of moved nodes during one loop
  size_t nb_moves = 2*n;
  // Initialize the degree vector
  // If we want to debug the function, we will calculate some additional values.
  // In particular, the following consistencies could be checked:
  // (1) - The difference in the quality function after a move should match
  //       the reported difference when calling diff_move.
  // (2) - The quality function should be exactly the same value after
  //       aggregating/collapsing the graph.

  // As long as we keep on improving and we don't exceed the
  // maximum number of iterations and number of moves.
  while (improv > this->eps &&
         nb_moves > n*this->delta &&
         itr < this->max_itr)
  {
    // Increase number of iterations
    itr += 1;

    // Initialize number of moves and improvement
    nb_moves = 0;
    improv = 0.0;

    // Should we use false and dirty check of constrained membership
    // (it is correct, but fast 'n dirty)
    int fast_n_dirty_constrained = false;

    // Establish vertex order
    // We normally initialize the normal vertex order
    // of considering node 0,1,...
    vector<size_t> vertex_order = range(n);
    // But if we use a random order, we shuffle this order.
    if (this->random_order)
      random_shuffle( vertex_order.begin(), vertex_order.end() );

    // For each node
    for(vector<size_t>::iterator it_vertex = vertex_order.begin();
        it_vertex != vertex_order.end(); ++it_vertex) {
      size_t v = *it_vertex; // The actual vertex we will now consider
      // Only take into account nodes of degree higher than zero
      if (graph->degree(v, IGRAPH_ALL) > 0)
      {
        // What is the current community of the node
        size_t v_comm = partition->membership(v);

        // What is the improvement per community if we move the node to one of
        // the other communities, and what is the maximum improvement?
        double max_improv = 0.0;
        double max_comm = v_comm;

        // Keep track of the possible improvements and (neighbouring) communities.
        size_t neigh_comm;
        double possible_improv;
        set<size_t>* neigh_comms = partition->get_neigh_comms(v, IGRAPH_ALL, constrained_membership);
        // Loop through the communities of the neighbours
        for(set<size_t>::iterator it_neigh_comm = neigh_comms->begin();
            it_neigh_comm != neigh_comms->end(); ++it_neigh_comm)
        {
          size_t neigh_comm = *it_neigh_comm;
          // Calculate the possible improvement of the moving the node to that community/
          double possible_improv = partition->diff_move(v, neigh_comm);
          // We're only interested in the maximum.
          if (possible_improv > max_improv)
          {
            max_improv = possible_improv;
            max_comm = neigh_comm;
          }
        }
        delete neigh_comms;

        #ifdef DEBUG
          // If we are debugging, calculate quality function
          double q1 = partition->quality();
        #endif
        // If we actually plan to move the nove
        if (max_comm != v_comm)
        {
          // Keep track of improvement
          improv += max_improv;
          // Actually move the node
          partition->move_node(v, max_comm);
          // Keep track of number of moves
          nb_moves += 1;
        }
        #ifdef DEBUG
          // If we are debugging, calculate quality function
          // and report difference
          double q2 = partition->quality();

          if (fabs((q2 - q1) - max_improv) > 1e-6)
          {
            cerr << "ERROR: Inconsistency while moving nodes, improvement as measured by quality function did not equal the improvement measured by the diff_move function." << endl;
            //throw Exception("ERROR: Inconsistency while moving nodes, improvement as measured by quality function did not equal the improvement measured by the diff_move function.");
          }
          cerr << "Move node " << v
              << " from " << v_comm << " to " << max_comm
              << " (diff_move=" << max_improv
              << ", q2 - q1=" << q2 - q1 << ")" << endl;
        #endif
      }
    }
    // Keep track of total improvement over multiple loops
    total_improv += improv;
  }
  partition->renumber_communities();

  return total_improv;
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
      // Only take into account nodes of degree higher than zero
      map<size_t, double> comm_improvs;
      size_t v_comm = -1;
      size_t neigh_comm;
      set<size_t>* neigh_comms = NULL;
      Graph* graph = NULL;
      MutableVertexPartition* partition = NULL;
      switch (consider_comms)
      {
        /****************************ALL COMMS**********************************/
        case ALL_COMMS:
          #ifdef DEBUG
            cerr << "Consider all communities." << endl;
          #endif
          for (size_t layer = 0; layer < nb_layers; layer++)
          {
            graph = graphs[layer];
            partition = partitions[layer];
            for(size_t comm = 0; comm < partition->nb_communities(); comm++)
            {
              // What is the current community of the node (this should be the same for all layers)
              v_comm = partition->membership(v);
              if (graph->degree(v, IGRAPH_ALL) > 0)
              {
                // Make sure to multiply it by the weight per layer
                comm_improvs[comm] += layer_weights[layer]*partition->diff_move(v, comm);
              }
            }
          }
          break;
        /****************************ALL NEIGH COMMS*****************************/
        case ALL_NEIGH_COMMS:
          #ifdef DEBUG
            cerr << "Consider all neighbour communities." << endl;
          #endif
          for (size_t layer = 0; layer < nb_layers; layer++)
          {
            graph = graphs[layer];
            partition = partitions[layer];
            neigh_comms = partition->get_neigh_comms(v, IGRAPH_ALL);
            for (set<size_t>::iterator neigh_comm_it = neigh_comms->begin();
                 neigh_comm_it != neigh_comms->end(); ++neigh_comm_it)
            {
              neigh_comm = *neigh_comm_it;
              // Make sure to multiply it by the weight per layer
              comm_improvs[neigh_comm] += layer_weights[layer]*partition->diff_move(v, neigh_comm);
            }
            delete neigh_comms;
          }
          break;
        /****************************RAND COMM***********************************/
        case RAND_COMM:
          neigh_comm = partitions[0]->membership(graphs[0]->get_random_node());
          #ifdef DEBUG
            cerr << "Consider random community " << neigh_comm << "." << endl;
          #endif
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
          #ifdef DEBUG
            cerr << "Consider random neighbour community " << neigh_comm << "." << endl;
          #endif
          for (size_t layer = 0; layer < nb_layers; layer++)
          {
            comm_improvs[neigh_comm] += layer_weights[layer]*partitions[layer]->diff_move(v, neigh_comm);
          }
          break;
      }
      size_t max_comm = v_comm;
      double max_improv = 0.0;
      // TODO: Not implemented yet to consider moving to an empty community for
      // several layers of graphs.
      // Determine the maximum improvement
      for (map<size_t, double>::iterator improv_it = comm_improvs.begin();
           improv_it != comm_improvs.end(); improv_it++)
      {
        size_t comm = improv_it->first;
        double improv = improv_it->second;
        if (improv > max_improv)
          max_comm = comm;
      }

      for (size_t layer = 0; layer < nb_layers; layer++)
      {
        MutableVertexPartition* partition = partitions[layer];

        #ifdef DEBUG
          // If we are debugging, calculate quality function
          double q1 = partition->quality();
        #endif
        // If we actually plan to move the nove
        if (max_comm != v_comm)
        {
          // Keep track of improvement
          improv += max_improv;
          // Actually move the node
          partition->move_node(v, max_comm);
          // Keep track of number of moves
          nb_moves += 1;
        }
        #ifdef DEBUG
          // If we are debugging, calculate quality function
          // and report difference
          double q2 = partition->quality();

          if (fabs((q2 - q1) - max_improv) > 1e-6)
          {
            cerr << "ERROR: Inconsistency while moving nodes, improvement as measured by quality function did not equal the improvement measured by the diff_move function." << endl;
            //throw Exception("ERROR: Inconsistency while moving nodes, improvement as measured by quality function did not equal the improvement measured by the diff_move function.");
          }
          cerr << "Move node " << v
              << " from " << v_comm << " to " << max_comm
              << " (diff_move=" << max_improv
              << ", q2 - q1=" << q2 - q1 << ")" << endl;
        #endif
      }
    }
    // Keep track of total improvement over multiple loops
    total_improv += improv;
  }

  partitions[0]->renumber_communities();
  vector<size_t> membership = partitions[0]->membership();
  for (size_t layer = 1; layer < nb_layers; layer++)
  {
    partitions[layer]->renumber_communities(membership);
  }
  return total_improv;
}

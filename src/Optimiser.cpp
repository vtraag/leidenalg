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
Optimiser::Optimiser(double eps, double delta, size_t max_itr, int random_order, int consider_comms)
{
  this->eps = eps;
  this->delta = delta;
  this->max_itr = max_itr;
  this->random_order = random_order;
  this->consider_comms = consider_comms;
}

Optimiser::Optimiser()
{
  this->eps = 1e-5;
  this->delta = 1e-2;
  this->max_itr = 10000;
  this->random_order = true;
  this->consider_comms = Optimiser::ALL_NEIGH_COMMS;
}

Optimiser::~Optimiser()
{
  //dtor
}

/*****************************************************************************
  Optimize the provided partition.
*****************************************************************************/
double Optimiser::optimize_partition(MutableVertexPartition* partition)
{
  #ifdef DEBUG
    cerr << "void Optimiser::optimize_partition(MutableVertexPartition* partition)" << endl;
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
  double improv = this->move_nodes(partition, this->consider_comms);
  // As long as there remains improvement iterate
  while (improv > this->eps)
  {
    // First collapse graph (i.e. community graph)
    collapsed_graph = graph->collapse_graph(partition);

    // Create collapsed partition (i.e. default partition of each node in its own community).
    collapsed_partition = partition->create(collapsed_graph);
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
    // Optimise partition for collapsed graph
    improv = this->move_nodes(collapsed_partition, this->consider_comms);
    // Make sure improvement on coarser scale is reflected on the
    // scale of the graph as a whole.
    partition->from_coarser_partition(collapsed_partition);

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
  Optimize the providede partitions simultaneously. We here use the sum
  of the difference of the moves as the overall quality function, each partition
  weighted by the layer weight.
*****************************************************************************/
double Optimiser::optimize_partition(vector<MutableVertexPartition*> partitions, vector<double> layer_weights)
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
    // But if we use a random order, we shuffly this order.
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
        vector<size_t>* neigh = NULL;
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
            delete neigh;
        }
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
      vector<size_t>* neigh = NULL;
      vector<size_t>* neigh_ext = NULL;
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

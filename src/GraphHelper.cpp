#include "GraphHelper.h"

#ifdef DEBUG
  using std::cerr;
  using std::endl;
#endif

vector<size_t> range(size_t n)
{
  vector<size_t> range_vec(n);
  for(size_t i = 0; i<n; i++)
    range_vec[i] = i;
  return range_vec;
}

queue<size_t> queue_range(size_t n)
{
  queue<size_t> range_vec;
  for(size_t i = 0; i<n; i++)
    range_vec.push(i);
  return range_vec;
}

bool orderCSize(const size_t* A, const size_t* B)
{

  if (A[1] == B[1])
  {
    if (A[2] == B[2])
      return A[0] < B[0];
    else
      return A[2] > B[2];
  }
  else
    return A[1] > B[1];
}

void shuffle(vector<size_t>& v, igraph_rng_t* rng)
{
  size_t n = v.size();
  for (size_t idx = n - 1; idx > 0; idx--)
  {
    size_t rand_idx = get_random_int(0, idx, rng);
    size_t tmp = v[idx];
    v[idx] = v[rand_idx];
    v[rand_idx] = tmp;
  }
}

/****************************************************************************
  The binary Kullback-Leibler divergence.
****************************************************************************/
double KL(double q, double p)
{
  double KL = 0.0;
  if (q > 0.0 && p > 0.0)
    KL += q*log(q/p);
  if (q < 1.0 && p < 1.0)
    KL += (1.0-q)*log((1.0-q)/(1.0-p));
  return KL;
}

double KLL(double q, double p)
{
  double KL = 0.0;
  if (q > 0.0 && p > 0.0)
    KL += q*log(q/p);
  if (q < 1.0 && p < 1.0)
    KL += (1.0-q)*log((1.0-q)/(1.0-p));
  if (q < p)
    KL *= -1;
  return KL;
}

Graph::Graph(igraph_t* graph,
  vector<double> const& edge_weights,
  vector<size_t> const& node_sizes,
  vector<double> const& node_self_weights, int correct_self_loops)
{
  this->_graph = graph;
  this->_remove_graph = false;

  if (edge_weights.size() != this->ecount())
    throw Exception("Edge weights vector inconsistent length with the edge count of the graph.");
  this->_edge_weights = edge_weights;
  this->_is_weighted = true;

  if (node_sizes.size() != this->vcount())
    throw Exception("Node size vector inconsistent length with the vertex count of the graph.");
  this->_node_sizes = node_sizes;

  if (node_self_weights.size() != this->vcount())
    throw Exception("Node self weights vector inconsistent length with the vertex count of the graph.");
  this->_node_self_weights = node_self_weights;

  this->_correct_self_loops = correct_self_loops;
  this->init_admin();
}

Graph::Graph(igraph_t* graph,
  vector<double> const& edge_weights,
  vector<size_t> const& node_sizes,
  vector<double> const& node_self_weights)
{
  this->_graph = graph;
  this->_remove_graph = false;

  if (edge_weights.size() != this->ecount())
    throw Exception("Edge weights vector inconsistent length with the edge count of the graph.");
  this->_edge_weights = edge_weights;
  this->_is_weighted = true;

  if (node_sizes.size() != this->vcount())
    throw Exception("Node size vector inconsistent length with the vertex count of the graph.");
  this->_node_sizes = node_sizes;

  this->_correct_self_loops = this->has_self_loops();

  this->_node_self_weights = node_self_weights;
  this->init_admin();
}

Graph::Graph(igraph_t* graph,
  vector<double> const& edge_weights,
  vector<size_t> const& node_sizes, int correct_self_loops)
{
  this->_graph = graph;
  this->_remove_graph = false;

  if (edge_weights.size() != this->ecount())
    throw Exception("Edge weights vector inconsistent length with the edge count of the graph.");
  this->_edge_weights = edge_weights;
  this->_is_weighted = true;

  if (node_sizes.size() != this->vcount())
    throw Exception("Node size vector inconsistent length with the vertex count of the graph.");
  this->_node_sizes = node_sizes;

  this->_correct_self_loops = correct_self_loops;
  this->init_admin();
  this->set_self_weights();
}

Graph::Graph(igraph_t* graph,
  vector<double> const& edge_weights,
  vector<size_t> const& node_sizes)
{
  this->_graph = graph;
  this->_remove_graph = false;
  if (edge_weights.size() != this->ecount())
    throw Exception("Edge weights vector inconsistent length with the edge count of the graph.");
  this->_edge_weights = edge_weights;
  this->_is_weighted = true;

  if (node_sizes.size() != this->vcount())
    throw Exception("Node size vector inconsistent length with the vertex count of the graph.");
  this->_node_sizes = node_sizes;

  this->_correct_self_loops = this->has_self_loops();

  this->init_admin();
  this->set_self_weights();
}

Graph::Graph(igraph_t* graph, vector<double> const& edge_weights, int correct_self_loops)
{
  this->_graph = graph;
  this->_remove_graph = false;
  this->_correct_self_loops = correct_self_loops;
  if (edge_weights.size() != this->ecount())
    throw Exception("Edge weights vector inconsistent length with the edge count of the graph.");
  this->_edge_weights = edge_weights;
  this->_is_weighted = true;
  this->set_default_node_size();
  this->init_admin();
  this->set_self_weights();
}

Graph::Graph(igraph_t* graph, vector<double> const& edge_weights)
{
  this->_graph = graph;
  this->_remove_graph = false;
  if (edge_weights.size() != this->ecount())
    throw Exception("Edge weights vector inconsistent length with the edge count of the graph.");
  this->_edge_weights = edge_weights;
  this->_is_weighted = true;

  this->_correct_self_loops = this->has_self_loops();

  this->set_default_node_size();
  this->init_admin();
  this->set_self_weights();
}

Graph::Graph(igraph_t* graph, vector<size_t> const& node_sizes, int correct_self_loops)
{
  this->_graph = graph;
  this->_remove_graph = false;
  this->_correct_self_loops = correct_self_loops;

  if (node_sizes.size() != this->vcount())
    throw Exception("Node size vector inconsistent length with the vertex count of the graph.");
  this->_node_sizes = node_sizes;

  this->set_default_edge_weight();
  this->_is_weighted = false;
  this->init_admin();
  this->set_self_weights();
}

Graph::Graph(igraph_t* graph, vector<size_t> const& node_sizes)
{
  this->_graph = graph;
  this->_remove_graph = false;
  this->set_defaults();
  this->_is_weighted = false;

  if (node_sizes.size() != this->vcount())
    throw Exception("Node size vector inconsistent length with the vertex count of the graph.");

  this->_node_sizes = node_sizes;

  this->_correct_self_loops = this->has_self_loops();

  this->init_admin();
  this->set_self_weights();
}

Graph::Graph(igraph_t* graph, int correct_self_loops)
{
  this->_graph = graph;
  this->_remove_graph = false;
  this->_correct_self_loops = correct_self_loops;
  this->set_defaults();
  this->_is_weighted = false;
  this->init_admin();
  this->set_self_weights();
}

Graph::Graph(igraph_t* graph)
{
  this->_graph = graph;
  this->_remove_graph = false;
  this->set_defaults();
  this->_is_weighted = false;

  this->_correct_self_loops = this->has_self_loops();

  this->init_admin();
  this->set_self_weights();
}

Graph::Graph()
{
  this->_graph = new igraph_t();
  this->_remove_graph = true;
  this->set_defaults();
  this->_is_weighted = false;
  this->_correct_self_loops = false;
  this->init_admin();
  this->set_self_weights();
}

Graph::~Graph()
{
  if (this->_remove_graph)
  {
    igraph_destroy(this->_graph);
    delete this->_graph;
  }
}

int Graph::has_self_loops()
{
  size_t m = this->ecount();
  igraph_vector_bool_t loop;
  igraph_vector_bool_init(&loop, m);
  igraph_is_loop(this->_graph, &loop, igraph_ess_all(IGRAPH_EDGEORDER_ID));

  int has_self_loops = false;
  for (size_t idx = 0; idx < m; idx++)
  {
    if (VECTOR(loop)[idx])
    {
      has_self_loops = true;
      break;
    }
  }
  igraph_vector_bool_destroy(&loop);
  return has_self_loops;
}

size_t Graph::possible_edges()
{
  return this->possible_edges(this->vcount());
}

size_t Graph::possible_edges(size_t n)
{
  size_t possible_edges = n*(n-1);
  if (!this->is_directed())
    possible_edges /= 2;
  if (this->correct_self_loops())
    possible_edges += n;

  return possible_edges;
}

void Graph::set_defaults()
{
  this->set_default_edge_weight();
  this->set_default_node_size();
}

void Graph::set_default_edge_weight()
{
  size_t m = this->ecount();

  // Set default edge weight of 1.0
  this->_edge_weights.clear(); this->_edge_weights.resize(m);
  fill(this->_edge_weights.begin(), this->_edge_weights.end(), 1.0);
  this->_is_weighted = false;
}

void Graph::set_default_node_size()
{
  size_t n = this->vcount();

  // Set default node size of 1
  this->_node_sizes.clear(); this->_node_sizes.resize(n);
  fill(this->_node_sizes.begin(), this->_node_sizes.end(), 1);
}

void Graph::set_self_weights()
{
  size_t n = this->vcount();

  // Set default self_weights of the total weight of any possible self-loops
  this->_node_self_weights.clear(); this->_node_self_weights.resize(n);
  for (size_t v = 0; v < n; v++)
  {
    #ifdef DEBUG
      cerr << "\t" << "Size node " << v << ": " << this->node_size(v) << endl;
    #endif
    double self_weight = 0.0;
    // There should be only one self loop
    igraph_integer_t eid;
    // Get edge id for self loop
    igraph_get_eid(this->_graph, &eid, v, v, this->is_directed(), false);
    if (eid >= 0)
      self_weight = this->edge_weight(eid);

    this->_node_self_weights[v] = self_weight;
    #ifdef DEBUG
      cerr << "\t" << "Self weight node " << v << ": " << self_weight << endl;
    #endif
  }
}

void Graph::init_admin()
{

  size_t m = this->ecount();

  // Determine total weight in the graph.
  this->_total_weight = 0.0;
  for (size_t e = 0; e < m; e++)
    this->_total_weight += this->edge_weight(e);

  // Make sure to multiply by 2 for undirected graphs
  //if (!this->is_directed())
  //  this->_total_weight *= 2.0;

  size_t n = this->vcount();

  this->_total_size = 0;
  for (size_t v = 0; v < n; v++)
    this->_total_size += this->node_size(v);

  igraph_vector_t weights;
  igraph_vector_t res;

  // Strength IN
  igraph_vector_init(&res, n);
  // Copy weights to an igraph_vector_t
  igraph_vector_init_copy(&weights, &this->_edge_weights[0], this->ecount());
  // Calculcate strength
  igraph_strength(this->_graph, &res, igraph_vss_all(), IGRAPH_IN, true, &weights);
  igraph_vector_destroy(&weights);

  // Assign to strength vector
  this->_strength_in.clear();
  this->_strength_in.resize(n);
  for (size_t v = 0; v < n; v++)
    this->_strength_in[v] = VECTOR(res)[v];
  igraph_vector_destroy(&res);

  // Strength OUT
  igraph_vector_init(&res, n);
  // Copy weights to an igraph_vector_t
  igraph_vector_init_copy(&weights, &this->_edge_weights[0], this->ecount());
  // Calculcate strength
  igraph_strength(this->_graph, &res, igraph_vss_all(), IGRAPH_OUT, true, &weights);
  igraph_vector_destroy(&weights);

  // Assign to strength vector
  this->_strength_out.clear();
  this->_strength_out.resize(n);
  for (size_t v = 0; v < n; v++)
    this->_strength_out[v] = VECTOR(res)[v];
  igraph_vector_destroy(&res);

  // Degree IN
  igraph_vector_init(&res, n);
  igraph_degree(this->_graph, &res, igraph_vss_all(), IGRAPH_IN, true);
  this->_degree_in.clear();
  this->_degree_in.resize(n);
  for (size_t v = 0; v < n; v++)
    this->_degree_in[v] = VECTOR(res)[v];
  igraph_vector_destroy(&res);

  // Degree OUT
  igraph_vector_init(&res, n);
  igraph_degree(this->_graph, &res, igraph_vss_all(), IGRAPH_OUT, true);
  this->_degree_out.clear();
  this->_degree_out.resize(n);
  for (size_t v = 0; v < n; v++)
    this->_degree_out[v] = VECTOR(res)[v];
  igraph_vector_destroy(&res);

  // Degree ALL
  igraph_vector_init(&res, n);
  igraph_degree(this->_graph, &res, igraph_vss_all(), IGRAPH_ALL, true);
  this->_degree_all.clear();
  this->_degree_all.resize(n);
  for (size_t v = 0; v < n; v++)
    this->_degree_all[v] = VECTOR(res)[v];
  igraph_vector_destroy(&res);

  // Calculate density;
  double w = this->total_weight();
  size_t n_size = this->total_size();

  // For now we default to not correcting self loops.
  // this->_correct_self_loops = false; (remove this as this is set in the constructor)

  double normalise = 0.0;
  if (this->_correct_self_loops)
    normalise = n_size*n_size;
  else
    normalise = n_size*(n_size - 1);

  if (this->is_directed())
    this->_density = w/normalise;
  else
    this->_density = 2*w/normalise;

  this->_current_node_cache_neigh_edges_from = n + 1;
  this->_current_node_cache_neigh_edges_to = n + 1;
  this->_current_node_cache_neigh_edges_all = n + 1;

  this->_current_node_cache_neigh_from = n + 1;
  this->_current_node_cache_neigh_to = n + 1;
  this->_current_node_cache_neigh_all = n + 1;
}

void Graph::cache_neighbour_edges(size_t v, igraph_neimode_t mode)
{
  #ifdef DEBUG
    cerr << "void Graph::cache_neighbour_edges(" << v << ", " << mode << ");" << endl;
  #endif
  size_t degree = this->degree(v, mode);
  #ifdef DEBUG
    cerr << "Degree: " << degree << endl;
  #endif

  igraph_vector_t incident_edges;
  igraph_vector_init(&incident_edges, degree);
  igraph_incident(this->_graph, &incident_edges, v, mode);

  vector<size_t>* _cached_neigh_edges = NULL;
  switch (mode)
  {
    case IGRAPH_IN:
      this->_current_node_cache_neigh_edges_from = v;
      _cached_neigh_edges = &(this->_cached_neigh_edges_from);
      break;
    case IGRAPH_OUT:
      this->_current_node_cache_neigh_edges_to = v;
      _cached_neigh_edges = &(this->_cached_neigh_edges_to);
      break;
    case IGRAPH_ALL:
      this->_current_node_cache_neigh_edges_all = v;
      _cached_neigh_edges = &(this->_cached_neigh_edges_all);
      break;
  }
  _cached_neigh_edges->assign(igraph_vector_e_ptr(&incident_edges, 0),
                              igraph_vector_e_ptr(&incident_edges, degree));
  #ifdef DEBUG
    cerr << "Number of edges: " << _cached_neigh_edges->size() << endl;
  #endif


  igraph_vector_destroy(&incident_edges);
  #ifdef DEBUG
    cerr << "exit void Graph::cache_neighbour_edges(" << v << ", " << mode << ");" << endl;
  #endif
}

vector<size_t> const& Graph::get_neighbour_edges(size_t v, igraph_neimode_t mode)
{
  switch (mode)
  {
    case IGRAPH_IN:
      if (this->_current_node_cache_neigh_edges_from != v)
      {
        cache_neighbour_edges(v, mode);
        this->_current_node_cache_neigh_edges_from = v;
      }
      return this->_cached_neigh_edges_from;
    case IGRAPH_OUT:
      if (this->_current_node_cache_neigh_edges_to != v)
      {
        cache_neighbour_edges(v, mode);
        this->_current_node_cache_neigh_edges_to = v;
      }
      return this->_cached_neigh_edges_to;
    case IGRAPH_ALL:
      if (this->_current_node_cache_neigh_edges_all != v)
      {
        cache_neighbour_edges(v, mode);
        this->_current_node_cache_neigh_edges_all = v;
      }
      return this->_cached_neigh_edges_all;
  }
  throw Exception("Incorrect model for getting neighbour edges.");
}

pair<size_t, size_t> Graph::get_endpoints(size_t e)
{
  igraph_integer_t from, to;
  igraph_edge(this->_graph, e,&from, &to);
  return make_pair<size_t, size_t>((size_t)from, (size_t)to);
}

void Graph::cache_neighbours(size_t v, igraph_neimode_t mode)
{
  #ifdef DEBUG
    cerr << "void Graph::cache_neighbours(" << v << ", " << mode << ");" << endl;
  #endif
  size_t degree = this->degree(v, mode);
  #ifdef DEBUG
    cerr << "Degree: " << degree << endl;
  #endif

  igraph_vector_t neighbours;
  igraph_vector_init(&neighbours, degree);
  igraph_neighbors(this->_graph, &neighbours, v, mode);

  vector<size_t>* _cached_neighs = NULL;
  switch (mode)
  {
    case IGRAPH_IN:
      this->_current_node_cache_neigh_from = v;
      _cached_neighs = &(this->_cached_neighs_from);
      break;
    case IGRAPH_OUT:
      this->_current_node_cache_neigh_to = v;
      _cached_neighs = &(this->_cached_neighs_to);
      break;
    case IGRAPH_ALL:
      this->_current_node_cache_neigh_all = v;
      _cached_neighs = &(this->_cached_neighs_all);
      break;
  }
  _cached_neighs->assign(igraph_vector_e_ptr(&neighbours, 0),igraph_vector_e_ptr(&neighbours, degree));
  igraph_vector_destroy(&neighbours);

  #ifdef DEBUG
    cerr << "Number of edges: " << _cached_neighs->size() << endl;
  #endif

  #ifdef DEBUG
    cerr << "exit void Graph::cache_neighbours(" << v << ", " << mode << ");" << endl;
  #endif
}

vector< size_t > const& Graph::get_neighbours(size_t v, igraph_neimode_t mode)
{
  switch (mode)
  {
    case IGRAPH_IN:
      if (this->_current_node_cache_neigh_from != v)
      {
        cache_neighbours(v, mode);
        this -> _current_node_cache_neigh_from = v;
      }
      #ifdef DEBUG
        cerr << "Returning " << this->_cached_neighs_from.size() << " incoming neighbours" << endl;
      #endif
      return this->_cached_neighs_from;
    case IGRAPH_OUT:
      if (this->_current_node_cache_neigh_to != v)
      {
        cache_neighbours(v, mode);
        this -> _current_node_cache_neigh_to = v;
      }
      #ifdef DEBUG
        cerr << "Returning " << this->_cached_neighs_to.size() << " incoming neighbours" << endl;
      #endif
      return this->_cached_neighs_to;
    case IGRAPH_ALL:
      if (this->_current_node_cache_neigh_all != v)
      {
        cache_neighbours(v, mode);
        this->_current_node_cache_neigh_all = v;
      }
      #ifdef DEBUG
        cerr << "Returning " << this->_cached_neighs_all.size() << " incoming neighbours" << endl;
      #endif
      return this->_cached_neighs_all;
  }
  throw Exception("Invalid mode for getting neighbours.");
}

/********************************************************************************
 * This should return a random neighbour in O(1)
 ********************************************************************************/
size_t Graph::get_random_neighbour(size_t v, igraph_neimode_t mode, igraph_rng_t* rng)
{
  size_t node=v;
  size_t rand_neigh = -1;

  if (this->degree(v, mode) <= 0)
    throw Exception("Cannot select a random neighbour for an isolated node.");

  if (igraph_is_directed(this->_graph) && mode != IGRAPH_ALL)
  {
    if (mode == IGRAPH_OUT)
    {
      // Get indices of where neighbours are
      size_t cum_degree_this_node = (size_t) VECTOR(this->_graph->os)[node];
      size_t cum_degree_next_node = (size_t) VECTOR(this->_graph->os)[node+1];
      // Get a random index from them
      size_t rand_neigh_idx = get_random_int(cum_degree_this_node, cum_degree_next_node - 1, rng);
      // Return the neighbour at that index
      #ifdef DEBUG
        cerr << "Degree: " << this->degree(node, mode) << " diff in cumulative: " << cum_degree_next_node - cum_degree_this_node << endl;
      #endif
      rand_neigh = VECTOR(this->_graph->to)[ (size_t)VECTOR(this->_graph->oi)[rand_neigh_idx] ];
    }
    else if (mode == IGRAPH_IN)
    {
      // Get indices of where neighbours are
      size_t cum_degree_this_node = (size_t) VECTOR(this->_graph->is)[node];
      size_t cum_degree_next_node = (size_t) VECTOR(this->_graph->is)[node+1];
      // Get a random index from them
      size_t rand_neigh_idx = get_random_int(cum_degree_this_node, cum_degree_next_node - 1, rng);
      #ifdef DEBUG
        cerr << "Degree: " << this->degree(node, mode) << " diff in cumulative: " << cum_degree_next_node - cum_degree_this_node << endl;
      #endif
      // Return the neighbour at that index
      rand_neigh = VECTOR(this->_graph->from)[ (size_t)VECTOR(this->_graph->ii)[rand_neigh_idx] ];
    }
  }
  else
  {
    // both in- and out- neighbors in a directed graph.
    size_t cum_outdegree_this_node = (size_t)VECTOR(this->_graph->os)[node];
    size_t cum_indegree_this_node  = (size_t)VECTOR(this->_graph->is)[node];

    size_t cum_outdegree_next_node = (size_t)VECTOR(this->_graph->os)[node+1];
    size_t cum_indegree_next_node  = (size_t)VECTOR(this->_graph->is)[node+1];

    size_t total_outdegree = cum_outdegree_next_node - cum_outdegree_this_node;
    size_t total_indegree = cum_indegree_next_node - cum_indegree_this_node;

    size_t rand_idx = get_random_int(0, total_outdegree + total_indegree - 1, rng);

    #ifdef DEBUG
      cerr << "Degree: " << this->degree(node, mode) << " diff in cumulative: " << total_outdegree + total_indegree << endl;
    #endif
    // From among in or out neighbours?
    if (rand_idx < total_outdegree)
    { // From among outgoing neighbours
      size_t rand_neigh_idx = cum_outdegree_this_node + rand_idx;
      rand_neigh = VECTOR(this->_graph->to)[ (size_t)VECTOR(this->_graph->oi)[rand_neigh_idx] ];
    }
    else
    { // From among incoming neighbours
      size_t rand_neigh_idx = cum_indegree_this_node + rand_idx - total_outdegree;
      rand_neigh = VECTOR(this->_graph->from)[ (size_t)VECTOR(this->_graph->ii)[rand_neigh_idx] ];
    }
  }

  return rand_neigh;
}

/****************************************************************************
  Creates a graph with communities as node and links as weights between
  communities.

  The weight of the edges in the new graph is simply the sum of the weight
  of the edges between the communities. The self weight of a node (i.e. the
  weight of its self loop) is the internal weight of a community. The size
  of a node in the new graph is simply the size of the community in the old
  graph.
*****************************************************************************/
Graph* Graph::collapse_graph(MutableVertexPartition* partition)
{
  #ifdef DEBUG
    cerr << "Graph* Graph::collapse_graph(vector<size_t> membership)" << endl;
  #endif
  size_t m = this->ecount();

  #ifdef DEBUG
    cerr << "Current graph has " << this->vcount() << " nodes and " << this->ecount() << " edges." << endl;
    cerr << "Collapsing to graph with " << partition->n_communities() << " nodes." << endl;
  #endif

  vector< map<size_t, double> > collapsed_edge_weights(partition->n_communities());

  igraph_integer_t v, u;
  for (size_t e = 0; e < m; e++)
  {
    double w = this->edge_weight(e);
    igraph_edge(this->_graph, e, &v, &u);
    size_t v_comm = partition->membership((size_t)v);
    size_t u_comm = partition->membership((size_t)u);
    if (collapsed_edge_weights[v_comm].count(u_comm) > 0)
      collapsed_edge_weights[v_comm][u_comm] += w;
    else
      collapsed_edge_weights[v_comm][u_comm] = w;
  }

  // Now create vector for edges, first determined the number of edges
  size_t m_collapsed = 0;
  size_t n_collapsed = partition->n_communities();

  for (vector< map<size_t, double> >::iterator itr = collapsed_edge_weights.begin();
       itr != collapsed_edge_weights.end(); itr++)
  {
      m_collapsed += itr->size();
  }

  igraph_vector_t edges;
  vector<double> collapsed_weights(m_collapsed, 0.0);
  double total_collapsed_weight = 0.0;

  igraph_vector_init(&edges, 2*m_collapsed); // Vector or edges with edges (edge[0], edge[1]), (edge[2], edge[3]), etc...

  size_t e_idx = 0;
  for (size_t v = 0; v < n_collapsed; v++)
  {
    for (map<size_t, double>::iterator itr = collapsed_edge_weights[v].begin();
         itr != collapsed_edge_weights[v].end(); itr++)
    {
      size_t u = itr->first;
      double w = itr->second;
      VECTOR(edges)[2*e_idx] = v;
      VECTOR(edges)[2*e_idx+1] = u;
      collapsed_weights[e_idx] = w;
      total_collapsed_weight += w;
      if (e_idx >= m_collapsed)
        throw Exception("Maximum number of possible edges exceeded.");
      // next edge
      e_idx += 1;
    }
  }

  // Create graph based on edges
  igraph_t* graph = new igraph_t();
  igraph_create(graph, &edges, n_collapsed, this->is_directed());
  igraph_vector_destroy(&edges);

  if ((size_t) igraph_vcount(graph) != partition->n_communities())
    throw Exception("Something went wrong with collapsing the graph.");

  // Calculate new node sizes
  vector<size_t> csizes(n_collapsed, 0);
  for (size_t c = 0; c < partition->n_communities(); c++)
    csizes[c] = partition->csize(c);

  Graph* G = new Graph(graph, collapsed_weights, csizes, this->_correct_self_loops);
  G->_remove_graph = true;
  #ifdef DEBUG
    cerr << "exit Graph::collapse_graph(vector<size_t> membership)" << endl << endl;
  #endif
  return G;
}

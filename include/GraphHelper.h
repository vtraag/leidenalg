#ifndef GRAPHHELPER_INCLUDED
#define GRAPHHELPER_INCLUDED

#include <igraph.h>
#include <vector>
#include <set>
#include <exception>
#include <deque>

//#ifdef DEBUG
#include <iostream>
  using std::cerr;
  using std::endl;
//#endif

class MutableVertexPartition;

using std::vector;
using std::pair;
using std::set;
using std::deque;
using std::make_pair;

vector<size_t> range(size_t n);

bool orderCSize(const size_t* A, const size_t* B);

double KL(double q, double p);
double KLL(double q, double p);

template <class T> T sum(vector<T> vec)
{
  T sum_of_elems = T();
  for (T x : vec)
      sum_of_elems += x;
  return sum_of_elems;
};

class Exception : public std::exception
{
  public:
    Exception(const char* str)
    {
      this->str = str;
    }

    virtual const char* what() const throw()
    {
      return this->str;
    }

  private:
    const char* str;

};

inline size_t get_random_int(size_t from, size_t to, igraph_rng_t* rng)
{
  return igraph_rng_get_integer(rng, from, to);
};

void shuffle(vector<size_t>& v, igraph_rng_t* rng);

class Graph
{
  public:
    Graph(igraph_t* graph,
      vector<double> const& edge_weights,
      vector<size_t> const& node_sizes,
      vector<double> const& node_self_weights, int correct_self_loops);
    Graph(igraph_t* graph,
      vector<double> const& edge_weights,
      vector<size_t> const& node_sizes,
      vector<double> const& node_self_weights);
    Graph(igraph_t* graph,
      vector<double> const& edge_weights,
      vector<size_t> const& node_sizes, int correct_self_loops);
    Graph(igraph_t* graph,
      vector<double> const& edge_weights,
      vector<size_t> const& node_sizes);
    Graph(igraph_t* graph, vector<double> const& edge_weights, int correct_self_loops);
    Graph(igraph_t* graph, vector<double> const& edge_weights);
    Graph(igraph_t* graph, vector<size_t> const& node_sizes, int correct_self_loops);
    Graph(igraph_t* graph, vector<size_t> const& node_sizes);
    Graph(igraph_t* graph, int correct_self_loops);
    Graph(igraph_t* graph);
    Graph();
    ~Graph();

    int has_self_loops();
    size_t possible_edges();
    size_t possible_edges(size_t n);

    Graph* collapse_graph(MutableVertexPartition* partition);

    vector<size_t> const& get_neighbour_edges(size_t v, igraph_neimode_t mode);
    vector<size_t> const& get_neighbours(size_t v, igraph_neimode_t mode);
    size_t get_random_neighbour(size_t v, igraph_neimode_t mode, igraph_rng_t* rng);

    inline size_t get_random_node(igraph_rng_t* rng)
    {
      return get_random_int(0, this->vcount() - 1, rng);
    };

    inline igraph_t* get_igraph() { return this->_graph; };

    inline size_t vcount() { return igraph_vcount(this->_graph); };
    inline size_t ecount() { return igraph_ecount(this->_graph); };
    inline double total_weight() { return this->_total_weight; };
    inline size_t total_size() { return this->_total_size; };
    inline int is_directed() { return this->_is_directed; };
    inline double density() { return this->_density; };
    inline int correct_self_loops() { return this->_correct_self_loops; };
    inline int is_weighted() { return this->_is_weighted; };

    // Get weight of edge based on attribute (or 1.0 if there is none).
    inline double edge_weight(size_t e)
    {
      #ifdef DEBUG
      if (e > this->_edge_weights.size())
        throw Exception("Edges outside of range of edge weights.");
      #endif
      return this->_edge_weights[e];
    };

    inline void edge(size_t eid, size_t &from, size_t &to) {
      from = IGRAPH_FROM(this->get_igraph(), eid);
      to = IGRAPH_TO(this->get_igraph(), eid);
    }

    inline vector<size_t> edge(size_t e)
    {
      vector<size_t> edge(2);
      this->edge(e, edge[0], edge[1]);
      return edge;
    }

    // Get size of node based on attribute (or 1.0 if there is none).
    inline size_t node_size(size_t v)
    { return this->_node_sizes[v]; };

    // Get self weight of node based on attribute (or set to 0.0 if there is none)
    inline double node_self_weight(size_t v)
    { return this->_node_self_weights[v]; };

    inline size_t degree(size_t v, igraph_neimode_t mode)
    {
      if (mode == IGRAPH_IN || !this->is_directed())
        return this->_degree_in[v];
      else if (mode == IGRAPH_OUT)
        return this->_degree_out[v];
      else if (mode == IGRAPH_ALL)
        return this->_degree_all[v];
      else
        throw Exception("Incorrect mode specified.");
    };

    inline double strength(size_t v, igraph_neimode_t mode)
    {
      if (mode == IGRAPH_IN || !this->is_directed())
        return this->_strength_in[v];
      else if (mode == IGRAPH_OUT)
        return this->_strength_out[v];
      else
        throw Exception("Incorrect mode specified.");
    };

  protected:

    int _remove_graph;

  private:
    igraph_t* _graph;
    igraph_vector_t _temp_igraph_vector;

    // Utility variables to easily access the strength of each node
    vector<double> _strength_in;
    vector<double> _strength_out;

    vector<size_t> _degree_in;
    vector<size_t> _degree_out;
    vector<size_t> _degree_all;

    vector<double> _edge_weights; // Used for the weight of the edges.
    vector<size_t> _node_sizes; // Used for the size of the nodes.
    vector<double> _node_self_weights; // Used for the self weight of the nodes.

    void cache_neighbours(size_t v, igraph_neimode_t mode);
    vector<size_t> _cached_neighs_from; size_t _current_node_cache_neigh_from;
    vector<size_t> _cached_neighs_to;   size_t _current_node_cache_neigh_to;
    vector<size_t> _cached_neighs_all;  size_t _current_node_cache_neigh_all;

    void cache_neighbour_edges(size_t v, igraph_neimode_t mode);
    vector<size_t> _cached_neigh_edges_from; size_t _current_node_cache_neigh_edges_from;
    vector<size_t> _cached_neigh_edges_to;   size_t _current_node_cache_neigh_edges_to;
    vector<size_t> _cached_neigh_edges_all;  size_t _current_node_cache_neigh_edges_all;

    double _total_weight;
    size_t _total_size;
    int _is_weighted;
    bool _is_directed;

    int _correct_self_loops;
    double _density;

    void init_admin();
    void set_defaults();
    void set_default_edge_weight();
    void set_default_node_size();
    void set_self_weights();

};

// We need this ugly way to include the MutableVertexPartition
// to overcome a circular linkage problem.
#include "MutableVertexPartition.h"

#endif // GRAPHHELPER_INCLUDED


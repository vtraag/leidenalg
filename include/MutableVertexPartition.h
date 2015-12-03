#ifndef MUTABLEVERTEXPARTITION_H
#define MUTABLEVERTEXPARTITION_H

#include <string>
#include "GraphHelper.h"
#include <map>
#include <set>
#include <utility>
#include <algorithm>

using std::string;
using std::map;
using std::set;
using std::make_pair;
using std::pair;
using std::sort;
using std::reverse;

/****************************************************************************
Contains a partition of graph.

This class contains the basic implementation for optimising a partition.
Specifically, it implements all the administration necessary to keep track of
the partition from various points of view. Internally, it keeps track of the
number of internal edges (or total weight), the size of the communities, the
total incoming degree (or weight) for a community, etc... When deriving from
this class, one can easily use this administration to provide their own
implementation.

In order to keep the administration up-to-date, all changes in partition
should be done through move_node. This function moves a node from one
community to another, and updates all the administration.

It is possible to manually update the membership vector, and then call
__init_admin() which completely refreshes all the administration. This is
only possible by updating the membership vector, not by changing some of the
other variables.

The basic idea is that diff_move computes the difference in the quality
function if we call move_node for the same move. Using this framework, the
Louvain method in the optimisation class can call these general functions in
order to optimise the quality function.
*****************************************************************************/

class MutableVertexPartition
{
  public:
    MutableVertexPartition(Graph* graph,
        vector<size_t> membership);
    MutableVertexPartition(Graph* graph);
    virtual MutableVertexPartition* create(Graph* graph);

    virtual ~MutableVertexPartition();

    inline size_t membership(size_t v) { return this->_membership[v]; };
    inline vector<size_t> membership() { return this->_membership; };

    size_t csize(size_t comm);
    set<size_t>* get_community(size_t comm);
    size_t nb_communities();

    void move_node(size_t v,size_t new_comm);
    virtual double diff_move(size_t v, size_t new_comm)
    {
      throw Exception("Function not implemented. This should be implented in a derived class, since the base class does not implement a specific method.");
    };
    virtual double quality()
    {
      throw Exception("Function not implemented. This should be implented in a derived class, since the base class does not implement a specific method.");
    };

    inline Graph* get_graph() { return this->graph; };

    void renumber_communities();
    void renumber_communities(vector<size_t> new_membership);
    void from_coarser_partition(MutableVertexPartition* partition);
    void from_partition(MutableVertexPartition* partition);

    inline double total_weight_in_comm(size_t comm) { return this->_total_weight_in_comm[comm]; };
    inline double total_weight_from_comm(size_t comm) { return this->_total_weight_from_comm[comm]; };
    inline double total_weight_to_comm(size_t comm) { return this->_total_weight_to_comm[comm]; };
    inline double total_weight_in_all_comms() { return this->_total_weight_in_all_comms; };
    inline size_t total_possible_edges_in_all_comms() { return this->_total_possible_edges_in_all_comms; };

    double weight_to_comm(size_t v, size_t comm);
    double weight_from_comm(size_t v, size_t comm);

    set<size_t>* get_neigh_comms(size_t v, igraph_neimode_t);

  protected:

    void init_admin();

    vector<size_t> _membership; // Membership vector, i.e. \sigma_i = c means that node i is in community c

    Graph* graph;

    // Keep track of each community (i.e. which community contains which nodes)
    vector< set<size_t>* > community;
    // Community size
    vector< size_t > _csize;

    double weight_vertex_tofrom_comm(size_t v, size_t comm, igraph_neimode_t mode);

    void set_default_attrs();

  private:

    // Keep track of the internal weight of each community
    vector<double> _total_weight_in_comm;
    // Keep track of the total weight to a community
    vector<double> _total_weight_to_comm;
    // Keep track of the total weight from a community
    vector<double> _total_weight_from_comm;
    // Keep track of the total internal weight
    double _total_weight_in_all_comms;
    size_t _total_possible_edges_in_all_comms;

    void clean_mem();
    void init_graph_admin();

};

#endif // MUTABLEVERTEXPARTITION_H

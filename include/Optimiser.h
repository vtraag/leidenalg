#ifndef OPTIMISER_H
#define OPTIMISER_H
#include "GraphHelper.h"
#include "MutableVertexPartition.h"
#include <set>
#include <map>

#include <iostream>
using std::cerr;
using std::endl;
using std::set;
using std::map;

/****************************************************************************
Class for doing community detection using the Louvain algorithm.

Given a certain partition type is calls diff_move for trying to move a node
to another community. It moves the node to the community that *maximises*
this diff_move. If no further improvement is possible, the graph is
aggregated (collapse_graph) and the method is reiterated on that graph.
****************************************************************************/

class Optimiser
{
  public:
    Optimiser();
    double optimise_partition(MutableVertexPartition* partition);
    template <class T> T* find_partition(Graph* graph);
    template <class T> T* find_partition(Graph* graph, double resolution_parameter);

    // The multiplex functions that simultaneously optimise multiple graphs and partitions (i.e. methods)
    // Each node will be in the same community in all graphs, and the graphs are expected to have identical nodes
    // Optionally we can loop over all possible communities instead of only the neighbours. In the case of negative
    // layer weights this may be necessary.
    double optimise_partition(vector<MutableVertexPartition*> partitions, vector<double> layer_weights);

    double move_nodes(MutableVertexPartition* partition);
    double move_nodes(MutableVertexPartition* partition, int consider_comms);
    double move_nodes(vector<MutableVertexPartition*> partitions, vector<double> layer_weights);
    double move_nodes(vector<MutableVertexPartition*> partitions, vector<double> layer_weights, int consider_comms, int consider_empty_community);

    virtual ~Optimiser();

    int consider_comms;  // Indicates how communities will be considered for improvement. Should be one of the parameters below
    int consider_empty_community; // Determine whether to consider moving nodes to an empty community

    static const int ALL_COMMS = 1;       // Consider all communities for improvement.
    static const int ALL_NEIGH_COMMS = 2; // Consider all neighbour communities for improvement.
    static const int RAND_COMM = 3;       // Consider a random commmunity for improvement.
    static const int RAND_NEIGH_COMM = 4; // Consider a random community among the neighbours for improvement.

  protected:

  private:
    void print_settings();
};

template <class T> T* Optimiser::find_partition(Graph* graph)
{
  T* partition = new T(graph);
  #ifdef DEBUG
    cerr << "Use default partition (all nodes in own community)" << endl;
  #endif
  this->optimise_partition(partition);
  return partition;
}

template <class T> T* Optimiser::find_partition(Graph* graph, double resolution_parameter)
{
  T* partition = new T(graph, resolution_parameter);
  #ifdef DEBUG
    cerr << "Use default partition (all nodes in own community)" << endl;
  #endif
  this->optimise_partition(partition);
  return partition;
}

#endif // OPTIMISER_H

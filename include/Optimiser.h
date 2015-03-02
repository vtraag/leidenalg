#ifndef OPTIMISER_H
#define OPTIMISER_H
#include "GraphHelper.h"
#include "MutableVertexPartition.h"
#include <algorithm>
#include <set>

#ifdef DEBUG
  using std::cerr;
  using std::endl;
#endif

using std::set;
using std::random_shuffle;

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
    Optimiser(double eps, double delta, size_t max_itr, int random_order, int consider_comms);
    Optimiser();
    double optimize_partition(MutableVertexPartition* partition);
    template <class T> T* find_partition(Graph* graph);
    template <class T> T* find_partition(Graph* graph, double resolution_parameter);
    double move_nodes(MutableVertexPartition* partition, int consider_comms);

    // The multiplex functions that simultaneously optimize multiple graphs and partitions (i.e. methods)
    // Each node will be in the same community in all graphs, and the graphs are expected to have identical nodes
    // Optionally we can loop over all possible communities instead of only the neighbours. In the case of negative
    // layer weights this may be necessary.
    double optimize_partition(vector<MutableVertexPartition*> partitions, vector<double> layer_weights);
    double move_nodes(vector<MutableVertexPartition*> partitions, vector<double> layer_weights, int consider_comms);

    virtual ~Optimiser();

    double eps;          // If the improvement falls below this threshold, stop iterating.
    double delta;        // If the number of nodes that moves falls below this threshold, stop iterating.
    size_t max_itr;      // Maximum number of iterations to perform.
    int random_order;    // If True the nodes will be traversed in a random order when optimising a quality function.
    int consider_comms;  // Indicates how communities will be considered for improvement. Should be one of the parameters below

    static const int ALL_COMMS = 1;       // Consider all communities for improvement.
    static const int ALL_NEIGH_COMMS = 2; // Consider all neighbour communities for improvement.
    static const int RAND_COMM = 3;       // Consider a random commmunity for improvement.
    static const int RAND_NEIGH_COMM = 4; // Consider a random community among the neighbours for improvement.

  protected:

  private:
};

template <class T> T* Optimiser::find_partition(Graph* graph)
{
  T* partition = new T(graph);
  #ifdef DEBUG
    cerr << "Use default partition (all nodes in own community)" << endl;
  #endif
  this->optimize_partition(partition);
  return partition;
}

template <class T> T* Optimiser::find_partition(Graph* graph, double resolution_parameter)
{
  T* partition = new T(graph, resolution_parameter);
  #ifdef DEBUG
    cerr << "Use default partition (all nodes in own community)" << endl;
  #endif
  this->optimize_partition(partition);
  return partition;
}

#endif // OPTIMISER_H

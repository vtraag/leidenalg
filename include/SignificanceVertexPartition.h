#ifndef SIGNIFICANCEVERTEXPARTITION_H
#define SIGNIFICANCEVERTEXPARTITION_H

#include <LinearResolutionParameterVertexPartition.h>

class SignificanceVertexPartition : public LinearResolutionParameterVertexPartition
{
  public:
    SignificanceVertexPartition(Graph* graph, vector<size_t> membership);
    SignificanceVertexPartition(Graph* graph);
    SignificanceVertexPartition(Graph* graph, vector<size_t> membership, double resolution_parameter);
    SignificanceVertexPartition(Graph* graph, double resolution_parameter);

    virtual ~SignificanceVertexPartition();
    virtual SignificanceVertexPartition* create(Graph* graph);

    virtual double diff_move(size_t v, size_t new_comm);
    virtual double quality();
  protected:
  private:
};

#endif // SIGNIFICANCEVERTEXPARTITION_H

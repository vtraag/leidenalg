#ifndef LINEARRESOLUTIONPARAMETERVERTEXPARTITION_H
#define LINEARRESOLUTIONPARAMETERVERTEXPARTITION_H

#include <MutableVertexPartition.h>

class LinearResolutionParameterVertexPartition : public MutableVertexPartition
{
  public:
    LinearResolutionParameterVertexPartition(Graph* graph,
          vector<size_t> membership, double resolution_parameter);
    LinearResolutionParameterVertexPartition(Graph* graph,
          vector<size_t> membership);
    LinearResolutionParameterVertexPartition(Graph* graph, double resolution_parameter);
    LinearResolutionParameterVertexPartition(Graph* graph);
    virtual ~LinearResolutionParameterVertexPartition();

  protected:
    double resolution_parameter;
  private:
};

#endif // LINEARRESOLUTIONPARAMETERVERTEXPARTITION_H

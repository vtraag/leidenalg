#ifndef RESOLUTIONPARAMETERVERTEXPARTITION_H
#define RESOLUTIONPARAMETERVERTEXPARTITION_H

#include <MutableVertexPartition.h>

class ResolutionParameterVertexPartition : public MutableVertexPartition
{
  public:
    ResolutionParameterVertexPartition(Graph* graph,
          vector<size_t> membership, double resolution_parameter);
    ResolutionParameterVertexPartition(Graph* graph,
          vector<size_t> membership);
    ResolutionParameterVertexPartition(Graph* graph, double resolution_parameter);
    ResolutionParameterVertexPartition(Graph* graph);
    virtual ~ResolutionParameterVertexPartition();

    double resolution_parameter;
  private:

};

#endif // RESOLUTIONPARAMETERVERTEXPARTITION_H

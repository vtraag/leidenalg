#ifndef SURPRISEVERTEXPARTITION_H
#define SURPRISEVERTEXPARTITION_H

#include "LinearResolutionParameterVertexPartition.h"

class SurpriseVertexPartition: public LinearResolutionParameterVertexPartition
{
  public:
    SurpriseVertexPartition(Graph* graph, vector<size_t> membership);
    SurpriseVertexPartition(Graph* graph);
    SurpriseVertexPartition(Graph* graph, vector<size_t> membership, double resolution_parameter);
    SurpriseVertexPartition(Graph* graph, double resolution_parameter);

    virtual ~SurpriseVertexPartition();
    virtual SurpriseVertexPartition* create(Graph* graph);

    virtual double diff_move(size_t v, size_t new_comm);
    virtual double quality();
  protected:
  private:
};

#endif // SURPRISEVERTEXPARTITION_H

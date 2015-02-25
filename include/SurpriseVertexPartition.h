#ifndef SURPRISEVERTEXPARTITION_H
#define SURPRISEVERTEXPARTITION_H

#include "MutableVertexPartition.h"

class SurpriseVertexPartition: public MutableVertexPartition
{
  public:
    SurpriseVertexPartition(Graph* graph, vector<size_t> membership);
    SurpriseVertexPartition(Graph* graph, SurpriseVertexPartition* partition);
    SurpriseVertexPartition(Graph* graph);
    virtual ~SurpriseVertexPartition();
    virtual SurpriseVertexPartition* create(Graph* graph);

    virtual double diff_move(size_t v, size_t new_comm);
    virtual double quality();
  protected:
  private:
};

#endif // SURPRISEVERTEXPARTITION_H

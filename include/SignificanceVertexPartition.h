#ifndef SIGNIFICANCEVERTEXPARTITION_H
#define SIGNIFICANCEVERTEXPARTITION_H

#include <MutableVertexPartition.h>


class SignificanceVertexPartition : public MutableVertexPartition
{
  public:
    SignificanceVertexPartition(Graph* graph, vector<size_t> membership);
    SignificanceVertexPartition(Graph* graph);
    virtual ~SignificanceVertexPartition();
    virtual SignificanceVertexPartition* create(Graph* graph);

    virtual double diff_move(size_t v, size_t new_comm);
    virtual double quality();
  protected:
  private:
};

#endif // SIGNIFICANCEVERTEXPARTITION_H

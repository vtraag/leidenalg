#ifndef CPMVERTEXPARTITION_H
#define CPMVERTEXPARTITION_H

#include <LinearResolutionParameterVertexPartition.h>

class CPMVertexPartition : public LinearResolutionParameterVertexPartition
{
  public:
    CPMVertexPartition(Graph* graph,
          vector<size_t> membership, double resolution_parameter);
    CPMVertexPartition(Graph* graph,
          vector<size_t> membership);
    CPMVertexPartition(Graph* graph,
      double resolution_parameter);
    CPMVertexPartition(Graph* graph);
    virtual ~CPMVertexPartition();
    virtual CPMVertexPartition* create(Graph* graph);

    virtual double diff_move(size_t v, size_t new_comm);
    virtual double quality();

  protected:
  private:
};

#endif // CPMVERTEXPARTITION_H

#ifndef SURPRISEVERTEXCover_H
#define SURPRISEVERTEXCover_H

#include "MutableVertexCover.h"

class SurpriseVertexCover: public MutableVertexCover
{
  public:
    SurpriseVertexCover(Graph* graph, vector< set<size_t>* > membership);
    SurpriseVertexCover(Graph* graph, SurpriseVertexCover* Cover);
    SurpriseVertexCover(Graph* graph);
    virtual ~SurpriseVertexCover();
    virtual SurpriseVertexCover* create(Graph* graph);

    virtual double diff_move(size_t v, size_t old_comm, size_t new_comm);
    virtual double diff_add(size_t v, size_t new_comm);
    virtual double diff_remove(size_t v, size_t old_comm);
    virtual double quality();
  protected:
  private:
};

#endif // SURPRISEVERTEXCover_H

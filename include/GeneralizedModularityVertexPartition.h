#ifndef GENERALIZEDMODULARITYVERTEXPARTITION_H
#define GENERALIZEDMODULARITYVERTEXPARTITION_H

#include <MutableVertexPartition.h>

class GeneralizedModularityVertexPartition : public MutableVertexPartition
{
  public:
    GeneralizedModularityVertexPartition(Graph* graph,
        vector<size_t> const& membership, vector<vector<double> > const& null_model);
    GeneralizedModularityVertexPartition(Graph* graph, vector<vector<double> > const& null_model);
    virtual ~GeneralizedModularityVertexPartition();
    virtual GeneralizedModularityVertexPartition* create(Graph* graph);
    virtual GeneralizedModularityVertexPartition* create(Graph* graph, vector<size_t> const& membership);
    vector<vector<double> > null_model;

    virtual double diff_move(size_t v, size_t new_comm);
    virtual double quality();

  protected:
    virtual void init_comm_loss_vectors();

  private:
    vector<vector<double> > _comm_loss_vectors;
    vector <double> _node_weight_to_communities;
    size_t _n_null_models;

};

#endif //GENERALIZEDMODULARITYVERTEXPARTITION_H

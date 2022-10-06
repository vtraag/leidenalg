#include "GeneralizedModularityVertexPartition.h"

GeneralizedModularityVertexPartition::GeneralizedModularityVertexPartition(Graph* graph,
      vector<size_t> const& membership, vector<vector<double> > const& null_model) :
        MutableVertexPartition(graph,
        membership)
{
  this->null_model = null_model;
  this->_n_null_models = null_model.size();
}

GeneralizedModularityVertexPartition::GeneralizedModularityVertexPartition(Graph* graph,
      vector<vector<double> > const& null_model) :
        MutableVertexPartition(graph)
{
  this->null_model = null_model;
  this->_n_null_models = null_model.size();
}


GeneralizedModularityVertexPartition::~GeneralizedModularityVertexPartition()
{ }

GeneralizedModularityVertexPartition* GeneralizedModularityVertexPartition::create(Graph* graph)
{
  return new GeneralizedModularityVertexPartition(graph, this->null_model);
}

GeneralizedModularityVertexPartition* GeneralizedModularityVertexPartition::create(Graph* graph, vector<size_t> const& membership)
{
  return new GeneralizedModularityVertexPartition(graph, membership, this->null_model);
}


GeneralizedModularityVertexPartition* GeneralizedModularityVertexPartition::create(Graph* graph, vector<size_t> const& membership, vector< vector<size_t> > communities)
{
  vector<vector<double> > null_model = this->_collapse_null_models(graph, communities);
  return new GeneralizedModularityVertexPartition(graph, membership, null_model);
}

vector<vector <double> > GeneralizedModularityVertexPartition::_collapse_null_models(Graph* graph, vector< vector<size_t> > communities)
{
    vector<vector<double> > null_model;
    null_model.clear();
    null_model.resize(this->_n_null_models);
    for (unsigned int k = 0; k < this->_n_null_models; ++k) 
    {
      null_model[k].clear();
      null_model[k].resize(graph->vcount());
      for (size_t u = 0; u < graph->vcount(); ++u)
      {
        vector<size_t> comm = communities[u];
        for (size_t v = 0; v < comm.size(); ++v){
            null_model[k][u] += this->null_model[k][comm[v]];
        }
      }
    }
    return null_model;
}

double GeneralizedModularityVertexPartition::diff_move(size_t v, size_t new_comm)
{
  double loss = 0;
  double gain = 0;

  size_t old_comm = this->_membership[v];
  if (new_comm == old_comm)
      return 0.0;
  
  double w_to_old = this->weight_to_comm(v, old_comm);
  double w_from_old = this->weight_from_comm(v, old_comm);
  double w_to_new = this->weight_to_comm(v, new_comm);
  double w_from_new = this->weight_from_comm(v, new_comm);
  double self_weight = this->graph->node_self_weight(v);

  loss = w_to_old + w_from_old;
  gain = w_to_new + w_from_new + 2 * self_weight;

  for (size_t u = 0; u < this->graph->vcount(); ++u){
    size_t u_comm = this->_membership[u];
    if (u_comm == old_comm){
      for (size_t m = 0; m < this->_n_null_models; m = m + 2) {
        loss -= this->null_model[m][u] * this->null_model[m + 1][v];
        loss -= this->null_model[m][v] * this->null_model[m + 1][u];
      }
    }
    if (u_comm == new_comm){
      for (size_t m = 0; m < this->_n_null_models; m = m + 2) {
        gain -= this->null_model[m][u] * this->null_model[m + 1][v];
        gain -= this->null_model[m][v] * this->null_model[m + 1][u];
      }
    }
  }

  for (size_t m = 0; m < this->_n_null_models; m = m + 2) {
    gain -= this->null_model[m][v] * this->null_model[m + 1][v];
    gain -= this->null_model[m][v] * this->null_model[m + 1][v];
  }
  return gain - loss;
}


double GeneralizedModularityVertexPartition::quality()
{
  double mod = 0.0;
  vector<double> mod_null;
  mod_null.clear();
  mod_null.resize(this->n_communities());

  size_t n = this->graph->vcount();
  for (size_t u = 0; u < n; ++u)
  {
    for (size_t v = 0; v < n; ++v)
     {
        size_t u_comm = this->_membership[u];
        size_t v_comm = this->_membership[v];
        if (u_comm == v_comm)
        {
          for (size_t m = 0; m < this->_n_null_models; m = m + 2) 
            mod_null[u_comm] += this->null_model[m][u] * this->null_model[m + 1][v];
        }
     }
  }

  for (size_t c = 0; c < this->n_communities(); ++c)
  {
    mod += this->total_weight_in_comm(c) - mod_null[c];
  }
  return mod;
}

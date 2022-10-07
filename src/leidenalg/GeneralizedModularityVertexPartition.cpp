#include "GeneralizedModularityVertexPartition.h"
/**
 * Generalised modularity relaxes the conditions between the adjacency matrix and the null model, thus one needs to keep track of the null model, and in particular accross graph collapsing.
 */

GeneralizedModularityVertexPartition::GeneralizedModularityVertexPartition(Graph* graph,
      vector<size_t> const& membership, vector<vector<double> > const& null_model) :
        MutableVertexPartition(graph, membership)
{
  this->null_model = null_model;
}

GeneralizedModularityVertexPartition::GeneralizedModularityVertexPartition(Graph* graph,
      vector<vector<double> > const& null_model) :
        MutableVertexPartition(graph)
{
  this->null_model = null_model;
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

/**
 * This constructor is necessary to be able to collapse the null model in the same way the graph is collapsed during optimisation.
 */
GeneralizedModularityVertexPartition* GeneralizedModularityVertexPartition::create(Graph* graph, vector<size_t> const& membership, vector< vector<size_t> > collapsed_communities)
{
    // collapsing null model first by summing original null_model over nodes in the same community used to collapsed the graph
    vector<vector<double> > collapsed_null_model;
    collapsed_null_model.clear();
    collapsed_null_model.resize(this->null_model.size());
    for (unsigned int k = 0; k < this->null_model.size(); ++k) 
    {
      collapsed_null_model[k].clear();
      collapsed_null_model[k].resize(graph->vcount());
      for (size_t u = 0; u < graph->vcount(); ++u)
      {
        vector<size_t> comm = collapsed_communities[u];
        for (size_t v = 0; v < comm.size(); ++v){
            collapsed_null_model[k][u] += this->null_model[k][comm[v]];
        }
      }
    }
 
  return new GeneralizedModularityVertexPartition(graph, membership, null_model);
}

/**
 * Computes the difference in generalized modularity by moving a node v into another community new_comm
 */
double GeneralizedModularityVertexPartition::diff_move(size_t v, size_t new_comm)
{
  double loss = 0;
  double gain = 0;

  size_t old_comm = this->_membership[v];

  // if we do not move to another community, quality does not change
  if (new_comm == old_comm)
      return 0.0;
 
  // compute the loss to remove from old_comm and gain to move to new_comm for quality matrix
  double w_to_old = this->weight_to_comm(v, old_comm);
  double w_from_old = this->weight_from_comm(v, old_comm);
  double w_to_new = this->weight_to_comm(v, new_comm);
  double w_from_new = this->weight_from_comm(v, new_comm);
  double self_weight = this->graph->node_self_weight(v);

  loss = w_to_old + w_from_old;
  gain = w_to_new + w_from_new + 2 * self_weight;
    
  // compute the gain and loss for null model
  for (size_t u = 0; u < this->graph->vcount(); ++u)
  {
    size_t u_comm = this->_membership[u];
    if (u_comm == old_comm)
      for (size_t m = 0; m < this->null_model.size(); m = m + 2) 
      {
        loss -= this->null_model[m][u] * this->null_model[m + 1][v];
        loss -= this->null_model[m][v] * this->null_model[m + 1][u];
      }
    if (u_comm == new_comm)
      for (size_t m = 0; m < this->null_model.size(); m = m + 2) 
      {
        gain -= this->null_model[m][u] * this->null_model[m + 1][v];
        gain -= this->null_model[m][v] * this->null_model[m + 1][u];
      }
  }

  // we also need to add the contribution of the added node to the new_comm
  for (size_t m = 0; m < this->null_model.size(); m = m + 2) 
  {
    gain -= this->null_model[m][v] * this->null_model[m + 1][v];
    gain -= this->null_model[m][v] * this->null_model[m + 1][v];
  }

  return gain - loss;
}

/**
 * Compute the quality function of the generalized modularity.
 */
double GeneralizedModularityVertexPartition::quality()
{
  vector<double> mod_null;
  mod_null.clear();
  mod_null.resize(this->n_communities());
    
  // compute the contribution from null model
  size_t n = this->graph->vcount();
  for (size_t u = 0; u < n; ++u)
    for (size_t v = 0; v < n; ++v)
     {
        size_t u_comm = this->_membership[u];
        size_t v_comm = this->_membership[v];
        if (u_comm == v_comm)
          for (size_t m = 0; m < this->null_model.size(); m = m + 2) 
            mod_null[u_comm] += this->null_model[m][u] * this->null_model[m + 1][v];
     }

  // compute quality
  double gen_mod = 0.0;
  for (size_t c = 0; c < this->n_communities(); ++c)
    gen_mod += this->total_weight_in_comm(c) - mod_null[c];
  return gen_mod;
}

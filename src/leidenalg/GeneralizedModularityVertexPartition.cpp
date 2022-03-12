#include "GeneralizedModularityVertexPartition.h"

#ifdef DEBUG
#include <iostream>
using std::cerr;
using std::endl;
#endif

GeneralizedModularityVertexPartition::GeneralizedModularityVertexPartition(Graph* graph,
      vector<size_t> const& membership, vector<vector<double> > const& null_model) :
        MutableVertexPartition(graph,
        membership)
{
  this->null_model = null_model;
  this->_n_null_models = null_model.size();
  this->init_comm_loss_vectors();
}

GeneralizedModularityVertexPartition::GeneralizedModularityVertexPartition(Graph* graph,
      vector<vector<double> > const& null_model) :
        MutableVertexPartition(graph)
{
  this->null_model = null_model;
  this->_n_null_models = null_model.size();
  this->init_comm_loss_vectors();
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

// compute comm_loss_vectors for diff_move computation
void GeneralizedModularityVertexPartition::init_comm_loss_vectors()
{

  this->_comm_loss_vectors.clear();
  this->_comm_loss_vectors.resize(this->_n_null_models);
  for (unsigned int k = 0; k < this->_n_null_models; ++k) 
  {
    this->_comm_loss_vectors[k].clear();
    this->_comm_loss_vectors[k].resize(this->n_communities());
    for (size_t v = 0; v < graph->vcount(); v++)
    {
      this->_comm_loss_vectors[k][this->_membership[v]] += this->null_model[k][v];
    }
  }
}

/*****************************************************************************
  Returns the difference in modularity if we move a node to a new community
*****************************************************************************/
double GeneralizedModularityVertexPartition::diff_move(size_t v, size_t new_comm)
{

  double comm_loss = 0;
  for (unsigned int m = 0; m < this->_n_null_models; m = m + 2) {
    comm_loss += this->null_model[m][v] * this->_comm_loss_vectors[m + 1][new_comm] 
        + null_model[m + 1][v] * this->_comm_loss_vectors[m][new_comm];
  }

  this->_node_weight_to_communities.clear();
  this->_node_weight_to_communities.resize(this->n_communities());

  vector<size_t> const& neighbours = this->graph->get_neighbours(v, IGRAPH_ALL);
  vector<size_t> const& neighbour_edges = this->graph->get_neighbour_edges(v, IGRAPH_ALL);
  for (size_t idx = 0; idx < neighbours.size(); idx++)
  {
    size_t u_comm = this->_membership[neighbours[idx]];
    this->_node_weight_to_communities[u_comm] += this->graph->edge_weight(neighbour_edges[idx]);
  }
   return 2 * this->_node_weight_to_communities[new_comm] - comm_loss;
}


/*****************************************************************************
  Give the generalized modularity of the partition.
******************************************************************************/
double GeneralizedModularityVertexPartition::quality()
{
  double mod = 0.0;
  vector<double> mod_null;
  mod_null.clear();
  mod_null.resize(this->n_communities());

  size_t n = graph->vcount();
  for (size_t u = 0; u < n; u++)
  {
    for (size_t v = 0; v < n; v++)
     {
        size_t u_comm = this->_membership[u];
        size_t v_comm = this->_membership[v];
        if (u_comm == v_comm)
        {
          // we sum over nul models which come in pairs
          for (size_t m = 0; m < this->_n_null_models; m = m + 2) 
          {
            mod_null[u_comm] += this->null_model[m][u] * this->null_model[m + 1][v];
          }
        }
     }
  }

  for (size_t c = 0; c < this->n_communities(); c++)
  {
    mod += 2 * this->total_weight_in_comm(c) - mod_null[c];
  }
  return mod;
}

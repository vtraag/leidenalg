#include "VertexCover.h"

VertexCover::VertexCover(vector<size_t>& membership)
{
  this->_membership.resize(membership.size());
  for (size_t i = 0; i < membership.size(); i++)
  {
    this->_membership[i].push_back(make_pair(membership[i], 1.0));
  }
  this->_cached_node = 0;

  if (!membership.empty())
    this->set_cache(0);
}

VertexCover::VertexCover(vector<vector<pair<size_t, double>>>& membership)
{
  this->_membership = membership;

  this->_cached_node = 0;
  if (!membership.empty())
    this->set_cache(0);
}

vector< pair<size_t, double> > VertexCover::get_memberships(size_t node)
{
  return this->_membership[node];
}

void VertexCover::cache_membership(size_t node)
{
  if (_cached_node != node)
  {
    this->reset_cache();
    this->set_cache(node);
  }
}

void VertexCover::set_cache(size_t node)
{
  for (const pair<size_t, double>& comm : this->_membership[_cached_node])
  {
    this->_cached_node_membership[comm.first] =comm.second;
  }
  this->_cached_node = node;
}

void VertexCover::reset_cache()
{
  for (const pair<size_t, double>& comm : this->_membership[_cached_node])
  {
    this->_cached_node_membership[comm.first] = 0.0;
  }
}

double VertexCover::get_membership(size_t node, size_t comm)
{
  cache_membership(node);
  return this->_cached_node_membership[comm];
}

VertexCover* VertexCover::collapse_cover(MutableVertexPartition* partition)
{
  vector< vector<size_t> > comms = partition->get_communities();
  vector<double> aggregate_node_membership;
  vector< vector< pair<size_t, double> > > aggregate_cover = vector< vector< pair<size_t, double> > >(comms.size());
  vector<size_t> aggregate_memberships;

  for (int i = 0; i < comms.size(); i++)
  {
    for (size_t v : comms[i])
    {
      for (const pair<size_t, double>& cover_weight : this->_membership[v])
      {
        size_t cover = cover_weight.first;
        double weight = cover_weight.second;
        if (aggregate_node_membership[cover] == 0.0)
          aggregate_memberships.push_back(cover);
        aggregate_node_membership[cover] += weight;
      }
    }

    for (size_t cover : aggregate_memberships)
    {
      aggregate_cover[i].push_back(make_pair(cover, aggregate_node_membership[cover]));
      // Reset membership weights for next aggregate node
      aggregate_node_membership[cover] = 0.0;
    }

    // Reset aggregate memberships for next aggregate node
    aggregate_memberships.clear();
  }

  return new VertexCover(aggregate_cover);
}

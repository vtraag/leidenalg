#include "MutableVertexCover.h"

#ifdef DEBUG
#include <iostream>
  using std::cerr;
  using std::endl;
#endif

/****************************************************************************
  Create a new vertex Cover.

  Parameters:
    graph            -- The igraph.Graph on which this Cover is defined.
    membership=None  -- The membership vector of this Cover, i.e. an
                        community number for each node. So membership[i] = c
                        implies that node i is in community c. If None, it is
                        initialised with each node in its own community.
    weight_attr=None -- What edge attribute should be used as a weight for the
                        edges? If None, the weight defaults to 1.
    size_attr=None   -- What node attribute should be used for keeping track
                        of the size of the node? In some methods (e.g. CPM or
                        Significance), we need to keep track of the total
                        size of the community. So when we aggregate/collapse
                        the graph, we should know how many nodes were in a
                        community. If None, the size of a node defaults to 1.
    self_weight_attr=None
                     -- What node attribute should be used for the self
                        weight? If None, the self_weight is
                        recalculated each time."""
*****************************************************************************/

MutableVertexCover::MutableVertexCover(Graph* graph,
      vector< set<size_t>* > membership)
{
  this->graph = graph;
  if (membership.size() != graph->vcount())
  {
    throw Exception("Membership vector has incorrect size.");
  }
  this->_membership = membership;
  this->init_admin();
}

MutableVertexCover::MutableVertexCover(Graph* graph)
{
  this->graph = graph;
  this->_membership = range_cover(graph->vcount());
  this->init_admin();
}

MutableVertexCover* MutableVertexCover::create(Graph* graph)
{
  return new MutableVertexCover(graph);
}

MutableVertexCover::~MutableVertexCover()
{
  this->clean_mem();
}

void MutableVertexCover::clean_mem()
{
  while (!this->community.empty())
  {
    delete this->community.back();
    this->community.pop_back();
  }
}

size_t MutableVertexCover::csize(size_t comm)
{
  size_t csize = this->_csize[comm];
  size_t alt_csize = this->community[comm]->size();
  return csize;
}

set<size_t>* MutableVertexCover::get_community(size_t comm)
{
  return this->community[comm];
}

size_t MutableVertexCover::nb_communities()
{
  return this->community.size();
}

// Get overlap
set<size_t>* MutableVertexCover::get_overlap(vector<size_t> comms)
{
  set<size_t>* intersection = NULL;
  for (vector<size_t>::iterator it = comms.begin();
        it != comms.end(); it++)
  {
    size_t comm = *it;
    set<size_t>* vertex_set = this->community[comm];
    if (intersection == NULL)
    {
      // Set pointer to first community set
      intersection = new set<size_t>();
      std::copy(
        vertex_set->begin(), vertex_set->end(),
        std::inserter( *intersection, intersection->begin() ) );
    }
    else
    {
      // Calculate intersection
      set<size_t> tmp;
      std::set_intersection(intersection->begin(), intersection->end(),
                            vertex_set->begin(), vertex_set->end(),
                            std::inserter(tmp, tmp.begin()));
      // Copy into intersection
      intersection->clear();
      std::copy(
        tmp.begin(), tmp.end(),
        std::inserter( *intersection, intersection->begin() ) );
    }
    if (intersection->empty())
      break;
  }
  return intersection;
}

size_t MutableVertexCover::csize_overlap(vector<size_t> comms)
{
  return this->get_overlap(comms)->size();
}

set<size_t>* MutableVertexCover::get_overlap(size_t comm1, size_t comm2)
{
  vector<size_t> comms(2);
  comms[0] = comm1; comms[1] = comm2;
  return this->get_overlap(comms);
}

size_t MutableVertexCover::csize_overlap(size_t comm1, size_t comm2)
{
  return this->get_overlap(comm1, comm2)->size();
}

size_t MutableVertexCover::possible_overlapping_edges()
{
  size_t n_overlap = 0;
  for (size_t c = 0; c < this->nb_communities(); c++)
  {
    for (size_t d = c + 1; d < this->nb_communities(); d++)
    {
      size_t n_cd = this->csize_overlap(c, d);
      size_t possible_edges = 0;
      if (this->graph->correct_self_loops())
        possible_edges = n_cd*n_cd/(2.0 - this->graph->is_directed());
      else
        possible_edges = n_cd*(n_cd-1)/(2.0 - this->graph->is_directed());
      n_overlap += possible_edges;
    }
  }
  return n_overlap;
}

/****************************************************************************
  Initialise all the administration based on the membership vector.
*****************************************************************************/
void MutableVertexCover::init_admin()
{
  #ifdef DEBUG
    cerr << "void MutableVertexCover::init_admin()" << endl;
  #endif
  size_t n = this->graph->vcount();

  // First determine number of communities (assuming they are consecutively numbered
  size_t nb_comms = 0;
  for (size_t i = 0; i < n; i++)
  {
    size_t m = max(*(this->_membership[i])) + 1;
    if (m > nb_comms)
      nb_comms = m;
  }

  // Reset administration
  this->community.clear();
  for (size_t i = 0; i < nb_comms; i++)
    this->community.push_back(new set<size_t>());
  this->_total_weight_in_comm.clear();
  this->_total_weight_in_comm.resize(nb_comms);
  this->_total_weight_from_comm.clear();
  this->_total_weight_from_comm.resize(nb_comms);
  this->_total_weight_to_comm.clear();
  this->_total_weight_to_comm.resize(nb_comms);
  this->_csize.clear();
  this->_csize.resize(nb_comms);

  this->_total_weight_in_all_comms = 0.0;
  for (size_t v = 0; v < n; v++)
  {
    set<size_t>* v_comms = this->_membership[v];
    for (set<size_t>::iterator it=v_comms->begin();
            it!=v_comms->end();
            it++)
    {
        size_t v_comm = *it;
        // Add this node to the community sets
        this->community[v_comm]->insert(v);
        // Update the community size
        this->_csize[v_comm] += this->graph->node_size(v);

        // Loop over all incident edges
        vector< pair<size_t, size_t> >* neigh_edges
          = this->graph->get_neighbour_edges(v, IGRAPH_OUT);

        for (vector< pair<size_t, size_t> >::iterator v_it = neigh_edges->begin();
             v_it != neigh_edges->end(); v_it++)
        {
          size_t u = v_it->first;
          size_t e = v_it->second;
          set<size_t>* u_comms = this->_membership[u];
          for (set<size_t>::iterator it_u=u_comms->begin();
                  it_u!=u_comms->end();
                  it_u++)
          {
              size_t u_comm = *it_u;
              // Get the weight of the edge
              double w = this->graph->edge_weight(e);
              // Add weight to the outgoing weight of community of v
              this->_total_weight_from_comm[v_comm] += w;
              #ifdef DEBUG
                cerr << "\t" << "Add (" << v << ", " << u << ") weight " << w << " to from_comm " << v_comm <<  "." << endl;
              #endif
              // Add weight to the incoming weight of community of u
              this->_total_weight_to_comm[u_comm] += w;
              #ifdef DEBUG
                cerr << "\t" << "Add (" << v << ", " << u << ") weight " << w << " to to_comm " << u_comm << "." << endl;
              #endif
              // If it is an edge within a community
              if (v_comm == u_comm)
              {
                if (!this->graph->is_directed())
                  w /= 2.0;
                this->_total_weight_in_comm[v_comm] += w;
                this->_total_weight_in_all_comms += w;
                #ifdef DEBUG
                  cerr << "\t" << "Add (" << v << ", " << u << ") weight " << w << " to in_comm " << v_comm << "." << endl;
                #endif
              }
          }
        }
        delete neigh_edges;
    }
  }

  this->_total_possible_edges_in_all_comms = 0;
  for (size_t c = 0; c < nb_comms; c++)
  {
    size_t n_c = this->csize(c);
    size_t possible_edges = 0;

    if (this->graph->correct_self_loops())
      possible_edges = n_c*n_c/(2.0 - this->graph->is_directed());
    else
      possible_edges = n_c*(n_c-1)/(2.0 - this->graph->is_directed());

    #ifdef DEBUG
      cerr << "\t" << "c=" << c << ", n_c=" << n_c << ", possible_edges=" << possible_edges << endl;
    #endif

    this->_total_possible_edges_in_all_comms += possible_edges;
  }

  #ifdef DEBUG
    cerr << "exit MutableVertexCover::init_admin()" << endl << endl;
  #endif

}

/****************************************************************************
 Renumber the communities so that they are numbered 0,...,q-1 where q is
 the number of communities. This also removes any empty communities, as they
 will not be given a new number.
*****************************************************************************/
void MutableVertexCover::renumber_communities()
{
  size_t nb_comms = this->nb_communities();

  // First sort the communities by size
  vector<pair<size_t,size_t> > csizes;
  for (size_t i = 0; i < nb_comms; i++)
  {
      csizes.push_back(make_pair(this->csize(i), i));
  }
  sort(csizes.begin(), csizes.end());
  reverse(csizes.begin(), csizes.end());

  // Then use the sort order to assign new communities,
  // such that the largest community gets the lowest index.
  vector<size_t> new_comm_id(nb_comms, 0);
  for (size_t i = 0; i < nb_comms; i++)
  {
    size_t comm = csizes[i].second;
    new_comm_id[comm] = i;
  }

  for (size_t i = 0; i < this->graph->vcount(); i++)
  {
    set<size_t>* comms = this->_membership[i];
    set<size_t>* new_comms = new set<size_t>();
    for (set<size_t>::iterator it = comms->begin();
         it != comms->end();
         it++)
    {
      new_comms->insert(new_comm_id[*it]);
    }
    delete comms;
    this->_membership[i] = new_comms;
  }

  this->clean_mem();
  this->init_admin();
}

/****************************************************************************
 Renumber the communities using the provided membership vector. Notice that this
 doesn't ensure any property of the community numbers.
*****************************************************************************/
void MutableVertexCover::renumber_communities(vector< set<size_t>* > new_membership)
{
  for (size_t i = 0; i < this->graph->vcount(); i++)
    this->_membership[i] = new_membership[i];

  this->clean_mem();
  this->init_admin();
}

/****************************************************************************
  Move a node to a new community and update the administration.
  Parameters:
    v        -- Node to move.
    new_comm -- To which community should it move.
*****************************************************************************/
void MutableVertexCover::move_node(size_t v, size_t old_comm, size_t new_comm)
{
  #ifdef DEBUG
    cerr << "void MutableVertexCover::move_node(" << v << ", " << old_comm << ", " << new_comm << ")" << endl;
  #endif
  // We should only move nodes if the node isn't already a member of the new community.
  if (new_comm == old_comm || this->_membership[v]->count(new_comm) > 0)
    return;
  // Move node and update internal administration

  // Keep track of all possible edges in all communities;
  size_t node_size = this->graph->node_size(v);

  // Incidentally, this is independent of whether we take into account self-loops or not
  // (i.e. whether we count as n_c^2 or as n_c(n_c - 1). Be careful to do this before the
  // adaptation of the community sizes, otherwise the calculations are incorrect.
  size_t cn = this->_csize[new_comm];
  size_t co = this->_csize[old_comm];
  _total_possible_edges_in_all_comms += 2.0*(ptrdiff_t)node_size*((ptrdiff_t)cn - (ptrdiff_t)co + (ptrdiff_t)node_size)/(2.0 - this->graph->is_directed());

  // Remove from old community
  this->community[old_comm]->erase(v);
  this->_csize[old_comm] -= node_size;

  // Add to new community
  this->community[new_comm]->insert(v);
  this->_csize[new_comm] += this->graph->node_size(v);

  // Switch outgoing links

  // Use set for incident edges, because self loop appears twice
  igraph_neimode_t modes[2] = {IGRAPH_OUT, IGRAPH_IN};
  for (size_t mode_i = 0; mode_i < 2; mode_i++)
  {
    igraph_neimode_t mode = modes[mode_i];
    vector< pair<size_t, size_t> >* neigh_edges
      = this->graph->get_neighbour_edges(v, mode);

    #ifdef DEBUG
      if (mode == IGRAPH_OUT)
        cerr << "\t" << "Looping over outgoing links." << endl;
      else if (mode == IGRAPH_IN)
        cerr << "\t" << "Looping over incoming links." << endl;
      else
        cerr << "\t" << "Looping over unknown mode." << endl;
    #endif

    for (vector< pair<size_t, size_t> >::iterator v_it = neigh_edges->begin();
         v_it != neigh_edges->end(); v_it++)
    {
      size_t u = v_it->first;
      size_t e = v_it->second;

      set<size_t>* u_comms = this->_membership[u];
      for (set<size_t>::iterator it_u=u_comms->begin();
              it_u!=u_comms->end();
              it_u++)
      {
        size_t u_comm = *it_u;
        // Get the weight of the edge
        double w = this->graph->edge_weight(e);
        if (mode == IGRAPH_OUT)
        {
          // Remove the weight from the outgoing weights of the old community
          this->_total_weight_from_comm[old_comm] -= w;
          // Add the weight to the outgoing weights of the new community
          this->_total_weight_from_comm[new_comm] += w;
          #ifdef DEBUG
            cerr << "\t" << "Moving link (" << v << "-" << u << ") "
                 << "outgoing weight " << w
                 << " from " << old_comm << " to " << new_comm
                 << "." << endl;
          #endif
        }
        else if (mode == IGRAPH_IN)
        {
          // Remove the weight from the outgoing weights of the old community
          this->_total_weight_to_comm[old_comm] -= w;
          // Add the weight to the outgoing weights of the new community
          this->_total_weight_to_comm[new_comm] += w;
          #ifdef DEBUG
            cerr << "\t" << "Moving link (" << v << "-" << u << ") "
                 << "incoming weight " << w
                 << " from " << old_comm << " to " << new_comm
                 << "." << endl;
          #endif
        }
        else
          throw Exception("Incorrect mode for updating the admin.");
        // Get internal weight (if it is an internal edge)
        double int_weight = w/(this->graph->is_directed() ? 1.0 : 2.0)/( u == v ? 2.0 : 1.0);
        // If it is an internal edge in the old community
        if (old_comm == u_comm)
        {
          // Remove the internal weight
          this->_total_weight_in_comm[old_comm] -= int_weight;
          this->_total_weight_in_all_comms -= int_weight;
          #ifdef DEBUG
            cerr << "\t" << "From link (" << v << "-" << u << ") "
                 << "remove internal weight " << int_weight
                 << " from " << old_comm << "." << endl;
          #endif
        }
        // If it is an internal edge in the new community
        // i.e. if u is in the new community, or if it is a self loop
        if ((new_comm == u_comm) || (u == v))
        {
          // Add the internal weight
          this->_total_weight_in_comm[new_comm] += int_weight;
          this->_total_weight_in_all_comms += int_weight;
          #ifdef DEBUG
            cerr << "\t" << "From link (" << v << "-" << u << ") "
                 << "add internal weight " << int_weight
                 << " to " << new_comm << "." << endl;
          #endif
        }
      }
    }
    delete neigh_edges;
  }
  #ifdef DEBUG
    // Check this->_total_weight_in_all_comms
    double check_total_weight_in_all_comms = 0.0;
    for (size_t c = 0; c < this->nb_communities(); c++)
      check_total_weight_in_all_comms += this->total_weight_in_comm(c);
    cerr << "Internal _total_weight_in_all_comms=" << this->_total_weight_in_all_comms
         << ", calculated check_total_weight_in_all_comms=" << check_total_weight_in_all_comms << endl;
  #endif
  // Update the membership vector
  this->_membership[v]->erase(old_comm);
  this->_membership[v]->insert(new_comm);
  #ifdef DEBUG
    cerr << "exit MutableVertexCover::move_node(" << v << ", " << new_comm << ")" << endl << endl;
  #endif
}


/****************************************************************************
 Read new Cover from another Cover.
****************************************************************************/
void MutableVertexCover::from_cover(MutableVertexCover* Cover)
{
  // Assign the membership of every node in the supplied Cover
  // to the one in this Cover
  for (size_t v = 0; v < this->graph->vcount(); v++)
    this->_membership[v] = Cover->membership(v);
  this->clean_mem();
  this->init_admin();
}

/****************************************************************************
 Calculate what is the total weight going from a node to a community.

    Parameters:
      v      -- The node which to check.
      comm   -- The community which to check.
*****************************************************************************/
double MutableVertexCover::weight_to_comm(size_t v, size_t comm)
{
  return this->weight_vertex_tofrom_comm(v, comm, IGRAPH_OUT);
}

/****************************************************************************
 Calculate what is the total weight going from a community to a node.

    Parameters:
      v      -- The node which to check.
      comm   -- The community which to check.
*****************************************************************************/
double MutableVertexCover::weight_from_comm(size_t v, size_t comm)
{
  return this->weight_vertex_tofrom_comm(v, comm, IGRAPH_IN);
}

/****************************************************************************
 Calculate what is the total weight going from/to a node to a community.

    Parameters:
      v      -- The node which to check.
      comm   -- The community which to check.
*****************************************************************************/
double MutableVertexCover::weight_vertex_tofrom_comm(size_t v, size_t comm, igraph_neimode_t mode)
{
  return this->graph->weight_tofrom_community(v, comm, &this->_membership, mode);
}

set<size_t>* MutableVertexCover::get_neigh_comms(size_t v, igraph_neimode_t mode)
{
  vector<size_t>* neigh = this->graph->get_neighbours(v, mode);
  set<size_t>* neigh_comms = new set<size_t>();
  for (size_t i=0; i < this->graph->degree(v, mode); i++)
  {
    set<size_t>* comms = this->_membership[(*neigh)[i]];
    for (set<size_t>::iterator it = comms->begin();
          it != comms->end();
          it++)
      neigh_comms->insert( *it );
  }
  delete neigh;
  return neigh_comms;
}

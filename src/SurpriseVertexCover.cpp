#include "SurpriseVertexCover.h"

#ifdef DEBUG
#include <iostream>
using std::cerr;
using std::endl;
#endif

SurpriseVertexCover::SurpriseVertexCover(Graph* graph,
      vector< set<size_t>* > membership) :
        MutableVertexCover(graph,
        membership)
{
  this->max_overlap = max(graph->_degree_all); //Maximum overlap
}

SurpriseVertexCover::SurpriseVertexCover(Graph* graph) :
        MutableVertexCover(graph)
{
  this->max_overlap = max(graph->_degree_all); //Maximum overlap
}

SurpriseVertexCover* SurpriseVertexCover::create(Graph* graph)
{
  return new SurpriseVertexCover(graph);
}

SurpriseVertexCover* SurpriseVertexCover::create(Graph* graph, vector< set<size_t>* > membership)
{
  return new SurpriseVertexCover(graph, membership);
}

SurpriseVertexCover::~SurpriseVertexCover()
{ }

double SurpriseVertexCover::diff_move(size_t v, size_t old_comm, size_t new_comm)
{
  #ifdef DEBUG
    cerr << "double SurpriseVertexCover::diff_move(" << v << ", " << old_comm << ", " << new_comm << ")" << endl;
  #endif
  size_t nsize = this->graph->node_size(v);
  #ifdef DEBUG
    cerr << "\t" << "nsize: " << nsize << endl;
  #endif
  double diff = 0.0;

  // Make sure we don't move from the same comm to the new comm
  // and also that the community is not already in the membership vector.
  if (new_comm != old_comm && this->_membership[v]->count(new_comm) == 0 && this->_membership[v]->count(old_comm) > 0)
  {
    double normalise = (2.0 - this->graph->is_directed());
    double m = this->graph->total_weight();
    size_t n = this->graph->total_size();
    size_t n2 = 0;

    if (this->graph->correct_self_loops())
      n2 = n*n/normalise;
    else
      n2 = n*(n-1)/normalise;
    #ifdef DEBUG
      cerr << "\t" << "Community: " << old_comm << " => " << new_comm << "." << endl;
      cerr << "\t" << "m: " << m << ", n2: " << n2 << "." << endl;
    #endif

    // Before move
    double mc = this->total_weight_in_all_comms();
    size_t nc2 = this->total_possible_edges_in_all_comms();
    #ifdef DEBUG
      cerr << "\t" << "mc: " << mc << ", nc2: " << nc2 << "." << endl;
    #endif

    // To old comm
    size_t n_old = this->csize(old_comm);
    double sw = this->graph->node_self_weight(v);
    double wtc = this->weight_to_comm(v, old_comm) - sw;
    double wfc = this->weight_from_comm(v, old_comm) - sw;
    #ifdef DEBUG
      cerr << "\t"  << "wtc: " << wtc << ", wfc: " << wfc << ", sw: " << sw << "." << endl;
    #endif
    double m_old = wtc/normalise + wfc/normalise + sw;
    #ifdef DEBUG
      cerr << "\t" << "m_old: " << m_old << ", n_old: " << n_old << "." << endl;
    #endif

    // To new comm
    size_t n_new = this->csize(new_comm);
    wtc = this->weight_to_comm(v, new_comm);
    wfc = this->weight_from_comm(v, new_comm);
    sw = this->graph->node_self_weight(v);
    #ifdef DEBUG
      cerr << "\t"  << "wtc: " << wtc << ", wfc: " << wfc << ", sw: " << sw << "." << endl;
    #endif
    double m_new = wtc/normalise + wfc/normalise + sw;
    #ifdef DEBUG
      cerr << "\t" << "m_new: " << m_new << ", n_new: " << n_new << "." << endl;
    #endif

    double delta_nc2 = 2*(ptrdiff_t)nsize*((ptrdiff_t)n_new - (ptrdiff_t)n_old + (ptrdiff_t)nsize)/normalise;
    double nc2_new = nc2 + delta_nc2;
    double mc_new = (mc - m_old + m_new);
    #ifdef DEBUG
      cerr << "\t" << "mc - m_old + m_new=" << (mc - m_old + m_new) << endl;
      cerr << "\t" << "nc2:\t" << nc2 << "." << endl;
      cerr << "\t" << "delta_nc2:\t" << delta_nc2 << "." << endl;
      cerr << "\t" << "nc2_new:\t" << nc2_new << "." << endl;
    #endif

    double q_new = (1./(double)this->max_overlap) * mc_new/m;
    double q     = (1./(double)this->max_overlap) * mc/m;
    double p_new = (1./(double)this->max_overlap) * nc2_new/(double)n2;
    double p     = (1./(double)this->max_overlap) * nc2/(double)n2;
    diff = m*KL(q_new, p_new) - m*KL(q, p);
    #ifdef DEBUG
      cerr << "\t" << "diff: " << diff << "." << endl;
    #endif
  }
  #ifdef DEBUG
    cerr << "exit double SurpriseVertexCover::diff_move(" << v << ", " << new_comm << ")" << endl;
    cerr << "return " << diff << endl << endl;
  #endif
  return diff;
}

double SurpriseVertexCover::diff_add(size_t v, size_t new_comm)
{
  #ifdef DEBUG
    cerr << "double SurpriseVertexCover::diff_add(" << v << ", " << new_comm << ")" << endl;
  #endif
  size_t nsize = this->graph->node_size(v);
  #ifdef DEBUG
    cerr << "\t" << "nsize: " << nsize << endl;
  #endif
  double diff = 0.0;
  // Make sure we don't move from the same comm to the new comm
  // and also that the community is not already in the membership vector.
  if (this->_membership[v]->count(new_comm) == 0)
  {
    double normalise = (2.0 - this->graph->is_directed());
    double m = this->graph->total_weight();
    size_t n = this->graph->total_size();
    size_t n2 = 0;

    if (this->graph->correct_self_loops())
      n2 = n*n/normalise;
    else
      n2 = n*(n-1)/normalise;
    #ifdef DEBUG
      cerr << "\t" << "Community: => " << new_comm << "." << endl;
      cerr << "\t" << "m: " << m << ", n2: " << n2 << "." << endl;
    #endif

    // Before move
    double mc = this->total_weight_in_all_comms();
    size_t nc2 = this->total_possible_edges_in_all_comms();
    #ifdef DEBUG
      cerr << "\t" << "mc: " << mc << ", nc2: " << nc2 << "." << endl;
    #endif

    // To new comm
    size_t n_new = this->csize(new_comm);
    double wtc = this->weight_to_comm(v, new_comm);
    double wfc = this->weight_from_comm(v, new_comm);
    double sw = this->graph->node_self_weight(v);
    #ifdef DEBUG
      cerr << "\t"  << "wtc: " << wtc << ", wfc: " << wfc << ", sw: " << sw << "." << endl;
    #endif
    double m_new = wtc/normalise + wfc/normalise + sw;
    #ifdef DEBUG
      cerr << "\t" << "m_new: " << m_new << ", n_new: " << n_new << "." << endl;
    #endif

    double delta_nc2 = (ptrdiff_t)nsize*(2*(ptrdiff_t)n_new + (ptrdiff_t)nsize - (1 - this->graph->correct_self_loops()))/normalise;
    double nc2_new = nc2 + delta_nc2;
    double mc_new = mc + m_new;
    #ifdef DEBUG
      cerr << "\t" << "mc + m_new=" << (mc + m_new) << endl;
      cerr << "\t" << "nc2:\t" << nc2 << "." << endl;
      cerr << "\t" << "delta_nc2:\t" << delta_nc2 << "." << endl;
      cerr << "\t" << "nc2_new:\t" << nc2_new << "." << endl;
    #endif

    double q_new = (1./(double)this->max_overlap) * mc_new/m;
    double q     = (1./(double)this->max_overlap) * mc/m;
    double p_new = (1./(double)this->max_overlap) * nc2_new/(double)n2;
    double p     = (1./(double)this->max_overlap) * nc2/(double)n2;
    diff = m*KL(q_new, p_new) - m*KL(q, p);
    #ifdef DEBUG
      cerr << "\t" << "diff: " << diff << "." << endl;
    #endif
  }
  #ifdef DEBUG
    cerr << "exit double SurpriseVertexCover::diff_add(" << v << ", " << new_comm << ")" << endl;
    cerr << "return " << diff << endl << endl;
  #endif
  return diff;
}

double SurpriseVertexCover::diff_remove(size_t v, size_t old_comm)
{
  #ifdef DEBUG
    cerr << "double SurpriseVertexCover::diff_remove(" << v << ", " << old_comm << ")" << endl;
  #endif
  size_t nsize = this->graph->node_size(v);
  #ifdef DEBUG
    cerr << "\t" << "nsize: " << nsize << endl;
  #endif
  double diff = 0.0;
  // Make sure we don't remove from an existing community
  if (this->_membership[v]->count(old_comm) > 0)
  {
    double normalise = (2.0 - this->graph->is_directed());
    double m = this->graph->total_weight();
    size_t n = this->graph->total_size();
    size_t n2 = 0;

    if (this->graph->correct_self_loops())
      n2 = n*n/normalise;
    else
      n2 = n*(n-1)/normalise;
    #ifdef DEBUG
      cerr << "\t" << "Community: " << old_comm << "." << endl;
      cerr << "\t" << "m: " << m << ", n2: " << n2 << "." << endl;
    #endif

    // Before move
    double mc = this->total_weight_in_all_comms();
    size_t nc2 = this->total_possible_edges_in_all_comms();
    #ifdef DEBUG
      cerr << "\t" << "mc: " << mc << ", nc2: " << nc2 << "." << endl;
    #endif

    // To old comm
    size_t n_old = this->csize(old_comm);
    double sw = this->graph->node_self_weight(v);
    double wtc = this->weight_to_comm(v, old_comm) - sw;
    double wfc = this->weight_from_comm(v, old_comm) - sw;
    #ifdef DEBUG
      cerr << "\t"  << "wtc: " << wtc << ", wfc: " << wfc << ", sw: " << sw << "." << endl;
    #endif
    double m_old = wtc/normalise + wfc/normalise + sw;
    #ifdef DEBUG
      cerr << "\t" << "m_old: " << m_old << ", n_old: " << n_old << "." << endl;
    #endif

    double delta_nc2 = -(ptrdiff_t)nsize*(2*(ptrdiff_t)n_old - (ptrdiff_t)nsize - (1 - this->graph->correct_self_loops()))/normalise;
    double nc2_new = nc2 + delta_nc2;
    double mc_new = mc - m_old;
    #ifdef DEBUG
      cerr << "\t" << "mc - m_old + m_new=" << (mc - m_old) << endl;
      cerr << "\t" << "nc2:\t" << nc2 << "." << endl;
      cerr << "\t" << "delta_nc2:\t" << delta_nc2 << "." << endl;
      cerr << "\t" << "nc2_new:\t" << nc2_new << "." << endl;
    #endif

    double q_new = (1./(double)this->max_overlap) * mc_new/m;
    double q     = (1./(double)this->max_overlap) * mc/m;
    double p_new = (1./(double)this->max_overlap) * nc2_new/(double)n2;
    double p     = (1./(double)this->max_overlap) * nc2/(double)n2;
    #ifdef DEBUG
      cerr << "\t" << "q_new=" << q_new << "\tp_new=" << p_new << endl;
      cerr << "\t" << "q=" << q_new << "\tp=" << p << endl;
    #endif

    diff = m*KL(q_new, p_new) - m*KL(q, p);
    #ifdef DEBUG
      cerr << "\t" << "diff: " << diff << "." << endl;
    #endif
  }
  #ifdef DEBUG
    cerr << "exit double SurpriseVertexCover::diff_remove(" << v << ", " << old_comm << ")" << endl;
    cerr << "return " << diff << endl << endl;
  #endif
  return diff;
}

double SurpriseVertexCover::quality()
{
  #ifdef DEBUG
    cerr << "double SurpriseVertexCover::quality()" << endl;
  #endif
  double normalise = (2.0 - this->graph->is_directed());
  double mc = this->total_weight_in_all_comms();
  size_t nc2 = this->total_possible_edges_in_all_comms();
  double m = this->graph->total_weight();
  size_t n = this->graph->total_size();

  size_t n2 = 0;
  if (this->graph->correct_self_loops())
    n2 = n*n/normalise;
  else
    n2 = n*(n-1)/normalise;

  #ifdef DEBUG
    cerr << "\t" << "mc=" << mc << ", m=" << m << ", nc2=" << nc2 << ", n2=" << n2 << "." << endl;
  #endif

  double q = (1./(double)this->max_overlap) * mc/m;
  double p = (1./(double)this->max_overlap) * nc2/(double)n2;
  #ifdef DEBUG
    cerr << "\t" << "q:\t" << q << "." << endl;
    cerr << "\t" << "p:\t" << p << "." << endl;
  #endif
  double S = m*KL(q, p);

  #ifdef DEBUG
    cerr << "exit SignificanceVertexCover::quality()" << endl;
    cerr << "return " << S << endl << endl;
  #endif
  return S;
}


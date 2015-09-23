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
{ }

SurpriseVertexCover::SurpriseVertexCover(Graph* graph) :
        MutableVertexCover(graph)
{ }

SurpriseVertexCover* SurpriseVertexCover::create(Graph* graph)
{
  return new SurpriseVertexCover(graph);
}

SurpriseVertexCover::~SurpriseVertexCover()
{ }

double SurpriseVertexCover::diff_move(size_t v, size_t old_comm, size_t new_comm)
{
  #ifdef DEBUG
    cerr << "virtual double SurpriseVertexCover::diff_move(" << v << ", " << old_comm << ", " << new_comm << ")" << endl;
  #endif
  size_t nsize = this->graph->node_size(v);
  #ifdef DEBUG
    cerr << "\t" << "nsize: " << nsize << endl;
  #endif
  double diff = 0.0;
  // Make sure we don't move from the same comm to the new comm
  // and also that the community is not already in the membership vector.
  if (new_comm != old_comm && this->_membership[v]->count(new_comm) == 0)
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

    double q = mc/(double)nc2;
    double delta_nc2 = 2*(ptrdiff_t)nsize*((ptrdiff_t)n_new - (ptrdiff_t)n_old + (ptrdiff_t)nsize)/normalise;
    double nc2_new = nc2 + delta_nc2;
    double q_new = (mc - m_old + m_new)/(double)nc2_new;
    double p = m/(double)n2;
    #ifdef DEBUG
      cerr << "\t" << "mc - m_old + m_new=" << (mc - m_old + m_new) << endl;
      cerr << "\t" << "q:\t" << q << "." << endl;
      cerr << "\t" << "q_new:\t" << q_new << "." << endl;
      cerr << "\t" << "p:\t" << p << "." << endl;
      cerr << "\t" << "nc2:\t" << nc2 << "." << endl;
      cerr << "\t" << "delta_nc2:\t" << delta_nc2 << "." << endl;
      cerr << "\t" << "nc2_new:\t" << nc2_new << "." << endl;
    #endif

    // The number of edges (without counting doubles) is simply the
    // total number of internal edges, minus the overlapping edges.
    size_t M_int = nc2 - this->total_possible_overlapping_edges();
    set<size_t>* comm_set = this->membership(v);
    ptrdiff_t delta_M_int = 0;
    for (set<size_t>::iterator it = comm_set->begin();
          it != comm_set->end(); it++)
    {
      size_t v_comm = *it;
      size_t n_ad = this->csize_overlap(v_comm, old_comm);
      size_t n_bd = this->csize_overlap(v_comm, new_comm);
      #ifdef DEBUG
        cerr << "\t" << "v_comm=" << v_comm << endl;
        cerr << "\t" << "overlap old=" << n_ad << endl;
        cerr << "\t" << "overlap new=" << n_bd << endl;
      #endif
      delta_M_int += 2*((ptrdiff_t)n_bd - (ptrdiff_t)n_ad + 1)/normalise;
    }
    size_t M_int_new = M_int + delta_M_int + delta_nc2;
    #ifdef DEBUG
      cerr << "\t" << "M_int=" << M_int << endl;
      cerr << "\t" << "delta M_int=" << delta_M_int << "." << endl;
      cerr << "\t" << "M_int_new=" << M_int_new << "." << endl;
    #endif

    diff = M_int_new*KL(q_new, p) - M_int*KL(q, p);
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

double SurpriseVertexCover::quality()
{
  #ifdef DEBUG
    cerr << "double SurpriseVertexCover::quality()" << endl;
  #endif
  double normalise = (2.0 - this->graph->is_directed());
  double mc = this->total_weight_in_all_comms();
  size_t nc2 = this->total_possible_edges_in_all_comms();
  size_t M_int = nc2 - this->total_possible_overlapping_edges();
  double m = this->graph->total_weight();
  size_t n = this->graph->total_size();

  size_t n2 = 0;
  if (this->graph->correct_self_loops())
    n2 = n*n/normalise;
  else
    n2 = n*(n-1)/normalise;

  #ifdef DEBUG
    cerr << "\t" << "mc=" << mc << ", m=" << m << ", nc2=" << nc2 << ", n2=" << n2 << ", M_int = " << M_int << "." << endl;
  #endif

  double q = mc/(double)nc2;
  double p = m/(double)n2;
  #ifdef DEBUG
    cerr << "\t" << "q:\t" << q << "." << endl;
    cerr << "\t" << "p:\t" << p << "." << endl;
  #endif
  double S = M_int*KL(q, p);

  #ifdef DEBUG
    cerr << "exit SignificanceVertexCover::quality()" << endl;
    cerr << "return " << S << endl << endl;
  #endif
  return S;
}

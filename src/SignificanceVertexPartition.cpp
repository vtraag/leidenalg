#include "SignificanceVertexPartition.h"

#ifdef DEBUG
#include <iostream>
using std::cerr;
using std::endl;
#endif

SignificanceVertexPartition::SignificanceVertexPartition(Graph* graph,
      vector<size_t> membership) :
        MutableVertexPartition(graph,
        membership)
{ }

SignificanceVertexPartition::SignificanceVertexPartition(Graph* graph) :
        MutableVertexPartition(graph)
{ }

SignificanceVertexPartition* SignificanceVertexPartition::create(Graph* graph)
{
  return new SignificanceVertexPartition(graph);
}

SignificanceVertexPartition::~SignificanceVertexPartition()
{ }

double SignificanceVertexPartition::diff_move(size_t v, size_t new_comm)
{
  #ifdef DEBUG
    cerr << "virtual double SignificanceVertexPartition::diff_move(" << v << ", " << new_comm << ")" << endl;
  #endif
  size_t old_comm = this->membership(v);
  size_t nsize = this->graph->node_size(v);
  double diff = 0.0;
  if (new_comm != old_comm)
  {
    double normalise = (2.0 - this->graph->is_directed());
    double p = this->graph->density();
    #ifdef DEBUG
      size_t n = this->graph->total_size();
      cerr << "\t" << "Community: " << old_comm << " => " << new_comm << "." << endl;
      cerr << "\t" << "n: " << n << ", m: " << this->graph->total_weight() << ", p: " << p << "." << endl;
    #endif

    //Old comm
    size_t n_old = this->csize(old_comm);
    double m_old = this->total_weight_in_comm(old_comm);
    double q_old = 0.0;
    if (n_old > 1)
      q_old = m_old/(n_old*(n_old - 1)/normalise);
    #ifdef DEBUG
      cerr << "\t" << "n_old: " << n_old << ", m_old: " << m_old << ", q_old: " << q_old << "." << endl;
    #endif
    // Old comm after move
    size_t n_oldx = n_old - nsize;
    double sw = this->graph->node_self_weight(v);
    // Be careful to exclude the self weight here, because this is include in the weight_to_comm function.
    double wtc = this->weight_to_comm(v, old_comm) - sw;
    double wfc = this->weight_from_comm(v, old_comm) - sw;
    #ifdef DEBUG
      cerr << "\t" << "wtc: " << wtc << ", wfc: " << wfc << ", sw: " << sw << "." << endl;
    #endif
    double m_oldx = m_old - wtc/normalise - wfc/normalise - sw;
    double q_oldx = 0.0;
    if (n_oldx > 1)
      q_oldx = m_oldx/(n_oldx*(n_oldx - 1)/normalise);
    #ifdef DEBUG
      cerr << "\t" << "n_oldx: " << n_oldx << ", m_oldx: " << m_oldx << ", q_oldx: " << q_oldx << "." << endl;
    #endif

    // New comm
    size_t n_new = this->csize(new_comm);
    double m_new = this->total_weight_in_comm(new_comm);
    double q_new = 0.0;
    if (n_new > 1)
      q_new = m_new/(n_new*(n_new - 1)/normalise);
    #ifdef DEBUG
      cerr << "\t" << "n_new: " << n_new << ", m_new: " << m_new << ", q_new: " << q_new << "." << endl;
    #endif

    // New comm after move
    size_t n_newx = n_new + nsize;
    wtc = this->weight_to_comm(v, new_comm);
    wfc = this->weight_from_comm(v, new_comm);
    sw = this->graph->node_self_weight(v);
    #ifdef DEBUG
      cerr << "\t" << "wtc: " << wtc << ", wfc: " << wfc << ", sw: " << sw << "." << endl;
    #endif
    double m_newx = m_new + wtc/normalise + wfc/normalise + sw;
    double q_newx = 0.0;
    if (n_newx > 1)
      q_newx = m_newx/float(n_newx*(n_newx - 1)/normalise);
    #ifdef DEBUG
      cerr << "\t" << "n_newx: " << n_newx << ", m_newx: " << m_newx << ", q_newx: " << q_newx << "." << endl;
    #endif

    // Calculate actual diff
    diff = - (double)n_old*(n_old-1)*KL(q_old, p)
                  + (double)n_oldx*(n_oldx-1)*KL(q_oldx, p)
                  - (double)n_new*(n_new-1)*KL(q_new, p)
                  + (double)n_newx*(n_newx-1)*KL(q_newx, p);
    #ifdef DEBUG
      cerr << "\t" << "diff: " << diff << "." << endl;
    #endif
  }
  #ifdef DEBUG
    cerr << "exit double SignificanceVertexPartition::diff_move(" << v << ", " << new_comm << ")" << endl;
    cerr << "return " << diff << endl << endl;
  #endif
  return diff;
}

/********************************************************************************
   Calculate the significance of the partition.
*********************************************************************************/
double SignificanceVertexPartition::quality()
{
  #ifdef DEBUG
    cerr << "double SignificanceVertexPartition::quality()";
    size_t n = this->graph->total_size();
  #endif
  double S = 0.0;
  double p = this->graph->density();
  #ifdef DEBUG
    cerr << "\t" << "n=" << n << ", m=" << this->graph->total_weight() << ", p=" << p << "." << endl;
  #endif
  for (size_t c = 0; c < this->nb_communities(); c++)
  {
    size_t n_c = this->csize(c);
    double m_c = this->total_weight_in_comm(c);
    double p_c = 0.0;
    if (n_c > 1)
    {
      p_c = m_c/(double)(n_c*(n_c - 1)/(2.0 - this->graph->is_directed()));
      #ifdef DEBUG
        cerr << "\t" << "c=" << c << ", n_c=" << n_c << ", m_c=" << m_c
           << ", p_c=" << p_c << ", p=" << p << ", KL=" << KL(p_c, p) << "." << endl;
      #endif
      S += KL(p_c, p)*n_c*(n_c - 1);
    }
    #ifdef DEBUG
    else
    {
      cerr << "\t" << "c=" << c << ", n_c=" << n_c << ", m_c=" << m_c << ", p_c=0.0." << endl;
    }
    #endif
  }
  #ifdef DEBUG
    cerr << "exit SignificanceVertexPartition::quality()" << endl;
    cerr << "return " << S << endl << endl;
  #endif
  return S;
}

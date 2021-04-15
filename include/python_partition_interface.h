#ifndef PYNTERFACE_PARTITION_H_INCLUDED
#define PYNTERFACE_PARTITION_H_INCLUDED

#include <Python.h>
#include <igraph.h>
#include "GraphHelper.h"
#include "ModularityVertexPartition.h"
#include "SignificanceVertexPartition.h"
#include "SurpriseVertexPartition.h"
#include "RBConfigurationVertexPartition.h"
#include "RBERVertexPartition.h"
#include "CPMVertexPartition.h"
#include "Optimiser.h"

#include <sstream>

#ifdef DEBUG
#include <iostream>
  using std::cerr;
  using std::endl;
#endif

MutableVertexPartition* create_partition(Graph* graph, char* method, vector<size_t>* initial_membership, double resolution_parameter);
MutableVertexPartition* create_partition_from_py(PyObject* py_obj_graph, char* method, PyObject* py_initial_membership, PyObject* py_weights, PyObject* py_node_sizes, double resolution_parameter);

Graph* create_graph_from_py(PyObject* py_obj_graph, PyObject* py_node_sizes);
Graph* create_graph_from_py(PyObject* py_obj_graph, PyObject* py_node_sizes, PyObject* py_weights);
Graph* create_graph_from_py(PyObject* py_obj_graph, PyObject* py_node_sizes, PyObject* py_weights, int check_positive_weight);

vector<size_t> create_size_t_vector(PyObject* py_list);

PyObject* capsule_MutableVertexPartition(MutableVertexPartition* partition);
MutableVertexPartition* decapsule_MutableVertexPartition(PyObject* py_partition);

void del_MutableVertexPartition(PyObject *self);

#ifdef __cplusplus
extern "C"
{
#endif
  PyObject* _new_ModularityVertexPartition(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _new_SignificanceVertexPartition(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _new_SurpriseVertexPartition(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _new_CPMVertexPartition(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _new_RBERVertexPartition(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _new_RBConfigurationVertexPartition(PyObject *self, PyObject *args, PyObject *keywds);

  PyObject* _MutableVertexPartition_diff_move(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _MutableVertexPartition_move_node(PyObject *self, PyObject *args, PyObject *keywds);

  PyObject* _MutableVertexPartition_aggregate_partition(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _MutableVertexPartition_get_py_igraph(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _MutableVertexPartition_from_coarse_partition(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _MutableVertexPartition_renumber_communities(PyObject *self, PyObject *args, PyObject *keywds);

  PyObject* _MutableVertexPartition_quality(PyObject *self, PyObject *args, PyObject *keywds);

  PyObject* _MutableVertexPartition_total_weight_in_comm(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _MutableVertexPartition_total_weight_from_comm(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _MutableVertexPartition_total_weight_to_comm(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _MutableVertexPartition_total_weight_in_all_comms(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _MutableVertexPartition_total_possible_edges_in_all_comms(PyObject *self, PyObject *args, PyObject *keywds);

  PyObject* _MutableVertexPartition_weight_to_comm(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _MutableVertexPartition_weight_from_comm(PyObject *self, PyObject *args, PyObject *keywds);

  PyObject* _MutableVertexPartition_get_membership(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _MutableVertexPartition_set_membership(PyObject *self, PyObject *args, PyObject *keywds);

  PyObject* _ResolutionParameterVertexPartition_get_resolution(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _ResolutionParameterVertexPartition_set_resolution(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _ResolutionParameterVertexPartition_quality(PyObject *self, PyObject *args, PyObject *keywds);

#ifdef __cplusplus
}
#endif
#endif // PYNTERFACE_PARTITION_H_INCLUDED

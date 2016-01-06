#ifndef PYNTERFACE_OPTIMISER_H_INCLUDED
#define PYNTERFACE_OPTIMISER_H_INCLUDED

#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

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
#include "python_partition_interface.h"

#ifdef DEBUG
#include <iostream>
  using std::cerr;
  using std::endl;
#endif

PyObject* capsule_Optimiser(Optimiser* optimiser);
Optimiser* decapsule_Optimiser(PyObject* py_optimiser);
void del_Optimiser(PyObject* py_optimiser);

#ifdef __cplusplus
extern "C"
{
#endif
  PyObject* _new_Optimiser(PyObject *self, PyObject *args);
  PyObject* _Optimiser_optimize_partition(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_optimize_partition_multiplex(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_move_nodes(PyObject *self, PyObject *args, PyObject *keywds);

  PyObject* _Optimiser_set_eps(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_set_delta(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_set_max_itr(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_set_random_order(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_set_consider_comms(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_set_move_individual(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_set_consider_empty_community(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_set_smart_local_move(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_set_aggregate_smart_local_move(PyObject *self, PyObject *args, PyObject *keywds);

  PyObject* _Optimiser_get_eps(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_get_delta(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_get_max_itr(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_get_random_order(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_get_consider_comms(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_get_move_individual(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_get_consider_empty_community(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_get_smart_local_move(PyObject *self, PyObject *args, PyObject *keywds);
  PyObject* _Optimiser_get_aggregate_smart_local_move(PyObject *self, PyObject *args, PyObject *keywds);

#ifdef __cplusplus
}
#endif
#endif // PYNTERFACE_OPTIMISER_H_INCLUDED

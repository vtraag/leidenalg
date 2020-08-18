#ifndef PYNTERFACE_H_INCLUDED
#define PYNTERFACE_H_INCLUDED

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
#include "python_optimiser_interface.h"

#ifdef __cplusplus
extern "C"
{
#endif

  PyObject* _set_rng_seed(PyObject *self, PyObject *args, PyObject *keywds);

  static PyMethodDef leiden_funcs[] = {

      {"_new_ModularityVertexPartition",                            (PyCFunction)_new_ModularityVertexPartition,                            METH_VARARGS | METH_KEYWORDS, ""},
      {"_new_SignificanceVertexPartition",                          (PyCFunction)_new_SignificanceVertexPartition,                          METH_VARARGS | METH_KEYWORDS, ""},
      {"_new_SurpriseVertexPartition",                              (PyCFunction)_new_SurpriseVertexPartition,                              METH_VARARGS | METH_KEYWORDS, ""},
      {"_new_CPMVertexPartition",                                   (PyCFunction)_new_CPMVertexPartition,                                   METH_VARARGS | METH_KEYWORDS, ""},
      {"_new_RBERVertexPartition",                                  (PyCFunction)_new_RBERVertexPartition,                                  METH_VARARGS | METH_KEYWORDS, ""},
      {"_new_RBConfigurationVertexPartition",                       (PyCFunction)_new_RBConfigurationVertexPartition,                       METH_VARARGS | METH_KEYWORDS, ""},

      {"_MutableVertexPartition_diff_move",                         (PyCFunction)_MutableVertexPartition_diff_move,                         METH_VARARGS | METH_KEYWORDS, ""},
      {"_MutableVertexPartition_move_node",                         (PyCFunction)_MutableVertexPartition_move_node,                         METH_VARARGS | METH_KEYWORDS, ""},
      {"_MutableVertexPartition_get_py_igraph",                     (PyCFunction)_MutableVertexPartition_get_py_igraph,                     METH_VARARGS | METH_KEYWORDS, ""},
      {"_MutableVertexPartition_aggregate_partition",               (PyCFunction)_MutableVertexPartition_aggregate_partition,               METH_VARARGS | METH_KEYWORDS, ""},
      {"_MutableVertexPartition_from_coarse_partition",             (PyCFunction)_MutableVertexPartition_from_coarse_partition,             METH_VARARGS | METH_KEYWORDS, ""},
      {"_MutableVertexPartition_renumber_communities",              (PyCFunction)_MutableVertexPartition_renumber_communities,              METH_VARARGS | METH_KEYWORDS, ""},

      {"_MutableVertexPartition_quality",                           (PyCFunction)_MutableVertexPartition_quality,                           METH_VARARGS | METH_KEYWORDS, ""},
      {"_MutableVertexPartition_total_weight_in_comm",              (PyCFunction)_MutableVertexPartition_total_weight_in_comm,              METH_VARARGS | METH_KEYWORDS, ""},
      {"_MutableVertexPartition_total_weight_from_comm",            (PyCFunction)_MutableVertexPartition_total_weight_from_comm,            METH_VARARGS | METH_KEYWORDS, ""},
      {"_MutableVertexPartition_total_weight_to_comm",              (PyCFunction)_MutableVertexPartition_total_weight_to_comm,              METH_VARARGS | METH_KEYWORDS, ""},
      {"_MutableVertexPartition_total_weight_in_all_comms",         (PyCFunction)_MutableVertexPartition_total_weight_in_all_comms,         METH_VARARGS | METH_KEYWORDS, ""},
      {"_MutableVertexPartition_total_possible_edges_in_all_comms", (PyCFunction)_MutableVertexPartition_total_possible_edges_in_all_comms, METH_VARARGS | METH_KEYWORDS, ""},
      {"_MutableVertexPartition_weight_to_comm",                    (PyCFunction)_MutableVertexPartition_weight_to_comm,                    METH_VARARGS | METH_KEYWORDS, ""},
      {"_MutableVertexPartition_weight_from_comm",                  (PyCFunction)_MutableVertexPartition_weight_from_comm,                  METH_VARARGS | METH_KEYWORDS, ""},
      {"_MutableVertexPartition_get_membership",                    (PyCFunction)_MutableVertexPartition_get_membership,                    METH_VARARGS | METH_KEYWORDS, ""},
      {"_MutableVertexPartition_set_membership",                    (PyCFunction)_MutableVertexPartition_set_membership,                    METH_VARARGS | METH_KEYWORDS, ""},
      {"_ResolutionParameterVertexPartition_get_resolution",        (PyCFunction)_ResolutionParameterVertexPartition_get_resolution,        METH_VARARGS | METH_KEYWORDS, ""},
      {"_ResolutionParameterVertexPartition_set_resolution",        (PyCFunction)_ResolutionParameterVertexPartition_set_resolution,        METH_VARARGS | METH_KEYWORDS, ""},
      {"_ResolutionParameterVertexPartition_quality",               (PyCFunction)_ResolutionParameterVertexPartition_quality,               METH_VARARGS | METH_KEYWORDS, ""},


      {"_new_Optimiser",                            (PyCFunction)_new_Optimiser,                            METH_NOARGS,                  ""},
      {"_Optimiser_optimise_partition",             (PyCFunction)_Optimiser_optimise_partition,             METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_optimise_partition_multiplex",   (PyCFunction)_Optimiser_optimise_partition_multiplex,   METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_move_nodes",                     (PyCFunction)_Optimiser_move_nodes,                     METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_move_nodes_constrained",         (PyCFunction)_Optimiser_move_nodes_constrained,         METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_merge_nodes",                    (PyCFunction)_Optimiser_merge_nodes,                    METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_merge_nodes_constrained",        (PyCFunction)_Optimiser_merge_nodes_constrained,        METH_VARARGS | METH_KEYWORDS, ""},

      {"_Optimiser_set_consider_comms",             (PyCFunction)_Optimiser_set_consider_comms,             METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_set_refine_consider_comms",      (PyCFunction)_Optimiser_set_refine_consider_comms,      METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_set_optimise_routine",           (PyCFunction)_Optimiser_set_optimise_routine,           METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_set_refine_routine",             (PyCFunction)_Optimiser_set_refine_routine,             METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_set_consider_empty_community",   (PyCFunction)_Optimiser_set_consider_empty_community,   METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_set_refine_partition",           (PyCFunction)_Optimiser_set_refine_partition,           METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_set_max_comm_size",              (PyCFunction)_Optimiser_set_max_comm_size,              METH_VARARGS | METH_KEYWORDS, ""},

      {"_Optimiser_get_consider_comms",             (PyCFunction)_Optimiser_get_consider_comms,             METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_get_refine_consider_comms",      (PyCFunction)_Optimiser_get_refine_consider_comms,      METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_get_optimise_routine",           (PyCFunction)_Optimiser_get_optimise_routine,           METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_get_refine_routine",             (PyCFunction)_Optimiser_get_refine_routine,             METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_get_consider_empty_community",   (PyCFunction)_Optimiser_get_consider_empty_community,   METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_get_refine_partition",           (PyCFunction)_Optimiser_get_refine_partition,           METH_VARARGS | METH_KEYWORDS, ""},
      {"_Optimiser_get_max_comm_size",              (PyCFunction)_Optimiser_get_max_comm_size,              METH_VARARGS | METH_KEYWORDS, ""},

      {"_Optimiser_set_rng_seed",                   (PyCFunction)_Optimiser_set_rng_seed,                   METH_VARARGS | METH_KEYWORDS, ""},

      {NULL}
  };

  struct module_state {
      PyObject *error;
  };

  #if PY_MAJOR_VERSION >= 3
  #define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
  #else
  #define GETSTATE(m) (&_state)
  static struct module_state _state;
  #endif

  #if PY_MAJOR_VERSION >= 3

  static int leiden_traverse(PyObject *m, visitproc visit, void *arg) {
      Py_VISIT(GETSTATE(m)->error);
      return 0;
  }

  static int leiden_clear(PyObject *m) {
      Py_CLEAR(GETSTATE(m)->error);
      return 0;
  }

  static struct PyModuleDef leidendef = {
          PyModuleDef_HEAD_INIT,
          "_c_leiden",
          NULL,
          sizeof(struct module_state),
          leiden_funcs,
          NULL,
          leiden_traverse,
          leiden_clear,
          NULL
  };

  #define INITERROR return NULL

  PyObject *
  PyInit__c_leiden(void)

  #else
  #define INITERROR return

  void
  init_c_leiden(void)
  #endif
  {
  #if PY_MAJOR_VERSION >= 3
      PyObject* module = PyModule_Create(&leidendef);
  #else
      PyObject *module = Py_InitModule3("_c_leiden", leiden_funcs, "Leiden extension using igraph.");
  #endif

      PyModule_AddIntConstant(module, "ALL_COMMS", Optimiser::ALL_COMMS);
      PyModule_AddIntConstant(module, "ALL_NEIGH_COMMS", Optimiser::ALL_NEIGH_COMMS);
      PyModule_AddIntConstant(module, "RAND_COMM", Optimiser::RAND_COMM);
      PyModule_AddIntConstant(module, "RAND_NEIGH_COMM", Optimiser::RAND_NEIGH_COMM);

      PyModule_AddIntConstant(module, "MOVE_NODES", Optimiser::MOVE_NODES);
      PyModule_AddIntConstant(module, "MERGE_NODES", Optimiser::MERGE_NODES);

      if (module == NULL)
          INITERROR;
      struct module_state *st = GETSTATE(module);

      st->error = PyErr_NewException("leidenalg.Error", NULL, NULL);
      if (st->error == NULL) {
          Py_DECREF(module);
          INITERROR;
      }

  #if PY_MAJOR_VERSION >= 3
      return module;
  #endif
  }

#ifdef __cplusplus
}
#endif
#endif // PYNTERFACE_H_INCLUDED

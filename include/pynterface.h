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

#ifdef __cplusplus
extern "C"
{
#endif
  static PyObject* _find_partition_multiplex(PyObject *self, PyObject *args, PyObject *keywds);
  static PyObject* _find_partition(PyObject *self, PyObject *args, PyObject *keywds);
  static PyObject* _quality(PyObject *self, PyObject *args, PyObject *keywds);

  static MutableVertexPartition* create_partition(Graph* graph, char* method, vector<size_t>* initial_membership, double resolution_parameter);
  static MutableVertexPartition* create_partition_from_py(PyObject* py_obj_graph, char* method, PyObject* py_initial_membership, PyObject* py_weights, double resolution_parameter);

  static char find_partition_multiplex_docs[] =
      "find_partition_multiplex( ): Finds an optimal partition for several graphs and methods at the same time.\n";
  static char find_partition_docs[] =
      "_find_partition( ): Find a the optimal partition using the louvain algorithm and the specified method for the supplied graph.\n";
  static char quality_docs[] =
      "_quality( ): Calculate the quality of the supplied partition using the indicated method.\n";

  static PyMethodDef louvain_funcs[] = {
      {"_find_partition_multiplex", (PyCFunction)_find_partition_multiplex,  METH_VARARGS | METH_KEYWORDS, find_partition_multiplex_docs},
      {"_find_partition", (PyCFunction)_find_partition,  METH_VARARGS | METH_KEYWORDS, find_partition_docs},
      {"_quality", (PyCFunction)_quality,  METH_VARARGS | METH_KEYWORDS, quality_docs},
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

  static int louvain_traverse(PyObject *m, visitproc visit, void *arg) {
      Py_VISIT(GETSTATE(m)->error);
      return 0;
  }

  static int louvain_clear(PyObject *m) {
      Py_CLEAR(GETSTATE(m)->error);
      return 0;
  }

  static struct PyModuleDef louvaindef = {
          PyModuleDef_HEAD_INIT,
          "_c_louvain",
          NULL,
          sizeof(struct module_state),
          louvain_funcs,
          NULL,
          louvain_traverse,
          louvain_clear,
          NULL
  };

  #define INITERROR return NULL

  PyObject *
  PyInit__c_louvain(void)

  #else
  #define INITERROR return

  void
  init_c_louvain(void)
  #endif
  {
  #if PY_MAJOR_VERSION >= 3
      PyObject* module = PyModule_Create(&louvaindef);
  #else
      PyObject *module = Py_InitModule3("_c_louvain", louvain_funcs, "Louvain extension using igraph.");
  #endif

      PyModule_AddIntConstant(module, "ALL_COMMS", Optimiser::ALL_COMMS);
      PyModule_AddIntConstant(module, "ALL_NEIGH_COMMS", Optimiser::ALL_NEIGH_COMMS);
      PyModule_AddIntConstant(module, "RAND_COMM", Optimiser::RAND_COMM);
      PyModule_AddIntConstant(module, "RAND_NEIGH_COMM", Optimiser::RAND_NEIGH_COMM);

      if (module == NULL)
          INITERROR;
      struct module_state *st = GETSTATE(module);

      st->error = PyErr_NewException("louvain.Error", NULL, NULL);
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

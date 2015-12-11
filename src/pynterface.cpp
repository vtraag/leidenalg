#include "pynterface.h"

PyObject* _find_partition_multiplex(PyObject *self, PyObject *args, PyObject *keywds)
{
  PyObject* py_layers = NULL;
  int consider_comms = Optimiser::ALL_NEIGH_COMMS;

  if (!PyArg_ParseTuple(args, "Oi", &py_layers, &consider_comms))
      return NULL;

  size_t nb_layers = PyList_Size(py_layers);

  #ifdef DEBUG
    cerr << "Parsing " << nb_layers << " layers." << endl;
  #endif

  #ifdef DEBUG
    cerr << "Consider comms " << consider_comms << " ." << endl;
  #endif

  // This is all done per layer.
  PyObject* py_obj_graph = NULL;
  char* method = "Modularity";
  double layer_weight = 1.0;
  PyObject* py_initial_membership = NULL;
  PyObject* py_weights = NULL;
  double resolution_parameter = 1.0;

  vector<MutableVertexPartition*> partitions(nb_layers, NULL);
  vector<double> layer_weights(nb_layers, 1.0);
  MutableVertexPartition* partition;
  for (size_t layer = 0; layer < nb_layers; layer++)
  {
    PyObject* layer_args = PyList_GetItem(py_layers, layer);
    #ifdef DEBUG
      cerr << "Got layer " << layer << " arguments." << endl;
    #endif
    PyArg_ParseTuple(layer_args, "OsdOOd", &py_obj_graph, &method, &layer_weight, &py_initial_membership, &py_weights, &resolution_parameter);
    #ifdef DEBUG
      cerr << "Got parameters: py_obj_graph=" << py_obj_graph
           << ", method=" << method
           << ", layer_weight=" << layer_weight
           << ", py_initial_membership=" << py_initial_membership
           << ", py_weights=" << py_weights
           << ", resolution_parameter=" << resolution_parameter
           << endl;
    #endif
    partition = create_partition_from_py(py_obj_graph, method, py_initial_membership, py_weights, resolution_parameter);
    partitions[layer] = partition;
    layer_weights[layer] = layer_weight;
  }

  size_t n;

  n = partitions[0]->get_graph()->vcount();

  for (size_t layer = 0; layer < nb_layers; layer++)
  {
    if (n != partition->get_graph()->vcount())
    {
      // First clean up before returning error.
      for (size_t layer; layer < nb_layers; layer++)
      {
        delete partitions[layer];
      }

      PyErr_SetString(PyExc_ValueError, "Inconsistent number of nodes.");
      return NULL;
    }
  }

  Optimiser opt;
  opt.consider_comms = consider_comms;

  double q = opt.optimize_partition(partitions, layer_weights);

  // The membership should be the same in all partitions
  // so simply take the first one.
  partition = partitions[0];
  PyObject* membership = PyList_New(n);
  for (size_t v = 0; v < n; v++)
  {
    #if PY_MAJOR_VERSION >= 3
      PyObject* item = PyLong_FromSize_t(partition->membership(v));
    #else
      PyObject* item = PyInt_FromSize_t(partition->membership(v));
    #endif
    PyList_SetItem(membership, v, item);
  }

  for (size_t layer; layer < nb_layers; layer++)
  {
    delete partitions[layer];
  }

  return membership;
}

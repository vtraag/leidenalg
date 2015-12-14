#include "python_optimiser_interface.h"

  PyObject* capsule_Optimiser(Optimiser* optimiser)
  {
    PyObject* py_optimiser = PyCapsule_New(optimiser, "louvain.Optimiser", del_Optimiser);
    return py_optimiser;
  }

  Optimiser* decapsule_Optimiser(PyObject* py_optimiser)
  {
    Optimiser* optimiser = (Optimiser*) PyCapsule_GetPointer(py_optimiser, "louvain.Optimiser");
    return optimiser;
  }

  void del_Optimiser(PyObject* py_optimiser)
  {
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    delete optimiser;
  }
#ifdef __cplusplus
extern "C"
{
#endif

  PyObject* _new_Optimiser(PyObject *self, PyObject *args, PyObject *keywds)
  {
    double eps = 1e-5;
    double delta = 1e-2;
    size_t max_itr = 10000;
    int random_order = true;
    int consider_comms = Optimiser::ALL_NEIGH_COMMS;

    static char* kwlist[] = {"eps", "delta", "max_itr", "random_order", "consider_comms", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "|ddKii", kwlist,
                                     &eps, &delta, &max_itr, &random_order, &consider_comms))
        return NULL;

    Optimiser* optimiser = new Optimiser(eps, delta, max_itr, random_order, consider_comms);
    PyObject* py_optimiser = capsule_Optimiser(optimiser);
    return py_optimiser;
  }

  PyObject* _Optimiser_optimize_partition(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    PyObject* py_partition = NULL;

    static char* kwlist[] = {"optimiser", "partition", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OO", kwlist,
                                     &py_optimiser, &py_partition))
        return NULL;

    #ifdef DEBUG
      cerr << "optimize_partition(" << py_partition << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif
    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);
    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    double q = optimiser->optimize_partition(partition);
    return PyFloat_FromDouble(q);
  }

  PyObject* _Optimiser_optimize_partition_multiplex(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    PyObject* py_partitions = NULL;
    PyObject* py_layer_weights = NULL;

    if (!PyArg_ParseTuple(args, "OOO", &py_optimiser, &py_partitions, &py_layer_weights))
        return NULL;

    size_t nb_partitions = PyList_Size(py_partitions);
    if (nb_partitions != PyList_Size(py_layer_weights))
    {
      PyErr_SetString(PyExc_ValueError, "Number of weights does not equal the number of partitions");
      return NULL;
    }

    #ifdef DEBUG
      cerr << "Parsing " << nb_partitions << " partitions." << endl;
    #endif

    // This is all done per layer.

    vector<MutableVertexPartition*> partitions(nb_partitions, NULL);
    vector<double> layer_weights(nb_partitions, 1.0);
    MutableVertexPartition* partition;
    for (size_t layer = 0; layer < nb_partitions; layer++)
    {
      PyObject* py_partition = PyList_GetItem(py_partitions, layer);
      #ifdef DEBUG
        cerr << "Capsule partition at address " << py_partition << endl;
      #endif
      MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);
      #ifdef DEBUG
        cerr << "Using partition at address " << partition << endl;
      #endif

      PyObject* layer_weight = PyList_GetItem(py_layer_weights, layer);
      #ifdef DEBUG
        cerr << "Layer weight " << layer_weight << endl;
      #endif

      partitions[layer] = partition;
      layer_weights[layer] = PyFloat_AsDouble(layer_weight);
    }

    size_t n;

    #ifdef DEBUG
      cerr << "Getting node count" << endl;
    #endif
    n = partitions[0]->get_graph()->vcount();
    #ifdef DEBUG
      cerr << "n=" << n << endl;
    #endif

    for (size_t layer = 0; layer < nb_partitions; layer++)
    {
      if (n != partitions[layer]->get_graph()->vcount())
      {
        PyErr_SetString(PyExc_ValueError, "Inconsistent number of nodes.");
        return NULL;
      }
    }

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    double q = optimiser->optimize_partition(partitions, layer_weights);
    return PyFloat_FromDouble(q);
  }

  PyObject* _Optimiser_move_nodes(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    PyObject* py_partition = NULL;
    int consider_comms = -1;

    static char* kwlist[] = {"optimiser", "partition", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OO|i", kwlist,
                                     &py_optimiser, &py_partition, &consider_comms))
        return NULL;

    #ifdef DEBUG
      cerr << "optimize_partition(" << py_partition << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif
    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);
    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    if (consider_comms < 0)
      consider_comms = optimiser->consider_comms;

    double q = optimiser->move_nodes(partition, consider_comms);
    return PyFloat_FromDouble(q);
  }

  PyObject* _Optimiser_set_eps(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    double eps = 1e-5;
    static char* kwlist[] = {"optimiser", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "Od", kwlist,
                                     &py_optimiser, &eps))
        return NULL;

    #ifdef DEBUG
      cerr << "set_eps(" << eps << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    optimiser->eps = eps;

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* _Optimiser_set_delta(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    double delta = 1e-2;
    static char* kwlist[] = {"optimiser", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "Od", kwlist,
                                     &py_optimiser, &delta))
        return NULL;

    #ifdef DEBUG
      cerr << "set_delta(" << delta << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    optimiser->delta = delta;

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* _Optimiser_set_max_itr(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    size_t max_itr = 10000;
    static char* kwlist[] = {"optimiser", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OK", kwlist,
                                     &py_optimiser, &max_itr))
        return NULL;

    #ifdef DEBUG
      cerr << "set_max_itr(" << max_itr << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    optimiser->max_itr = max_itr;

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* _Optimiser_set_random_order(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    int random_order = true;
    static char* kwlist[] = {"optimiser", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "Oi", kwlist,
                                     &py_optimiser, &random_order))
        return NULL;

    #ifdef DEBUG
      cerr << "set_random_order(" << random_order << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    optimiser->random_order = random_order;

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* _Optimiser_set_consider_comms(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    int consider_comms = Optimiser::ALL_NEIGH_COMMS;
    static char* kwlist[] = {"optimiser", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "Od", kwlist,
                                     &py_optimiser, &consider_comms))
        return NULL;

    #ifdef DEBUG
      cerr << "set_consider_comms(" << consider_comms << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    optimiser->consider_comms = consider_comms;

    Py_INCREF(Py_None);
    return Py_None;
  }

#ifdef __cplusplus
}
#endif
#undef DEBUG

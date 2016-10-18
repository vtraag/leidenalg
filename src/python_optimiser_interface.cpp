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

  PyObject* _new_Optimiser(PyObject *self, PyObject *args)
  {
    if (args != NULL)
    {
      PyErr_BadArgument();
      return NULL;
    }

    Optimiser* optimiser = new Optimiser();
    PyObject* py_optimiser = capsule_Optimiser(optimiser);
    return py_optimiser;
  }

  PyObject* _Optimiser_optimise_partition(PyObject *self, PyObject *args, PyObject *keywds)
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
      cerr << "optimise_partition(" << py_partition << ");" << endl;
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

    double q = optimiser->optimise_partition(partition);
    return PyFloat_FromDouble(q);
  }

  PyObject* _Optimiser_optimise_partition_multiplex(PyObject *self, PyObject *args, PyObject *keywds)
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
        cerr << "Layer weight " << PyFloat_AsDouble(layer_weight) << endl;
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

    double q = optimiser->optimise_partition(partitions, layer_weights);
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
      cerr << "optimise_partition(" << py_partition << ");" << endl;
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

  PyObject* _Optimiser_move_nodes_constrained(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    PyObject* py_partition = NULL;
    PyObject* py_constrained_partition = NULL;
    int consider_comms = -1;

    static char* kwlist[] = {"optimiser", "partition", "constrained_partition", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OOO", kwlist,
                                     &py_optimiser, &py_partition, &py_constrained_partition))
        return NULL;

    #ifdef DEBUG
      cerr << "optimise_partition(" << py_partition << ");" << endl;
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

    #ifdef DEBUG
      cerr << "Capsule constrained partition at address " << py_partition << endl;
    #endif
    MutableVertexPartition* constrained_partition = decapsule_MutableVertexPartition(py_constrained_partition);
    #ifdef DEBUG
      cerr << "Using constrained partition at address " << partition << endl;
    #endif

    double q = optimiser->move_nodes_constrained(partition, constrained_partition);
    return PyFloat_FromDouble(q);
  }

  PyObject* _Optimiser_set_eps(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    double eps = 1e-5;
    static char* kwlist[] = {"optimiser", "eps", NULL};

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

  PyObject* _Optimiser_get_eps(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    static char* kwlist[] = {"optimiser", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "O", kwlist,
                                     &py_optimiser))
        return NULL;

    #ifdef DEBUG
      cerr << "get_eps();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    return PyFloat_FromDouble(optimiser->eps);
  }

  PyObject* _Optimiser_set_delta(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    double delta = 1e-2;
    static char* kwlist[] = {"optimiser", "delta", NULL};

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

  PyObject* _Optimiser_get_delta(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    static char* kwlist[] = {"optimiser", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "O", kwlist,
                                     &py_optimiser))
        return NULL;

    #ifdef DEBUG
      cerr << "get_delta();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    return PyFloat_FromDouble(optimiser->delta);
  }

  PyObject* _Optimiser_set_max_itr(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    size_t max_itr = 10000;
    static char* kwlist[] = {"optimiser", "max_itr", NULL};

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

  PyObject* _Optimiser_get_max_itr(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    static char* kwlist[] = {"optimiser", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "O", kwlist,
                                     &py_optimiser))
        return NULL;

    #ifdef DEBUG
      cerr << "get_max_itr();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    return PyInt_FromSize_t(optimiser->max_itr);
  }

  PyObject* _Optimiser_set_random_order(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    int random_order = true;
    static char* kwlist[] = {"optimiser", "random_order", NULL};

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

  PyObject* _Optimiser_get_random_order(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    static char* kwlist[] = {"optimiser", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "O", kwlist,
                                     &py_optimiser))
        return NULL;

    #ifdef DEBUG
      cerr << "get_random_order();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    return PyBool_FromLong(optimiser->random_order);
  }

  PyObject* _Optimiser_set_consider_comms(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    int consider_comms = Optimiser::ALL_NEIGH_COMMS;
    static char* kwlist[] = {"optimiser", "consider_comms", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "Oi", kwlist,
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

  PyObject* _Optimiser_get_consider_comms(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    static char* kwlist[] = {"optimiser", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "O", kwlist,
                                     &py_optimiser))
        return NULL;

    #ifdef DEBUG
      cerr << "get_consider_comms();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    return PyInt_FromLong(optimiser->consider_comms);
  }

  PyObject* _Optimiser_set_consider_empty_community(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    int consider_empty_community = false;
    static char* kwlist[] = {"optimiser", "consider_empty_community", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "Oi", kwlist,
                                     &py_optimiser, &consider_empty_community))
        return NULL;

    #ifdef DEBUG
      cerr << "set_consider_empty_community(" << consider_empty_community << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    optimiser->consider_empty_community = consider_empty_community;

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* _Optimiser_get_consider_empty_community(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    static char* kwlist[] = {"optimiser", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "O", kwlist,
                                     &py_optimiser))
        return NULL;

    #ifdef DEBUG
      cerr << "get_consider_empty_community();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    return PyBool_FromLong(optimiser->consider_empty_community);
  }

  PyObject* _Optimiser_set_smart_local_move(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    int smart_local_move = false;
    static char* kwlist[] = {"optimiser", "smart_local_move", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "Oi", kwlist,
                                     &py_optimiser, &smart_local_move))
        return NULL;

    #ifdef DEBUG
      cerr << "set_smart_local_move(" << smart_local_move << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    optimiser->refine_partition = smart_local_move;

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* _Optimiser_get_smart_local_move(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    static char* kwlist[] = {"optimiser", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "O", kwlist,
                                     &py_optimiser))
        return NULL;

    #ifdef DEBUG
      cerr << "get_smart_local_move();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    return PyBool_FromLong(optimiser->refine_partition);
  }
#ifdef __cplusplus
}
#endif

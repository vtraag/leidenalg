#include "python_optimiser_interface.h"

  PyObject* capsule_Optimiser(Optimiser* optimiser)
  {
    PyObject* py_optimiser = PyCapsule_New(optimiser, "leidenalg.Optimiser", del_Optimiser);
    return py_optimiser;
  }

  Optimiser* decapsule_Optimiser(PyObject* py_optimiser)
  {
    Optimiser* optimiser = (Optimiser*) PyCapsule_GetPointer(py_optimiser, "leidenalg.Optimiser");
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
    PyObject* py_is_membership_fixed = NULL;

    static char* kwlist[] = {"optimiser", "partition", "is_membership_fixed", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OO|O", kwlist,
                                     &py_optimiser, &py_partition,
                                     &py_is_membership_fixed))
        return NULL;

    #ifdef DEBUG
      cerr << "optimise_partition(" << py_partition << ", is_membership_fixed=" << py_is_membership_fixed << ");" << endl;
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

    size_t n = partition->get_graph()->vcount();
    vector<bool> is_membership_fixed(n, false);
    if (py_is_membership_fixed != NULL && py_is_membership_fixed != Py_None)
    {
      #ifdef DEBUG
        cerr << "Reading is_membership_fixed." << endl;
      #endif

      size_t nb_is_membership_fixed = PyList_Size(py_is_membership_fixed);
      if (nb_is_membership_fixed != n)
      {
        throw Exception("Node size vector not the same size as the number of nodes.");
      }

      for (size_t v = 0; v < n; v++)
      {
        PyObject* py_item = PyList_GetItem(py_is_membership_fixed, v);
        is_membership_fixed[v] = PyObject_IsTrue(py_item);
      }
    }

    double q = 0.0;
    try
    {
      q = optimiser->optimise_partition(partition, is_membership_fixed);
    }
    catch (std::exception e)
    {
      PyErr_SetString(PyExc_ValueError, e.what());
      return NULL;
    }
    return PyFloat_FromDouble(q);
  }

  PyObject* _Optimiser_optimise_partition_multiplex(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    PyObject* py_partitions = NULL;
    PyObject* py_layer_weights = NULL;
    PyObject* py_is_membership_fixed = NULL;

    static char* kwlist[] = {"optimiser", "partitions", "layer_weights", "is_membership_fixed", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OOO|O", kwlist,
                                     &py_optimiser, &py_partitions,
                                     &py_layer_weights, &py_is_membership_fixed))
        return NULL;

    size_t nb_partitions = PyList_Size(py_partitions);
    if (nb_partitions != PyList_Size(py_layer_weights))
    {
      PyErr_SetString(PyExc_ValueError, "Number of layer weights does not equal the number of partitions");
      return NULL;
    }

    #ifdef DEBUG
      cerr << "Parsing " << nb_partitions << " partitions." << endl;
    #endif

    // This is all done per layer.

    vector<MutableVertexPartition*> partitions(nb_partitions);
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

      partitions[layer] = partition;


      if (PyNumber_Check(layer_weight))
      {
        layer_weights[layer] = PyFloat_AsDouble(layer_weight);
      }
      else
      {
        PyErr_SetString(PyExc_TypeError, "Expected floating value for layer weight.");
        return NULL;
      }

      if (isnan(layer_weights[layer]))
        throw Exception("Cannot accept NaN weights.");
    }

    if (nb_partitions == 0)
      return NULL;

    size_t n = partitions[0]->get_graph()->vcount();
    vector<bool> is_membership_fixed(n, false);
    if (py_is_membership_fixed != NULL && py_is_membership_fixed != Py_None)
    {
      #ifdef DEBUG
        cerr << "Reading is_membership_fixed." << endl;
      #endif

      size_t nb_is_membership_fixed = PyList_Size(py_is_membership_fixed);
      if (nb_is_membership_fixed != n)
      {
        throw Exception("Node size vector not the same size as the number of nodes.");
      }

      for (size_t v = 0; v < n; v++)
      {
        PyObject* py_item = PyList_GetItem(py_is_membership_fixed, v);
        is_membership_fixed[v] = PyObject_IsTrue(py_item);
      }
    }


    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    double q = 0.0;
    try
    {
      q = optimiser->optimise_partition(partitions, layer_weights, is_membership_fixed);
    }
    catch (std::exception e)
    {
      PyErr_SetString(PyExc_ValueError, e.what());
      return NULL;
    }
    return PyFloat_FromDouble(q);
  }

  PyObject* _Optimiser_move_nodes(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    PyObject* py_partition = NULL;
    PyObject* py_is_membership_fixed = NULL;
    int consider_comms = -1;

    static char* kwlist[] = {"optimiser", "partition", "is_membership_fixed", "consider_comms", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OO|Oi", kwlist,
                                     &py_optimiser, &py_partition,
                                     &py_is_membership_fixed, &consider_comms))
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

    size_t n = partition->get_graph()->vcount();
    vector<bool> is_membership_fixed(n, false);
    if (py_is_membership_fixed != NULL && py_is_membership_fixed != Py_None)
    {
      #ifdef DEBUG
        cerr << "Reading is_membership_fixed." << endl;
      #endif

      size_t nb_is_membership_fixed = PyList_Size(py_is_membership_fixed);
      if (nb_is_membership_fixed != n)
      {
        throw Exception("Node size vector not the same size as the number of nodes.");
      }

      for (size_t v = 0; v < n; v++)
      {
        PyObject* py_item = PyList_GetItem(py_is_membership_fixed, v);
        is_membership_fixed[v] = PyObject_IsTrue(py_item);
      }
    }

    if (consider_comms < 0)
      consider_comms = optimiser->consider_comms;

    double q  = 0.0;
    try
    {
      q = optimiser->move_nodes(partition, is_membership_fixed, consider_comms, true);
    }
    catch (std::exception e)
    {
      PyErr_SetString(PyExc_ValueError, e.what());
      return NULL;
    }
    return PyFloat_FromDouble(q);
  }

  PyObject* _Optimiser_merge_nodes(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    PyObject* py_partition = NULL;
    PyObject* py_is_membership_fixed = NULL;
    int consider_comms = -1;

    static char* kwlist[] = {"optimiser", "partition", "is_membership_fixed", "consider_comms", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OO|Oi", kwlist,
                                     &py_optimiser, &py_partition,
                                     &py_is_membership_fixed, &consider_comms))
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

    size_t n = partition->get_graph()->vcount();
    vector<bool> is_membership_fixed(n, false);
    if (py_is_membership_fixed != NULL && py_is_membership_fixed != Py_None)
    {
      #ifdef DEBUG
        cerr << "Reading is_membership_fixed." << endl;
      #endif

      size_t nb_is_membership_fixed = PyList_Size(py_is_membership_fixed);
      if (nb_is_membership_fixed != n)
      {
        throw Exception("Node size vector not the same size as the number of nodes.");
      }

      for (size_t v = 0; v < n; v++)
      {
        PyObject* py_item = PyList_GetItem(py_is_membership_fixed, v);
        is_membership_fixed[v] = PyObject_IsTrue(py_item);
      }
    }

    if (consider_comms < 0)
      consider_comms = optimiser->consider_comms;

    double q = 0.0;
    try
    {
      q = optimiser->merge_nodes(partition, is_membership_fixed, consider_comms, true);
    }
    catch (std::exception e)
    {
      PyErr_SetString(PyExc_ValueError, e.what());
      return NULL;
    }
    return PyFloat_FromDouble(q);
  }

  PyObject* _Optimiser_move_nodes_constrained(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    PyObject* py_partition = NULL;
    PyObject* py_constrained_partition = NULL;
    int consider_comms = -1;

    static char* kwlist[] = {"optimiser", "partition", "constrained_partition", "consider_comms", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OOO|i", kwlist,
                                     &py_optimiser, &py_partition, &py_constrained_partition, &consider_comms))
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

    if (consider_comms < 0)
      consider_comms = optimiser->refine_consider_comms;

    double q = 0.0;
    try
    {
      q = optimiser->move_nodes_constrained(partition, consider_comms, constrained_partition);
    }
    catch (std::exception e)
    {
      PyErr_SetString(PyExc_ValueError, e.what());
      return NULL;
    }
    return PyFloat_FromDouble(q);
  }

  PyObject* _Optimiser_merge_nodes_constrained(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    PyObject* py_partition = NULL;
    PyObject* py_constrained_partition = NULL;
    int consider_comms = -1;

    static char* kwlist[] = {"optimiser", "partition", "constrained_partition", "consider_comms", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OOO|i", kwlist,
                                     &py_optimiser, &py_partition, &py_constrained_partition, &consider_comms))
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

    if (consider_comms < 0)
      consider_comms = optimiser->refine_consider_comms;

    double q = 0.0;
    try
    {
      q = optimiser->merge_nodes_constrained(partition, consider_comms, constrained_partition);
    }
    catch (std::exception e)
    {
      PyErr_SetString(PyExc_ValueError, e.what());
      return NULL;
    }
    return PyFloat_FromDouble(q);
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

    #ifdef IS_PY3K
    return PyLong_FromLong(optimiser->consider_comms);
    #else
    return PyInt_FromLong(optimiser->consider_comms);
    #endif
  }

  PyObject* _Optimiser_set_refine_consider_comms(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    int refine_consider_comms = Optimiser::ALL_NEIGH_COMMS;
    static char* kwlist[] = {"optimiser", "refine_consider_comms", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "Oi", kwlist,
                                     &py_optimiser, &refine_consider_comms))
        return NULL;

    #ifdef DEBUG
      cerr << "set_refine_consider_comms(" << refine_consider_comms << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    optimiser->refine_consider_comms = refine_consider_comms;

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* _Optimiser_get_refine_consider_comms(PyObject *self, PyObject *args, PyObject *keywds)
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
      cerr << "get_refine_consider_comms();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    #ifdef IS_PY3K
    return PyLong_FromLong(optimiser->refine_consider_comms);
    #else
    return PyInt_FromLong(optimiser->refine_consider_comms);
    #endif
  }

  PyObject* _Optimiser_set_optimise_routine(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    int optimise_routine = Optimiser::ALL_NEIGH_COMMS;
    static char* kwlist[] = {"optimiser", "optimise_routine", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "Oi", kwlist,
                                     &py_optimiser, &optimise_routine))
        return NULL;

    #ifdef DEBUG
      cerr << "set_optimise_routine(" << optimise_routine << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    optimiser->optimise_routine = optimise_routine;

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* _Optimiser_get_optimise_routine(PyObject *self, PyObject *args, PyObject *keywds)
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
      cerr << "get_optimise_routine();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    #ifdef IS_PY3K
    return PyLong_FromLong(optimiser->optimise_routine);
    #else
    return PyInt_FromLong(optimiser->optimise_routine);
    #endif
  }

  PyObject* _Optimiser_set_refine_routine(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    int refine_routine = Optimiser::ALL_NEIGH_COMMS;
    static char* kwlist[] = {"optimiser", "refine_routine", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "Oi", kwlist,
                                     &py_optimiser, &refine_routine))
        return NULL;

    #ifdef DEBUG
      cerr << "set_refine_routine(" << refine_routine << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    optimiser->refine_routine = refine_routine;

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* _Optimiser_get_refine_routine(PyObject *self, PyObject *args, PyObject *keywds)
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
      cerr << "get_refine_routine();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    #ifdef IS_PY3K
    return PyLong_FromLong(optimiser->refine_routine);
    #else
    return PyInt_FromLong(optimiser->refine_routine);
    #endif
  }

  PyObject* _Optimiser_set_consider_empty_community(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    int consider_empty_community = true;
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

    #ifdef DEBUG
      cerr << "Setting consider_empty_community to " << consider_empty_community << endl;
    #endif
    optimiser->consider_empty_community = consider_empty_community;
    #ifdef DEBUG
      cerr << "Set consider_empty_community to " << optimiser->consider_empty_community << endl;
    #endif

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
      cerr << "Returning " << optimiser->consider_empty_community << endl;
    #endif

    return PyBool_FromLong(optimiser->consider_empty_community);
  }

  PyObject* _Optimiser_set_refine_partition(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    int refine_partition = false;
    static char* kwlist[] = {"optimiser", "refine_partition", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "Oi", kwlist,
                                     &py_optimiser, &refine_partition))
        return NULL;

    #ifdef DEBUG
      cerr << "set_refine_partition(" << refine_partition << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    optimiser->refine_partition = refine_partition;

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* _Optimiser_get_refine_partition(PyObject *self, PyObject *args, PyObject *keywds)
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
      cerr << "get_refine_partition();" << endl;
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

  PyObject* _Optimiser_set_max_comm_size(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    size_t max_comm_size = 0;
    static char* kwlist[] = {"optimiser", "max_comm_size", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "On", kwlist,
                                     &py_optimiser, &max_comm_size))
        return NULL;

    #ifdef DEBUG
      cerr << "set_max_comm_size(" << max_comm_size << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    optimiser->max_comm_size = max_comm_size;

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* _Optimiser_get_max_comm_size(PyObject *self, PyObject *args, PyObject *keywds)
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
      cerr << "get_max_comm_size();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    return PyLong_FromSize_t(optimiser->max_comm_size);
  }

  PyObject* _Optimiser_set_rng_seed(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_optimiser = NULL;
    int seed = 0;
    static char* kwlist[] = {"optimiser", "seed", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "Oi", kwlist,
                                    &py_optimiser, &seed))
       return NULL;

    #ifdef DEBUG
      cerr << "set_rng_seed(" << seed << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule optimiser at address " << py_optimiser << endl;
    #endif
    Optimiser* optimiser = decapsule_Optimiser(py_optimiser);
    #ifdef DEBUG
      cerr << "Using optimiser at address " << optimiser << endl;
    #endif

    #ifdef DEBUG
      cerr << "Setting seed to " << seed << endl;
    #endif
    optimiser->set_rng_seed(seed);

    Py_INCREF(Py_None);
    return Py_None;
  }
#ifdef __cplusplus
}
#endif

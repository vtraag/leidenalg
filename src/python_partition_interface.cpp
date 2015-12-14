#include "python_partition_interface.h"

#define DEBUG

  MutableVertexPartition* create_partition(Graph* graph, char* method, vector<size_t>* initial_membership, double resolution_parameter)
  {
    MutableVertexPartition* partition;
    #ifdef DEBUG
      cerr << "Creating partition for graph at address " << graph
           << " for method " << method << " using resolution " << resolution_parameter
           << " (if relevant)." << endl;
    #endif
    if (strcmp(method, "Modularity") == 0)
    {
      if (initial_membership != NULL)
        partition = new ModularityVertexPartition(graph, *initial_membership);
      else
        partition = new ModularityVertexPartition(graph);
    }
    else if (strcmp(method, "Significance") == 0)
    {
      // Make sure no weight has been indicated, because Significance is not suited for that.
      if (graph->is_weighted())
      {
        PyErr_SetString(PyExc_ValueError, "Significance is not suited for optimisation on weighted graphs. Please consider a different method.");
        delete graph;
        return NULL;
      }
      if (initial_membership != NULL)
        partition = new SignificanceVertexPartition(graph, *initial_membership);
      else
        partition = new SignificanceVertexPartition(graph);
    }
    else if (strcmp(method, "Surprise") == 0)
    {
      if (initial_membership != NULL)
        partition = new SurpriseVertexPartition(graph, *initial_membership);
      else
        partition = new SurpriseVertexPartition(graph);
    }
    else if (strcmp(method, "RBConfiguration") == 0)
    {
      if (initial_membership != NULL)
        partition = new RBConfigurationVertexPartition(graph, *initial_membership, resolution_parameter);
      else
        partition = new RBConfigurationVertexPartition(graph, resolution_parameter);
    }
    else if (strcmp(method, "RBER") == 0)
    {
      if (initial_membership != NULL)
        partition = new RBERVertexPartition(graph, *initial_membership, resolution_parameter);
      else
        partition = new RBERVertexPartition(graph, resolution_parameter);
    }
    else if (strcmp(method, "CPM") == 0)
    {
      if (initial_membership != NULL)
        partition = new CPMVertexPartition(graph, *initial_membership, resolution_parameter);
      else
        partition = new CPMVertexPartition(graph, resolution_parameter);
    }
    else
    {
      PyErr_SetString(PyExc_ValueError, "Non-existing method for optimization specified.");
      delete graph;
      return NULL;
    }
    #ifdef DEBUG
      cerr << "Created partition at address " << partition << endl;
    #endif
    partition->destructor_delete_graph = true;
    return partition;
  }

  // The graph is also created, don't forget to remove it!
  MutableVertexPartition* create_partition_from_py(PyObject* py_obj_graph, char* method, PyObject* py_initial_membership, PyObject* py_weights, double resolution_parameter)
  {
    #if PY_MAJOR_VERSION >= 3
      igraph_t* py_graph = (igraph_t*) PyCapsule_GetPointer(py_obj_graph, NULL);
    #else
      igraph_t* py_graph = (igraph_t*) PyCObject_AsVoidPtr(py_obj_graph);
    #endif
    #ifdef DEBUG
      cerr << "Got igraph_t " << py_graph << endl;
    #endif

    // If necessary create a weighted graph
    Graph* graph = NULL;
    #ifdef DEBUG
      cerr << "Creating graph."<< endl;
    #endif
    if (py_weights != NULL && py_weights != Py_None)
    {
      #ifdef DEBUG
        cerr << "Reading weights." << endl;
      #endif
      vector<double> weights;
      size_t m = PyList_Size(py_weights);
      weights.resize(m);
      for (size_t e = 0; e < m; e++)
        weights[e] = PyFloat_AsDouble(PyList_GetItem(py_weights, e));

      graph = new Graph(py_graph, weights);
    }
    else
    {
      graph = new Graph(py_graph);
    }
    #ifdef DEBUG
      cerr << "Created graph " << graph << endl;
      cerr << "Number of nodes " << graph->vcount() << endl;
      cerr << "Number of edges " << graph->ecount() << endl;
      cerr << "Total weight " << graph->total_weight() << endl;
    #endif

    vector<size_t> initial_membership;
    int has_initial_membership = false;
    // If necessary create an initial partition
    if (py_initial_membership != NULL && py_initial_membership != Py_None)
    {
      #ifdef DEBUG
        cerr << "Reading initial membership." << endl;
      #endif
      has_initial_membership = true;
      size_t n = PyList_Size(py_initial_membership);
      initial_membership.resize(n);
      for (size_t v = 0; v < n; v++)
        initial_membership[v] = PyLong_AsLong(PyList_GetItem(py_initial_membership, v));
    }

    MutableVertexPartition* partition;
    if (has_initial_membership)
      partition = create_partition(graph, method, &initial_membership, resolution_parameter);
    else
      partition = create_partition(graph, method, NULL, resolution_parameter);

    return partition;
  }

  PyObject* capsule_MutableVertexPartition(MutableVertexPartition* partition)
  {
    PyObject* py_partition = PyCapsule_New(partition, "louvain.MutableVertexPartition", del_MutableVertexPartition);
    return py_partition;
  }

  MutableVertexPartition* decapsule_MutableVertexPartition(PyObject* py_partition)
  {
    MutableVertexPartition* partition = (MutableVertexPartition*) PyCapsule_GetPointer(py_partition, "louvain.MutableVertexPartition");
    return partition;
  }

  void del_MutableVertexPartition(PyObject* py_partition)
  {
    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);
    delete partition;
  }

#ifdef __cplusplus
extern "C"
{
#endif
  PyObject* _new_MutableVertexPartition(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_obj_graph = NULL;
    PyObject* py_initial_membership = NULL;
    char* method = "Modularity";
    PyObject* py_weights = NULL;
    double resolution_parameter = 1.0;

    static char* kwlist[] = {"graph", "method", "initial_membership", "weights", "resolution_parameter", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "Os|OOd", kwlist,
                                     &py_obj_graph, &method, &py_initial_membership, &py_weights, &resolution_parameter))
        return NULL;

    MutableVertexPartition* partition = create_partition_from_py(py_obj_graph, method, py_initial_membership, py_weights, resolution_parameter);

    PyObject* py_partition = capsule_MutableVertexPartition(partition);
    #ifdef DEBUG
      cerr << "Created capsule partition at address " << py_partition << endl;
    #endif
    return py_partition;
  }

  PyObject* _MutableVertexPartition_get_py_igraph(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_partition = NULL;

    static char* kwlist[] = {"partition", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "O", kwlist,
                                     &py_partition))
        return NULL;

    #ifdef DEBUG
      cerr << "get_py_igraph();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif

    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);

    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    Graph* graph = partition->get_graph();

    size_t n = graph->vcount();
    size_t m = graph->ecount();

    PyObject* edges = PyList_New(m);
    for (size_t e = 0; e < m; e++)
    {
      vector<size_t> edge = graph->edge(e);
      PyList_SetItem(edges, e, Py_BuildValue("(KK)", edge[0], edge[1]));
    }

    PyObject* weights = PyList_New(m);
    for (size_t e = 0; e < m; e++)
    {
      PyObject* item = PyFloat_FromDouble(graph->edge_weight(e));
      PyList_SetItem(weights, e, item);
    }

    PyObject* node_sizes = PyList_New(n);
    for (size_t v = 0; v < n; v++)
    {
      #if PY_MAJOR_VERSION >= 3
        PyObject* item = PyLong_FromSize_t(graph->node_size(v));
      #else
        PyObject* item = PyInt_FromSize_t(graph->node_size(v));
      #endif
      PyList_SetItem(node_sizes, v, item);
    }

    return Py_BuildValue("lOOO", n, edges, weights, node_sizes);
  }

  PyObject* _MutableVertexPartition_from_coarser_partition(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_partition = NULL;
    PyObject* py_membership = NULL;

    static char* kwlist[] = {"partition", "membership", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OO", kwlist,
                                     &py_partition, &py_membership))
        return NULL;

    #ifdef DEBUG
      cerr << "from_coarser_partition();" << endl;
    #endif

    size_t n = PyList_Size(py_membership);
    vector<size_t> membership;
    membership.resize(n);
    for (size_t v = 0; v < n; v++)
      membership[v] = PyLong_AsLong(PyList_GetItem(py_membership, v));

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif

    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);

    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    partition->from_coarser_partition(membership);

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* _MutableVertexPartition_diff_move(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_partition = NULL;
    size_t v;
    size_t new_comm;

    static char* kwlist[] = {"partition", "v", "new_comm", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OKK", kwlist,
                                     &py_partition, &v, &new_comm))
        return NULL;

    #ifdef DEBUG
      cerr << "diff_move(" << v << ", " << new_comm << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif

    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);

    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    double diff = partition->diff_move(v, new_comm);
    return PyFloat_FromDouble(diff);
  }

  PyObject* _MutableVertexPartition_move_node(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_partition = NULL;
    size_t v;
    size_t new_comm;

    static char* kwlist[] = {"partition", "v", "new_comm", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OKK", kwlist,
                                     &py_partition, &v, &new_comm))
        return NULL;

    #ifdef DEBUG
      cerr << "move_node(" << v << ", " << new_comm << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif

    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);

    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    partition->move_node(v, new_comm);

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* _MutableVertexPartition_quality(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_partition = NULL;

    static char* kwlist[] = {"partition", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "O", kwlist,
                                     &py_partition))
        return NULL;

    #ifdef DEBUG
      cerr << "quality();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif

    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);

    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    double q = partition->quality();
    return PyFloat_FromDouble(q);
  }

  PyObject* _MutableVertexPartition_aggregate_partition(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_partition = NULL;

    static char* kwlist[] = {"partition", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "O", kwlist,
                                     &py_partition))
        return NULL;

    #ifdef DEBUG
      cerr << "aggregate_partition();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif

    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);

    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    // First collapse graph (i.e. community graph)
    Graph* collapsed_graph = partition->get_graph()->collapse_graph(partition);

    // Create collapsed partition (i.e. default partition of each node in its own community).
    MutableVertexPartition* collapsed_partition = partition->create(collapsed_graph);
    collapsed_partition->destructor_delete_graph = true;

    PyObject* py_collapsed_partition = capsule_MutableVertexPartition(collapsed_partition);
    return py_collapsed_partition;
  }

  PyObject* _MutableVertexPartition_total_weight_in_comm(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_partition = NULL;
    size_t comm;

    static char* kwlist[] = {"partition", "comm", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OK", kwlist,
                                     &py_partition, &comm))
        return NULL;

    #ifdef DEBUG
      cerr << "total_weight_in_comm(" << comm << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif

    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);

    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    double w = partition->total_weight_in_comm(comm);
    return PyFloat_FromDouble(w);
  }

  PyObject* _MutableVertexPartition_total_weight_from_comm(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_partition = NULL;
    size_t comm;

    static char* kwlist[] = {"partition", "comm", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OK", kwlist,
                                     &py_partition, &comm))
        return NULL;

    #ifdef DEBUG
      cerr << "total_weight_from_comm(" << comm << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif

    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);

    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    double w = partition->total_weight_from_comm(comm);
    return PyFloat_FromDouble(w);
  }

  PyObject* _MutableVertexPartition_total_weight_to_comm(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_partition = NULL;
    size_t comm;

    static char* kwlist[] = {"partition", "comm", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OK", kwlist,
                                     &py_partition, &comm))
        return NULL;

    #ifdef DEBUG
      cerr << "total_weight_to_comm(" << comm << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif

    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);

    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    double w = partition->total_weight_to_comm(comm);
    return PyFloat_FromDouble(w);
  }

  PyObject* _MutableVertexPartition_total_weight_in_all_comms(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_partition = NULL;

    static char* kwlist[] = {"partition", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "O", kwlist,
                                     &py_partition))
        return NULL;

    #ifdef DEBUG
      cerr << "total_weight_in_all_comms();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif

    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);

    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    double w = partition->total_weight_in_all_comms();
    return PyFloat_FromDouble(w);
  }

  PyObject* _MutableVertexPartition_total_possible_edges_in_all_comms(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_partition = NULL;

    static char* kwlist[] = {"partition", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "O", kwlist,
                                     &py_partition))
        return NULL;

    #ifdef DEBUG
      cerr << "total_possible_edges_in_all_comms();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif

    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);

    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    size_t e = partition->total_possible_edges_in_all_comms();
    return PyLong_FromSize_t(e);
  }

  PyObject* _MutableVertexPartition_weight_to_comm(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_partition = NULL;
    size_t v;
    size_t comm;

    static char* kwlist[] = {"partition", "v", "comm", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OKK", kwlist,
                                     &py_partition, &v, &comm))
        return NULL;

    #ifdef DEBUG
      cerr << "weight_to_comm(" << v << ", " << comm << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif

    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);

    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    double diff = partition->weight_to_comm(v, comm);
    return PyFloat_FromDouble(diff);
  }

  PyObject* _MutableVertexPartition_weight_from_comm(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_partition = NULL;
    size_t v;
    size_t comm;

    static char* kwlist[] = {"partition", "v", "comm", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OKK", kwlist,
                                     &py_partition, &v, &comm))
        return NULL;

    #ifdef DEBUG
      cerr << "weight_to_comm(" << v << ", " << comm << ");" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif

    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);

    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    double diff = partition->weight_to_comm(v, comm);
    return PyFloat_FromDouble(diff);
  }

  PyObject* _MutableVertexPartition_membership(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_partition = NULL;
    static char* kwlist[] = {"partition", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "O", kwlist,
                                     &py_partition))
        return NULL;

    #ifdef DEBUG
      cerr << "membership();" << endl;
    #endif

    #ifdef DEBUG
      cerr << "Capsule partition at address " << py_partition << endl;
    #endif

    MutableVertexPartition* partition = decapsule_MutableVertexPartition(py_partition);

    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    size_t n = partition->get_graph()->vcount();
    PyObject* py_membership = PyList_New(n);
    for (size_t v = 0; v < n; v++)
    {
      #if PY_MAJOR_VERSION >= 3
        PyObject* item = PyLong_FromSize_t(partition->membership(v));
      #else
        PyObject* item = PyInt_FromSize_t(partition->membership(v));
      #endif
      PyList_SetItem(py_membership, v, item);
    }
    return py_membership;
  }

#ifdef __cplusplus
}
#endif
#undef DEBUG

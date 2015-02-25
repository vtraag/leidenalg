#include "pynterface.h"

#ifdef __cplusplus
extern "C"
{
#endif
  static PyObject* _find_partition_multiplex(PyObject *self, PyObject *args, PyObject *keywds)
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
          delete partitions[layer]->get_graph();
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
      #if PY_MAJOR_VERSION >= 3
        PyList_SetItem(membership, v, PyLong_FromLong(partition->membership(v)));
      #else
        PyList_SetItem(membership, v, PyInt_FromLong(partition->membership(v)));
      #endif


    for (size_t layer; layer < nb_layers; layer++)
    {
      delete partitions[layer]->get_graph();
      delete partitions[layer];
    }

    return Py_BuildValue("Od", membership, q);
  }

  static PyObject* _find_partition(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_obj_graph = NULL;
    PyObject* py_initial_membership = NULL;
    char* method = "Modularity";
    PyObject* py_weights = NULL;
    double resolution_parameter = 1.0;
    int consider_comms = Optimiser::ALL_NEIGH_COMMS;

    static char* kwlist[] = {"graph", "method", "initial_membership", "weights", "resolution_parameter", "consider_all_comms", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "Os|OOdi", kwlist,
                                     &py_obj_graph, &method, &py_initial_membership, &py_weights, &resolution_parameter, &consider_comms))
        return NULL;

    #ifdef DEBUG
      cerr << "Consider comms " << consider_comms << " ." << endl;
    #endif

    Optimiser opt;
    opt.consider_comms = consider_comms;

    MutableVertexPartition* partition = create_partition_from_py(py_obj_graph, method, py_initial_membership, py_weights, resolution_parameter);

    #ifdef DEBUG
      cerr << "Using partition at address " << partition << endl;
    #endif

    if (partition == NULL)
    {
      if (PyErr_Occurred() == NULL)
        PyErr_SetString(PyExc_ValueError, "Could not initialize partition. Please check parameters or contact the maintainer.");
      return NULL;
    }

    opt.optimize_partition(partition);

    #ifdef DEBUG
      cerr << "Partition contains " << partition->nb_communities() << " communities, quality "
           << partition->quality() << "." << endl;
    #endif

    size_t n = partition->get_graph()->vcount();
    PyObject* membership = PyList_New(n);
    for (size_t v = 0; v < n; v++)
      #if PY_MAJOR_VERSION >= 3
        PyList_SetItem(membership, v, PyLong_FromLong(partition->membership(v)));
      #else
        PyList_SetItem(membership, v, PyInt_FromLong(partition->membership(v)));
      #endif

    double q = partition->quality();

    delete partition->get_graph();
    delete partition;

    return Py_BuildValue("Od", membership, q);
  }

  static PyObject* _quality(PyObject *self, PyObject *args, PyObject *keywds)
  {
    PyObject* py_obj_graph = NULL;
    PyObject* py_membership = NULL;
    char* method = "Modularity";
    PyObject* py_weights = NULL;
    double resolution_parameter = 1.0;

    static char* kwlist[] = {"graph", "membership", "method", "weights", "resolution_parameter", NULL};

    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "OO|sOd", kwlist,
                                     &py_obj_graph, &py_membership, &method, &py_weights, &resolution_parameter))
        return NULL;

    #ifdef DEBUG
      cerr << "Method: " << method << endl;
    #endif

    MutableVertexPartition* partition = create_partition_from_py(py_obj_graph, method, py_membership, py_weights, resolution_parameter);

    if (partition == NULL)
    {
        if (PyErr_Occurred() == NULL)
          PyErr_SetString(PyExc_ValueError, "Could not initialize partition. Please check parameters or contact the maintainer.");
        return NULL;
    }

    double q = partition->quality();
    delete partition->get_graph(); // Don't forget to remove the graph
    delete partition;

    return Py_BuildValue("d", q);
  }

  static MutableVertexPartition* create_partition(Graph* graph, char* method, vector<size_t>* initial_membership, double resolution_parameter)
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
    return partition;
  }

  // The graph is also created, don't forget to remove it!
  static MutableVertexPartition* create_partition_from_py(PyObject* py_obj_graph, char* method, PyObject* py_initial_membership, PyObject* py_weights, double resolution_parameter)
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
        weights[e] = PyLong_AsLong(PyList_GetItem(py_weights, e));

      graph = new Graph(py_graph, weights);
    }
    else
    {
      graph = new Graph(py_graph);
    }
    #ifdef DEBUG
      cerr << "Created graph " << graph << endl;
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

#ifdef __cplusplus
}
#endif

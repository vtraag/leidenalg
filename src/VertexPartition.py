import igraph as _ig
from . import _c_louvain
from .functions import _get_py_capsule
import sys
# Check if working with Python 3
PY3 = (sys.version > '3');

class MutableVertexPartition(_ig.VertexClustering):
  """ Contains a partition of graph.

  This class contains the basic implementation for optimising a partition.
  Specifically, it implements all the administration necessary to keep track of
  the partition from various points of view. Internally, it keeps track of the
  number of internal edges (or total weight), the size of the communities, the
  total incoming degree (or weight) for a community, etc... When deriving from
  this class, one can easily use this administration to provide their own
  implementation.

  In order to keep the administration up-to-date, all changes in partition
  should be done through move_node. This function moves a node from one
  community to another, and updates all the administration.

  It is possible to manually update the membership vector, and then call
  __init_admin() which completely refreshes all the administration. This is
  only possible by updating the membership vector, not by changing some of the
  other variables.

  The basic idea is that diff_move computes the difference in the quality
  function if we call move_node for the same move. Using this framework, the
  Louvain method in the optimisation class can call these general functions in
  order to optimise the quality function.
  """
  def __init__(self, graph):
    super(MutableVertexPartition, self).__init__(graph);

  # Init
  def __init__(self, graph, method, initial_membership=None,
      weight=None, resolution_parameter=1.0):
    """ Create a new vertex partition.

    Parameters:
      graph            -- The igraph.Graph on which this partition is defined.
      membership=None  -- The membership vector of this partition, i.e. an
                          community number for each node. So membership[i] = c
                          implies that node i is in community c. If None, it is
                          initialised with each node in its own community.
      weight_attr=None -- What edge attribute should be used as a weight for the
                          edges? If None, the weight defaults to 1."""
    if not initial_membership:
      initial_membership = range(graph.vcount());
    super(MutableVertexPartition, self).__init__(graph, initial_membership);
    self._method = method;
    pygraph_t = _get_py_capsule(graph);
    if weight is not None:
      if isinstance(weight, str):
        weight = graph.es[weight];
      else:
        # Make sure it is a list
        weight = list(weight);
    if initial_membership is not None:
      gen = _ig.UniqueIdGenerator();
      initial_membership = [gen[m] for m in initial_membership];
    self._partition = _c_louvain._new_MutableVertexPartition(pygraph_t, method, initial_membership, weight, resolution_parameter);

  @classmethod
  def _FromCPartition(cls, partition):
    n, edges, weights, node_sizes = _c_louvain._MutableVertexPartition_get_py_igraph(partition);
    graph = _ig.Graph(n=n,
                     edges=edges,
                     edge_attrs={'weight': weights},
                     vertex_attrs={'node_size': node_sizes});
    new_partition = cls(graph);
    new_partition._partition = partition;
    new_partition._update_internal_membership();
    return new_partition;

  def _update_internal_membership(self):
    self._membership = _c_louvain._MutableVertexPartition_membership(self._partition);
    # Reset the length of the object, i.e. the number of communities
    if len(self._membership)>0:
        self._len = max(m for m in self._membership if m is not None)+1
    else:
        self._len = 0

  # Calculate improvement *if* we move this node
  def diff_move(self,v,new_comm):
    """ Calculate the difference in the quality function if we were to move
    this node. In this base class, the quality function is always simply 0.0,
    and so the diff_move is also always 0.0. For implementing actual quality
    functions, one should derive from this base class and implement their own
    diff_move and quality funcion.

    The difference returned by diff_move should be equivalent to first
    determining the quality of the partition, then calling move_node, and then
    determining again the quality of the partition and looking at the
    difference. In other words

    diff = partition.diff_move(v, new_comm);
    q1 = partition.quality();
    move_node(v, new_comm);
    q2 = partition.quality();

    Then diff == q2 - q1."""
    return _c_louvain._MutableVertexPartition_diff_move(self._partition, v, new_comm);

  def aggregate_partition(self):
    return self._FromCPartition(_c_louvain._MutableVertexPartition_aggregate_partition(self._partition));

  def move_node(self,v,new_comm):
    _c_louvain._MutableVertexPartition_move_node(self._partition, v, new_comm);
    # Make sure this move is also reflected in the membership vector of the python object
    self._membership[v] = new_comm;
    self._modularity_dirty = True;

  def from_coarser_partition(self, partition):
    """ Read new communities from coarser partition assuming that the community
    represents a node in the coarser partition (with the same index as the
    community number). """
    # Read the coarser partition
    _c_louvain._MutableVertexPartition_from_coarser_partition(self._partition, partition.membership);
    self._update_internal_membership();

  def quality(self):
    return _c_louvain._MutableVertexPartition_quality(self._partition);

  def total_weight_in_comm(self, comm):
     return _c_louvain._MutableVertexPartition_total_weight_in_comm(self._partition, comm);

  def total_weight_from_comm(self, comm):
     return _c_louvain._MutableVertexPartition_total_weight_from_comm(self._partition, comm);

  def total_weight_to_comm(self, comm):
     return _c_louvain._MutableVertexPartition_total_weight_to_comm(self._partition, comm);

  def total_weight_in_all_comms(self):
     return _c_louvain._MutableVertexPartition_total_weight_in_all_comms(self._partition);

  def total_possible_edges_in_all_comms(self):
     return _c_louvain._MutableVertexPartition_total_possible_edges_in_all_comms(self._partition);

  def weight_to_comm(self, v, comm):
     return _c_louvain._MutableVertexPartition_weight_to_comm(self._partition, v, comm);

  def weight_from_comm(self, v, comm):
     return _c_louvain._MutableVertexPartition_weight_from_comm(self._partition, v, comm);

class ModularityVertexPartition(MutableVertexPartition):
  """ Implements the diff_move and quality function in order to optimise
  modularity. """
  def __init__(self, graph, initial_membership=None,
      weight=None):
    super(ModularityVertexPartition, self).__init__(graph, 'Modularity', initial_membership, weight);

class SurpriseVertexPartition(MutableVertexPartition):
  """ Implements the diff_move and quality function in order to optimise
  Surprise. """
  def __init__(self, graph, initial_membership=None,
      weight=None):
    super(SurpriseVertexPartition, self).__init__(graph, 'Surprise', initial_membership, weight);

class SignficanceVertexPartition(MutableVertexPartition):
  """ Implements the diff_move and quality function in order to optimise
  Significance. """
  def __init__(self, graph, initial_membership=None,
      weight=None):
    super(SignficanceVertexPartition, self).__init__(graph, 'Significance', initial_membership, weight);

class LinearResolutionParameterVertexPartition(MutableVertexPartition):
  """ Some quality functions have a linear resolution parameter, for which the
  basis is implemented here.

  With a linear resolution parameter, we mean that the objective function has
  the form:

  H = E - gamma F

  where gamma is some resolution parameter and E and F arbitrary other
  functions of the partition.

  One thing that can be easily done on these type of quality functions, is
  bisectioning on the gamma function (also assuming that E is a stepwise
  decreasing monotonic function, cf. CPM).
  """
  def __init__(self, graph, method, initial_membership=None,
      weight=None, resolution_parameter=1.0):
    super(LinearResolutionParameterVertexPartition, self).__init__(graph, method,
          initial_membership, weight, resolution_parameter);
    self.resolution = resolution;

  def bisect_value(self):
    """  Give the value on which we can perform bisectioning. If p1 and p2 are
    two different optimal partitions for two different resolution parameters
    g1 and g2, then if p1.bisect_value() == p2.bisect_value() the two
    partitions should be optimal for both g1 and g2."""
    return 0.0;

class RBERVertexPartition(MutableVertexPartition):
  """ Implements the diff_move and quality function in order to optimise
  RBER, which uses a Erdos-Renyi graph as a null model. """
  def __init__(self, graph, resolution_parameter=1.0, initial_membership=None,
      weight=None):
    super(RBERVertexPartition, self).__init__(graph, 'RBER', initial_membership, weight, resolution_parameter);

class RBConfigurationVertexPartition(MutableVertexPartition):
  """ Implements the diff_move and quality function in order to optimise
  RB Configuration model (i.e. modularity with a resolution parameter). """
  def __init__(self, graph, resolution_parameter=1.0, initial_membership=None,
      weight=None):
    super(RBConfigurationVertexPartition, self).__init__(graph, 'RBConfiguration', initial_membership, weight, resolution_parameter);

class CPMVertexPartition(MutableVertexPartition):
  """ Implements the diff_move and quality function in order to optimise
  CPM. """
  def __init__(self, graph, resolution_parameter=1.0, initial_membership=None,
      weight=None):
    super(CPMVertexPartition, self).__init__(graph, 'CPM', initial_membership, weight, resolution_parameter);

import igraph as _ig
from . import _c_louvain
from .functions import _get_py_capsule
import sys
# Check if working with Python 3
PY3 = (sys.version > '3');

class MutableVertexPartition(_ig.VertexClustering):
  """ Contains a partition of graph, derives from :class:`ig.VertexClustering`.

  This class contains the basic implementation for optimising a partition.
  Specifically, it implements all the administration necessary to keep track of
  the partition from various points of view. Internally, it keeps track of the
  number of internal edges (or total weight), the size of the communities, the
  total incoming degree (or weight) for a community, et cetera.

  In order to keep the administration up-to-date, all changes in a partition
  should be done through :func:`~louvain.VertexPartition.MutableVertexPartition.move_node`
  or :func:`~louvain.VertexPartition.MutableVertexPartition.set_membership`.
  The first moves a node from one community to another, and updates the administration.
  The latter simply updates the membership vector and updates the administration.

  The basic idea is that :func:`~louvain.VertexPartition.MutableVertexPartition.diff_move`
  computes the difference in the quality
  function if we would call :func:`~louvain.VertexPartition.MutableVertexPartition.move_node` for the same move. These functions are
  overridden in any derived classes to provide an actual implementation. These
  functions are used by :class:`~louvain.Optimiser` to optimise the partition.

  .. warning::
    This base class should never be used in practice, since only derived classes
    provide an actual implementation.

  """

  # Init
  def __init__(self, graph, initial_membership=None):
    """
    Parameters
    ----------
    graph
      The `ig.Graph` on which this partition is defined.

    membership
      The membership vector of this partition. Membership[i] = c implies that node i
      is in community c. If None, it is initialised with a singleton partition
      community, i.e. membership[i] = i.
    """
    super(MutableVertexPartition, self).__init__(graph, initial_membership);

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
    self._membership = _c_louvain._MutableVertexPartition_get_membership(self._partition);
    # Reset the length of the object, i.e. the number of communities
    if len(self._membership)>0:
        self._len = max(m for m in self._membership if m is not None)+1
    else:
        self._len = 0

  def set_membership(self, membership):
    """ Set membership. """
    _c_louvain._MutableVertexPartition_set_membership(self._partition, list(membership));
    self._update_internal_membership();

  # Calculate improvement *if* we move this node
  def diff_move(self,v,new_comm):
    """ Calculate the difference in the quality function if node ``v`` is
    moved to community ``new_comm``.

    Parameters
    ----------
    v
      The node to move.

    new_comm
      The community to move to.

    Returns
    -------
    float
      Difference in quality function.

    Notes
    -----
    The difference returned by diff_move should be equivalent to first
    determining the quality of the partition, then calling move_node, and then
    determining again the quality of the partition and looking at the
    difference. In other words

    >>> diff = partition.diff_move(v, new_comm);
    >>> q1 = partition.quality();
    >>> move_node(v, new_comm);
    >>> q2 = partition.quality();
    >>> diff == q2 - q1
    True

    .. warning::
      Only derived classes provide actual implementations, the base class provides no implementation
      for this function.

    """
    return _c_louvain._MutableVertexPartition_diff_move(self._partition, v, new_comm);

  def aggregate_partition(self):
    """ Aggregate the graph according to the current partition and provide a
        default partition for it.
    """
    return self._FromCPartition(_c_louvain._MutableVertexPartition_aggregate_partition(self._partition));

  def move_node(self,v,new_comm):
    """ Move node ``v`` to community ``new_comm``.

    Parameters
    ----------
    v
      Node to move.

    new_comm
      Community to move to.

    Returns
    -------
    float
      Difference in quality function.
    """
    diff = _c_louvain._MutableVertexPartition_move_node(self._partition, v, new_comm);
    # Make sure this move is also reflected in the membership vector of the python object
    self._membership[v] = new_comm;
    self._modularity_dirty = True;
    return diff;

  def from_coarser_partition(self, partition):
    """ Update current partition according to coarser partition.

    Parameters
    ----------
    partition
      The coarser partition used to update the current partition.

    Notes
    -----
    This function is to be used to determine the correct partition
    for an aggregated graph. In particular, suppose we move nodes
    and then get an aggregate graph.

    >>> optimiser.move_nodes(partition);
    >>> aggregate_partition = partition.aggregate_partition();

    Now we also move nodes in the aggregate partition

    >>> optimiser.move_nodes(aggregate_partition);

    Now we improved the quality function of ``aggregate_partition``, but
    this is not yet reflected in the original ``partition``. We can thus
    call

    >>> partition.from_coarser_partition(aggregate_partition)

    so that ``partition`` now reflects the changes made to
    ``aggregate_partition``.
    """
    # Read the coarser partition
    _c_louvain._MutableVertexPartition_from_coarser_partition(self._partition, partition.membership);
    self._update_internal_membership();

  def renumber_communities(self):
    """ Renumber the communities so that they are numbered in decreasing size.

    Notes
    -----
    The sort is not necessarilly stable.
    """
    _c_louvain._MutableVertexPartition_renumber_communities(self._partition);
    self._update_internal_membership();

  def quality(self):
    """ The current quality of the partition. """
    return _c_louvain._MutableVertexPartition_quality(self._partition);

  def total_weight_in_comm(self, comm):
    """ The total weight (i.e. number of edges) within a community.

    Parameters
    ----------
    comm
      Community

    See Also
    --------
    total_weight_to_comm
    total_weight_from_comm
    total_weight_in_all_comms
    """
    return _c_louvain._MutableVertexPartition_total_weight_in_comm(self._partition, comm);

  def total_weight_from_comm(self, comm):
    """ The total weight (i.e. number of edges) from a community.

    Parameters
    ----------
    comm
      Community

    Notes
    -----
    This includes all edges, also the ones that are internal to a community.
    Sometimes this is also referedd to as the community (out)degree.

    See Also
    --------
    total_weight_to_comm
    total_weight_in_comm
    total_weight_in_all_comms
    """
    return _c_louvain._MutableVertexPartition_total_weight_from_comm(self._partition, comm);

  def total_weight_to_comm(self, comm):
    """ The total weight (i.e. number of edges) to a community.

    Parameters
    ----------
    comm
      Community

    Notes
    -----
    This includes all edges, also the ones that are internal to a community.
    Sometimes this is also referedd to as the community (in)degree.

    See Also
    --------
    total_weight_from_comm
    total_weight_in_comm
    total_weight_in_all_comms
    """
    return _c_louvain._MutableVertexPartition_total_weight_to_comm(self._partition, comm);

  def total_weight_in_all_comms(self):
    """ The total weight (i.e. number of edges) within all communities.

    Notes
    -----
    This should be equal to simply the sum of ``total_weight_in_comm`` for all communities.

    See Also
    --------
    total_weight_to_comm
    total_weight_from_comm
    total_weight_in_comm
    """
    return _c_louvain._MutableVertexPartition_total_weight_in_all_comms(self._partition);

  def total_possible_edges_in_all_comms(self):
    """ The total possible number of edges in all communities.

    Notes
    -----
    If we denote by :math:`n_c` the number of nodes in community :math:`c`, this is simply

    .. math :: \sum_c \binom{n_c}{2}

    """
    return _c_louvain._MutableVertexPartition_total_possible_edges_in_all_comms(self._partition);

  def weight_to_comm(self, v, comm):
    """ The total number of edges (or sum of weights) from node ``v`` to community ``comm``

    See Also
    --------
    weight_from_comm
    """
    return _c_louvain._MutableVertexPartition_weight_to_comm(self._partition, v, comm);

  def weight_from_comm(self, v, comm):
    """ The total number of edges (or sum of weights) to node ``v`` from community ``comm``

    See Also
    --------
    weight_to_comm
    """
    return _c_louvain._MutableVertexPartition_weight_from_comm(self._partition, v, comm);

class ModularityVertexPartition(MutableVertexPartition):
  """ Implements modularity.

  Notes
  -----
  The quality function is

  .. math::
    Q = \\frac{1}{2m} \\sum_{ij} \\left(A_{ij} - \\frac{k_i k_j}{2m} \\right)\\delta(\\sigma_i, \\sigma_j)

  where :math:`A` is the adjacency matrix, :math:`k_i` is the degree of node :math:`i`,
  :math:`m` is the total number of edges, :math:`\sigma_i` denotes the community of node :math:`i`
  and :math:`\delta(\sigma_i, \\sigma_j) = 1` if :math:`\sigma_i = \\sigma_j` and `0` otherwise.

  This can alternatively be formulated as a sum over communities:

  .. math::
    Q = \\frac{1}{2m} \\sum_{c} \\left(e_c - \\frac{K_c^2}{4m} \\right)

  where :math:`e_c` is the number of internal edges of community :math:`c` and
  :math:`K_c = \\sum_{i \\mid \\sigma_i = c} k_i` is the total degree of nodes in community
  :math:`c`.

  References
  ----------
  .. [1] Newman, M. E. J., & Girvan, M. (2004). Finding and evaluating community structure in networks.
         Physical Review E, 69(2), 026113+.
         `10.1103/PhysRevE.69.026113 <http://doi.org/10.1103/PhysRevE.69.026113>`_
   """
  def __init__(self, graph, initial_membership=None, weights=None):
    """
    Parameters
    ----------
    graph : :class:`ig.Graph`
      Graph to define the partition on

    initial_membership : list of int
      Initial membership for the partition. If :obj:`None` then defaults to a singleton partition.

    weights : list of double, or edge attribute
      Weights of edges. Can be either an iterable or an edge attribute.
    """
    super(ModularityVertexPartition, self).__init__(graph, initial_membership);
    pygraph_t = _get_py_capsule(graph);

    if weights is not None:
      if isinstance(weights, str):
        weights = graph.es[weights];
      else:
        # Make sure it is a list
        weights = list(weights);

    self._partition = _c_louvain._new_ModularityVertexPartition(pygraph_t,
        initial_membership, weights);
    self._update_internal_membership();


class SurpriseVertexPartition(MutableVertexPartition):
  """ Implements Surprise. """

  def __init__(self, graph, initial_membership=None, weights=None, node_sizes=None):
    """
    Parameters
    ----------
    graph : :class:`ig.Graph`
      Graph to define the partition on

    initial_membership : list of int
      Initial membership for the partition. If :obj:`None` then defaults to a singleton partition.

    weights : list of double, or edge attribute
      Weights of edges. Can be either an iterable or an edge attribute.

    node_sizes : list of int, or vertex attribute
      Sizes of nodes are necessary to know the size of communities in aggregate graphs. Usually this
      is set to 1 for all nodes, but in specific cases this could be changed.
    """
    super(SurpriseVertexPartition, self).__init__(graph, initial_membership);

    pygraph_t = _get_py_capsule(graph);

    if weights is not None:
      if isinstance(weights, str):
        weights = graph.es[weights];
      else:
        # Make sure it is a list
        weights = list(weights);

    self._partition = _c_louvain._new_SurpriseVertexPartition(pygraph_t,
        initial_membership, weights);
    self._update_internal_membership();

class SignificanceVertexPartition(MutableVertexPartition):
  """ Implements Significance. """
  def __init__(self, graph, initial_membership=None, node_sizes=None):
    """
    Parameters
    ----------
    graph : :class:`ig.Graph`
      Graph to define the partition on

    initial_membership : list of int
      Initial membership for the partition. If :obj:`None` then defaults to a singleton partition.

    node_sizes : list of int, or vertex attribute
      Sizes of nodes are necessary to know the size of communities in aggregate graphs. Usually this
      is set to 1 for all nodes, but in specific cases this could be changed.
    """
    super(SignificanceVertexPartition, self).__init__(graph, initial_membership);

    pygraph_t = _get_py_capsule(graph);

    self._partition = _c_louvain._new_SignificanceVertexPartition(pygraph_t, initial_membership);
    self._update_internal_membership();

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
  def __init__(self, graph, initial_membership=None):
    super(LinearResolutionParameterVertexPartition, self).__init__(graph, initial_membership);

  #########################################################3
  # resolution parameter
  @property
  def resolution_parameter(self):
    """ Resolution parameter. """
    return _c_louvain._ResolutionParameterVertexPartition_set_resolution(self._partition, value);

  @resolution_parameter.setter
  def resolution_parameter(self, value):
    return _c_louvain._ResolutionParameterVertexPartition_get_resolution(self._partition);

  def bisect_value(self):
    """ Give the value on which we can perform bisectioning. If p1 and p2 are
    two different optimal partitions for two different resolution parameters
    g1 and g2, then if p1.bisect_value() == p2.bisect_value() the two
    partitions should be optimal for both g1 and g2."""
    return self.total_weight_in_all_comms();

class RBERVertexPartition(LinearResolutionParameterVertexPartition):
  """ Implements the diff_move and quality function in order to optimise
  RBER, which uses a Erdos-Renyi graph as a null model. """
  def __init__(self, graph, initial_membership=None, weights=None, node_sizes=None, resolution_parameter=1.0):
    """
    Parameters
    ----------
    graph : :class:`ig.Graph`
      Graph to define the partition on.

    initial_membership : list of int
      Initial membership for the partition. If :obj:`None` then defaults to a singleton partition.

    weights : list of double, or edge attribute
      Weights of edges. Can be either an iterable or an edge attribute.

    node_sizes : list of int, or vertex attribute
      Sizes of nodes are necessary to know the size of communities in aggregate graphs. Usually this
      is set to 1 for all nodes, but in specific cases this could be changed.

    resolution_parameter : double
      Resolution parameter.
    """
    super(RBERVertexPartition, self).__init__(graph, initial_membership);

    pygraph_t = _get_py_capsule(graph);

    if weights is not None:
      if isinstance(weights, str):
        weights = graph.es[weights];
      else:
        # Make sure it is a list
        weights = list(weights);

    if node_sizes is not None:
      if isinstance(node_sizes, str):
        node_sizes = graph.vs[node_sizes];
      else:
        # Make sure it is a list
        node_sizes = list(node_sizes);

    self._partition = _c_louvain._new_RBERVertexPartition(pygraph_t,
        initial_membership, weights, node_sizes, resolution_parameter);
    self._update_internal_membership();

class RBConfigurationVertexPartition(LinearResolutionParameterVertexPartition):
  """ Implements Reichardt and Bornholdt's Potts model with a configuration null model.
  This quality function uses a linear resolution parameter.

  Notes
  -----
  The quality function is

  .. math::
    Q = \\sum_{ij} \\left(A_{ij} - \\gamma \\frac{k_i k_j}{2m} \\right)\\delta(\\sigma_i, \\sigma_j)

  where :math:`A` is the adjacency matrix, :math:`k_i` is the degree of node :math:`i`,
  :math:`m` is the total number of edges, :math:`\sigma_i` denotes the community of node :math:`i`,
  :math:`\delta(\sigma_i, \\sigma_j) = 1` if :math:`\sigma_i = \\sigma_j` and `0` otherwise, and, finally
  :math:`\\gamma` is a resolution parameter.

  This can alternatively be formulated as a sum over communities:

  .. math::
    Q = \\sum_{c} \\left(e_c - \\gamma\\frac{K_c^2}{4m} \\right)

  where :math:`e_c` is the number of internal edges of community :math:`c` and
  :math:`K_c = \\sum_{i \\mid \\sigma_i = c} k_i` is the total degree of nodes in community
  :math:`c`.

  Note that this is the same as :class:`ModularityVertexPartition` for :math:`\\gamma=1` and using the
  normalisation by :math:`2m`.

  References
  ----------
  .. [1] Reichardt, J., & Bornholdt, S. (2006). Statistical mechanics of community detection.
         Physical Review E, 74(1), 016110+. `10.1103/PhysRevE.74.016110 <http://doi.org/10.1103/PhysRevE.74.016110>`_

   """
  def __init__(self, graph, initial_membership=None, weights=None, resolution_parameter=1.0):
    """
    Parameters
    ----------
    graph : :class:`ig.Graph`
      Graph to define the partition on.

    initial_membership : list of int
      Initial membership for the partition. If :obj:`None` then defaults to a singleton partition.

    weights : list of double, or edge attribute
      Weights of edges. Can be either an iterable or an edge attribute.

    resolution_parameter : double
      Resolution parameter.
    """
    super(RBConfigurationVertexPartition, self).__init__(graph, initial_membership);

    pygraph_t = _get_py_capsule(graph);

    if weights is not None:
      if isinstance(weights, str):
        weights = graph.es[weights];
      else:
        # Make sure it is a list
        weights = list(weights);

    self._partition = _c_louvain._new_RBConfigurationVertexPartition(pygraph_t,
        initial_membership, weights, resolution_parameter);
    self._update_internal_membership();

class CPMVertexPartition(LinearResolutionParameterVertexPartition):
  """ Implements the diff_move and quality function in order to optimise
  CPM. """
  def __init__(self, graph, initial_membership=None, weights=None, node_sizes=None, resolution_parameter=1.0):
    """
    Parameters
    ----------
    graph : :class:`ig.Graph`
      Graph to define the partition on.

    initial_membership : list of int
      Initial membership for the partition. If :obj:`None` then defaults to a singleton partition.

    weights : list of double, or edge attribute
      Weights of edges. Can be either an iterable or an edge attribute.

    node_sizes : list of int, or vertex attribute
      Sizes of nodes are necessary to know the size of communities in aggregate graphs. Usually this
      is set to 1 for all nodes, but in specific cases this could be changed.

    resolution_parameter : double
      Resolution parameter.
    """
    super(CPMVertexPartition, self).__init__(graph, initial_membership);

    pygraph_t = _get_py_capsule(graph);

    if weights is not None:
      if isinstance(weights, str):
        weights = graph.es[weights];
      else:
        # Make sure it is a list
        weights = list(weights);

    if node_sizes is not None:
      if isinstance(node_sizes, str):
        node_sizes = graph.vs[node_sizes];
      else:
        # Make sure it is a list
        node_sizes = list(node_sizes);

    self._partition = _c_louvain._new_CPMVertexPartition(pygraph_t,
        initial_membership, weights, node_sizes, resolution_parameter);
    self._update_internal_membership();

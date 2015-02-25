import igraph as _ig
from . import _c_louvain
from ._c_louvain import ALL_COMMS
from ._c_louvain import ALL_NEIGH_COMMS
from ._c_louvain import RAND_COMM
from ._c_louvain import RAND_NEIGH_COMM
from collections import namedtuple
from collections import OrderedDict
import logging
from math import log, sqrt

import sys
# Check if working with Python 3
PY3 = (sys.version > '3');

class Layer:
  """
  This class makes sure that each layer is properly encoded for use in the
  ``find_partition_multiplex`` function.

  Keyword arguments:

  graph
    The graph for this layer.

  method
    The method used for this layer (see package documentation for details).

  layer_weight
    The weight used for weighing this layer in the overall quality.

  initial_membership
    The initial membership to start the optimization with.

  weight
    The edge attribute from which to use the weight for this layer.

  resolution_parameter
    The resolution parameter used for this layer.

  """
  def __init__(self, graph, method, layer_weight=1.0, initial_membership=None, weight=None, resolution_parameter=1.0):
    self.graph = graph;
    self.method = method;
    self.layer_weight = layer_weight;
    self.initial_membership = initial_membership;
    self.weight = weight;
    self.resolution_parameter = resolution_parameter;

def __get_py_capsule(graph):
  if PY3:
    return graph.__graph_as_capsule();
  else:
    return graph.__graph_as_cobject();

def find_partition_multiplex(layers, consider_comms=ALL_NEIGH_COMMS):
  """
  Method for detecting communities using the Louvain algorithm. This functions
  finds the optimal partition for all layers given the specified methods. For
  the various possible methods see package documentation. This considers all
  graphs in all layers, in which each node may be differently connected, but all
  nodes must appear in all graphs. Furthermore, they should have identical
  indices in the graph (i.e. node i is assumed to be the same node in all
  graphs).  The quality of this partition is simply the sum of the individual
  qualities for the various partitions, weighted by the layer_weight. If we
  denote by q_k the quality of layer k and the weight by w_k, the overall
  quality is then
  q = sum_k w_k*q_k.

  Notice that this is particularly useful for graphs containing negative links.
  When separating the graph in two graphs, the one containing only the positive
  links, and the other only the negative link, by supplying a negative weight to
  the latter layer, we try to find relatively many positive links within a
  community and relatively many negative links between communities.

  Keyword arguments:

  layers
    A list of all layers containing the additional information. See ``Layer``
    class for more information.

  consider_comms=ALL_NEIGH_COMMS
    This parameter determines which communities to consider when moving a node.

    ALL_COMMS
      Consider all communities always.

    ALL_NEIGH_COMMS
      Consider all communities of the neighbours

    RAND_COMM
      Consider only a single random community

    ALL_NEIGH_COMMS
      Consider only a single random community of the neighbours. Notice that
      this is sampled from the set of all neighbours so that the communities are
      sampled with respective frequency.

    In ordinary cases it is usually not necessary to alter this parameter. The
    default choice of considering all communities of the neighbours works
    relatively well, and is relatively fast. However, in the case of negative
    weights, it may be better to move a node to a community to which it is not
    connected, so that one would need to consider all communities.
    Alternatively, by only selecting a single random community from the
    neighbours to consider, one can considerably speed up the algorithm, without
    loosing too much quality.

  returns: membership vector and the quality

  We return the membership vector instead of a partition, because the latter
  needs an underlying graph.
  """
  args = [];
  for layer in layers:
    if layer.weight is not None:
      if isinstance(layer.weight, str):
        layer.weight = layer.graph.es[layer.weight];
      else:
        # Make sure it is a list
        layer.weight = list(layer.weight);
    args.append((__get_py_capsule(layer.graph), layer.method,
           layer.layer_weight, layer.initial_membership, layer.weight,
           layer.resolution_parameter));
  membership, quality = _c_louvain._find_partition_multiplex(args, consider_comms);
  return membership, quality;

def find_partition(graph, method, initial_membership=None, weight=None,
    resolution_parameter=1.0, consider_comms=ALL_NEIGH_COMMS):
  """
  Method for detecting communities using the Louvain algorithm. This functions
  finds the optimal partition given the specified method. For the various possible
  methods see package documentation.

  Keyword arguments:

  graph
    The graph for which to find the optimal partition.

  method
    The type of partition which will be used during optimisation.

  initial_membership=None
    If provided, the optimisation will start with this initial membership.
    Should be a list that contains any unique identified for a community, which
    is converted to a numeric representation. Since communities can never be
    split, the number of communities in this initial partition provides an upper
    bound.

  weight=None
    If provided, indicates the edge attribute to use as a weight. (N.B. note
    that Significance is not suited for weighted graphs).

  resolution_parameter=1.0
    For those methods that use a resolution parameter, this is indicated here.

  consider_comms=ALL_NEIGH_COMMS
    This parameter determines which communities to consider when moving a node.

    ALL_COMMS
      Consider all communities always.

    ALL_NEIGH_COMMS
      Consider all communities of the neighbours

    RAND_COMM
      Consider only a single random community

    ALL_NEIGH_COMMS
      Consider only a single random community of the neighbours. Notice that
      this is sampled from the set of all neighbours so that the communities are
      sampled with respective frequency.

    In ordinary cases it is usually not necessary to alter this parameter. The
    default choice of considering all communities of the neighbours works
    relatively well, and is relatively fast. However, in the case of negative
    weights, it may be better to move a node to a community to which it is not
    connected, so that one would need to consider all communities.
    Alternatively, by only selecting a single random community from the
    neighbours to consider, one can considerably speed up the algorithm, without
    loosing too much quality.

  The quality of the partition, as measured by the indicated method is
  provided in the returned partition as partition.quality.

  returns: optimized partition."""
  pygraph_t = __get_py_capsule(graph);
  if weight is not None:
    if isinstance(weight, str):
      weight = graph.es[weight];
    else:
      # Make sure it is a list
      weight = list(weight);
  if initial_membership is not None:
    gen = _ig.UniqueIdGenerator;
    initial_membership = [gen[m] for m in initial_membership];
  membership, quality = _c_louvain._find_partition(pygraph_t, method, initial_membership, weight, resolution_parameter, consider_comms);
  partition = _ig.VertexClustering(graph, membership);
  partition.quality = quality;
  return partition;

def quality(graph, partition, method, weight=None, resolution_parameter=1.0):
  """ Returns the quality of the partition as measured by the indicated method.
  For the various possible methods see package documentation.
  Keyword arguments:

  graph
    The graph for to use for the calculation of the quality of the partition.

  partition
    The partition for which to measure the quality.

  method
    The type of partition which will be used during optimisation.

  initial_membership=None
    If provided, the optimisation will start with this initial membership.
    Should be a list that contains any unique identified for a community, which
    is converted to a numeric representation. Since communities can never be
    split, the number of communities in this initial partition provides an upper
    bound.

  weight=None
    If provided, indicates the edge attribute to use as a weight. (N.B. note
    that Significance is not suited for weighted graphs).

  resolution_parameter=1.0
    For those methods that use a resolution parameter, this is indicated here.

  returns: quality of the partition.
  """
  pygraph_t = __get_py_capsule(graph);
  membership = partition.membership;
  if weight is not None:
    if isinstance(weight, str):
      weight = graph.es[weight];
    else:
      # Make sure it is a list
      weight = list(weight);
  if len(membership) != graph.vcount():
    raise Exception("Membership length inconsistent with graph.");
  return _c_louvain._quality(pygraph_t, membership, method, weight, resolution_parameter);

def total_internal_edges(partition, weight=None):
  """ Returns the total number of internal edges (or internal weight in case of
  a weighted graph). Used for bisectioning on resolution parameters, see ``bisect``. """
  if weight is None:
    return sum([H.ecount() for H in partition.subgraphs()]);
  else:
    return sum([sum(H.es[weight]) for H in partition.subgraphs()]);

def bisect(
      graph,
      method,
      resolution_range,
      weight=None,
      consider_comms=ALL_NEIGH_COMMS,
      bisect_func=total_internal_edges,
      min_diff_bisect_value=1,
      min_diff_resolution=1e-3,
      linear_bisection=False,
      ):
  """ Use bisectioning on the resolution parameter in order to construct a
  resolution profile.

  Keyword arguments:

  graph
    The graph for which to find the optimal partition(s).

  method
    The method used to find a partition (must support resolution parameters
    obviously).

  resolution_range
    The range of resolution values that we would like to scan.

  weight=None
    If provided, indicates the edge attribute to use as a weight.

  consider_comms=ALL_NEIGH_COMMS
    This parameter determines which communities to consider when moving a node.
    Please refer to the documentation of `find_partition` for more information
    on this parameter.

  bisect_func=total_internal_edges
    The function used for bisectioning. For the methods currently implemented,
    this should usually not be altered.

  min_diff_bisect_value=1
    The difference in the value returned by the bisect_func below which the
    bisectioning stops (i.e. by default, a difference of a single edge does not
    trigger further bisectioning).

  min_diff_resolution=1e-3
    The difference in resolution below which the bisectioning stops. For
    positive differences, the logarithmic difference is used by default, i.e.
    ``diff = log(res_1) - log(res_2) = log(res_1/res_2)``, for which ``diff >
    min_diff_resolution`` to continue bisectioning. Set the linear_bisection to
    true in order to use only linear bisectioning (in the case of negative
    resolution parameters for example, which can happen with negative weights).

  linear_bisection=False
    Whether the bisectioning will be done on a linear or on a logarithmic basis
    (if possible).

  returns: a list of partitions and resolution values.
    """
  # Helper function for cleaning values to be a stepwise function
  def clean_stepwise(bisect_values):
    # We only need to keep the changes in the bisection values
    bisect_list = sorted([(res, part.bisect_value) for res, part in
      bisect_values.iteritems()], key=lambda x: x[0]);
    for (res1, v1), (res2, v2) \
        in zip(bisect_list,
               bisect_list[1:]):
      # If two consecutive bisection values are the same, remove the second
      # resolution parameter
      if v1 == v2:
        del bisect_values[res2];
  # We assume here that the bisection values are
  # monotonically decreasing with increasing resolution
  # parameter values
  def ensure_monotonicity(bisect_values, new_res):
    for res, bisect_part in bisect_values.iteritems():
      # If at a lower resolution value there were lower bisection values, we
      # should update them in order to maintain monotonicity
      if res < new_res and \
         bisect_part.bisect_value < bisect_values[new_res].bisect_value:
        bisect_values[res] = bisect_values[new_res];
      # If at a higher resolution value there were higher bisection values, we
      # should update them in order to maintain monotonicity
      elif res > new_res and \
         bisect_part.bisect_value > bisect_values[new_res].bisect_value:
        bisect_values[res] = bisect_values[new_res];
  # Start actual bisectioning
  bisect_values = {};
  stack_res_range = [];
  # Push first range onto the stack
  stack_res_range.append(resolution_range);
  # Make sure the bisection values are calculated
  # The namedtuple we will use in the bisection function
  BisectPartition = namedtuple('BisectPartition',
      ['partition', 'bisect_value']);
  partition = find_partition(graph=graph, method=method, weight=weight,
                                 resolution_parameter=resolution_range[0], consider_comms=consider_comms);
  bisect_values[resolution_range[0]] = BisectPartition(partition=partition,
                              bisect_value=bisect_func(partition));
  partition = find_partition(graph=graph, method=method, weight=weight,
                                 resolution_parameter=resolution_range[1], consider_comms=consider_comms);
  bisect_values[resolution_range[1]] = BisectPartition(partition=partition,
                              bisect_value=bisect_func(partition));
  # While stack of ranges not yet empty
  while stack_res_range:
    # Get the current range from the stack
    current_range = stack_res_range.pop();
    # Get the difference in bisection values
    diff_bisect_value = abs(bisect_values[current_range[0]].bisect_value -
                            bisect_values[current_range[1]].bisect_value);
    # Get the difference in resolution parameter (in log space if 0 is not in
    # the interval (assuming only non-negative resolution parameters).
    if current_range[0] > 0 and current_range[1] > 0 and not linear_bisection:
      diff_resolution = log(current_range[1]/current_range[0]);
    else:
      diff_resolution = abs(current_range[1] - current_range[0]);
    # Check if we still want to scan a smaller interval
    logging.info('Range=[{0}, {1}], diff_res={2}, diff_bisect={3}'.format(
        current_range[0], current_range[1], diff_resolution, diff_bisect_value));
    # If we would like to bisect this interval
    if diff_bisect_value > min_diff_bisect_value and \
       diff_resolution > min_diff_resolution:
      # Determine new resolution value
      if current_range[0] > 0 and current_range[1] > 0 and not linear_bisection:
        new_res = sqrt(current_range[1]*current_range[0]);
      else:
        new_res = sum(current_range)/2.0;
      # Bisect left (push on stack)
      stack_res_range.append((current_range[0], new_res));
      # Bisect right (push on stack)
      stack_res_range.append((new_res, current_range[1]));
      # If we haven't scanned this resolution value yet,
      # do so now
      if not bisect_values.has_key(new_res):
        partition = find_partition(graph, method=method, weight=weight,
                                       resolution_parameter=new_res, consider_comms=consider_comms);
        bisect_values[new_res] = BisectPartition(partition=partition,
                                    bisect_value=bisect_func(partition));
        logging.info('Resolution={0}, Resolution Value={1}'.format(new_res,
          bisect_func(partition)));
      # Because of stochastic differences in different runs, the monotonicity
      # of the bisection values might be violated, so check for any
      # inconsistencies
      ensure_monotonicity(bisect_values, new_res);
  # Ensure we only keep those resolution values for which
  # the bisection values actually changed, instead of all of them
  clean_stepwise(bisect_values);
  # Use an ordered dict so that when iterating over it, the results appear in
  # increasing order based on the resolution value.
  return OrderedDict(sorted(((res, part) for res, part in
    bisect_values.iteritems()), key=lambda x: x[0]));

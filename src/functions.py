import igraph as _ig
from . import _c_louvain
from ._c_louvain import ALL_COMMS
from ._c_louvain import ALL_NEIGH_COMMS
from ._c_louvain import RAND_COMM
from ._c_louvain import RAND_NEIGH_COMM
from ._c_louvain import RAND_WEIGHT_NEIGH_COMM

from collections import namedtuple
from collections import OrderedDict
import logging
from math import log, sqrt

import sys
# Check if working with Python 3
PY3 = (sys.version > '3');

def _get_py_capsule(graph):
  if PY3:
    return graph.__graph_as_capsule();
  else:
    return graph.__graph_as_cobject();

from .VertexPartition import *
from .Optimiser import *

class Layer:
  """
  This class makes sure that each layer is properly encoded for use in the
  ``find_partition_multiplex`` function.

  Keyword arguments:

  graph
    The graph for this layer.

  partition_type
    The type of partition used for this layer (see package documentation for details).

  layer_weight
    The weight used for weighing this layer in the overall quality.

  initial_membership
    The initial membership to start the optimization with.

  weight
    The edge attribute from which to use the weight for this layer.

  resolution_parameter
    The resolution parameter used for this layer.

  """
  def __init__(self, graph, partition_type, layer_weight=1.0, initial_membership=None, weight=None, resolution_parameter=1.0, **kwargs):
    self.graph = graph;
    self.partition_type = partition_type;
    self.layer_weight = layer_weight;
    self.initial_membership = initial_membership;
    self.weight = weight;
    self.resolution_parameter = resolution_parameter;
    self.kwargs = kwargs;

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
  args = []
  partitions = [];
  layer_weights = [];
  for layer in layers:
    if layer.weight is not None:
      if isinstance(layer.weight, str):
        layer.weight = layer.graph.es[layer.weight];
      else:
        # Make sure it is a list
        layer.weight = list(layer.weight);
    partition = layer.partition_type(layer.graph,
                                     layer.method,
                                     weight=layer.weight,
                                     initial_membership=layer.initial_membership,
                                     **layer.kwargs);
    layer_weights.append(layer.layer_weight);
    partitions.append(partition);
  optimiser = Optimiser();
  quality = optimiser.find_partition_multiplex(partitions, layer_weights);
  return partitions[0].membership, quality;

def find_partition(graph, partition_type,
    initial_membership=None, weight=None, consider_comms=ALL_NEIGH_COMMS, **kwargs):
  """
  Method for detecting communities using the Louvain algorithm. This functions
  finds the optimal partition given the specified method. For the various possible
  methods see package documentation.

  Keyword arguments:

  graph
    The graph for which to find the optimal partition.

  partition_type
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
  partition = partition_type(graph,
                             initial_membership=initial_membership,
                             weight=weight,
                             **kwargs);
  optimiser = Optimiser();
  optimiser.consider_comms = consider_comms;
  optimiser.optimize_partition(partition);
  return partition;

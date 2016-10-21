import igraph as _ig
from . import _c_louvain
from ._c_louvain import ALL_COMMS
from ._c_louvain import ALL_NEIGH_COMMS
from ._c_louvain import RAND_COMM
from ._c_louvain import RAND_NEIGH_COMM

from ._c_louvain import MOVE_NODES
from ._c_louvain import MERGE_NODES

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

  returns: optimised partition."""
  partition = partition_type(graph,
                             initial_membership=initial_membership,
                             weight=weight,
                             **kwargs);
  optimiser = Optimiser();
  optimiser.consider_comms = consider_comms;
  optimiser.optimise_partition(partition);
  return partition;

def set_rng_seed(seed):
  _c_louvain._set_rng_seed(seed);

def find_partition_multiplex(graphs, partition_type, **kwargs):
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
  n_layers = len(graphs);
  partitions = [];
  layer_weights = [1]*n_layers;
  for graph in graphs:
    partitions.append(partition_type(graph, **kwargs));
  optimiser = Optimiser();
  quality = optimiser.optimise_partition_multiplex(partitions, layer_weights);
  return partitions[0].membership, quality;

def find_partition_temporal(H_time_slices, partition_type,
                            interslice_weight=1,
                            optimiser=None,
                            slice_attr='slice', vertex_id_attr='id',
                            edge_type_attr='type', weight_attr='weight',
                            **kwargs):
  # Create layers
  G_layers, G_interslice, G = time_slice_to_layer_graph(H_time_slices,
                                                        interslice_weight,
                                                        slice_attr,
                                                        vertex_id_attr,
                                                        edge_type_attr,
                                                        weight_attr);
  # Optimise partitions
  partitions = [partition_type(H, node_sizes='node_size', weight=weight_attr, **kwargs) for H in G_layers];
  # We can always take the same interslice partition, as this should have no cost in the optimisation.
  partition_interslice = CPMVertexPartition(G_interslice, resolution_parameter=0, node_sizes='node_size', weight=weight_attr);
  if optimiser is None:
    optimiser = Optimiser();
  optimiser.optimise_partition_multiplex(partitions + [partition_interslice]);
  # Transform results back into original form.
  membership = {(v[slice_attr], v[vertex_id_attr]): m for v, m in zip(G.vs, partitions[0].membership)}

  membership_time_slices = [];
  for slice_idx, H in enumerate(H_time_slices):
    membership_slice = [membership[(slice_idx, v[vertex_id_attr])] for v in H.vs];
    membership_time_slices.append(list(membership_slice));
  return membership_time_slices;

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
# These are helper functions to create a proper
# disjoint union in python. The igraph implementation
# currently does not keep the attributes when creating
# a disjoint_union.
def get_attrs_or_nones(seq, attr_name):
  try:
    return seq[attr_name]
  except KeyError:
    return [None] * len(seq)

def disjoint_union_attrs(graphs):
  G = _ig.Graph.disjoint_union(graphs[0], graphs[1:]);

  vertex_attributes = set(sum([H.vertex_attributes() for H in graphs], []));
  edge_attributes = set(sum([H.edge_attributes() for H in graphs], []));

  for attr in vertex_attributes:
    attr_value = sum([get_attrs_or_nones(H.vs, attr) for H in graphs], []);
    G.vs[attr] = attr_value;
  for attr in edge_attributes:
    attr_value = sum([get_attrs_or_nones(H.es, attr) for H in graphs], []);
    G.es[attr] = attr_value;

  return G;

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
# Conversion to layer graphs
def slice_graph_to_layer_graph(G_slices,
                               slice_attr='slice',
                               vertex_id_attr='id',
                               edge_type_attr='type',
                               weight_attr='weight'):
  """
  This function converts a graph of slices to layers so that they
  can be used with the louvain package. This function assumes that the slices are
  represented by nodes in the slice graph (i.e. G_slices), and stored in the
  attribute `slice_attr`. In other words, G_slices.vs[slice_attr] should be a list
  of graphs. The slices will be converted to layers, and nodes in different slices
  will be coupled if the two slices are connected in G_slices. The weight of the
  coupling is determined by the weight of this link in G_slices.
  """
  ##%%

  # Create disjoint union of the time graphs
  for v_slice in G_slices.vs:
    H = v_slice[slice_attr];
    H.vs[slice_attr] = v_slice.index;

  G = disjoint_union_attrs(G_slices.vs[slice_attr]);
  G.es[edge_type_attr] = 'intraslice';
  ##%%
  for v_slice in G_slices.vs:
    for u_slice in v_slice.neighbors(mode=_ig.OUT):
      if v_slice.index < u_slice.index or G_slices.is_directed():
        nodes_v = G.vs.select(lambda v: v[slice_attr]==v_slice.index)[vertex_id_attr];
        if len(set(nodes_v)) != len(nodes_v):
          err = '\n'.join(
            ['\t{0} {1} times'.format(item, count) for item, count in Counter(nodes_v).items() if count > 1]
            );
          raise ValueError('No unique IDs for slice {0}, require unique IDs:\n{1}'.format(v_slice.index, err));
        nodes_u = G.vs.select(lambda v: v[slice_attr]==u_slice.index)[vertex_id_attr];
        if len(set(nodes_u)) != len(nodes_u):
          err = '\n'.join(
            ['\t{0} {1} times'.format(item, count) for item, count in Counter(nodes_u).items() if count > 1]
            );
          raise ValueError('No unique IDs for slice {0}, require unique IDs:\n{1}'.format(u_slice.index, err));
        common_nodes = set(nodes_v).intersection(set(nodes_u));
        nodes_v = sorted([v for v in G.vs if v[slice_attr] == v_slice.index and v[vertex_id_attr] in common_nodes], key=lambda v: v[vertex_id_attr]);
        nodes_u = sorted([v for v in G.vs if v[slice_attr] == u_slice.index and v[vertex_id_attr] in common_nodes], key=lambda v: v[vertex_id_attr]);
        edges = zip(nodes_v, nodes_u);
        e_start = G.ecount();
        G.add_edges(edges);
        e_end = G.ecount();
        e_idx = range(e_start, e_end);
        interslice_weight = G_slices.es[G_slices.get_eid(v_slice, u_slice)][weight_attr];
        G.es[e_idx][weight_attr] = interslice_weight;
        G.es[e_idx][edge_type_attr] = 'interslice';

  ##%%
  # Convert aggregate graph to individual layers for each time slice.
  G_layers = [None]*G_slices.vcount();
  for v_slice in G_slices.vs:
    H = G.subgraph_edges(G.es.select(_within=[v.index for v in G.vs if v[slice_attr] == v_slice.index]), delete_vertices=False);
    H.vs['node_size'] = [1 if v[slice_attr] == v_slice.index else 0 for v in H.vs];
    G_layers[v_slice.index] = H;

  # Create one graph for the interslice links.
  G_interslice = G.subgraph_edges(G.es.select(type_eq='interslice'), delete_vertices=False);
  G_interslice.vs['node_size'] = 0;
  ##%%
  return G_layers, G_interslice, G;

def time_slice_to_layer_graph(H_time_slices,
                              interslice_weight=1,
                              slice_attr='slice',
                              vertex_id_attr='id',
                              edge_type_attr='type',
                              weight_attr='weight'):
  G_slices = _ig.Graph.Tree(len(H_time_slices), 1, mode=_ig.TREE_UNDIRECTED);
  G_slices.es[weight_attr] = interslice_weight;
  G_slices.vs[slice_attr] = H_time_slices;
  return slice_graph_to_layer_graph(G_slices,
                                    slice_attr,
                                    vertex_id_attr,
                                    edge_type_attr,
                                    weight_attr);

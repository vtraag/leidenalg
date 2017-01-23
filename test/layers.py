import igraph as ig;
import numpy as np;
from collections import Counter;
import louvain
import pandas as pd;
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
  G = ig.Graph.disjoint_union(graphs[0], graphs[1:]);
  
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
# This functions 
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
    for u_slice in v_slice.neighbors(mode=ig.OUT):
      if v_slice.index < u_slice.index or G_slices.is_directed():
        nodes_v = G.vs.select(lambda v: v[slice_attr]==v_slice.index)[vertex_id_attr];
        if len(set(nodes_v)) != len(nodes_v):
          err = '\n'.join(
            ['\t{0} {1} times'.format(item, count) for item, count in Counter(nodes_v).items() if count > 1]
            );  
          raise Exception('No unique IDs for slice {0}, require unique IDs:\n{1}'.format(v_slice.index, err));
        nodes_u = G.vs.select(lambda v: v[slice_attr]==u_slice.index)[vertex_id_attr];
        if len(set(nodes_u)) != len(nodes_u):
          err = '\n'.join(
            ['\t{0} {1} times'.format(item, count) for item, count in Counter(nodes_u).items() if count > 1]
            );
          raise Exception('No unique IDs for slice {0}, require unique IDs:\n{1}'.format(u_slice.index, err));
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
    H.vs['node_size'] = (np.array(H.vs[slice_attr]) == v_slice.index).astype(int);
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
  G_slices = ig.Graph.Tree(len(H_time_slices), 1, mode=ig.TREE_UNDIRECTED);
  G_slices.es[weight_attr] = interslice_weight;
  G_slices.vs[slice_attr] = H_time_slices;
  return slice_graph_to_layer_graph(G_slices, 
                                    slice_attr, 
                                    vertex_id_attr,
                                    edge_type_attr, 
                                    weight_attr);

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
  partition_interslice = louvain.CPMVertexPartition(G_interslice, resolution_parameter=0, node_sizes='node_size', weight=weight_attr);
  if optimiser is None:
    optimiser = louvain.Optimiser();
  optimiser.optimise_partition_multiplex(partitions + [partition_interslice]);
  # Transform results back into original form.  
  G.vs['community'] = partitions[0].membership;
  membership_df = pd.DataFrame({attr: G.vs[attr] for attr in G.vertex_attributes()})\
        .set_index([slice_attr, vertex_id_attr]);
  membership_time_slices = [];
  for slice_idx, H in enumerate(H_time_slices):
    membership = membership_df.loc[slice_idx].loc[H.vs[vertex_id_attr], 'community'];
    membership_time_slices.append(list(membership));
  return membership_time_slices;                                


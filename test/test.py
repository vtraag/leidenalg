import igraph as ig
import louvain
import unittest
import numpy as np
import pandas as pd
import sys
#%%
opt = louvain.Optimiser();
opt.consider_empty_community = True;
print(opt.consider_empty_community);
opt.consider_empty_community = False;
print(opt.consider_empty_community);
sys.exit();
#%%
#G = ig.Graph.Famous('Zachary');
n = 20;
k = 3;
G = ig.Graph.Erdos_Renyi(n, m=k*n);
G.es['weight'] = 2*np.random.rand(G.ecount())-1;
#%%
G_pos = G.subgraph_edges(G.es.select(weight_gt = 0), delete_vertices=False);
G_neg = G.subgraph_edges(G.es.select(weight_lt = 0), delete_vertices=False);
G_neg.es['weight'] = [-w for w in G_neg.es['weight']];
#%%
optimiser = louvain.Optimiser();
part_pos = louvain.ModularityVertexPartition(G_pos, weights='weight');
part_neg = louvain.ModularityVertexPartition(G_neg, weights='weight');
optimiser.optimise_partition_multiplex(
  [part_pos, part_neg],
  layer_weights=[1,-1]);

#%%
optimiser = louvain.Optimiser();
partition = louvain.ModularityVertexPartition(G, weights='weight');
improv = 1;
while improv > 0:
  improv = optimiser.optimise_partition(partition);
  print(partition.quality())

#%%
partition = louvain.ModularityVertexPartition(G);
partition_agg = partition.aggregate_partition();
while optimiser.move_nodes(partition_agg):
  partition.from_coarse_partition(partition_agg);
  partition_agg = partition_agg.aggregate_partition();
#%%
G = ig.Graph.Famous('Zachary');
optimiser = louvain.Optimiser();
partition = louvain.ModularityVertexPartition(G);
optimiser.optimise_partition(partition);
refined_partition = louvain.ModularityVertexPartition(G);
optimiser.move_nodes_constrained(refined_partition, constrained_partition=partition);
partition_agg = refined_partition.aggregate_partition(membership_partition=partition);

#%%
nb_moves = 1;
while nb_moves > 0:
  nb_moves = 0;

#%%

in_comm = [0]*len(partition);
degree_in_comm = [0]*len(partition);
degree_out_comm = [0]*len(partition);
for e in G.es:
  c1 = partition.membership[e.source];
  c2 = partition.membership[e.target];
  if (c1 == c2):
      in_comm[c1] += 1;
  degree_out_comm[c1] += 1;
  degree_in_comm[c2] += 1;

m = float(G.ecount());
s = 0;
for c in range(len(partition)):
  tmp1 = degree_out_comm[c]/(m);
  tmp2 = degree_in_comm[c]/(m);
  s += in_comm[c]/(m);
  s -= tmp1*tmp2;

s#%%
s = sum([partition.total_weight_in_comm(c) for c, _ in enumerate(partition)])

#%%
G = ig.Graph.Famous('Zachary');
optimal_partition = G.community_optimal_modularity();
layout = G.layout_fruchterman_reingold()
#%%
partition = louvain.find_partition(G, louvain.ModularityVertexPartition);
ig.plot(partition, layout=layout)
#%%
partition = louvain.find_partition(G, louvain.CPMVertexPartition, resolution_parameter=0.05)
ig.plot(partition, layout=layout)
#%%
G = ig.Graph.Famous('Zachary');
optimiser = louvain.Optimiser();
profile = optimiser.resolution_profile(G, louvain.CPMVertexPartition, resolution_range=(0,1));
#%%
ig.plot(profile[1].partition)
#%%
partition = louvain.CPMVertexPartition(G, resolution_parameter=p);
print optimiser.merge_nodes(partition)
#%%
partition = louvain.CPMVertexPartition(G, resolution_parameter=p);
print optimiser.merge_nodes(partition)
#%%
res_param = G.density();
partition = louvain.CPMVertexPartition(G, resolution_parameter=res_param);
H = G.subgraph_edges([], delete_vertices=False);
H.vs['node_sizes'] = [1 if v.index < n/3 else 0 for v in H.vs];
partition_1 = louvain.CPMVertexPartition(H, node_sizes=H.vs['node_sizes'], resolution_parameter=res_param);
H = G.subgraph_edges([], delete_vertices=False);
H.vs['node_sizes'] = [1 if v.index > 2*n/3 else 0 for v in H.vs];
partition_2 = louvain.CPMVertexPartition(H, node_sizes=H.vs['node_sizes'], resolution_parameter=res_param);
#%%
opt = louvain.Optimiser();
opt.optimise_partition_multiplex(
  [partition, partition_1, partition_2],  
  layer_weights=[1,1,1]);
partition.quality()
#%%
G = ig.Graph.Famous('Zachary');
layout = G.layout_auto();
#%%
partition = louvain.find_partition(G, louvain.CPMVertexPartition, resolution_parameter=0.06);
q = partition.quality();
##%%
prev_q = 0
while prev_q < q:
  prev_q = q;
  q = opt.optimise_partition(partition);
##%%
ig.plot(partition, layout=layout)
#%%
G = ig.Graph.Full(100);
#%%
partition = louvain.CPMVertexPartition(G, resolution_parameter=0.5);
opt = louvain.Optimiser();
opt.move_nodes(partition, consider_comms=louvain.ALL_NEIGH_COMMS);
#%%
partition = louvain.CPMVertexPartition(G, resolution_parameter=0.5);
#%%
opt = louvain.Optimiser();
opt.merge_nodes(partition, consider_comms=louvain.ALL_NEIGH_COMMS);
#%%
partition = louvain.CPMVertexPartition(G, resolution_parameter=0.5);
opt = louvain.Optimiser();
opt.merge_nodes(partition, consider_comms=louvain.RAND_NEIGH_COMM);
#%%
partition = louvain.CPMVertexPartition(G, resolution_parameter=0.5);
opt = louvain.Optimiser();
opt.move_nodes(partition, consider_comms=louvain.ALL_NEIGH_COMMS);

#%%
partition = louvain.CPMVertexPartition(G, resolution_parameter=0.1);
while 0 < opt.move_nodes(partition, consider_comms=louvain.ALL_NEIGH_COMMS):
  pass;
for v in G.vs:
  neigh_comms = set(partition.membership[u.index] for u in v.neighbors());
  for c in neigh_comms:
    if partition.diff_move(v.index, c) > 0:
      print v, c;
      
#%%
G = reduce(ig.Graph.disjoint_union, (ig.Graph.Tree(10, 3, mode=ig.TREE_UNDIRECTED) for i in range(10)));
partition = louvain.CPMVertexPartition(G, resolution_parameter=0);
#%%
G = ig.Graph.Full_Bipartite(50, 50);
G.es['weight'] = -0.1;
partition = louvain.CPMVertexPartition(G, resolution_parameter=-.1, weight='weight');
opt.consider_comms=louvain.ALL_COMMS;
opt.optimise_partition(partition);

#%%
optimiser = louvain.Optimiser();
n = 10;
n_slices = 50;
interslice_weight = 1;

H_time_slices = [];
for idx in range(n_slices):
  #H.vs['id'] = range(idx, n + idx);
  H = ig.Graph.Lattice([n], 1)
  H.vs['id'] = range(n);
  H.es['weight'] = 1;
  H_time_slices.append(H);

gamma = 0.5;
membership_time_slices = louvain.find_partition_temporal(H_time_slices, louvain.CPMVertexPartition, 
                                                 interslice_weight=interslice_weight, resolution_parameter=gamma);
#%%            
G_layers, G_interslice, G = louvain.time_slice_to_layer_graph(H_time_slices);
partitions = [louvain.CPMVertexPartition(H, node_sizes='node_size', weight='weight', resolution_parameter=gamma) for H in G_layers];
partition_interslice = louvain.CPMVertexPartition(G_interslice, resolution_parameter=0, node_sizes='node_size', weight='weight'); 
#%%
optimiser.optimise_partition_multiplex(partitions + [partition_interslice])

#%%

membership = {(v['slice'], v['id']): m for v, m in zip(G.vs, partitions[0].membership)}

membership_time_slices = [];
for slice_idx, H in enumerate(H_time_slices):
  membership_slice = [membership[(slice_idx, v['id'])] for v in H.vs];
  membership_time_slices.append(list(membership_slice));

#%%
G_1 = ig.Graph.Tree(20, 3);
G_2 = ig.Graph.Tree(10, 3);
louvain.find_partition_multiplex([G_1, G_2], louvain.ModularityVertexPartition)

#%% Unidrected graphs
n_list = np.logspace(2, 5, 10).astype(int);
repl = 10;
k = 10;
results = [];
opt = louvain.Optimiser();
for n in n_list:
  print('Testing n={0}'.format(n));
  G = ig.Graph.Erdos_Renyi(n, m=k*n);
  for repl_idx in range(repl):
    partition = louvain.SignificanceVertexPartition(G);
    opt.optimise_partition(partition);
    results.append([n, repl_idx, partition.quality()]);

result_df = pd.DataFrame(results, columns=['n', 'run', 'Significance']);
result_df = result_df.groupby(['n'])['Significance'].mean().reset_index();

import matplotlib.pyplot as plt
n = result_df['n'];
plt.plot(n, result_df['Significance']);
plt.plot(n, 0.5*n*np.log(n));
#%% Directed graphs
n_list = np.logspace(2, 5, 10).astype(int);
repl = 10;
k = 10;
results = [];
opt = louvain.Optimiser();
for n in n_list:
  print('Testing n={0}'.format(n));
  G = ig.Graph.Erdos_Renyi(n, m=k*n, directed=True);
  for repl_idx in range(repl):
    partition = louvain.SignificanceVertexPartition(G);
    opt.optimise_partition(partition);
    results.append([n, repl_idx, partition.quality()]);

result_df = pd.DataFrame(results, columns=['n', 'run', 'Significance']);
result_df = result_df.groupby(['n'])['Significance'].mean().reset_index();

import matplotlib.pyplot as plt
n = result_df['n'];
plt.plot(n, result_df['Significance']);
plt.plot(n, 0.5*n*np.log(n));
#%% Unidrected graphs
n_list = np.logspace(2, 5, 10).astype(int);
repl = 10;
k = 10;
results = [];
opt = louvain.Optimiser();
for n in n_list:
  print('Testing n={0}'.format(n));
  G = ig.Graph.Erdos_Renyi(n, m=k*n);
  for repl_idx in range(repl):
    partition = louvain.SurpriseVertexPartition(G);
    opt.optimise_partition(partition);
    results.append([n, repl_idx, partition.quality()]);

result_df = pd.DataFrame(results, columns=['n', 'run', 'Surprise']);
result_df = result_df.groupby(['n'])['Surprise'].mean().reset_index();

import matplotlib.pyplot as plt
n = result_df['n'];
plt.plot(n, result_df['Surprise']);
plt.plot(n, 0.5*n*np.log(n));
#%%

G = ig.Graph.Famous('Zachary');
optimiser = louvain.Optimiser();
profile = optimiser.resolution_profile(G, louvain.CPMVertexPartition, resolution_range=(0,1), min_diff_resolution=1e-12, until_stable=True);
#%%
import matplotlib.pyplot as plt
plt.figure(figsize=(10, 6));
plt.step([p.resolution_parameter for p in profile], [p.total_weight_in_all_comms() for p in profile])
plt.xscale('log')
plt.xlabel('Resolution parameter')
plt.ylabel('Total internal edges')
#%%
louvain.SignificanceVertexPartition.FromPartition(profile[3]).quality()
#%%
plt.figure(figsize=(10, 6));
plt.step([p.resolution_parameter for p in profile], [len(p) for p in profile])
plt.xscale('log')
plt.xlabel('Resolution parameter')
plt.ylabel('Total internal edges')

#%%
res = np.linspace(0, 0.05, 100)
plt.plot(res, [profile[0].quality(r) for r in res])
plt.plot(res, [profile[1].quality(r) for r in res])
plt.legend(['res0', 'res1'])
#%%
resolution_range = (0,1);
partition_left = louvain.find_partition(G, louvain.CPMVertexPartition, resolution_parameter=resolution_range[0]);
partition_right = louvain.find_partition(G, louvain.CPMVertexPartition, resolution_parameter=resolution_range[1]);
#%%
partition_left.resolution_parameter = resolution_range[1];
#%%
G = ig.Graph.Formula('1 -- 2 -- 3');

#%%
n = 100;
G_1 = ig.Graph.Lattice([n], 1);
G_1.vs['id'] = range(n);
G_2 = ig.Graph.Lattice([n], 1);
G_2.vs['id'] = range(n);
membership, improvement = louvain.find_partition_temporal([G_1, G_2], 
                                                         louvain.CPMVertexPartition, 
                                                         resolution_parameter=1.0/n,
                                                         interslice_weight=1);
                                                          
membership = np.array(membership).T;

#%%
G = ig.Graph.Famous('Zachary');
partition = louvain.ModularityVertexPartition(G);
diff = partition.move_node(0, 1)

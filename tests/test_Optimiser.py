import unittest
import igraph as ig
import leidenalg

import sys
PY3 = (sys.version > '3');

if PY3:
  from functools import reduce

class OptimiserTest(unittest.TestCase):

  def setUp(self):
    self.optimiser = leidenalg.Optimiser();

  def test_move_nodes(self):
    G = ig.Graph.Full(100);
    partition = leidenalg.CPMVertexPartition(G, resolution_parameter=0.5);
    self.optimiser.move_nodes(partition, consider_comms=leidenalg.ALL_NEIGH_COMMS);
    self.assertListEqual(
        partition.sizes(), [100],
        msg="CPMVertexPartition(resolution_parameter=0.5) of complete graph after move nodes incorrect.");

  def test_merge_nodes(self):
    G = ig.Graph.Full(100);
    partition = leidenalg.CPMVertexPartition(G, resolution_parameter=0.5);
    self.optimiser.merge_nodes(partition, consider_comms=leidenalg.ALL_NEIGH_COMMS);
    self.assertListEqual(
        partition.sizes(), [100],
        msg="CPMVertexPartition(resolution_parameter=0.5) of complete graph after merge nodes incorrect.");
    self.assertEqual(
        partition.total_weight_in_all_comms(),
        G.ecount(),
        msg="total_weight_in_all_comms not equal to ecount of graph.");

  def test_diff_move_node_optimality(self):
    G = ig.Graph.Erdos_Renyi(100, p=5./100, directed=False, loops=False);
    partition = leidenalg.CPMVertexPartition(G, resolution_parameter=0.1);
    while 0 < self.optimiser.move_nodes(partition, consider_comms=leidenalg.ALL_NEIGH_COMMS):
      pass;
    for v in G.vs:
      neigh_comms = set(partition.membership[u.index] for u in v.neighbors());
      for c in neigh_comms:
        self.assertLessEqual(
          partition.diff_move(v.index, c), 1e-10, # Allow for a small difference up to rounding error.
          msg="Was able to move a node to a better community, violating node optimality.");

  def test_optimiser(self):
    G = reduce(ig.Graph.disjoint_union, (ig.Graph.Tree(10, 3, mode=ig.TREE_UNDIRECTED) for i in range(10)));
    partition = leidenalg.CPMVertexPartition(G, resolution_parameter=0);
    self.optimiser.consider_comms=leidenalg.ALL_NEIGH_COMMS;
    self.optimiser.optimise_partition(partition);
    self.assertListEqual(
        partition.sizes(), 10*[10],
        msg="After optimising partition failed to find different components with CPMVertexPartition(resolution_parameter=0)");

  def test_neg_weight_bipartite(self):
    G = ig.Graph.Full_Bipartite(50, 50);
    G.es['weight'] = -0.1;
    partition = leidenalg.CPMVertexPartition(G, resolution_parameter=-0.1, weights='weight');
    self.optimiser.consider_comms=leidenalg.ALL_COMMS;
    self.optimiser.optimise_partition(partition);
    self.assertListEqual(
        partition.sizes(), 2*[50],
        msg="After optimising partition failed to find bipartite structure with CPMVertexPartition(resolution_parameter=-0.1)");

  def test_resolution_profile(self):
    G = ig.Graph.Famous('Zachary');
    profile = self.optimiser.resolution_profile(G, leidenalg.CPMVertexPartition, resolution_range=(0,1));
    self.assertListEqual(
      profile[0].sizes(), [G.vcount()],
      msg="Resolution profile incorrect: at resolution 0, not equal to a single community for CPM.");
    self.assertListEqual(
      profile[-1].sizes(), [1]*G.vcount(),
      msg="Resolution profile incorrect: at resolution 1, not equal to a singleton partition for CPM.");

#%%
if __name__ == '__main__':
  #%%
  unittest.main(verbosity=3);
  suite = unittest.TestLoader().discover('.');
  unittest.TextTestRunner(verbosity=1).run(suite);

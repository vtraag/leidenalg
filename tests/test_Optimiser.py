import unittest
import igraph as ig
import louvain

class BaseTest:
  @ddt
  class OptimiserTest(unittest.TestCase):

    def setUp(self):
      self.optimiser = louvain.Optimiser();

    def test_move_nodes():
      G = ig.Graph.Full(100);
      partition = louvain.CPMVertexPartition(G, resolution_parameter=0.5);
      opt = louvain.Optimiser();
      opt.move_nodes(partition);
      self.assertAlmostEqual(
          partition.quality(), 0,
          places=5,
          msg="Quality of CPMPartition(resolution_parameter=0.5) of complete graph after move nodes is not equal to 0.0.".format(
          q2 - q1, diff));

#%%
if __name__ == '__main__':
  #%%
  unittest.main(verbosity=3);
  suite = unittest.TestLoader().discover('.');
  unittest.TextTestRunner(verbosity=1).run(suite);

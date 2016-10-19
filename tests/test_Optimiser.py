import unittest
import igraph as ig
import louvain

class BaseTest:
  @ddt
  class OptimiserTest(unittest.TestCase):

    def setUp(self):
      self.optimiser = louvain.Optimiser();

#%%
if __name__ == '__main__':
  #%%
  unittest.main(verbosity=3);
  suite = unittest.TestLoader().discover('.');
  unittest.TextTestRunner(verbosity=1).run(suite);

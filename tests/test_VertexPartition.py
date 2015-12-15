import unittest
import igraph as ig
import louvain

from ddt import ddt, data, unpack

#%%
  
graphs = [
    ig.Graph.Famous('Zachary'), 
    ig.Graph.Erdos_Renyi(100, p=1./100)
    ];
    
class BaseTest:
  @ddt
  class MutableVertexPartitionTest(unittest.TestCase):
    
    def setUp(self):
      self.optimiser = louvain.Optimiser();
    
    @data(*graphs)
    def test_move_nodes(self, graph):
      partition = self.partition_type(graph);
      v = 0;
      while (graph.degree(v) == 0):
        v += 1;
      u = graph.neighbors(v)[0];
      q1 = partition.quality();
      diff = partition.diff_move(v, partition.membership[u]);
      partition.move_node(v, partition.membership[u]);
      q2 = partition.quality();
      self.assertAlmostEqual(q2 - q1, diff,
                       "Difference in quality ({0}) not equal to calculated difference ({1})".format(q2 - q1, diff));

    @data(*graphs)
    def test_aggregate_partition(self, graph):
      partition = self.partition_type(graph);
      self.optimiser.move_nodes(partition);
      aggregate_partition = partition.aggregate_partition();
      self.assertAlmostEqual(partition.quality(), aggregate_partition.quality(),
                       'Quality not equal for aggregate partition.');
      self.optimiser.move_nodes(aggregate_partition);
      partition.from_coarser_partition(aggregate_partition);
      self.assertAlmostEqual(partition.quality(), aggregate_partition.quality(),
                       'Quality not equal from coarser partition.');    
  
class ModularityVertexPartitionTest(BaseTest.MutableVertexPartitionTest):
  def setUp(self):
    super(ModularityVertexPartitionTest, self).setUp();
    self.partition_type = louvain.ModularityVertexPartition;

class RBERVertexPartitionTest(BaseTest.MutableVertexPartitionTest):
  def setUp(self):
    super(RBERVertexPartitionTest, self).setUp();
    self.partition_type = louvain.RBERVertexPartition;
    
class RBConfigurationVertexPartitionTest(BaseTest.MutableVertexPartitionTest):
  def setUp(self):
    super(RBConfigurationVertexPartitionTest, self).setUp();
    self.partition_type = louvain.RBConfigurationVertexPartition;
    
class CPMVertexPartitionTest(BaseTest.MutableVertexPartitionTest):
  def setUp(self):
    super(CPMVertexPartitionTest, self).setUp();
    self.partition_type = louvain.CPMVertexPartition;
    
class SurpriseVertexPartitionTest(BaseTest.MutableVertexPartitionTest):
  def setUp(self):
    super(SurpriseVertexPartitionTest, self).setUp();
    self.partition_type = louvain.SurpriseVertexPartition;
    
class SignificanceVertexPartitionTest(BaseTest.MutableVertexPartitionTest):
  def setUp(self):
    super(SignificanceVertexPartitionTest, self).setUp();
    self.partition_type = louvain.SignficanceVertexPartition;

#%%
if __name__ == '__main__':
  #%%
  #unittest.main(verbosity=3);
  suite = unittest.TestLoader().discover('.');
  unittest.TextTestRunner(verbosity=2).run(suite);

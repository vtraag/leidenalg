import unittest
import igraph as ig
import louvain

#%%


class BaseTest:
  class MutableVertexPartitionTest(unittest.TestCase):
    
    def setUp(self):
      self.graph = ig.Graph.Famous('Zachary');
      self.optimiser = louvain.Optimiser();
      
    def test_aggregate_partition(self):
      partition = self.partition_type(self.graph);
      self.optimiser.move_nodes(partition);
      aggregate_partition = partition.aggregate_partition();
      self.assertEqual(partition.quality(), aggregate_partition.quality(),
                       'Quality not equal for aggregate partition.');
      self.optimiser.move_nodes(aggregate_partition);
      partition.from_coarser_partition(aggregate_partition);
      self.assertEqual(partition.quality(), aggregate_partition.quality(),
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

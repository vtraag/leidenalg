import unittest
import igraph as ig
import leidenalg
import random
from copy import deepcopy

from ddt import ddt, data, unpack

#%%

def name_object(obj, name):
  obj.__name__ = name
  return obj

graphs = [
    ###########################################################################
    # Zachary karate network
    name_object(ig.Graph.Famous('Zachary'),
                'Zachary'),

    ###########################################################################
    # ER Networks
    # Undirected no loop
    name_object(ig.Graph.Erdos_Renyi(100, p=1./100, directed=False, loops=False),
                'ER_k1_undirected_no_loops'),
    name_object(ig.Graph.Erdos_Renyi(100, p=5./100, directed=False, loops=False),
                'ER_k5_undirected_no_loops'),
    # Directed no loop
    name_object(ig.Graph.Erdos_Renyi(100, p=1./100, directed=True, loops=False),
                'ER_k1_directed_no_loops'),
    name_object(ig.Graph.Erdos_Renyi(100, p=5./100, directed=True, loops=False),
                'ER_k5_directed_no_loops'),
    # Undirected loops
    name_object(ig.Graph.Erdos_Renyi(100, p=1./100, directed=False, loops=True),
                'ER_k1_undirected_loops'),
    name_object(ig.Graph.Erdos_Renyi(100, p=5./100, directed=False, loops=True),
                'ER_k5_undirected_loops'),
    # Directed loops
    name_object(ig.Graph.Erdos_Renyi(100, p=1./100, directed=True, loops=True),
                'ER_k1_directed_loops'),
    name_object(ig.Graph.Erdos_Renyi(100, p=5./100, directed=True, loops=True),
                'ER_k5_directed_loops'),

    ###########################################################################
    # Tree
    name_object(ig.Graph.Tree(100, 3, type=ig.TREE_UNDIRECTED),
                'Tree_undirected'),
    name_object(ig.Graph.Tree(100, 3, type=ig.TREE_OUT),
                'Tree_directed_out'),
    name_object(ig.Graph.Tree(100, 3, type=ig.TREE_IN),
                'Tree_directed_in'),

    ###########################################################################
    # Lattice
    name_object(ig.Graph.Lattice([100], nei=3, directed=False, mutual=True, circular=True),
                'Lattice_undirected'),
    name_object(ig.Graph.Lattice([100], nei=3, directed=True, mutual=False, circular=True),
                'Lattice_directed')
    ]

bipartite_graph = name_object(
    ig.Graph.Bipartite([0, 0, 0, 0, 1, 1, 1, 1],
                       [[0, 4],
                        [0, 5],
                        [0, 6],
                        [1, 4],
                        [1, 5],
                        [2, 6],
                        [2, 7],
                        [3, 6],
                        [3, 7],
                        [3, 5]]),
              'bipartite_example')

def make_weighted(G):
  m = G.ecount()
  G.es['weight'] = [random.random() for i in range(G.ecount())]
  G.__name__ += '_weighted'
  return G

graphs += [make_weighted(H) for H in graphs]

class BaseTest:
  @ddt
  class MutableVertexPartitionTest(unittest.TestCase):

    def setUp(self):
      self.optimiser = leidenalg.Optimiser()

    @data(*graphs)
    def test_move_nodes(self, graph):
      if 'weight' in graph.es.attributes() and self.partition_type == leidenalg.SignificanceVertexPartition:
        raise unittest.SkipTest('Significance doesn\'t handle weighted graphs')

      if 'weight' in graph.es.attributes():
        partition = self.partition_type(graph, weights='weight')
      else:
        partition = self.partition_type(graph)
      for v in range(graph.vcount()):
        if graph.degree(v) >= 1:
          u = graph.neighbors(v)[0]
          diff = partition.diff_move(v, partition.membership[u])
          q1 = partition.quality()
          partition.move_node(v, partition.membership[u])
          q2 = partition.quality()
          self.assertAlmostEqual(
              q2 - q1,
              diff,
              places=5,
              msg="Difference in quality ({0}) not equal to calculated difference ({1})".format(
              q2 - q1, diff))

    @data(*graphs)
    def test_aggregate_partition(self, graph):
      if 'weight' in graph.es.attributes() and self.partition_type != leidenalg.SignificanceVertexPartition:
        partition = self.partition_type(graph, weights='weight')
      else:
        partition = self.partition_type(graph)
      self.optimiser.move_nodes(partition)
      aggregate_partition = partition.aggregate_partition()
      self.assertAlmostEqual(
          partition.quality(),
          aggregate_partition.quality(),
          places=5,
          msg='Quality not equal for aggregate partition.')
      self.optimiser.move_nodes(aggregate_partition)
      partition.from_coarse_partition(aggregate_partition)
      self.assertAlmostEqual(
          partition.quality(),
          aggregate_partition.quality(),
          places=5,
          msg='Quality not equal from coarser partition.')

    @data(*graphs)
    def test_total_weight_in_all_comms(self, graph):
      if 'weight' in graph.es.attributes() and self.partition_type != leidenalg.SignificanceVertexPartition:
        partition = self.partition_type(graph, weights='weight')
      else:
        partition = self.partition_type(graph)
      self.optimiser.optimise_partition(partition)
      s = sum([partition.total_weight_in_comm(c) for c,_ in enumerate(partition)])
      self.assertAlmostEqual(
        s,
        partition.total_weight_in_all_comms(),
        places=5,
        msg='Total weight in all communities ({0}) not equal to the sum of the weight in all communities ({1}).'.format(
          s, partition.total_weight_in_all_comms())
        )

    @data(*graphs)
    def test_copy(self, graph):
      if 'weight' in graph.es.attributes() and self.partition_type != leidenalg.SignificanceVertexPartition:
        partition = self.partition_type(graph, weights='weight')
      else:
        partition = self.partition_type(graph)

      self.optimiser.optimise_partition(partition)

      partition2 = deepcopy(partition)

      self.assertAlmostEqual(
        partition.quality(),
        partition2.quality(),
        places=5,
        msg='Quality of deepcopy ({0}) not equal to quality of original partition ({1}).'.format(
          partition.quality(), partition2.quality())
        )

      if (partition2.membership[0] == 0):
        partition2.move_node(0, 1)
      else:
        partition2.move_node(0, 0)

      self.assertNotEqual(
        partition.membership[0],
        partition2.membership[0],
        msg='Moving node 0 in the deepcopy to community {0} results in community membership {1} for node 0 also in original partition.'.format(
          partition.membership[0], partition2.membership[0])
        )


class ModularityVertexPartitionTest(BaseTest.MutableVertexPartitionTest):
  def setUp(self):
    super(ModularityVertexPartitionTest, self).setUp()
    self.partition_type = leidenalg.ModularityVertexPartition

class RBERVertexPartitionTest(BaseTest.MutableVertexPartitionTest):
  def setUp(self):
    super(RBERVertexPartitionTest, self).setUp()
    self.partition_type = leidenalg.RBERVertexPartition

class RBConfigurationVertexPartitionTest(BaseTest.MutableVertexPartitionTest):
  def setUp(self):
    super(RBConfigurationVertexPartitionTest, self).setUp()
    self.partition_type = leidenalg.RBConfigurationVertexPartition

class CPMVertexPartitionTest(BaseTest.MutableVertexPartitionTest):
  def setUp(self):
    super(CPMVertexPartitionTest, self).setUp()
    self.partition_type = leidenalg.CPMVertexPartition

  def test_Bipartite(self):
    graph = bipartite_graph
    partition, partition_0, partition_1 = \
        leidenalg.CPMVertexPartition.Bipartite(graph, resolution_parameter_01=0.2)
    self.optimiser.optimise_partition_multiplex(
            [partition, partition_0, partition_1],
            layer_weights=[1, -1, -1])
    self.assertEqual(len(partition), 1)

class SurpriseVertexPartitionTest(BaseTest.MutableVertexPartitionTest):
  def setUp(self):
    super(SurpriseVertexPartitionTest, self).setUp()
    self.partition_type = leidenalg.SurpriseVertexPartition

class SignificanceVertexPartitionTest(BaseTest.MutableVertexPartitionTest):
  def setUp(self):
    super(SignificanceVertexPartitionTest, self).setUp()
    self.partition_type = leidenalg.SignificanceVertexPartition

#%%
if __name__ == '__main__':
  #%%
  unittest.main(verbosity=3)
  suite = unittest.TestLoader().discover('.')
  unittest.TextTestRunner(verbosity=1).run(suite)

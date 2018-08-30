Advanced
========

The basic interface explained in the :ref:`Introduction` should provide you
enough to start detecting communities. However, perhaps you want to improve the
partitions further or want to do some more advanced analysis. In this section,
we will explain this in more detail.

Optimiser
---------

Although the package provides simple access to the function
:func:`~leiden.find_partition`, there is actually an underlying
:class:`~leiden.Optimiser` class that is doing the actual work. We can also
explicitly construct an :class:`~leiden.Optimiser` object:

>>> optimiser = leiden.Optimiser()

The function :func:`~leiden.find_partition` then does nothing else then
calling :func:`~leiden.Optimiser.optimise_partition` on the provided
partition.

.. testsetup::
   
   G = ig.Graph.Erdos_Renyi(100, p=5./100)
   partition = leiden.CPMVertexPartition(G)

>>> diff = optimiser.optimise_partition(partition)

But :func:`~leiden.Optimiser.optimise_partition` simply tries to improve any
provided partition. We can thus try to repeatedly call
:func:`~leiden.Optimiser.optimise_partition` to keep on improving the current
partition:

>>> G = ig.Graph.Erdos_Renyi(100, p=5./100)
>>> partition = leiden.ModularityVertexPartition(G)
>>> improv = 1
>>> while improv > 0: 
...   improv = optimiser.optimise_partition(partition)

Even if a call to :func:`~leiden.Optimiser.optimise_partition` did not improve
the current partition, it is still possible that a next call will improve the
partition. Of course, if the current partition is already optimal, this will
never happen, but it is not possible to decide whether a partition is optimal.

The :func:`~leiden.Optimiser.optimise_partition` itself is built on two other
basic algorithms: :func:`~leiden.Optimiser.move_nodes` and
:func:`~leiden.Optimiser.merge_nodes`. You can also call these functions
yourself. For example:

>>> diff = optimiser.move_nodes(partition)

or

>>> optimiser.merge_nodes(partition);

The usual strategy in the Leiden algorithm is then to aggregate the partition
and repeat the :func:`~leiden.Optimiser.move_nodes` on the aggregated
partition. We can easily repeat that:

>>> partition = leiden.ModularityVertexPartition(G)
>>> while optimiser.move_nodes(partition) > 0: 
...   partition = partition.aggregate_partition()

This summarises the whole Leiden algorithm in just three lines of code.
Although this finds the final aggregate partition, this leaves it unclear the
actual partition on the level of the individual nodes. In order to do that, we
need to update the membership based on the aggregate partition, for which we
use the function
:func:`~leiden.VertexPartition.MutableVertexPartition.from_coarse_partition`.

>>> partition = leiden.ModularityVertexPartition(G)
>>> partition_agg = partition.aggregate_partition()
>>> while optimiser.move_nodes(partition_agg):
...   partition.from_coarse_partition(partition_agg)
...   partition_agg = partition_agg.aggregate_partition()

Now ``partition_agg`` contains the aggregate partition and ``partition``
contains the actual partition of the original graph ``G``. Of course,
``partition_agg.quality() == partition.quality()`` (save some rounding).

Instead of :func:`~leiden.Optimiser.move_nodes`, you could also use
:func:`~leiden.Optimiser.merge_nodes`. These functions depend on choosing
particular alternative communities, the documentation of the functions provides
more detail.

One possibility is that rather than aggregating the partition based on the
current partition, you can first refine the partition and then aggregate it.
This can be done using the functions
:func:`~leiden.Optimiser.move_nodes_constrained` and
:func:`~leiden.Optimiser.merge_nodes_constrained`.

These functions in turn rely on two key functions of the partition:
:func:`~leiden.VertexPartition.MutableVertexPartition.diff_move` and
:func:`~leiden.VertexPartition.MutableVertexPartition.move_node`. The first
calculates the difference when moving a node, and the latter actually moves the
node, and updates all necessary internal administration. The
:func:`~leiden.Optimiser.move_nodes` then does some as follows

>>> for v in G.vs:
...   best_comm = max(range(len(partition)),
...                   key=lambda c: partition.diff_move(v.index, c))
...   partition.move_node(v.index, best_comm)

The actual implementation is more complicated, but this gives the general idea.

Resolution profile
------------------

Some methods accept so-called resolution parameters, such as
:class:`~leiden.CPMVertexPartition` or
:class:`~leiden.RBConfigurationVertexPartition`. Although some method may seem
to have some 'natural' resolution, in reality this is often quite arbitrary.
However, the methods implemented here (which depend in a linear way on
resolution parameters) allow for an effective scanning of a full range for the
resolution parameter. In particular, these methods somehow can be formulated as
:math:`Q = E - \gamma N` where :math:`E` and :math:`N` are some other
quantities. In the case for :class:`~leiden.CPMVertexPartition` for example,
:math:`E = \sum_c m_c` is the number of internal edges and :math:`N = \sum_c
\binom{n_c}{2}` is the sum of the internal possible edges. The essential
insight for these formulations [1]_ is that if there is an optimal partition
for both :math:`\gamma_1` and :math:`\gamma_2` then the partition is also
optimal for all :math:`\gamma_1 \leq \gamma \leq \gamma_2`.

Such a resolution profile can be constructed using the
:class:`~leiden.Optimiser` object. 

>>> G = ig.Graph.Famous('Zachary')
>>> optimiser = leiden.Optimiser()
>>> profile = optimiser.resolution_profile(G, leiden.CPMVertexPartition, 
...                                        resolution_range=(0,1))

Plotting the resolution parameter versus the total number of internal edges we
thus obtain something as follows:

.. image:: figures/resolution_profile.png

Now ``profile`` contains a list of partitions of the specified type
(:class:`~leiden.CPMVertexPartition` in this case) for
resolution parameters at which there was a change. In particular,
``profile[i]`` should be better until ``profile[i+1]``, or stated otherwise for
any resolution parameter between ``profile[i].resolution_parameter`` and
``profile[i+1].resolution_parameter`` the partition at position ``i`` should be
better. Of course, there will be some variations because
:func:`~leiden.Optimiser.optimise_partition` will find partitions of varying
quality. The change points can then also vary for different runs. 

This function repeatedly calls :func:`~leiden.Optimiser.optimise_partition`
and can therefore require a lot of time. Especially for resolution parameters
right around a change point there may be many possible partitions, thus
requiring a lot of runs.

References
----------
.. [1] Traag, V. A., Krings, G., & Van Dooren, P. (2013). Significant scales in
       community structure. Scientific Reports, 3, 2930.  `10.1038/srep02930
       <http://doi.org/10.1038/srep02930>`_

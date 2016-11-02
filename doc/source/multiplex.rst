Multiplex
=========

The implementation of multiplex community detection builds on ideas in [1]_.
The most basic form simply considers two or more graphs which are defined on the
same vertex set, but which have differing edge sets. In this context, each node
is identified with a single community, and cannot have different communities for
different graphs. We call this *layers* of graphs in this context. This format
is actually more flexible than it looks, but you have to construct the layer
graphs in a smart way. Instead of having layers of graphs which are always
identified on the same vertex set, you could define *slices* of graphs which do
not necessarily have the same vertex set. Using slices we would like to assign a
node to a community for each slice, so that the community for a node can be
different for different slices, rather than always being the same for all
layers. We can translate *slices* into *layers* but it is not an easy
transformation to grasp fully. But by doing so, we can again rely on the same
machinery we developed for dealing with layers.

Layer multiplex
---------------

If we have two graphs which are identified on exactly the same vertex set, we
say we have two *layers*. For example, suppose graph ``G_telephone`` contains
the communication between friends over the telephone and that the graph
``G_email`` contains the communication between friends via mail. The exact same
vertex set then means that ``G_telephone.vs[i]`` is identical to the node
``G_email.vs[i]``. For each layer we can separately specify the type of
partition that we look for. In principle they could be different for each layer,
but for now we will assume the type of partition is the same for all layers.
The quality of all partitions combined is simply the sum of the individual
qualities for the various partitions, weighted by the ``layer_weight``. If we
denote by :math:`q_k` the quality of layer :math:`k` and the weight by
:math:`w_k`, the overall quality is then

.. math:: q = \sum_k w_k q_k.

The optimisation algorithm is no different from the standard algorithm. We
simply calculate the overall difference of moving a node to another community as
the sum of the individual differences in all partitions. The rest (aggregating
and repeating on the aggregate partition) simple proceeds as usual.

The most straightforward way to use this is then to use
:func:`louvain.find_partition_multiplex`:

>>> membership, improv = louvain.find_partition_multiplex(
...                        [G_telephone, G_email],
...                        louvain.ModularityVertexPartition);

.. note:: You may need to carefully reflect how you want to weigh the importance
of an individual layer. Since the :class:`ModularityVertexPartition` is
normalised by the number of links, you essentially weigh layers the same,
independent of the number of links. This may be undesirable, in which case it
may be better to use :class:`RBConfigurationVertexPartition`, which is
unnormalised. Alternatively, you may specify different ``layer_weights``.

Similar to the simpler function :func:`louvain.find_partition`, it is a simple
helper function. The function returns a membership vector, because the
membership for all layers is identical. You can also control the partitions and
optimisation in more detail. Perhaps it is better to use
:func:`CPMVertexPartition` with different resolution parameter for example for
different layers of the graph.  For example, using email creates a more
connected structure because multiple people can be involved in a single mail,
which may require a higher resolution parameter for the email graph.

>>> part_telephone = louvain.CPMVertexPartition(
...                    G_telephone, resolution_parameter=0.01);
>>> part_email = louvain.CPMVertexPartition(
...                    G_email, resolution_parameter=0.3);
>>> optimiser.optimise_partition_multiplex([part_telephone, part_email]);

Note that ``part_telephone`` and ``part_email`` contain exactly the same
partition, in the sense that ``part_telephone.membership ==
part_email.membership``. The underlying graph is of course different, and hence
the individual quality will also be different.

Some layers may have a more important role in the partition and this can be
indicated by the ``layer_weight``. Using half the weight for the email layer for
example would be possible as follows:

>>> optimiser.optimise_partition_multiplex(
...   [part_telephone, part_email],
...   layer_weights=[1,0.5]);

Negative links
^^^^^^^^^^^^^^

The layer weights are especially useful when negative links are present,
representing for example conflict or animosity. Most methods (except CPM) only
accept positive weights. In order to deal with graphs that do have negative
links, a solution is to separate the graph into two layers: one layer with
positive links, the other with only negative links. In general, we would like to
have relatively many positive links within communities, while for negative links
the opposite holds: we want many negative links between communities. We can
easily do this within the multiplex layer framework by passing in a negative
layer weight. For example, suppose we have a graph ``G`` with possibly negative
weights. We can then separate it into a positive and negative graph as follows:

>>> G_pos = G.subgraph_edges(G.es.select(weight_gt = 0), delete_vertices=False);
>>> G_neg = G.subgraph_edges(G.es.select(weight_lt = 0), delete_vertices=False);
>>> G_neg.es['weight'] = [-w for w in G_neg.es['weight']];

We can then simply detect communities using;

>>> part_pos = louvain.ModularityVertexPartition(G_pos, weights='weight');
>>> part_neg = louvain.ModularityVertexPartition(G_neg, weights='weight');
>>> optimiser.optimise_partition_multiplex(
...   [part_pos, part_neg],
...   layer_weights=[1,-1]);

Slices to layers
----------------

Temporal community detection
----------------------------

References
----------
.. [1] Mucha, P. J., Richardson, T., Macon, K., Porter, M. A., & Onnela, J.-P.
       (2010). Community structure in time-dependent, multiscale, and multiplex
       networks. Science, 328(5980), 876–8. `10.1126/science.1184819 <http://doi.org/10.1126/science.1184819>`_

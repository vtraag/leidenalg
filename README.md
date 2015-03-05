INTRODUCTION
============

This package implements the louvain algorithm [1] in ``C++`` and exposes it to
``python``. It relies on (python-)``igraph`` for it to function. Besides the
relative flexibility of the implementation, it also scales well, and can be run
on graphs of millions of nodes (as long as they can fit in memory). The core
function is ``find_partition`` which finds the optimal partition using the
louvain algorithm for a number of different methods. The original implementation
is available from https://sites.google.com/site/findcommunities/. The methods
currently implemented are:

Modularity
  This method compares the actual graph to the expected graph, taking into
  account the degree of the nodes [2]. The expected graph is based on a
  configuration null-model.

RBConfiguration
  This is an extension of modularity which includes a resolution parameter [3].
  In general, a higher resolution parameter will lead to smaller communities.

RBER
  A variant of the previous method that instead of a configuration null-model
  uses a Erdös-Rényi null-model in which each edge has the same probability of
  appearing [3].

CPM
  This method compares to a fixed resolution parameter, so that it finds
  communities that have an internal density higher than the resolution
  parameter, and is separated from other communities with a density lowerer than
  the resolution parameter [4].

Significance
  This is a probabilistic method based on the idea of assessing the probability
  of finding such dense subgraphs in an (ER) random graph [5].

Surprise
  Another probabilistic method, but rather than the probability of finding dense
  subgraphs, it focuses on the probability of so many edges within communities
  [6, 7].


INSTALLATION
============

In short, for Unix: ``sudo pip install louvain``.
For Windows: download the binary installers.

For Unix like systems it is possible to install from source. For Windows this is
overly complicated, and you are recommended to use the binary installation files.
There are two things that are needed by this package: the igraph c core library
and the python-igraph python package. For both, please see http://igraph.org.

There are basically two installation modes, similar to the python-igraph package
itself (from which most of the setup.py comes).

1. No C core library is installed yet. The packages will be compiled and linked
   statically to an automatically downloaded version of the C core library of
   igraph.
2. A C core library is already installed. In this case, the package will link
   dynamically to the already installed version. This is probably also the
   version that is used by the igraph package, but you may want to double check
   this.

In case the python-igraph package is already installed before, make sure that
both use the **same versions**.

The cleanest setup it to install and compile the C core library yourself (make
sure that the header files are also included, e.g. install also the development
package from igraph). Then both the python-igraph package, as well as this
package are compiled and (dynamically) linked to the same C core library.

TROUBLESHOOTING
===============

In case of any problems, best to start over with a clean environment. Make sure
you remove the python-igraph package completely, remove the C core library and
remove the louvain package. Then, do a complete reinstall starting from ``pip
install louvain``. In case you want a dynamic library be sure to then install
the C core library from source before. Make sure you **install the same
versions**.

USAGE
=====

There is no standalone version of louvain-igraph, and you will always need
python to access it. There are no plans for developing a standalone version or R
support. So, use python. Please refer to the documentation within the python
package for more details on function calls and parameters.

To start, make sure to import the packages:
```python
import louvain
import igraph as ig
```

We'll create a random graph for testing purposes:
```python
G = ig.Graph.Erdos_Renyi(100, 0.1);
```

For simply finding a partition use:
```python
part = louvain.find_partition(G, method='Modularity');
```

Notice that ``part`` now contains an additional variable, ``part.quality`` which
stores the quality of the partition as calculated by the used method. You can
always get the quality of the partition using another method by calling
```python
part.significance = louvain.quality(G, partition, method='Significance');
```

You can also find partition for multiplex graphs. For each layer you then
specify the objective function, and the overall objective function is simply the
sum over all layers, weighted by some weight. If we denote by ``q_k`` the quality
of layer ``k`` and the weight by ``w_k``, the overall quality is then ``q = sum_k
w_k q_k``.  This can also be useful in case you have negative links. In
principle, this could also be used to detect temporal communities in a dynamic
setting, cf. [8].

For example, assuming you have a graph with positive weights ``G_positive`` and
a graph with negative weights ``G_negative``, and you want to use Modularity for
finding a partition, you can use
```python
membership, quality = louvain.find_partition_multiplex([
louvain.Layer(graph=G_positive, method='Modularity', layer_weight=1.0),
louvain.Layer(graph=G_negative, method='Modularity', layer_weight=-1.0)])
```

Notice the negative layer weight is ``-1.0`` for the negative graph, since we
want those edges to fall between communities rather than within. One particular
problem when using negative links, is that the optimal community is no longer
guaranteed to be connected (it may be a multipartite partition). You may
therefore need the options consider_comms=ALL_COMMS to improve the quality of
the partition. Notice that this runs much slower than only considering
neighbouring communities (which is the default).

Various methods (such as Reichardt and Bornholdt's Potts model, or CPM) support
a (linear) resolution parameter, which can be effectively bisected, cf. [5]. You
can do this by calling:
```python
res_parts = louvain.bisect(G, method='CPM', resolution_range=[0,1]);
```
Notice this may take some time to run, as it effectively calls
louvain.find_partition for various resolution parameters (depending on the
settings possibly hundreds of times).

Then ``res_parts`` is a dictionary containing as keys the resolution, and as
values a ``NamedTuple`` with variables ``partition`` and ``bisect_value``, which
contains the partition and the value at which the resolution was bisected (the
value of the ``bisect_func`` of the ``bisect`` function). You could for example
plot the bisection value of all the found partitions by using:
```python
import pandas as pd
import matplotlib.pyplot as plt
res_df = pd.DataFrame({
         'resolution': res_parts.keys(),
         'bisect_value': [bisect.bisect_value for bisect in res_parts.values()]});
plt.step(res_df['resolution'], res_df['bisect_value']);
plt.xscale('log');
```

REFERENCES
==========

Please cite the references appropriately in case they are used.

1. Blondel, V. D., Guillaume, J.-L., Lambiotte, R. & Lefebvre, E. Fast unfolding
   of communities in large networks. J. Stat. Mech. 2008, P10008 (2008).
2. Newman, M. & Girvan, M. Finding and evaluating community structure in networks.
   Physical Review E 69, 026113 (2004).
3. Reichardt, J. & Bornholdt, S. Partitioning and modularity of graphs with arbitrary
   degree distribution. Physical Review E 76, 015102 (2007).
4. Traag, V. A., Van Dooren, P. & Nesterov, Y. Narrow scope for resolution-limit-free
   community detection. Physical Review E 84, 016114 (2011).
5. Traag, V. A., Krings, G. & Van Dooren, P. Significant scales in community structure.
   Scientific Reports 3, 2930 (2013).
6. Aldecoa, R. & Marín, I. Surprise maximization reveals the community structure
   of complex networks. Scientific reports 3, 1060 (2013).
7. Traag, V.A., Aldecoa, R. & Delvenne, J.-C. Detecting communities using Asymptotical
   Surprise. Forthcoming (2015).
8. Mucha, P. J., Richardson, T., Macon, K., Porter, M. A. & Onnela, J.-P.
   Community structure in time-dependent, multiscale, and multiplex networks.
   Science 328, 876–8 (2010).

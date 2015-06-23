OVERVIEW
========

The basic structure for the algorithm is as follows. There is a general class
called Optimiser which implements the basic Louvain algorithm. In order to
optimize any specific quality function it needs to know the difference when
moving around nodes. The interface for this is provided in the (abstract) class
called MutableVertexPartition, through the diff_move function. When actually
moving a node, this class takes care of handling all the administrative tasks
(i.e. updating membership vector, community sets, number of internal links,
etc...). The actual quality is provided by the quality function as provided in
the MutableVertexPartition.

The GraphHelper class implements a number of functionalities that are required
for running the algorithm. Besides some auxiliary functions, it also takes care
of aggregating the graph based on the communities (i.e. contracting all the
vertices in the same community). This creates a layer between the igraph library
and the actual implementation, so that another graph library could in potential
also be used instead of igraph (although there are currently no plans for doing
so).

If needed you can include a DEBUG symbol at appropriate places (and make sure
you include <iostream>). This will provide some (at times overly detailed)
information while running the algorithm. Most importantly, it will make two
important consistency checks: (1) after moving a node, is the difference in the
quality functions the same as calculated by the diff_move function; and (2)
after aggregating a graph, is the quality of the aggregated graph still the same
as the quality of the partition of the graph before aggregating. You can use
this to see if at least the definition of diff_move and the definition of
quality are consistent.

EXTENDING
=========

Implementing a new method thus required to re-implement the functions diff_move
and quality. This is done by creating a new class which derived from
MutableVertexPartition and simply implementing these functions. You should take
care in first deriving the correct expressions for you particular method. Be
aware that your functions should also work correctly in the case of an
aggregated graph (where moving around nodes essentially means moving around
communities). Notice that most administrative tasks when moving around a node is
handled by MutableVertexPartition. However, it may be that some particularly
property is necessary for your method, but that it is not tracked yet (e.g. the
number of triangles within a community or something). In that case, you may
override the function move_node and implements this yourself, but be sure to
still call the base function in order to ensure the rest of the administration
remains consistent.

After implementing this class, you should be able to run tests in C++. Be aware
though that your method is not yet exposed to the python interface. In
particular, you should add your method to the create_partition function in the
pynterface.cpp function. You can add documentation in the files __init__.py,
functions.py and README.md.

Please use GitHub to fork a new project if you want to implement your own
method. That way, we could re-integrate your contributions easily.

TESTING
=======

You can install this package locally for testing by using `` sudo python
setup.py install --force --record files.txt `` The `--force` argument ensure
that everything is recompiled and `--record` argument records all the installed
files to `files.txt`. That way, you can remove the installed pacakges by simply
running `cat files.txt | sudo xargs rm`. Please notice that you must restart
python in order to reload the package. 

Before starting to test your method in python, you may want to test your method
in `C++` first. This can be easily by simply creating some `main.cpp` file and
compiling it against this package.  You could run a simple test by simply
creating some Erdös-Rènyi graph, and running the method on it. Here is a small
example test:
```C++
  igraph_t graph;
  size_t n = 10000;
  size_t k = 10;
  double p = (double)k/(double)n;
  igraph_erdos_renyi_game(&graph, IGRAPH_ERDOS_RENYI_GNP, n, p,
                           IGRAPH_UNDIRECTED, IGRAPH_NO_LOOPS);

  size_t m = igraph_ecount(&graph);
  Graph* G = new Graph(&graph, vector<double>(m, 0.4));

  Optimiser* opt = new Optimiser();
  opt->consider_comms = Optimiser::ALL_NEIGH_COMMS;
  opt->random_order = true;

  MutableVertexPartition* partition = new ModularityVertexPartition(G);

  opt->optimize_partition(partition);

  delete G;
  igraph_destroy(&graph);
  delete opt;
```

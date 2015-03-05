# -*- coding: utf-8 -*-
"""
This package implements the louvain algorithm in C++ and exposes it to
python. It relies on (python-)igraph for it to function. Besides the relative
flexibility of the implementation, it also scales well, and can be run on graphs
of millions of nodes (as long as they can fit in memory). The core function is
``find_partition`` which finds the optimal partition using the louvain algorithm for
a number of different methods. The methods currently implemented are:

Modularity
  This method compares the actual graph to the expected graph, taking into
  account the degree of the nodes [1]. The expected graph is based on a
  configuration null-model.

RBConfiguration
  This is an extension of modularity which includes a resolution parameter [2].
  In general, a higher resolution parameter will lead to smaller communities.

RBER
  A variant of the previous method that instead of a configuration null-model
  uses a Erdös-Rényi null-model in which each edge has the same probability of
  appearing [2].

CPM
  This method compares to a fixed resolution parameter, so that it finds
  communities that have an internal density higher than the resolution
  parameter, and is separated from other communities with a density lowerer than
  the resolution parameter [3].

Significance
  This is a probabilistic method based on the idea of assessing the probability
  of finding such dense subgraphs in an (ER) random graph [4].

Surprise
  Another probabilistic method, but rather than the probability of finding dense
  subgraphs, it focuses on the probability of so many edges within communities
  [5, 6].

REFERENCES
==========

Please cite the references appropriately in case they are used.

1. Newman, M. & Girvan, M. Finding and evaluating community structure in networks.
   Physical Review E 69, 026113 (2004).
2. Reichardt, J. & Bornholdt, S. Partitioning and modularity of graphs with arbitrary
   degree distribution. Physical Review E 76, 015102 (2007).
3. Traag, V. A., Van Dooren, P. & Nesterov, Y. Narrow scope for resolution-limit-free
   community detection. Physical Review E 84, 016114 (2011).
4. Traag, V. A., Krings, G. & Van Dooren, P. Significant scales in community structure.
   Scientific Reports 3, 2930 (2013).
5. Aldecoa, R. & Marín, I. Surprise maximization reveals the community structure
   of complex networks. Scientific reports 3, 1060 (2013).
6. Traag, V.A., Aldecoa, R. & Delvenne, J.-C. Detecting communities using Asymptotical
   Surprise. Forthcoming (2015).

"""
from .functions import find_partition
from .functions import find_partition_multiplex
from .functions import quality
from .functions import total_internal_edges
from .functions import total_internal_edges
from .functions import bisect
from .functions import Layer
from .functions import ALL_COMMS
from .functions import ALL_NEIGH_COMMS
from .functions import RAND_COMM
from .functions import RAND_NEIGH_COMM

from pkg_resources import get_distribution, DistributionNotFound
import os.path

try:
    _dist = get_distribution('louvain')
    # Normalize case for Windows systems
    dist_loc = os.path.normcase(_dist.location)
    here = os.path.normcase(__file__)
    if not here.startswith(os.path.join(dist_loc, 'louvain')):
        # not installed, but there is another version that *is*
        raise DistributionNotFound
except DistributionNotFound:
    __version__ = 'Please install this project with setup.py'
else:
    __version__ = _dist.version

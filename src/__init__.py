# -*- coding: utf-8 -*-
r""" This package implements the louvain algorithm in ``C++`` and exposes it to
python.  It relies on ``(python-)igraph`` for it to function. Besides the
relative flexibility of the implementation, it also scales well, and can be run
on graphs of millions of nodes (as long as they can fit in memory). Each method
is represented by a different class, all of whom derive from
:class:`~louvain.VertexPartition.MutableVertexPartition`. In addition,
multiplex graphs are supported as layers, which also supports multislice
representations.

Examples
--------

The simplest example just finds a partition using modularity

  >>> G = ig.Graph.Tree(100, 3);
  >>> partition = louvain.find_partition(G, louvain.ModularityVertexPartition);

Alternatively, one can access the different optimisation routines individually
and construct partitions oneself. These partitions can then be optimised by
constructing an :class:`Optimiser` object and running
:func:`~Optimiser.optimise_partition`.

  >>> G = ig.Graph.Tree(100, 3);
  >>> partition = louvain.CPMVertexPartition(G, resolution_parameter = 0.1)
  >>> optimiser = louvain.Optimiser();
  >>> optimiser.optimise_partition(partition);

The :class:`Optimiser` class contains also the different subroutines that are
used internally by :func:`~Optimiser.optimise_partition`. In addition, through
the Optimiser class there are various options available for changing some of
the optimisation procedure which can affect both speed and quality, which are
not immediately available in :func:`louvain.find_partition`.
"""
from .functions import ALL_COMMS
from .functions import ALL_NEIGH_COMMS
from .functions import RAND_COMM
from .functions import RAND_NEIGH_COMM

from .functions import find_partition
from .functions import find_partition_multiplex
from .functions import find_partition_temporal
from .functions import slices_to_layers
from .functions import time_slices_to_layers
from .functions import set_rng_seed

from .Optimiser import Optimiser
from .VertexPartition import ModularityVertexPartition
from .VertexPartition import SurpriseVertexPartition
from .VertexPartition import SignificanceVertexPartition
from .VertexPartition import RBERVertexPartition
from .VertexPartition import RBConfigurationVertexPartition
from .VertexPartition import CPMVertexPartition

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

from ._version import get_versions
__version__ = get_versions()['version']
del get_versions

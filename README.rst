leidenalg
==============

This package implements the Leiden algorithm in ``C++`` and exposes it to
``python``.  It relies on ``(python-)igraph`` for it to function. Besides the
relative flexibility of the implementation, it also scales well, and can be run
on graphs of millions of nodes (as long as they can fit in memory). The core
function is ``find_partition`` which finds the optimal partition using the
Leiden algorithm [1]_, which is an extension of the Louvain algorithm [2]_ for a
number of different methods. The methods currently implemented are (1)
modularity [3]_, (2) Reichardt and Bornholdt's model using the configuration
null model and the Erdös-Rényi null model [4]_, (3) the Constant Potts model
(CPM) [5]_, (4) Significance [6]_, and finally (5) Surprise [7]_. In addition,
it supports multiplex partition optimisation allowing community detection on for
example negative links [8]_ or multiple time slices [9]_. There is the
possibility of only partially optimising a partition, so that some community
assignments remain fixed [10]_. It also provides some support for community
detection on bipartite graphs. See the `documentation
<http://leidenalg.readthedocs.io/en/latest/>`_ for more information.


.. image:: https://readthedocs.org/projects/leidenalg/badge
                :target: http://leidenalg.readthedocs.io/en/latest/
                :alt: Leiden documentation status

.. image:: https://github.com/vtraag/leidenalg/actions/workflows/build.yml/badge.svg?branch=master
                :target: https://github.com/vtraag/leidenalg/actions/workflows/build.yml
                :alt: Leiden build status (GitHub Actions)

.. image:: https://zenodo.org/badge/146722095.svg
                :target: https://zenodo.org/badge/latestdoi/146722095
                :alt: DOI

.. image:: https://anaconda.org/conda-forge/leidenalg/badges/version.svg
                :target: https://anaconda.org/conda-forge/leidenalg
                :alt: Anaconda (conda-forge)

Installation
------------

In short: ``pip install leidenalg``. All major platforms are supported on
Python>=3.6, earlier versions of Python are no longer supported. Alternatively,
you can install from Anaconda (channel ``conda-forge``).

For Unix like systems it is possible to install from source. For Windows this is
overly complicated, and you are recommended to use the binary wheels. The igraph
``C`` core library is provided within this package, and is automatically
compiled. If you encounter any issue with compilation, please see
http://igraph.org.

Make sure you have all necessary tools for compilation. In Ubuntu this can be
installed using ``sudo apt-get install build-essential autoconf automake flex
bison``, please refer to the documentation for your specific system.  Make sure
that not only ``gcc`` is installed, but also ``g++``, as the ``leidenalg``
package is programmed in ``C++``.

You can check if all went well by running a variety of tests using ``python
setup.py test``.

There are basically two installation modes, similar to the python-igraph package
itself (from which most of the setup.py comes).

1. No ``C`` core library is installed yet. The ``C`` core
   library of igraph that is provided within the ``leidenalg`` package is
   compiled.
2. A ``C`` core library is already installed. In this case, you may link
   dynamically to the already installed version by specifying
   ``--no-pkg-config``. This is probably also the version that is used by the
   igraph package, but you may want to double check this.

In case the ``python-igraph`` package is already installed before, make sure that
both use the **same versions** (at least the same minor version, which should be
API compatible).

Troubleshooting
---------------

In case of any problems, best to start over with a clean environment. Make sure
you remove the ``python-igraph`` package completely, remove the ``C`` core
library and remove the ``leidenalg`` package. Then, do a complete reinstall
starting from ``pip install leidenalg``. In case you want a dynamic library be
sure to then install the ``C`` core library from source before. Make sure you
**install the same versions**.

Usage
-----

There is no standalone version of ``leidenalg``, and you will always need python
to access it. There are no plans at the moment for developing a standalone
version or R support. However, there have been various efforts to port the
package to R. These typically do not offer all available functionality or have
some other limitations, but nonetheless may be very useful. The available ports
are:

- https://github.com/cole-trapnell-lab/leidenbase
- https://github.com/TomKellyGenetics/leiden
- https://github.com/kharchenkolab/leidenAlg

Please refer to the documentation for more details
on function calls and parameters.

This implementation is made for flexibility, but ``igraph`` nowadays also
includes an implementation of the Leiden algorithm internally. That
implementation is less flexible: the implementation only works on undirected
graphs, and only CPM and modularity are supported. It is likely to be
substantially faster though.

Just to get you started, below the essential parts.
To start, make sure to import the packages:

>>> import leidenalg
>>> import igraph as ig

We'll create a random graph for testing purposes:

>>> G = ig.Graph.Erdos_Renyi(100, 0.1);

For simply finding a partition use:

>>> part = leidenalg.find_partition(G, leidenalg.ModularityVertexPartition);

Contribute
----------

Source code: https://github.com/vtraag/leidenalg

Issue tracking: https://github.com/vtraag/leidenalg/issues

See the documentation on `Implementation` for more details on how to
contribute new methods.

References
----------

Please cite the references appropriately in case they are used.

.. [1] Traag, V.A., Waltman. L., Van Eck, N.-J. (2018). From Louvain to
       Leiden: guaranteeing well-connected communities. Scientific reports, 9(1), 5233.
       `10.1038/s41598-019-41695-z <http://dx.doi.org/10.1038/s41598-019-41695-z>`_

.. [2] Blondel, V. D., Guillaume, J.-L., Lambiotte, R., & Lefebvre, E. (2008).
       Fast unfolding of communities in large networks. Journal of Statistical
       Mechanics: Theory and Experiment, 10008(10), 6.
       `10.1088/1742-5468/2008/10/P10008 <http://doi.org/10.1088/1742-5468/2008/10/P10008>`_

.. [3] Newman, M. E. J., & Girvan, M. (2004). Finding and evaluating community
       structure in networks. Physical Review E, 69(2), 026113.
       `10.1103/PhysRevE.69.026113 <http://doi.org/10.1103/PhysRevE.69.026113>`_

.. [4] Reichardt, J., & Bornholdt, S. (2006). Statistical mechanics of
       community detection. Physical Review E, 74(1), 016110.
       `10.1103/PhysRevE.74.016110 <http://doi.org/10.1103/PhysRevE.74.016110>`_

.. [5] Traag, V. A., Van Dooren, P., & Nesterov, Y. (2011). Narrow scope for
       resolution-limit-free community detection. Physical Review E, 84(1),
       016114.  `10.1103/PhysRevE.84.016114
       <http://doi.org/10.1103/PhysRevE.84.016114>`_

.. [6] Traag, V. A., Krings, G., & Van Dooren, P. (2013). Significant scales in
       community structure. Scientific Reports, 3, 2930.  `10.1038/srep02930
       <http://doi.org/10.1038/srep02930>`_

.. [7] Traag, V. A., Aldecoa, R., & Delvenne, J.-C. (2015). Detecting
       communities using asymptotical surprise. Physical Review E, 92(2),
       022816.  `10.1103/PhysRevE.92.022816
       <http://doi.org/10.1103/PhysRevE.92.022816>`_

.. [8] Traag, V. A., & Bruggeman, J. (2009). Community detection in networks
       with positive and negative links. Physical Review E, 80(3), 036115.
       `10.1103/PhysRevE.80.036115
       <http://doi.org/10.1103/PhysRevE.80.036115>`_

.. [9] Mucha, P. J., Richardson, T., Macon, K., Porter, M. A., & Onnela, J.-P.
       (2010). Community structure in time-dependent, multiscale, and multiplex
       networks. Science, 328(5980), 876–8. `10.1126/science.1184819
       <http://doi.org/10.1126/science.1184819>`_

.. [10] Zanini, F., Berghuis, B. A., Jones, R. C., Robilant, B. N. di,
        Nong, R. Y., Norton, J., Clarke, Michael F., Quake, S. R. (2019).
        northstar: leveraging cell atlases to identify healthy and neoplastic
        cells in transcriptomes from human tumors. BioRxiv, 820928.
        `10.1101/820928 <https://doi.org/10.1101/820928>`_

Licence
-------

Copyright (C) 2020 V.A. Traag

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see http://www.gnu.org/licenses/.


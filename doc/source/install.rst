Installation
============

In short, for Unix: ``sudo pip install leidenalg``.  For Windows: download the
binary installers from `PyPi <https://pypi.org/project/leidenalg/>`_.
Alternatively, use `Anaconda <https://www.anaconda.com/distribution/>`_ and get
the conda packages from the `vtraag channel
<https://anaconda.org/vtraag/leidenalg>`_, which supports both Unix, Mac OS and
Windows.

For Unix like systems it is possible to install from source. For Windows this
is overly complicated, and you are recommended to use the binary installation
files.  There are two things that are needed by this package: the igraph ``C``
core library and the python-igraph python package. For both, please see
http://igraph.org. 

Make sure you have all necessary tools for compilation. In Ubuntu this can be
installed using ``sudo apt-get install build-essential``, please refer to the
documentation for your specific system.  Make sure that not only ``gcc`` is
installed, but also ``g++``, as the leidenalg package is programmed in ``C++``.
Note that to compile ``igraph`` itself, you also need to install
``libxml2-dev``.

You can check if all went well by running a variety of tests using ``python
setup.py test``.

There are basically two installation modes, similar to the python-igraph
package itself (from which most of the setup.py comes).

1. No ``C`` core library is installed yet. The packages will be compiled and
   linked statically to an automatically downloaded version of the ``C`` core
   library of igraph.
2. A ``C`` core library is already installed. In this case, the package will
   link dynamically to the already installed version. This is probably also the
   version that is used by the igraph package, but you may want to double check
   this.

In case the python-igraph package is already installed before, make sure that
both use the **same versions**.

The cleanest setup it to install and compile the ``C`` core library yourself
(make sure that the header files are also included, e.g. install also the
development package from igraph). Then both the python-igraph package, as well
as this package are compiled and (dynamically) linked to the same ``C`` core
library.

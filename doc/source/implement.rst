Implementation
==============

If you have a cool new idea for a better method, and you want to optimise it,
you can easily plug it in the current package. This section explains how the
package is setup internally, and how you can extend it. Most of this concerns
``C++``, and Python only comes in when exposing the classes from ``C++``.

The core of the Leiden algorithm is implemented in ``C++`` in the package
``libleidenalg``, available from https://github.com/vtraag/libleidenalg. This
``C++`` library is used as the core for the Python package, which is just an
interface to the underlying ``C++`` library. New methods need to be added to the
``C++`` library first, and then exposed to the Python interface. For more
information, please see the contributing guide of the ``libleidenalg`` package
at https://github.com/vtraag/libleidenalg/blob/main/CONTRIBUTING.md

Python
------

Exposing the method to ``python`` takes a bit more effort. There are various
places in which you need to change/add things. In the following, we assume you
created a new class called ``CoolVertexPartition`` in ``libleidenalg``. Please
then follow the following steps:

1. Your own new VertexPartition class should add some specific methods. In
   particular, you need to ensure you create a method

   .. code-block:: c++

      CoolVertexPartition* CoolVertexPartition::create(Graph* graph)
      {
        return new CoolVertexPartition(graph);
      }


   and

   .. code-block:: c++

      CoolVertexPartition* CoolVertexPartition::create(Graph* graph, vector<size_t> const& membership)
      {
        return new CoolVertexPartition(graph, membership);
      }
  

   These methods ensure that based on a current partition, we can create a new
   partition (without knowing its type).

2. In ``python_partition_interface.cpp`` some methods need to be added. In
   particular

   .. code-block:: c++

      PyObject* _new_CoolVertexPartition(PyObject *self, PyObject *args, PyObject *keywds)


   You should be able to simply copy an existing method, and adapt it to your
   own needs.

3. These methods need to be exposed in ``pynterface.h``. In particular, you
   need to add the method you created in step (2) to ``leiden_funcs[]``.
   Again, you should be able to simply copy an existing line.

4. You can then finally create the Python class in ``VertexPartition.py``. The
   base class derives from the :class:`VertexClustering` from :mod:`igraph`, so
   that it is compatible with all operations in :mod:`igraph`. You should add
   the method as follows::

     class CoolVertexPartition(MutableVertexPartition):

       def __init__(self, ... ):
         ...

   Again, you should be able to copy the outline for another class and adapt it
   to your own needs. Don't forget to change to ``docstring`` to update the
   documentation so that everybody knows how your new cool method works.

5. Expose your newly created ``python`` class directly in ``__init__.py`` by
   importing it::
    
     from .VertexPartition import CoolVertexPartition

That's it! You're done and should now be able to find communities using your
new :class:`CoolVertexPartition`:

>>> la.find_partition(G, la.CoolVertexPartition); # doctest: +SKIP



Implementation
==============

If you have a cool new idea for a better method, and you want to optimise it,
you can easily plug it in the current tool. This section explains how the
package is setup internally, and how you can extend it. Most of this concerns
``C++``, and ``python`` only comes in when exposing the resulting classes.

Method
------

All methods in the end derive from :class:`MutableVertexPartition`, which
implements almost all necessary details, such as moving actual nodes while
maintaining the internal administration. Similarly, it provides all the
necessary functionality for initialising a partition. Additionally, there are
two abstract classes that derive from this base class:
:class:`ResolutionParameterVertexPartition` and
:class:`LinearResolutionParameterVertexPartition` (which in turn derives from
the former class). If you want a method with a resolution parameter, you should
derive from one of these two classes, otherwise, simply from the base class 
:class:`MutableVertexPartition`.

There are two functions that you need to implement yourself: :func:`diff_move`
and :func:`quality`. Note that they should always be consistent, so that we can
double check the internal consistency. You should also ensure that the
diff_move function can be correctly used on any aggregate graph (i.e. moving a
node in the aggregate graph indeed corresponds to moving a set of nodes in the
individual graph).

That's it. In principle, you could now use and test the method in ``C++``.

Python
------

Exposing the method to ``python`` takes a bit more effort. There are various
places in which you need to change/add things. In the following, we assume you
created a new class called ``CoolVertexPartition``. In order of dependencies, it
goes as follows:

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



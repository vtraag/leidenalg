import igraph as _ig
from . import _c_louvain
from ._c_louvain import ALL_COMMS
from ._c_louvain import ALL_NEIGH_COMMS
from ._c_louvain import RAND_COMM
from ._c_louvain import RAND_NEIGH_COMM
from collections import namedtuple
from collections import OrderedDict
import logging
from math import log, sqrt

import sys
# Check if working with Python 3
PY3 = (sys.version > '3');

def _get_py_capsule(graph):
  if PY3:
    return graph.__graph_as_capsule();
  else:
    return graph.__graph_as_cobject();

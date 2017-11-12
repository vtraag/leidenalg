#include "pynterface.h"

  PyObject* _set_rng_seed(PyObject *self, PyObject *args, PyObject *keywds)
  {
    static char* kwlist[] = {"seed", NULL};
    size_t seed = 0;
    #ifdef DEBUG
      cerr << "Parsing arguments..." << endl;
    #endif

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "i", kwlist,
                                     &seed))
        return NULL;

    set_rng_seed(seed);

    Py_INCREF(Py_None);
    return Py_None;
  }

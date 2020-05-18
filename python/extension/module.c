#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "compile.h"
#include "load.h"
#include "load_extended.h"

static PyMethodDef SereMethods[] = {
    {"compile",  sere_compile_expr, METH_VARARGS,
     "Compile SERE expression."},
    {"load",  sere_load, METH_VARARGS,
     "Load compiled SERE."},
    {"load_extended",  sere_load_extended, METH_VARARGS,
     "Load compiled SERE in extended mode."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef seremodule = {
    PyModuleDef_HEAD_INIT,
    "serec",   /* name of module */
    NULL/*sere_doc*/, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    SereMethods
};

PyMODINIT_FUNC
PyInit_serec(void) {
  if (PyType_Ready(&CompiledSereType) < 0)
    return NULL;
  Py_INCREF(&CompiledSereType);

  if (PyType_Ready(&ContextSereType) < 0)
    return NULL;
  Py_INCREF(&ContextSereType);

  if (PyType_Ready(&ExtendedContextSereType) < 0)
    return NULL;
  Py_INCREF(&ExtendedContextSereType);

  if (PyType_Ready(&ExtendedMatchObjectType) < 0)
    return NULL;
  Py_INCREF(&ExtendedMatchObjectType);

  return PyModule_Create(&seremodule);
}

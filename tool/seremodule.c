#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

#include "sere/sere.h"

#define ALLOC_PY_OBJECT(type) (type*)type##Type.tp_alloc(&type##Type, 1);

typedef struct {
  PyObject_HEAD
  struct sere_compiled result;
} CompiledSere;

static void
CompiledSere_dealloc(CompiledSere *self) {
  if (self)
    sere_release(&self->result);
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
CompiledSere_content(CompiledSere *self, PyObject *Py_UNUSED(ignored)) {
  return PyBytes_FromStringAndSize(self->result.content, self->result.content_size);
}

static PyMethodDef CompiledSere_methods[] = {
    {"content", (PyCFunction) CompiledSere_content, METH_NOARGS,
     "Return the compiled SERE"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject CompiledSereType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "sere.Compiled",
    .tp_doc = "Compiled objects",
    .tp_basicsize = sizeof(CompiledSere),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = (destructor) CompiledSere_dealloc,
    .tp_methods = CompiledSere_methods,
};

typedef struct {
  PyObject_HEAD
  void* sere;
} ContextSere;

static void
ContextSere_dealloc(ContextSere *self) {
  if (self)
    sere_context_release(self->sere);
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
ContextSere_advance(ContextSere *self, PyObject *args) {
  const char *atomics;
  Py_ssize_t atomics_count;

  if (!PyArg_ParseTuple(args, "y#", &atomics, &atomics_count))
    return NULL;

  if (atomics_count < 0)
    return NULL;

  sere_context_advance(self->sere, atomics, (size_t) atomics_count);
  return PyLong_FromLong(1);
}

static PyObject *
ContextSere_get_result(ContextSere *self, PyObject *Py_UNUSED(ignored)) {
  if (!self)
    return NULL;
  int result;
  sere_context_get_result(self->sere, &result);
  return PyLong_FromLong(result);
}

static void
ContextSere_reset(ContextSere *self, PyObject *Py_UNUSED(ignored)) {
  sere_context_reset(self->sere);
}

static PyObject *
ContextSere_atomic_count(ContextSere *self, PyObject *Py_UNUSED(ignored)) {
  size_t result;
  sere_context_atomic_count(self->sere, &result);
  return PyLong_FromSize_t((long)result);
}

static PyObject *
ContextSere_atomic_name(ContextSere *self, PyObject *args) {
  unsigned int id;

  if (!self)
    return NULL;

  if (!PyArg_ParseTuple(args, "I", &id))
    return NULL;

  const char* name = NULL;

  sere_context_atomic_name(self->sere, id, &name);
  if (name == NULL) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  return PyBytes_FromString(name);
}

static PyMethodDef ContextSere_methods[] = {
    {
     "atomic_count",
     (PyCFunction) ContextSere_atomic_count,
     METH_NOARGS,
     "Returns number of atomic predicate in SERE"
    }, {
     "atomic_name",
     (PyCFunction) ContextSere_atomic_name,
     METH_VARARGS,
     "Returns name of a particular atomic predicate"
    }, {
     "reset",
     (PyCFunction) ContextSere_reset,
     METH_NOARGS,
     "Resets SERE context"
    }, {
     "advance",
     (PyCFunction) ContextSere_advance,
     METH_VARARGS,
     "Feeds SERE context with new event"
    }, {
     "get_result",
     (PyCFunction) ContextSere_get_result,
     METH_NOARGS,
     "Returns matching results"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject ContextSereType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "sere.Context",
    .tp_doc = "SERE context",
    .tp_basicsize = sizeof(ContextSere),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = (destructor) ContextSere_dealloc,
    .tp_methods = ContextSere_methods,
};

static PyObject *
sere_compile_expr(PyObject *self, PyObject *args) {
  const char *expr;
  const char *target;
  struct sere_options opts
    = {
       SERE_TARGET_DFASL,
       SERE_FORMAT_JSON,
       0,
       0 };

  if (!PyArg_ParseTuple(args, "ss", &expr, &target))
    return NULL;

  if (strcmp(target, "nfasl") == 0) {
    opts.target = SERE_TARGET_NFASL;
  } else if (strcmp(target, "dfasl") == 0) {
    opts.target = SERE_TARGET_DFASL;
  } else {
    return NULL;
  }

  CompiledSere* compiled = ALLOC_PY_OBJECT(CompiledSere);
  //(CompiledSere*)CompiledSereType.tp_alloc(&CompiledSereType, 1);

  if (compiled == NULL) {
    return NULL;
  }

  int r = sere_compile(expr,
                       &opts,
                       &compiled->result);

  if (r != 0) {
    Py_XDECREF((PyObject *)compiled);
    compiled = NULL;
  }

  return (PyObject *)compiled;
}


static PyObject *
sere_load(PyObject *self, PyObject *args) {
  const char* content;
  Py_ssize_t content_size;

  if (!PyArg_ParseTuple(args, "y#", &content, &content_size))
    return NULL;

  ContextSere* context = ALLOC_PY_OBJECT(ContextSere);

  if (context == NULL) {
    return NULL;
  }

  int r = sere_context_load(content, (size_t)content_size, &context->sere);

  if (r != 0) {
    Py_XDECREF((PyObject *)context);
    context = NULL;
    Py_INCREF(Py_None);
    return Py_None;
  }

  return (PyObject *)context;
}

static PyMethodDef SereMethods[] = {
    {"compile",  sere_compile_expr, METH_VARARGS,
     "Compile SERE expression."},
    {"load",  sere_load, METH_VARARGS,
     "Load compiled SERE."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef seremodule = {
    PyModuleDef_HEAD_INIT,
    "sere",   /* name of module */
    NULL/*sere_doc*/, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    SereMethods
};

PyMODINIT_FUNC
PyInit_sere(void) {
  if (PyType_Ready(&CompiledSereType) < 0)
    return NULL;
  Py_INCREF(&CompiledSereType);

  if (PyType_Ready(&ContextSereType) < 0)
    return NULL;
  Py_INCREF(&ContextSereType);

  return PyModule_Create(&seremodule);
}

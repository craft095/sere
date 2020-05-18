#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "helpers.h"

#include "api/sere.h"

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
ContextSere_advance(ContextSere *self, PyObject *Py_UNUSED(ignored)) {
  sere_context_advance(self->sere);
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

static PyObject *
ContextSere_reset(ContextSere *self, PyObject *Py_UNUSED(ignored)) {
  sere_context_reset(self->sere);
  Py_INCREF(Py_None);
  return Py_None;
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

  int r = sere_context_atomic_name(self->sere, id, &name);
  if (r != 0) {
    PyErr_SetString(PyExc_ValueError, "incorrect atomic predicate index");
    return NULL;
  }
  return PyBytes_FromString(name);
}

static PyObject *
ContextSere_set_atomic(ContextSere *self, PyObject *args) {
  unsigned int id;

  if (!self)
    return NULL;

  if (!PyArg_ParseTuple(args, "I", &id))
    return NULL;

  int r = sere_context_set_atomic(self->sere, id);
  if (r != 0) {
    PyErr_SetString(PyExc_ValueError, "incorrect atomic predicate index");
    return NULL;
  }

  Py_INCREF(Py_None);
  return Py_None;
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
     "set_atomic",
     (PyCFunction) ContextSere_set_atomic,
     METH_VARARGS,
     "Sets a particular atomic predicate to TRUE"
    }, {
     "reset",
     (PyCFunction) ContextSere_reset,
     METH_NOARGS,
     "Resets SERE context"
    }, {
     "advance",
     (PyCFunction) ContextSere_advance,
     METH_NOARGS,
     "Feeds SERE context with new event"
    }, {
     "get_result",
     (PyCFunction) ContextSere_get_result,
     METH_NOARGS,
     "Returns matching results"
    },
    {NULL}  /* Sentinel */
};

PyTypeObject ContextSereType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "sere.Context",
    .tp_doc = "SERE context",
    .tp_basicsize = sizeof(ContextSere),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = (destructor) ContextSere_dealloc,
    .tp_methods = ContextSere_methods,
};

PyObject *
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

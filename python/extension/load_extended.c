#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "helpers.h"

#include "load_extended.h"
#include "api/sere.h"

typedef struct {
    PyObject_HEAD
    unsigned int match;
    unsigned long longest;
    unsigned long shortest;
    unsigned long horizon;
} ExtendedMatchObject;

static PyMemberDef ExtendedMatch_members[] = {
    {"match", T_UINT, offsetof(ExtendedMatchObject, match), Match_Failed,
     "match result"},
    {"longest", T_ULONG, offsetof(ExtendedMatchObject, longest), 0,
     "length of longest match"},
    {"shortest", T_ULONG, offsetof(ExtendedMatchObject, shortest), 0,
     "length of shortest match"},
    {"horizon", T_ULONG, offsetof(ExtendedMatchObject, horizon), 0,
     "length of tracked part of a stream"},
    {NULL}
};

PyTypeObject ExtendedMatchObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "sere.ExtendedMatch",
    .tp_doc = "Extended Match",
    .tp_basicsize = sizeof(ExtendedMatchObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_members = ExtendedMatch_members,
};

typedef struct {
  PyObject_HEAD
  void* sere;
} ExtendedContextSere;

static void
ExtendedContextSere_dealloc(ExtendedContextSere *self) {
  if (self)
    sere_context_extended_release(self->sere);
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
ExtendedContextSere_advance(ExtendedContextSere *self, PyObject *Py_UNUSED(ignored)) {
  sere_context_extended_advance(self->sere);
  return PyLong_FromLong(1);
}

static PyObject *
ExtendedContextSere_to_dot(ExtendedContextSere *self, PyObject *args) {
  const char* file;
  if (!PyArg_ParseTuple(args, "s", &file))
    return NULL;

  int r = sere_context_extended_to_dot(self->sere, file);

  if (r != 0) {
    PyErr_SetObject(PyExc_ValueError, PyLong_FromLong(r));
    return NULL;
  }
  return PyLong_FromLong(0);
}

static PyObject *
ExtendedContextSere_get_result(ExtendedContextSere *self, PyObject *Py_UNUSED(ignored)) {
  if (!self)
    return NULL;

  ExtendedMatchObject* match = ALLOC_PY_OBJECT(ExtendedMatchObject);

  if (match == NULL) {
    return NULL;
  }

  struct ExtendedMatch result = {0};
  sere_context_extended_get_result(self->sere, &result);

  match->match = result.match;
  switch (result.match) {
  case Match_Ok:
    match->shortest = result.ok.shortest;
    match->longest = result.ok.longest;
    match->horizon = result.ok.horizon;
    break;
  case Match_Partial:
    match->horizon = result.partial.horizon;
    break;
  }

  return (PyObject *)match;
}

static PyObject *
ExtendedContextSere_reset(ExtendedContextSere *self, PyObject *Py_UNUSED(ignored)) {
  sere_context_extended_reset(self->sere);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
ExtendedContextSere_atomic_count(ExtendedContextSere *self, PyObject *Py_UNUSED(ignored)) {
  size_t result;
  sere_context_extended_atomic_count(self->sere, &result);
  return PyLong_FromSize_t((long)result);
}

static PyObject *
ExtendedContextSere_atomic_name(ExtendedContextSere *self, PyObject *args) {
  unsigned int id;

  if (!self)
    return NULL;

  if (!PyArg_ParseTuple(args, "I", &id))
    return NULL;

  const char* name = NULL;

  int r = sere_context_extended_atomic_name(self->sere, id, &name);
  if (r != 0) {
    PyErr_SetString(PyExc_ValueError, "incorrect atomic predicate index");
    return NULL;
  }
  return PyBytes_FromString(name);
}

static PyObject *
ExtendedContextSere_set_atomic(ExtendedContextSere *self, PyObject *args) {
  unsigned int id;

  if (!self)
    return NULL;

  if (!PyArg_ParseTuple(args, "I", &id))
    return NULL;

  int r = sere_context_extended_set_atomic(self->sere, id);
  if (r != 0) {
    PyErr_SetString(PyExc_ValueError, "incorrect atomic predicate index");
    return NULL;
  }

  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef ExtendedContextSere_methods[] = {
    {
     "atomic_count",
     (PyCFunction) ExtendedContextSere_atomic_count,
     METH_NOARGS,
     "Returns number of atomic predicate in SERE"
    }, {
     "atomic_name",
     (PyCFunction) ExtendedContextSere_atomic_name,
     METH_VARARGS,
     "Returns name of a particular atomic predicate"
    }, {
     "set_atomic",
     (PyCFunction) ExtendedContextSere_set_atomic,
     METH_VARARGS,
     "Sets a particular atomic predicate to TRUE"
    }, {
     "reset",
     (PyCFunction) ExtendedContextSere_reset,
     METH_NOARGS,
     "Resets SERE context"
    }, {
     "advance",
     (PyCFunction) ExtendedContextSere_advance,
     METH_NOARGS,
     "Feeds SERE context with new event"
    }, {
     "get_result",
     (PyCFunction) ExtendedContextSere_get_result,
     METH_NOARGS,
     "Returns matching results"
    }, {
     "to_dot",
     (PyCFunction) ExtendedContextSere_to_dot,
     METH_VARARGS,
     "Write Graphviz's DOT file"
    },
    {NULL}  /* Sentinel */
};

PyTypeObject ExtendedContextSereType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "sere.ExtendedContext",
    .tp_doc = "SERE context",
    .tp_basicsize = sizeof(ExtendedContextSere),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = (destructor) ExtendedContextSere_dealloc,
    .tp_methods = ExtendedContextSere_methods,
};

PyObject *
sere_load_extended(PyObject *self, PyObject *args) {
  const char* content;
  Py_ssize_t content_size;

  if (!PyArg_ParseTuple(args, "y#", &content, &content_size))
    return NULL;

  ExtendedContextSere* context = ALLOC_PY_OBJECT(ExtendedContextSere);

  if (context == NULL) {
    return NULL;
  }

  int r = sere_context_extended_load(content, (size_t)content_size, &context->sere);

  if (r != 0) {
    Py_XDECREF((PyObject *)context);
    context = NULL;
    Py_INCREF(Py_None);
    return Py_None;
  }

  return (PyObject *)context;
}

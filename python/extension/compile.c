#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "helpers.h"
#include "api/sere.h"

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

PyTypeObject CompiledSereType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "sere.Compiled",
    .tp_doc = "Compiled objects",
    .tp_basicsize = sizeof(CompiledSere),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = (destructor) CompiledSere_dealloc,
    .tp_methods = CompiledSere_methods,
};

PyObject *
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

  if (compiled == NULL) {
    return NULL;
  }

  int r = sere_compile(expr,
                       &opts,
                       &compiled->result);

  if (r != 0) {
    PyErr_SetString(PyExc_ValueError, compiled->result.error);
    Py_XDECREF((PyObject *)compiled);
    return NULL;
  }

  return (PyObject *)compiled;
}

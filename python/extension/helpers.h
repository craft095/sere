#ifndef PYTHON_HELPERS_H
#define PYTHON_HELPERS_H

#define ALLOC_PY_OBJECT(type) (type*)type##Type.tp_alloc(&type##Type, 1);

#endif // PYTHON_HELPERS_H

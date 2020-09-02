#ifndef __ASYNCMRCACHE_PY_H
#define __ASYNCMRCACHE_PY_H

#include <Python.h>

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

#define IS_PY3K 1

PyMODINIT_FUNC PyInit_asynmrcache(void);

#endif

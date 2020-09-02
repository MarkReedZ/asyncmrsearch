#include "module.h"
#include "mrclient.h"
#include "mrprotocol.h"

static struct PyModuleDef internals_module = {
    PyModuleDef_HEAD_INIT,
    "internals",
    NULL,
    -1, 
    NULL, /* m_methods */
    NULL, /* m_reload */
    NULL, /* m_traverse */
    NULL, /* m_clear */
    NULL  /* m_free */
};

PyMODINIT_FUNC PyInit_internals(void)
{
  if (PyType_Ready(&MrClientType  ) < 0) return NULL;
  if (PyType_Ready(&MrServerType  ) < 0) return NULL;
  if (PyType_Ready(&MrProtocolType) < 0) return NULL;
    
  PyObject *m = PyModule_Create(&internals_module);

  Py_INCREF(&MrClientType);
  PyModule_AddObject(m, "MrClient", (PyObject *)&MrClientType);
  Py_INCREF(&MrServerType);
  PyModule_AddObject(m, "MrServer", (PyObject *)&MrServerType);
  Py_INCREF(&MrProtocolType);
  PyModule_AddObject(m, "MrProtocol", (PyObject *)&MrProtocolType);

  return m;
}

#pragma once

#include <Python.h>
#include <stdbool.h>
#include "mrprotocol.h"
#include "mrserver.h"

typedef struct MrClient {
  PyObject_HEAD
  PyObject *b;
  PyObject *pause;
  PyObject *resume;
  MrServer **servers; 
  int num_servers;
  int num_healthy;
} MrClient;

extern PyTypeObject MrClientType;

PyObject *MrClient_new    (PyTypeObject* self, PyObject *args, PyObject *kwargs);
int       MrClient_init   (MrClient* self,    PyObject *args, PyObject *kwargs);
void      MrClient_dealloc(MrClient* self);

PyObject *MrClient_cinit(MrClient* self);
void MrClient_setup(MrClient *self);
PyObject *MrClient_pause_writing(MrClient* self);
PyObject *MrClient_resume_writing(MrClient* self);

void      MrClient_server_lost( MrClient* self );
PyObject *MrClient_server_back_online( MrClient* self );

PyObject *MrClient_get(MrClient* self, PyObject *key);
PyObject *MrClient_set(MrClient* self, PyObject *args);
PyObject *MrClient_stat(MrClient* self, PyObject *args);
//int MrClient_get(MrClient* self, char *key, void *fn, void *connection );
//PyObject *MrClient_set(MrClient* self, PyObject *args);


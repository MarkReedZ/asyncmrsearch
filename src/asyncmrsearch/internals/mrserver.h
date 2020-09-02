#pragma once

#include <Python.h>
#include <stdbool.h>
#include "mrprotocol.h"

typedef struct MrClient MrClient;

typedef struct MrServer {
  PyObject_HEAD
  MrProtocol *conn;

  PyObject *py_conns;
  PyObject *conns;
  PyObject *queues;
 
  bool closed; 
  char reconnecting;
  int next_conn;
  int num_conns;
  MrClient *client;
} MrServer;

//typedef struct MrServer MrServer;

extern PyTypeObject MrServerType;

PyObject *MrServer_new(PyTypeObject  *self, PyObject *args, PyObject *kwargs);
int       MrServer_init(    MrServer *self, PyObject *args, PyObject *kwargs);
void      MrServer_dealloc( MrServer *self );

void MrServer_cinit(MrServer* self, MrClient *client);
int MrServer_add_connection( MrServer *self, MrProtocol *conn) ;
void MrServer_connection_lost( MrServer* self, MrProtocol* conn );
void MrServer_next_connection( MrServer* self );

PyObject *MrServer_pause_writing(MrServer* self);
PyObject *MrServer_resume_writing(MrServer* self);
PyObject *MrServer_server_restored(MrServer* self);



//int MrServer_get( MrServer *self, char *k, void *fn, void *connection);
//int MrServer_set( MrServer *self, char *k, int ksz, char* d, int dsz );

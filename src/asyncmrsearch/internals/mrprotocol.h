#pragma once

#include "Python.h"
#include <stdbool.h>

typedef struct {
  PyObject_HEAD

  PyObject* transport;
  PyObject* write;
  PyObject* server;
  PyObject* client; // TODO remove
  PyObject* respq;
  PyObject* respq_put_nowait;

  bool closed;
  char *buf;
  int buf_sz;
  int max_sz;

} MrProtocol;

extern PyTypeObject MrProtocolType;


PyObject *MrProtocol_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
int       MrProtocol_init(   MrProtocol* self, PyObject *args, PyObject *kw);
void      MrProtocol_dealloc(MrProtocol* self);
PyObject *MrProtocol_pause_writing(MrProtocol* self);
PyObject *MrProtocol_resume_writing(MrProtocol* self);



PyObject* MrProtocol_connection_made (MrProtocol* self, PyObject* transport);
void*     MrProtocol_close           (MrProtocol* self);
PyObject* MrProtocol_connection_lost (MrProtocol* self, PyObject* args);
PyObject* MrProtocol_data_received   (MrProtocol* self, PyObject* data);
PyObject* MrProtocol_eof_received    (MrProtocol* self);

//int MrProtocol_asyncGet( MrProtocol* self, char *key, void *fn, void *connection );
//int MrProtocol_asyncSet( MrProtocol* self, char *key, char *val, int val_sz );


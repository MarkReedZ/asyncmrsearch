

#include "mrprotocol.h"
#include "mrclient.h"
#include "Python.h"
#include "common.h"
#include <errno.h>
#include <string.h>

static PyMethodDef MrProtocol_methods[] = {
  {"connection_made", (PyCFunction)MrProtocol_connection_made, METH_O,       ""},
  {"eof_received",    (PyCFunction)MrProtocol_eof_received,    METH_NOARGS,  ""},
  {"connection_lost", (PyCFunction)MrProtocol_connection_lost, METH_VARARGS, ""},
  {"data_received",   (PyCFunction)MrProtocol_data_received,   METH_O,       ""},
  {"pause_writing",   (PyCFunction)MrProtocol_pause_writing,   METH_NOARGS,  ""},
  {"resume_writing",  (PyCFunction)MrProtocol_resume_writing,   METH_NOARGS,  ""},
  {NULL}
};


PyTypeObject MrProtocolType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "mrasync.MrProtocol",      /* tp_name */
  sizeof(MrProtocol),          /* tp_basicsize */
  0,                         /* tp_itemsize */
  (destructor)MrProtocol_dealloc, /* tp_dealloc */
  0,                         /* tp_print */
  0,                         /* tp_getattr */
  0,                         /* tp_setattr */
  0,                         /* tp_reserved */
  0,                         /* tp_repr */
  0,                         /* tp_as_number */
  0,                         /* tp_as_sequence */
  0,                         /* tp_as_mapping */
  0,                         /* tp_hash  */
  0,                         /* tp_call */
  0,                         /* tp_str */
  0,                         /* tp_getattro */
  0,                         /* tp_setattro */
  0,                         /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT,        /* tp_flags */
  "MrCache client protocol", /* tp_doc */
  0,                         /* tp_traverse */
  0,                         /* tp_clear */
  0,                         /* tp_richcompare */
  0,                         /* tp_weaklistoffset */
  0,                         /* tp_iter */
  0,                         /* tp_iternext */
  MrProtocol_methods,          /* tp_methods */
  0,                         /* tp_members */
  0,//Protocol_getset,           /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)MrProtocol_init,   /* tp_init */
  0,                         /* tp_alloc */
  MrProtocol_new,              /* tp_new */
};


static char *memp_noreply = " noreply\r\n";

PyObject * MrProtocol_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  MrProtocol* self = NULL;
  DBG printf("Mr protocol new\n");

  self = (MrProtocol*)type->tp_alloc(type, 0);
  if(!self) goto finally;

  self->transport = NULL;
  self->write = NULL;
  self->client = NULL;
  self->server = NULL;

  self->max_sz = 32*1024;
  self->buf_sz = 0;
  self->buf = malloc( self->max_sz );

  finally:
  return (PyObject*)self;
}

void MrProtocol_dealloc(MrProtocol* self)
{
  free(self->buf);
  Py_XDECREF(self->transport);
  Py_XDECREF(self->write);
  Py_XDECREF(self->respq);
  Py_XDECREF(self->respq_put_nowait);
  Py_TYPE(self)->tp_free((PyObject*)self);
}

int MrProtocol_init(MrProtocol* self, PyObject *args, PyObject *kw)
{
  DBG printf("Mr protocol init\n");
  self->closed = true;

  if(!PyArg_ParseTuple(args, "O", &self->server)) return -1;
  Py_INCREF(self->server);

  return 0;
}

PyObject* MrProtocol_connection_made(MrProtocol* self, PyObject* transport)
{
  DBG printf("MrProtocol conn made\n");
  self->transport = transport;
  Py_INCREF(self->transport);

  self->closed = false;

  if(!(self->write      = PyObject_GetAttrString(transport, "write"))) return NULL;

  // Add connection sets my self->respq
  MrServer_add_connection( (MrServer*)(self->server), self );
  if(!(self->respq_put_nowait = PyObject_GetAttrString(self->respq, "put_nowait"))) return NULL;
  DBG printf("MrProtocol conn made done\n");
  Py_RETURN_NONE;
}

void* MrProtocol_close(MrProtocol* self)
{
  DBG printf(" proto close %p\n",self);
  void* result = self;


  PyObject* close = PyObject_GetAttrString(self->transport, "close");
  if(!close) return NULL;
  PyObject* tmp = PyObject_CallFunctionObjArgs(close, NULL);
  Py_XDECREF(close);
  if(!tmp) return NULL;
  Py_DECREF(tmp);
  self->closed = true;

  return result;

}

PyObject* MrProtocol_eof_received(MrProtocol* self) {
  DBG printf("MrProtocol eof received\n");
  Py_RETURN_NONE; // Closes the connection and conn lost will be called next
}

PyObject* MrProtocol_connection_lost(MrProtocol* self, PyObject* args)
{
  DBG printf("MrProtocol conn lost\n");
  self->closed = true;
  MrServer_connection_lost((MrServer*)self->server, self );
  Py_RETURN_NONE;
}

PyObject *MrProtocol_pause_writing(MrProtocol* self) {
  //MrClient_pause_writing((MrClient*)self->client);
  Py_RETURN_NONE;
}
PyObject *MrProtocol_resume_writing(MrProtocol* self) {
  //MrClient_resume_writing((MrClient*)self->client);
  Py_RETURN_NONE;
}

void buf_append(MrProtocol* self, char *data, int len ) {
  //DBG printf(" append cur %d \n", c->cur_sz);
  if ( (self->buf_sz + len) > self->max_sz ) {
    while ( (self->buf_sz + len) > self->max_sz ) self->max_sz <<= 1;
    self->buf = realloc( self->buf, self->max_sz );
  }
  memcpy( self->buf + self->buf_sz, data, len );
  self->buf_sz += len;
  //DBG printf(" append cur now %d \n", c->cur_sz);
  //DBG print_buffer( c->buf, c->cur_sz );

}

PyObject* MrProtocol_data_received(MrProtocol* self, PyObject* data)
{

  if (self->closed) Py_RETURN_NONE;
  DBG printf("memcached protocol - data recvd\n");
  DBG PyObject_Print( data, stdout, 0 ); 
  DBG printf("\n");

  PyObject *o;

  char *p, *start;
  Py_ssize_t l;
  if(PyBytes_AsStringAndSize(data, &start, &l) == -1) Py_RETURN_NONE;
  int data_left = l;

  // If we have partial data for this connection
  if ( self->buf_sz ) {
    buf_append( self, start, l );
    p = self->buf;
    data_left = self->buf_sz;
    self->buf_sz = 0;
  } else {
    p = start;
  }


  int num = 0;
  do {

    if ( data_left < 2 ) {
      buf_append( self, p, data_left );
      Py_RETURN_NONE;
      printf("data left < 2\n");
      exit(1);
    }
    int cmd = p[1];

    if ( cmd == 1 ) { // get response
      if ( data_left < 6 ) {
        buf_append( self, p, data_left );
        Py_RETURN_NONE;
        data_left = 0;
        exit(1);
      }
      uint32_t sz  = *((uint32_t*)(p+2));
     
      if ( data_left < (6+sz*8) ) {
        buf_append( self, p, data_left );
        Py_RETURN_NONE;
      }
      p += 6;

      uint64_t *items = (uint64_t*)p;
      PyObject *ret = PyList_New(sz);
      for (int i = 0; i < sz; i++ ) {
        PyList_SET_ITEM( ret, i, PyLong_FromLong(items[i]) );
      }
      
      //PyObject *ret = PyBytes_FromStringAndSize( p, sz );


      if ( ret && sz > 0 ) {
        if(!(o = PyObject_CallFunctionObjArgs(self->respq_put_nowait, ret, NULL))) return NULL;
      } else {
        if(!(o = PyObject_CallFunctionObjArgs(self->respq_put_nowait, Py_None, NULL))) return NULL;
      }
      Py_DECREF(o);
      Py_XDECREF(ret);

      p += sz*8;
      data_left -= 6+sz*8;

    } else {
      //PyObject_Print( data, stdout, 0 ); 
      printf("Unexpected response opcode!!\n");
      MrProtocol_close(self);
      MrServer_connection_lost((MrServer*)self->server, self );
      Py_RETURN_NONE;
    }
  }
  while ( data_left );
/*
  if(PyBytes_AsStringAndSize(data, &start, &l) == -1) Py_RETURN_NONE;

  p = start;
  do {
    // No session found
    if ( p[0] == 'E' ) {
      p += 5;
      tMrCallback cb = self->queue[self->queue_start].cb;
      cb(self->queue[self->queue_start].connection, NULL, 0);
      self->queue_start = (self->queue_start+1)%self->queue_sz;
    }
    // Session found
    // TODO The session key length must be 32 , allow variable and check performance
    else if ( p[0] == 'V' ) {
      p += 50;
      int vlen = 0;
      while( *p != '\r' ) {
        vlen = (*p-'0') + 10*vlen;
        p += 1;
      } 
      p += 2;

      if ( l < (60+vlen) ) {
        printf("Partial memc response! vlen %d l %ld\n",vlen,l);
        PyObject_Print( data, stdout, 0 ); 
        printf("\n");
        exit(1);
      }

      char *buf = malloc( vlen );
      memcpy(buf, p, vlen);
      tMrCallback cb = self->queue[self->queue_start].cb;
      cb(self->queue[self->queue_start].connection, buf, vlen);
      free(buf);
      self->queue_start = (self->queue_start+1)%self->queue_sz;

      p += vlen + 7;  
        
    } else {
      printf("Bad memc response data len %ld\n", strlen(p));
      PyObject_Print( data, stdout, 0 ); 
      printf("\n");
      exit(1);
    }
  }
  while ( p < (start+l) );
*/

  Py_RETURN_NONE;
}



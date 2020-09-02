
#include <Python.h>
#include <stdbool.h>

#include "mrclient.h"
#include "common.h"
#include "module.h"
#include "city.h"
#include "mrserver.h"




static PyMethodDef MrClient_methods[] = {
  {"cinit", (PyCFunction)MrClient_cinit, METH_NOARGS,   ""},
  {"_get", (PyCFunction)MrClient_get, METH_O,   ""},
  {"_set", (PyCFunction)MrClient_set, METH_VARARGS,   ""},
  {"_stat", (PyCFunction)MrClient_stat, METH_VARARGS,   ""},
  {"server_back_online",  (PyCFunction)MrClient_server_back_online, METH_NOARGS,   ""},
  {"pause_writing",  (PyCFunction)MrClient_pause_writing, METH_NOARGS,   ""},
  {"resume_writing", (PyCFunction)MrClient_resume_writing, METH_NOARGS,   ""},
  {NULL}
};

PyTypeObject MrClientType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "internals.MrClient",       /* tp_name */
  sizeof(MrClient),          /* tp_basicsize */
  0,                         /* tp_itemsize */
  (destructor)MrClient_dealloc, /* tp_dealloc */
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
  0,
  0,                         /* tp_setattro */
  0,                         /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /* tp_flags */
  "MrClient",                /* tp_doc */
  0,                         /* tp_traverse */
  0,                         /* tp_clear */
  0,                         /* tp_richcompare */
  0,                         /* tp_weaklistoffset */
  0,                         /* tp_iter */
  0,                         /* tp_iternext */
  MrClient_methods,           /* tp_methods */
  0,                         /* tp_members */
  0,            /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)MrClient_init,    /* tp_init */
  0,                         /* tp_alloc */
  MrClient_new,              /* tp_new */
};

PyObject *MrClient_new(PyTypeObject* type, PyObject *args, PyObject *kwargs) {
  MrClient* self = NULL;
  self = (MrClient*)type->tp_alloc(type, 0);
  return (PyObject*)self;
}

void MrClient_dealloc(MrClient* self) {
}


#define MAX_SERVERS 8192
#define BUFLEN 2*1024*1024
static char buf[BUFLEN+1]; // TODO dynamically size the buffer
static int srvmap[MAX_SERVERS];

int MrClient_init(MrClient* self, PyObject *args, PyObject *kwargs) {

  if(!(self->pause      = PyObject_GetAttrString((PyObject*)self, "pause"))) return 1;
  if(!(self->resume     = PyObject_GetAttrString((PyObject*)self, "resume"))) return 1;

  return 0;
}

static int paused = 0;
PyObject *MrClient_cinit(MrClient* self) {
  PyObject *srvlist = PyObject_GetAttrString((PyObject*)self, "servers");
  if ( srvlist == NULL ) { printf("Server list must not be empty\n"); exit(1); }

  int n = PyList_GET_SIZE(srvlist);
  self->num_servers = 0;
  self->servers = malloc( n * sizeof(PyObject*) );
  for (int i=0; i < n; i++ ) {
    MrServer *o = (MrServer*)PyList_GET_ITEM(srvlist, i);
    self->servers[self->num_servers++] = o; 
    MrServer_cinit(o, self);
  }
  self->num_healthy = self->num_servers;
  MrClient_setup(self);

  Py_RETURN_NONE;
}

void MrClient_setup(MrClient *self) {

  self->num_healthy = 0;
  for ( int i=0; i < self->num_servers; i++ ) {
    if ( !self->servers[i]->reconnecting ) self->num_healthy += 1;
  }
  if ( self->num_healthy == 0 ) return;
  

  int seg = MAX_SERVERS / self->num_servers;
  for ( int i=0; i < MAX_SERVERS; i++ ) {
    int s = i/seg;
    while ( self->servers[s]->reconnecting ) { s = (s+1) % self->num_servers; }
    srvmap[i] = s;
  }

}
PyObject *MrClient_server_back_online(MrClient* self) {
  MrClient_setup(self);
  Py_RETURN_NONE;
}


PyObject *MrClient_pause_writing(MrClient* self) {
  paused = 1;
  PyObject_CallFunction(self->pause, NULL);
  Py_RETURN_NONE;
}
PyObject *MrClient_resume_writing(MrClient* self) {
  paused = 0;
  PyObject_CallFunction(self->resume, NULL);
  Py_RETURN_NONE;
}

void MrClient_server_lost( MrClient* self ) {
  MrClient_setup(self);
}

// TODO https://docs.python.org/3/c-api/buffer.html  Use buffer to modify an existing bytes object instead of creating here

PyObject *MrClient_get(MrClient* self, PyObject *terms) {

  if ( self->num_healthy == 0 ) {
    PyErr_SetString(PyExc_ValueError, "We lost connection to all the servers"); //TODO error class
//exc_data = Py_BuildValue("(is)", err.code, uv_strerror(err));
        //if (exc_data != NULL) {
            //PyErr_SetObject(PyExc_UVError, exc_data);
//PyExc_UVError = PyErr_NewException("pyuv.error.UVError", NULL, NULL);
    return NULL;
  }

  PyObject *o, *bytes;

  // TODO Accept unicode or bytes and throw if not
  if ( !PyList_Check(terms) ) {
    PyErr_SetString(PyExc_TypeError, "Argument must be a list of strings");
    return NULL;
  }

  int len = PyList_Size(terms);
  PyObject *term;
  Py_ssize_t sz;
  for (int i = 0; i<len; i++ ) {
  	char *t = PyUnicode_AsUTF8AndSize(PyList_GET_ITEM(terms,i), &sz);
    
    int s = srvmap[CityHash64(t, sz) & 0x1FFF];
    MrServer *srv = self->servers[s];
    MrServer_next_connection(srv);

    buf[0] = 0;
    buf[1] = 2;
    buf[2] = sz;
    memcpy(buf+3, t, sz);

    bytes = PyBytes_FromStringAndSize( buf, 3+sz );
    DBG PyObject_Print(bytes, stdout, 0);
    DBG printf("\n");
    if(!(o = PyObject_CallFunctionObjArgs(srv->conn->write, bytes, NULL))) return NULL;
    Py_DECREF(o);
    Py_DECREF(bytes);

    // TODO we return the response q so can't loop through terms 
    //      return a list of queues and await them then return to caller after merging the lists
    Py_INCREF(srv->conn->respq);
    return srv->conn->respq;

  }
 
  Py_RETURN_NONE; 
}

PyObject *MrClient_set(MrClient* self, PyObject *args) { //PyObject *key, PyObject *val) {

  if ( paused ) Py_RETURN_NONE;
  if ( self->num_healthy == 0 ) {
    PyErr_SetString(PyExc_ValueError, "We lost connection to all the servers"); //TODO error class
    return NULL;
  }

  PyObject *bytes;//, *txt;
  char *txt;
  uint64_t id;

  if (!PyArg_ParseTuple(args, "ks", &id, &txt)) return NULL;

  //Py_ssize_t len;
  //char *p = PyUnicode_AsUTF8AndSize(txt, &len);
  Py_ssize_t len = strlen(txt);
  //char *p = PyUnicode_AsUTF8AndSize(txt, &len);
  

  buf[0] = 0;
  buf[1] = 1;
  uint64_t *idp = (uint64_t*)(buf+2);
  *idp = id;

  uint64_t *txtlenp = (uint64_t*)(buf+10);
  *txtlenp = len;
  memcpy(buf+18, txt, len);

  // TODO PyObject_GetBuffer and Py_buffer modify a bytes object instead of creating a new one each time? Can you modify ob_size?
  //      Can you just modify this? ((PyBytesObject *)op)->ob_sval; The conn->write copies the data so its possible?
  bytes = PyBytes_FromStringAndSize( buf, 18+*txtlenp );

  int s = srvmap[*idp & 0x1FFF];
  MrServer *srv = self->servers[s];
  MrServer_next_connection(srv);

  DBG PyObject_Print(bytes, stdout, 0); 
  DBG printf("\n");

  PyObject *o; if(!(o = PyObject_CallFunctionObjArgs(srv->conn->write, bytes, NULL))) return NULL;

  Py_DECREF(o);
  Py_DECREF(bytes);
  Py_RETURN_NONE; 
}

PyObject *MrClient_stat(MrClient* self, PyObject *args) { //PyObject *key, PyObject *val) {
  PyObject *bytes;

  buf[0] = 0;
  buf[1] = 3;
  
  bytes = PyBytes_FromStringAndSize( buf, 2 );

  for ( int s=0; s<self->num_servers;s++ ) {
    if ( self->servers[s]->reconnecting ) continue;
    MrServer *srv = self->servers[s];
    PyObject *o;
    if(!(o = PyObject_CallFunctionObjArgs(srv->conn->write, bytes, NULL))) return NULL;
    Py_DECREF(o);
  }

  Py_DECREF(bytes);
  Py_RETURN_NONE; // There is no response, why was I returning the Q
}


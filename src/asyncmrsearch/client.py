
import asyncio
import socket, time
import os
import asyncmrsearch
from .server import Server

async def create_client( servers, loop=None, connection_timeout=1,lost_cb=None ):
  loop = loop if loop is not None else asyncio.get_event_loop()

  if not isinstance(servers, list):
    raise ValueError("The create_client server(s) argument takes a list of (host, port) tuples")

  pool_size = 1;
  c = Client( loop, servers, pool_size, connection_timeout, lost_cb=lost_cb)
  await c.setup_connections()
  return c
  
class Client(asyncmrsearch.CMrClient):
  def __init__(self, loop, servers, pool_size=2, connection_timeout=1, lost_cb=None):

    if not isinstance(servers, list):
      raise ValueError("The create_client server(s) argument takes a list of (host, port) tuples")

    super().__init__()
    self.loop = loop
    self.servers = []
    self.connection_timeout = connection_timeout
    self.pool_size = pool_size

    self.debug_data = b""
    self.lost_cb = lost_cb
    self._pause_waiter = None
    self.q = None

    for srv in servers:
      self.servers.append( Server(loop, self, srv[0], srv[1], pool_size=pool_size) )

    self.cinit()

  def pause(self):
    self._pause_waiter = self.loop.create_future()
  def resume(self):
    waiter = self._pause_waiter
    if waiter is not None:
      self._pause_waiter = None
      if not waiter.done():
        waiter.set_result(None)

  async def setup_connections(self):
    for srv in self.servers:
      await srv.setup_connections()
    

  async def close(self):
    for srv in self.servers:
      await srv.close()

  async def get(self, key):
    q = self._get(key)
    ret = await q.get()
    q.task_done()
    return ret

  async def add(self, itemid, txt):
    self._set(itemid, txt)
    #if self._pause_waiter != None:
      #await self._pause_waiter

  def stat(self):
    self._stat()


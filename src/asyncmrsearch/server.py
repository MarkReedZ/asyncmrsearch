import asyncio
import asyncmrsearch

import gc #DELME

class Server(asyncmrsearch.CMrServer):
  def __init__(self, loop, client, host, port, pool_size=1):
    self.loop = loop
    self.client = client
    self.pool_size = pool_size
    self.host = host
    self.port = port
    self.queues = []
    self.reconnecting = False
    self.connection_attempts = 0
    for x in range(pool_size):
      self.queues.append( asyncio.Queue(loop=self.loop) )
    #print(self.queues)
    self.conns = []
    self.transports = []

    #self._pause_waiter = None TODO here or client?

  async def setup_connections(self):
    try:
      for x in range(self.pool_size):
        fut = self.loop.create_connection(lambda: asyncmrsearch.MrProtocol(self), self.host, self.port)
        try:
          (trans, protocol) = await asyncio.wait_for(fut, timeout=None) #self.connection_timeout)
          self.conns.append( protocol )
          self.transports.append( trans )
        except asyncio.TimeoutError:
          print("TODO timeout on connection")
          exit(1)

    except ConnectionRefusedError:
      print("Could not connect to the mrsearch server(s)")
      exit(1)
    except Exception as e:
      print("setup_connections exception:", e)
      exit(1)

  def connection_timeout(self):
    pass

  async def close(self):
    for t in self.transports:
      t.close()

  def clear_queues(self):
    for q in self.queues:
      try:
        while not q.empty():
          q.get_nowait()
      except Exception as e:
        print( "WTF",e,type(q), q )
        print(self.queues)
        exit()

  def lost_connection(self):
    #print("A queues: ",self.queues)
    #if self.lost_cb: self.lost_cb() TODO? The lost conn cb doesn't really make sense? Maybe it makes sense when all conns to this server are down?
    #print( "    Lost connection to",s.host, "port",s.port )
    if not self.reconnecting:
      for t in self.transports:
        if not t.is_closing():
          t.close()
      self.conns = []
      self.transports = []
      self.reconnecting = True
      self.clear_queues()
      asyncio.ensure_future( self.reconnect() )

  async def reconnect(self):
    self.conns = []
    self.transports = []
    while True:

      if self.connection_attempts < 3:
        await asyncio.sleep(5)
      elif self.connection_attempts < 10:
        await asyncio.sleep(30)
      else:
        await asyncio.sleep(360)

      try:
        self.connection_attempts += 1
        for x in range(self.pool_size):
          fut = self.loop.create_connection(lambda: asyncmrsearch.MrProtocol(self), self.host, self.port)
          try:
            (trans, protocol) = await asyncio.wait_for(fut, timeout=1)
            self.conns.append( protocol )
            self.transports.append( trans )
          except asyncio.TimeoutError:
            print("Reconnect timed out to", s.host, "port", s.port)
            asyncio.ensure_future( self.reconnect() )
            return
        self.connection_attempts = 0
        self.reconnecting = False 
        self.server_restored()
        self.client.server_back_online()
        return
      except ConnectionRefusedError:
        print("Reconnect failed to", self.host, "port", self.port)
      except Exception as e:
        print(e)


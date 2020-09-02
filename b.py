
# Notes
# longer keys is faster because of the sse instruction on key > 16 TODO speed up the key < 16?
#

import asyncio
import asyncmrcache
import time

loop = asyncio.get_event_loop()

import uvloop
asyncio.set_event_loop_policy(uvloop.EventLoopPolicy())

mc = None
async def setup():
  global rc
  rc = await asyncmrcache.create_client("localhost", loop, pool_size=1)


async def bench():

  async def timget(func, *args, **kwargs):
    num = 0
    numfuts = 100
    for x in range(100):
      futs = []
      for y in range(numfuts):
        futs.append( func(*args,**kwargs) )
      v = await asyncio.gather(*futs)

    s = time.time()
    for x in range(1000):
      futs = []
      for y in range(numfuts):
        futs.append( func(*args,**kwargs) )
      v = await asyncio.gather(*futs)
      num+=numfuts
    e = time.time()
    print( num/(e-s)," Requests/second")
  
  
  async def tim(func, *args,**kwargs):
    num = 0
    s = time.time()
    for x in range(10000):
      v = await func(*args,**kwargs)
      num+=1
    e = time.time()
    print( num/(e-s)," Requests/second")

  print("\nBenchmarking get\n")
  await timget(rc.get, b"test")


  print("")
  print("")


loop.run_until_complete(setup())
loop.run_until_complete(bench())

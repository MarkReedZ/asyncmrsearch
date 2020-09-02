
import asyncio, time
import asyncmrcache

import uvloop
asyncio.set_event_loop_policy(uvloop.EventLoopPolicy())

def lcb(client):
  print("Lost connection")

async def run(loop):

  rc = await asyncmrcache.create_client("localhost", loop, pool_size=1,lost_cb=lcb)

  hit = 0
  miss = 0

  num = 600000
  keys = []
  for x in range(num):
    keys.append(b'test'+str(x).encode())

  for x in range(num):
    rc.set(keys[x], b"testingthisislongersurelyitcantfit") 
    if (x % 10000) == 0: 
      await asyncio.sleep(0.01)
      #for z in range(10000):
        #ret = await rc.get(z)
  for x in range(num):
    ret = await rc.get(keys[x])
    if ret:
      hit += 1
    else:
      print(keys[x], " misses??")
      print( await rc.get(keys[x]) )
      exit()
      miss += 1
  print( hit, miss )

  if 0: 
    hit = 0
    miss = 0
    for z in range(10000):
      ret = await rc.get(z)
      if ret:
        hit += 1
      else:
        miss += 1
    print( hit, miss )
    

  await rc.close()


if __name__ == '__main__':
  loop = asyncio.get_event_loop()
  loop.run_until_complete(run(loop))
  loop.close()



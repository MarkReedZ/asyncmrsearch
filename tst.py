
import asyncio, time
import asyncmrsearch

import tracemalloc
tracemalloc.start()

import uvloop
asyncio.set_event_loop_policy(uvloop.EventLoopPolicy())

def lcb(client):
  pass
  #test( client.debug_data )

async def run(loop):

  rc = await asyncmrsearch.create_client( [("localhost",7011)], loop, lost_cb=lcb)

  await rc.add(1,"this is a cat")
  await rc.add(2,"cat is great")
  await rc.add(3,"my cats are awesome")
  await rc.add(4,"my cat dog")

  print(await rc.get(["cat"]))

  return


if __name__ == '__main__':
  loop = asyncio.get_event_loop()
  loop.run_until_complete(run(loop))
  loop.close()
  print("DONE")



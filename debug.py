
import asyncio, time
import asyncmrcache

def err(data, i):
  print( "ERROR ", data[i-10:i+10] )
  o = open( "err.out", "wb" )
  o.write(data)
  o.close()
  #exit()

vlen = 100
#vlen = 4

def test(data):
  
  i = 0
  while i < len(data):
    if data[i + 0] != 0: err(data,i)
    if data[i + 1] != 1: err(data,i)
    for x in range(vlen):
      if data[i+6+x] != 97 and data[i+6+x] != 98: err(data,i+6+x)
    i += vlen+6

def lcb(client):
  test( client.debug_data )
    

async def run(loop):

  rc = await asyncmrcache.create_client("localhost", loop, pool_size=1,lost_cb=lcb)
  #await rc.set(1, b"test") 
  #await rc.set(1, b"Like any other social media site Facebook has length requirements when it comes to writing on the wall, providing status, messaging and commenting. Understanding how many characters you can use, enables you to more effectively use Facebook as a business or campaign tool. Private messaging is one of the main ways that people interact on Facebook. This type of direct messaging can be either an instant message (chat), or a regular email-type message. For both instant and regular messaging, there is a 20,000 character limit. A Facebook status may have character limits, but considering that it is at 63,206 characters, unless you are writing War and Peace, you should be fine. Facebook has raised this number 12 times to accommodate users status and feedback. Facebook wall posts have a 5000 character limit, but truncation begins at 477 characters. This enables someone to write a thoughtful response or create something similar to a blog.")
  x = b'a' * vlen
  y = b'b' * vlen

  if 0:
    await rc.set(b"test11", x )
    z = await rc.get(b"test11")
    print(len(z))
    return

  #await rc.set(b"key123", x )
  #z = await rc.get(b"key123")
  #print("YAY", len(z))
  #return

  num = 0
  await rc.set(b"test11", x )
  await rc.set(b"test12", y )
  futs = []
  st = time.time()
  for z in range(300):
    for y in range(100):
      futs.append( rc.get(b"test11") )
      num += 1
      #await asyncio.sleep(0.1)
    rets = await asyncio.gather(*futs)
    futs = []
    #test( rc.debug_data )
    #rc.clear_data()
  print( time.time()-st )
  
  await rc.close()
  return


if __name__ == '__main__':
  loop = asyncio.get_event_loop()
  loop.run_until_complete(run(loop))
  loop.close()



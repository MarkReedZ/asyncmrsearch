
#
# Random traffic to test Mrcache
#

vlen = 100

import asyncio, time, random
from collections import deque

#import tracemalloc
#tracemalloc.start()

hits = 0
misses = 0


vdata = [
  b"0"*vlen,
  b"1"*vlen,
  b"2"*vlen,
  b"3"*vlen,
  b"4"*vlen,
  b"5"*vlen,
  b"6"*vlen,
  b"7"*vlen,
  b"8"*vlen,
  b"9"*vlen
]

def getLastNum(k):
  digit = k[-1]
  return digit - ord("0")

def getNum(k):
  k = k[4:]
  return int(k)

def checkValue(k, v, n):
  global hits, misses
   
  if v == None:
    misses += 1
    return

  hits += 1
  #if getNum(k) > n:
    #if v != None:
      #print("Should have missed ", k, v, n)
      #exit(1)
    #return
  
  exp = vdata[getLastNum(k)] 
  if exp != v:
    print("n  : ", n)
    print("key: ", k)
    print("exp: ", exp)
    print("val: ", v)
    exit(1)

import asyncmrcache
import zstandard as zstd
#import objgraph
def lost_conn():
  pass

async def setup(rc):
  rc.hot = deque(maxlen=1000)
  for x in range(1000):
    rc.hot.append( ("test"+str(x)).encode("utf-8") )
  for k in rc.hot:
    await rc.set( k, vdata[getLastNum(k)] )
  

def getRandomHotKey(rc):
  return random.choice( rc.hot )

async def run(loop):

  rc = await asyncmrcache.create_client([("localhost",7000)], loop)
  await setup(rc)

  #snapshot1 = tracemalloc.take_snapshot()

  hval = b'h'*vlen*2
  val = b'v'*vlen;
 
  n = 1000
  nxt = 100000
  while 1:

    futs = []
    fut_keys = []

    # Do some hot gets
    for y in range(10):
      k = getRandomHotKey(rc)
      futs.append( rc.get(k) )
      fut_keys.append(k)
    
    # Do some hot sets
    for y in range(2):
      k = getRandomHotKey(rc)
      await rc.set( k, vdata[getLastNum(k)] )

    # Do some hits and misses
    if 1:
      for y in range(2):
        i = random.randrange( n-100, n-1 )
        k = ("test"+str(i)).encode("utf-8") 
        futs.append( rc.get(k) )
        fut_keys.append(k)
      for y in range(2):
        i = random.randrange( n+100, n+10000 )
        k = ("test"+str(i)).encode("utf-8") 
        futs.append( rc.get(k) )
        fut_keys.append(k)

    # Add new keys
    for y in range(3):
      k = ("test"+str(n)).encode("utf-8") 
      await rc.set( k, vdata[getLastNum(k)] )
      n += 1
      # Sometimes add new key to hot
      if random.randrange(200) < 2:
        rc.hot.append(k)
    
    rets = await asyncio.gather(*futs)

    for i in range(len(rets)):
      v = rets[i]
      k = fut_keys[i]
      checkValue(k, v, n)
      

    if n % 1000 == 0: 
      print(" ",n)
      #if n > 2000000:
        #break
    if n > nxt:
      nxt += 100000
      rc.stat()     
  
  #snapshot2 = tracemalloc.take_snapshot()
  #top_stats = snapshot2.compare_to(snapshot1, 'lineno')
  #print("[ Top 10 differences ]")
  #for stat in top_stats[:10]:
    #print(stat)

  await rc.close()
  return


if __name__ == '__main__':
  loop = asyncio.get_event_loop()
  try:
    loop.run_until_complete(run(loop))
  except KeyboardInterrupt:
    print( hits, misses, hits+misses )
  loop.close()



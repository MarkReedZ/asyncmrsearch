
import pytest
import asyncio, time
import asyncmrcache

@pytest.fixture(scope="session")
def event_loop():
    loop = asyncio.get_event_loop()
    yield loop

@pytest.fixture(scope="module")
async def client():
  rc = await asyncmrcache.create_client( [("localhost",7000)], asyncio.get_event_loop() )
  yield rc
  await rc.close()

@pytest.mark.asyncio
async def test_set(client):

  await client.set(b"key1",b"vvvv")
  val = b"a" * 200000
  await client.set(b"key2",val)
  val = b"a" * 2000000
  await client.set(b"key3",val)

  with pytest.raises(TypeError) as exc:
    await client.set("key1",b"value1")
    assert "argument 1 must be bytes" in str(exc.value)
    await client.set(b"key1","value1")
    assert "argument 2 must be bytes" in str(exc.value)

  key = b"a" * 65535
  await client.set(key,b"someval")

  key = b"a" * 65537
  val = b"a" * 2 * 1024 * 1024
  with pytest.raises(ValueError) as exc:
    await client.set(b"kkkk",val)
    assert "greater than the max" in str(exc.value)
    await client.set(key, b"fdsafsd")
    assert "key length greater than the maximum of 65535" in str(exc.value)

  await client.set(b"key1",b"value1")


@pytest.mark.asyncio
async def test_get(client):
  ret = await client.get(b"key1")
  assert ret == b"value1"
  ret = await client.get(b"key2")
  val = b"a" * 200000
  assert ret == val
  ret = await client.get(b"key3")
  val = b"a" * 2000000
  assert ret == val




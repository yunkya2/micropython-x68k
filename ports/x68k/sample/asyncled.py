import asyncio
import machine

async def blink(led, delay):
    l = machine.Pin(led, machine.Pin.OUT)
    for _ in range(20 // delay):
        print(f'toggle {led}')
        l.toggle()
        await asyncio.sleep(delay)

async def main():
    await asyncio.gather(blink(0, 0.2), blink(1, 0.5), blink(2, 1))

asyncio.run(main())

import x68k
import random
from struct import pack

t = x68k.TVRam()

for a in range(100):
    x0 = random.randint(0,767)
    y0 = random.randint(0,511)
    x1 = random.randint(0,767)
    y1 = random.randint(0,511)
    c = random.randint(1,15)
    t.line(0,x0,y0,x1-x0,y1-y0)

buf=bytearray(pack('2h',768,4))+bytearray(768//8*4)

for a in range(128):
    t.get(0,0,buf)
    t.rascpy(1,0,512//4-1,1,1)
    t.put(0,512-4,buf)

import x68k
from struct import pack
import random

x68k.crtmod(14, True)

for a in range(100):
    x0 = random.randint(0,255)
    y0 = random.randint(0,255)
    x1 = random.randint(0,255)
    y1 = random.randint(0,255)
    c = random.randint(0,0xffff)
    x68k.iocs(x68k.i.LINE,a1=pack('6h',x0,y0,x1,y1,c,0xffff))

    x0 = random.randint(0,255)
    y0 = random.randint(0,255)
    c = random.randint(0,0xffff)
    x68k.iocs(x68k.i.SYMBOL,a1=pack('2hl2bh2b',x0,y0,x68k.getaddr("X68000"),2,2,c,0,0))

import x68k
from struct import pack
from uctypes import addressof
import random

x68k.crtmod(14, True)

a = 0
while (a := a + 1) < 100:
    if x68k.iocs(x68k.i.B_SFTSNS) & 0x01:
        break
    if x68k.iocs(x68k.i.B_SFTSNS) & 0x80:
        a = 0

    x0 = random.randint(0,255)
    y0 = random.randint(0,255)
    x1 = random.randint(0,255)
    y1 = random.randint(0,255)
    c = random.randint(0,0xffff)
    x68k.iocs(x68k.i.LINE,a1=pack('6h',x0,y0,x1,y1,c,0xffff))

    x0 = random.randint(0,255)
    y0 = random.randint(0,255)
    c = random.randint(0,0xffff)
    x68k.iocs(x68k.i.SYMBOL,a1=pack('2hl2bh2b',x0,y0,addressof("X68000"),2,2,c,0,0))

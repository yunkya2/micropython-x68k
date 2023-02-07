import x68k
from struct import pack
import random

x68k.crtmod(0x10,True)

for a in range(200):
    x = random.randint(0,767)
    y = random.randint(0,511)
    r = random.randint(1,250)
    c = random.randint(1,15)
    
    x68k.iocs(x68k.i.CIRCLE,a1=pack('7h',x,y,r,c,0,360,256))

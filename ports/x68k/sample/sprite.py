import x68k
from random import randint
from binascii import unhexlify

def rgb(r,g,b):
    return (g << 11) + (r << 6) + (b << 1)

x68k.crtmod(6)
s = x68k.Sprite()
s.init()
s.clr()
s.disp()

s.defcg(0, b'\x00'*32, 0)
s.defcg(1, unhexlify(\
        "ffffffff" +\
        "feeeeeef" +\
        "feeeeeef" +\
        "feeeeeef" +\
        "feeeeeef" +\
        "feeeeeef" +\
        "feeeeeef" +\
        "ffffffff"), 0)

s.defcg(1, unhexlify(\
        "0000001111000000" +\
        "0000111111110000" +\
        "0001131111112000" +\
        "0011331111111200" +\
        "0113311111111120" +\
        "0131111111111120" +\
        "1111111111111122" +\
        "1111111111111122" +\
        "1111111111111122" +\
        "1111111111111122" +\
        "0111111111111220" +\
        "0111111111111220" +\
        "0011111111122200" +\
        "0001111112222000" +\
        "0000111122220000" +\
        "0000002222000000"))

s.palet(14, (rgb(28,0,0),rgb(31,0,0)))
s.palet(1, (rgb(28,28,28),rgb(15,28,28),rgb(31,31,31)))
s.palet(1, (rgb(10,28,28),rgb(10,15,28),rgb(31,31,31)),2)

s.bgfill(0,0x101)
s.bgdisp(0)

BALLS = const(2)
spx = []
spy = []
spdx = []
spdy = []
sppat = []
sppri = []
for i in range(BALLS):
    spx.append(randint(5,250 - 16))
    spy.append(randint(5,250 - 16))
    spdx.append(1 - randint(0,1)*2)
    spdy.append(1 - randint(0,1)*2)
    sppat.append(0x01 + ((i % 2)+1)*0x100)
    sppri.append(3 if i < BALLS / 2 else 1)

x68k.curoff()

while True:
    if x68k.iocs(x68k.i.B_SFTSNS):
        break
    s.bgput(0,randint(0,31),randint(0,31),0x100)
    for i in range(BALLS):
        s.set(i,spx[i]+16,spy[i]+16,sppat[i],sppri[i], 0)
        if spx[i] <= 0 or spx[i] >= 255 - 16:
            spdx[i] = -spdx[i]
        if spy[i] <= 0 or spy[i] >= 255 - 16:
            spdy[i] = -spdy[i]
        spx[i] += spdx[i] * 2
        spy[i] += spdy[i] * 2
    x68k.vsync()

x68k.curon()

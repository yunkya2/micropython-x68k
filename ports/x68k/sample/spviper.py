import x68k
from random import randint
from binascii import unhexlify
from struct import pack

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

BALLS = const(30)
SPREG = const(0xeb0000)
BGREG = const(0xebc000)

balls = bytearray()
for i in range(BALLS):
    x = randint(5,250 - 16)
    y = randint(5,250 - 16)
    dx = (1 - randint(0,1)*2) * 2
    dy = (1 - randint(0,1)*2) * 2
    pat = (0x01 + ((i % 2)+1)*0x100)
    pri = (3 if i < BALLS / 2 else 1)
    s.set(i, 0, 0, pat, pri, 0)
    balls += pack('4l',x, y, dx, dy, pat, pri)

@micropython.viper
def move(balls):
    bp = ptr32(balls)
    sp = ptr16(SPREG)
    bg = ptr16(BGREG)
    i = 0
    for j in range(BALLS):
        sp[j*4 + 0] = bp[i + 0] + 16
        sp[j*4 + 1] = bp[i + 1] + 16
        i += 4
    bg[int(randint(0,31)) + (int(randint(0,31))<<6)] = 0x100
    i = 0
    for j in range(BALLS):
        if bp[i] <= 0 or bp[i] >= 255 - 16:
            bp[i + 2] = bp[i + 2] * -1
        if bp[i + 1] <= 0 or bp[i + 1] >= 255 - 16:
            bp[i + 3] = bp[i + 3] * -1
        bp[i + 0] += bp[i + 2]
        bp[i + 1] += bp[i + 3]
        i += 4

x68k.curoff()
with x68k.Super():
    while True:
        if x68k.iocs(x68k.i.B_SFTSNS):
            break
        x68k.vsync()
        move(balls)
x68k.curon()

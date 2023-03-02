import x68k
from random import randint
from binascii import unhexlify
from struct import pack
import micropython
micropython.alloc_emergency_exception_buf(100)

def rgb(r,g,b):
    return (g << 11) + (r << 6) + (b << 1)

x68k.crtmod(6, True)
s = x68k.Sprite()
s.init()
s.clr()
s.disp()

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

s.palet(1, (rgb(28,28,28),rgb(15,28,28),rgb(31,31,31)))
s.palet(1, (rgb(10,28,28),rgb(10,15,28),rgb(31,31,31)),2)

BALLS = const(30)
class Balls:
    def __init__(self, s):
        self.sched_ref = self.move
        self.balls = bytearray()
        for i in range(BALLS):
            x = randint(5,250 - 16)
            y = randint(5,250 - 16)
            dx = (1 - randint(0,1)*2) * 2
            dy = (1 - randint(0,1)*2) * 2
            pat = (0x01 + ((i % 2)+1)*0x100)
            s.set(i, 0, 0, pat, 3, 0)
            self.balls += pack('4h', x, y, dx, dy)

    @micropython.viper
    def disp(self, arg):
        SPREG = const(0xeb0000)
        bp = ptr16(self.balls)
        sp = ptr16(SPREG)
        i = 0
        for j in range(BALLS):
            sp[j*4 + 0] = bp[i + 0] + 16
            sp[j*4 + 1] = bp[i + 1] + 16
            i += 4
        micropython.schedule(self.sched_ref, 0)

    @micropython.viper
    def move(self, arg):
        bp = ptr16(self.balls)
        i = 0
        for j in range(BALLS):
            if bp[i] <= 0 or bp[i] >= 255 - 16:
                bp[i + 2] = bp[i + 2] * -1
            if bp[i + 1] <= 0 or bp[i + 1] >= 255 - 16:
                bp[i + 3] = bp[i + 3] * -1
            bp[i + 0] += bp[i + 2]
            bp[i + 1] += bp[i + 3]
            i += 4

g=x68k.GVRam()
a = Balls(s)

x68k.curoff()
with x68k.Super(), x68k.IntVSync(a.disp, a):
    while True:
        if x68k.iocs(x68k.i.B_SFTSNS):
            break
        x0 = randint(0,255)
        y0 = randint(0,255)
        x1 = randint(0,255)
        y1 = randint(0,255)
        c = randint(1,7)
        g.box(x0, y0, x1, y1, c*2)
x68k.curon()

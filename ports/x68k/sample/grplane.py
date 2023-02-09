import x68k
import random
import machine

REG_GPIP = const(0xE88001)

x68k.crtmod(4, True)

g0 = x68k.GVRam(0)
g1 = x68k.GVRam(1)
g2 = x68k.GVRam(2)
g3 = x68k.GVRam(3)
x68k.vpage(15)

for a in range(10):
    x0 = random.randint(0,511)
    y0 = random.randint(0,511)
    x1 = random.randint(0,511)
    y1 = random.randint(0,511)
    c = random.randint(1,15)
    g0.line(x0, y0, x1, y1, c)

    x0 = random.randint(0,511)
    y0 = random.randint(0,511)
    c = random.randint(1,15)
    g1.symbol(x0, y0, "X68000", 2, 2, c)

    x0 = random.randint(0,511)
    y0 = random.randint(0,511)
    r = random.randint(1,200)
    c = random.randint(1,15)
    g2.circle(x0, y0, r, c)

    x0 = random.randint(0,511)
    y0 = random.randint(0,511)
    x1 = random.randint(0,511)
    y1 = random.randint(0,511)
    c = random.randint(1,15)
    g3.fill(x0, y0, x1, y1, c)

g0x = g1x = g2y = g3y = 0
while True:
    # check shift key to exit
    if x68k.iocs(x68k.i.B_SFTSNS):
        break

    # wait vblank
    with x68k.Super():
        while (machine.mem8[ REG_GPIP ] & 0x10) == 0:
            pass
        while (machine.mem8[ REG_GPIP ] & 0x10) != 0:
            pass

    g0.home(g0x, 0)
    g0x = (g0x + 2) % 512
    g1.home(g1x, 0)
    g1x = (g1x + 512 - 2) % 512
    g2.home(0, g2y)
    g2y = (g2y + 2) % 512
    g3.home(0, g3y)
    g3y = (g3y + 512 - 2) % 512


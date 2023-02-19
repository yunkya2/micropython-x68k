import x68k
from struct import pack
import random
import math

REG_GPIP = const(0xe88001)
REG_SCRX = const(0xe80018)

@micropython.asm_m68k
def raster(a,b,x,en):
    moveal(REG_GPIP,a1)
    moveal(REG_SCRX,a2)
    moveal(fp[8],a3)
    moveal(fp[12],a4)
    movel(fp[16],d6)
    movel(fp[20],d5)

    addw(d6,d6)
    andiw(0x1ff,d6)
    movew(255,d7)

    oriw(0x0700,sr)         # disable intr

    # wait for VDISP

    label(loop1)
    moveb([a1],d1)
    andib(0x10,d1)
    bnes(loop1)

    label(loop2)
    moveb([a1],d1)
    andib(0x10,d1)
    beqs(loop2)

    # wait for HSYNC

    label(loop3)
    moveb([a1],d1)
    bpls(loop3)

    label(loop4)
    moveb([a1],d1)
    bmis(loop4)

    # wait for HDISP

    label(loop5)
    moveb([a1],d1)
    bpls(loop5)

    # set scroll register

    movew([a3.inc],d0)
    movew(d0,[a2])

    tstw(d5)
    beqs(skip)

    addw([a4,d6.w],d0)
    movew(d0,[-2,a3])
    addqw(2,d6)
    andiw(0x1ff,d6)
    label(skip)

    # wait for HSYNC

    label(loop6)
    moveb([a1],d1)
    bmis(loop6)

    dbra(d7,loop3)
    andiw(0xf8ff,sr)        # enable intr


def main():
    x68k.crtmod(6, True)

    g0 = x68k.GVRam(0)
    g0.window(0,0,511,255)

    for a in range(30):
        x0 = random.randint(0,511)
        y0 = random.randint(0,255)
        c = random.randint(1,0xf)
        g0.symbol(x0,y0,"X68000",2,2,c,2)

    xpos=bytearray(256*2)
    sintable = bytearray(0)
    for x in range(256):
        r = int(9+4*math.sin(math.pi*x/64))
        sintable+=pack('h',int(r))

    x68k.curoff()
    with x68k.Super():
        for a in range(500):
            raster(xpos,sintable,a,1)
            raster(xpos,sintable,a,0)
    x68k.curon()

main()

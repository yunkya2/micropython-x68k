import x68k
import framebuf
import random

x68k.crtmod(14, True)
with x68k.Super():
    fb = framebuf.FrameBuffer(x68k.gvram(), 256, 256, framebuf.RGB565, 512)

    for a in range(100):
        x0 = random.randint(0,255)
        y0 = random.randint(0,255)
        x1 = random.randint(0,255)
        y1 = random.randint(0,255)
        c = random.randint(0,0xffff)
        fb.line(x0, y0, x1, y1, c)

        x0 = random.randint(0,255)
        y0 = random.randint(0,255)
        c = random.randint(0,0xffff)
        fb.text("X68000", x0, y0, c)

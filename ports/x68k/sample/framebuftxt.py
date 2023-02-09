import x68k
import framebuf
import random

#x68k.crtmod(16)
with x68k.Super():
    fb = framebuf.FrameBuffer(x68k.tvram(), 768, 512, framebuf.MONO_HLSB, 1024)

    for a in range(100):
        x0 = random.randint(0,767)
        y0 = random.randint(0,511)
        x1 = random.randint(0,767)
        y1 = random.randint(0,511)
        fb.line(x0, y0, x1, y1, 1)

        x0 = random.randint(0,767)
        y0 = random.randint(0,511)
        fb.text("X68000", x0, y0, 1)

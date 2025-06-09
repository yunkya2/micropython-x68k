import sys
import x68k

try:
    x68k.loadfnc('BASIC2/GRAPH.FNC')
    x68k.loadfnc('BASIC2/MOUSE.FNC')
except:
    print("Human68kシステムディスクのBASIC2ディレクトリをコピーしてください")
    sys.exit(1)

x68k.crtmod(12,True)

x = x68k.xarray_int()
y = x68k.xarray_int()
x0=0
y0=0

mouse(4)
mouse(2)

for i in range(60 * 10):
    mspos(x, y)
    line(x0,y0,x[0],y[0],rgb(31,31,0),0xaaaa)
    x0 = x[0]
    y0 = y[0]
    x68k.vsync()

mouse(0)

sys.exit(0)

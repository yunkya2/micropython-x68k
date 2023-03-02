import time

COUNT = const(50000)

def loop():
    for _ in range(COUNT):
        pass

@micropython.native
def loop_native():
    for _ in range(COUNT):
        pass

@micropython.viper
def loop_viper():
    for _ in range(COUNT):
        pass

def measure(fn, name):
    a=time.ticks_ms()
    fn()
    b=time.ticks_ms()
    print(name + " : {}sec".format((b - a)/1000))

measure(loop, "normal")
measure(loop_native, "native")
measure(loop_viper, "viper ")


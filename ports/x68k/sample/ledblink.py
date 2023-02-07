from machine import Pin
import time

for i in range(7):
    LED = Pin(i, Pin.OUT)
    for j in range(10):
            LED.toggle()
            time.sleep_ms(100)

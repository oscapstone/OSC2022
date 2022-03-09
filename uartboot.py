#!/bin/python3
import sys
import serial
import time

if(len(sys.argv) != 2):
    print("Usage: ./uartboot.py <serial>")
    exit(1)

port = '/dev/pts/' + sys.argv[1]
baudrate = 115200

ser = serial.Serial(port, baudrate)
with open('kernel8.img', 'rb') as f:
    data = f.read()
    kernal_size = str(len(data))
    print(f'kernal size: {kernal_size}')
    ser.write(kernal_size.encode().ljust(10, b'\0'))
    ser.flush()
    time.sleep(0.5)
    
    ser.write(data)
    ser.flush()
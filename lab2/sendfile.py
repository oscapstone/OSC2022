#!/usr/bin/env python3

import sys
import threading

devname = input("Enter a device name: ")
fname = input("Enter the kernel filename: ")

file = open(fname, "rb")

term = open(devname, "wb+", buffering=0)

def readtty():
    while True:
        byte = term.read(1)
        sys.stdout.write(byte.decode())

t = threading.Thread(target=readtty)

t.start()

kernbyte = file.read(1)
while kernbyte != b"":
    term.write(kernbyte)
    kernbyte = file.read(1)

sys.stdout.write("kernel is sent\n")
sys.exit(0)

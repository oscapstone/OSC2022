#! /usr/bin/python3

import sys
import struct


tty_name = sys.argv[1]

with open(tty_name, "wb+", buffering = 0) as tty:
    while(1):
        t = tty.read(1)
        print(t)
            

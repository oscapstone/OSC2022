from pwn import *
import sys

pty = int(sys.argv[1])
load_kernel = (len(sys.argv) >= 3) and sys.argv[2] == "upload"

#r = serialtube("/dev/pts/%d"%pty, convert_newlines=False)
r = serialtube("/dev/ttyS%d"%pty, convert_newlines=False)

if load_kernel:
    kernel = open("kernel8.img", "rb").read()
    input("@")
    r.send(str(len(kernel))+"\n")
    #print(r.recv())
    #r.interactive()
    print(r.recvuntil(b") : "))
    r.send(kernel)


r.interactive()
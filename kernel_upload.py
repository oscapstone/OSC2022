from pwn import *

kernel = open("kernel8.img", "rb").read()
#r = serialtube("/dev/pts/11", convert_newlines=False)
r = serialtube("/dev/ttyS5", convert_newlines=False)


input("@")
r.send(str(len(kernel))+"\n")
#print(r.recv())
#r.interactive()
print(r.recvuntil(b") : "))
r.send(kernel)


r.interactive()
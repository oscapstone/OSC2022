import os
import sys
import serial

uart = serial.Serial("/dev/pts/6")
uart.baudrate = 115200

print(uart.readline().replace(b'\r\n',b'').decode())
print(uart.readline().replace(b'\r\n',b'').decode())
print(uart.readline().replace(b'\r\n',b'').decode())
print(uart.readline().replace(b'\r\n',b'').decode())
print(uart.read_until(b"img size :").decode(), end='')

args = sys.argv
img = args[1]
size = str( os.path.getsize(img) )

uart.write(size.encode()+b'\n')
uart.readline()
print(uart.readline().replace(b'\r\n',b'').decode())

with open(img, "rb", buffering = 0) as kernel:
	for _ in range(int(size)):
		uart.write(kernel.read(1))
		print(uart.readline().replace(b'\r\n',b'').decode())

#print(uart.readline().replace(b'\r\n',b'').decode())

uart.close()
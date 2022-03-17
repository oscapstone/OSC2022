import os
import sys
import serial
import signal
from tqdm import tqdm


def handler(signum, frame):
        raise Exception("end of time")

def read_all():
        while True:
                print(uart.read().decode(), end='')

uart = serial.Serial("/dev/tty.usbserial-0001")
uart.baudrate = 115200

print(uart.readline().replace(b'\r\n',b'').decode())
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
        for i in tqdm(range(int(size))):
                tmp = kernel.read(1)
                uart.write(tmp)
                check = uart.readline().replace(b'\r\n',b'').decode()[-2:]
#               print(i, ': ', check, end='\r')
#               if (tmp.hex().upper() != check):
#                       print(tmp.hex().upper(), check)


timeout=0.01
cmd = ""
while cmd != "exit":
        signal.signal(signal.SIGALRM, handler)
        signal.setitimer(signal.ITIMER_REAL,.05)
        try:
                read_all()
        except Exception as e:
                pass

        cmd = input()
        uart.write((cmd+'\n').encode())
        uart.readline()

uart.close()
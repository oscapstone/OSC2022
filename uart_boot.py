import sys
import serial
import time

# def calculate_checksum(kb):
#     n = 0
#     for b in kb:
#         n = (n + b) % (2 ** 32)
#     return n
def send_string(_serial, s):
    _serial.write(bytes(s, "ascii"))

def send_int(_serial, number):
    number_in_bytes = number.to_bytes(4, byteorder='big')
    _serial.write(number_in_bytes)
kernel_path = "kernel8.img"
tty_path = "/dev/"
baud_rate = 115200

arg = input(tty_path)
tty_path += arg

tty_serial = serial.Serial(tty_path,baud_rate)
with open(kernel_path,"rb") as k:
    with open(tty_path, "wb", buffering = 0) as tty:

        kernel = k.read()
        k_size = len(kernel)
        

        print("kernel size: ",k_size)
        send_string(tty,"bootload\n")
        #tty.write(bytearray(start_code.encode("utf-8")))
        time.sleep(1)
        send_int(tty,k_size)

        time.sleep(1)
        tty.write(kernel)

        print("Send done")


import sys
import argparse
import time
import serial
from uart_client import getch

# command: sudo python3 send_img_win.py

def readStream(serialPort):
    time.sleep(1)
    while serialPort.in_waiting > 0:
        serialString = serialPort.read(1)
        try:
            c = serialString.decode("UTF-8")
            sys.stdout.write(c)
        except:
            print(serialString, end='')

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("--kernel", type=str, default="./kernel8.img", help="path to kernel")
    parser.add_argument("--port", type=str, default="COM3", help="port for uart")

    args = parser.parse_args()


    with open(args.kernel, "rb") as f:
        img = f.read()

    size = len(img)
    serialPort = serial.Serial(port=args.port, baudrate=115200, timeout=None)
    serialPort.write(size.to_bytes(4, 'little'))
    readStream(serialPort)

    for t, i in enumerate(img):
        serialPort.write(i.to_bytes(1, 'little'))
        serialPort.flush()
        serialString = serialPort.readline()
        print(t, serialString.decode("UTF-8")[:-1], sep=', ', end='\r')
        sys.stdout.flush()

    print(f"Transfer {size} Bytes Done")
    readStream(serialPort)


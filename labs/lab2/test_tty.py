import serial
import time
import argparse


if __name__ == "__main__":



    parser = argparse.ArgumentParser()

    parser.add_argument('tty_path', type=str, help='the path of tty device')

    args = parser.parse_args()
    tty_path = args.tty_path


    ser = serial.Serial(tty_path, 115200, timeout=0.5, bytesize=8, parity = 'N', stopbits=1)

    print(ser.name)
    print(ser.baudrate)
    print(ser.port)


    while True:

        # ser.write(bytes("hello", encoding='utf8'))
        ser.write(b'ls\n')
        ser.reset_output_buffer()
        # print("what you get")
        for line in ser.readlines():
            print(line)


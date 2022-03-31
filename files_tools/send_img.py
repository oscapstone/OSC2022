import os
import argparse
import time

# command: sudo python3 send_img.py --kernel ./kernel/build/kernel8.img

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("--kernel", type=str, default="./kernel/build/kernel8.img", help="path to kernel")
    parser.add_argument("--port", type=str, default="/dev/pts/2", help="port for uart")

    args = parser.parse_args()


    with open(args.kernel, "rb") as f:
        img = f.read()

    size = len(img)
    with open(args.port, "wb", buffering = 0) as tty:
        tty.write(size.to_bytes(4, 'little'))
        tty.write(img)

    print(f"Transfer {size} Bytes Done")
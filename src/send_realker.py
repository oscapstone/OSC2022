import sys
import os

if __name__ == "__main__":

    # check usage
    if len(sys.argv) < 3:
        print("wrong usage: use python send_realker.py <kernel image path> <serial device path>")
        exit(0)
    
    with open(sys.argv[2], "wb", buffering=0) as tty:
        with open(sys.argv[1], "rb", buffering=0) as kernel:
            # get the size of the kernel image
            kernel_size = os.path.getsize(sys.argv[1])
            print(f"sending {sys.argv[1]} ({kernel_size} bytes) to {sys.argv[2]}...")

            # send both size and the content of the kernel to the rbpi
            tty.write(kernel_size.to_bytes(4, "big"))
            print(kernel_size.to_bytes(4, "big"))
            for i in range(0, kernel_size):
                tty.write(kernel.read())
    
    print("finish sending")
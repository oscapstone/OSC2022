import sys
import os
from tqdm import tqdm
import time

def main(fd):
    path = "build/kernel8.img"
    kernel_size = os.path.getsize(path)
    kernel_img = open(path, "rb", buffering=0)
    tty = open(fd, "wb", buffering=0)

    start_code = "hello"
    
    tty.write(bytearray(start_code.encode("utf-8")))
    tty.write(kernel_size.to_bytes(4, "big"))
    #tty.write(0x01020304.to_bytes(4, "big"))
    
    for b in kernel_size.to_bytes(4, "big"):
        print(b)

    # time.sleep(5)
    print("Sending kernel8.img(%d bytes) to %s ...\n" % (kernel_size, sys.argv[1]))
    progress = tqdm(total=kernel_size)
    time.sleep(10)

    for i in range(0, kernel_size):
        tty.write(kernel_img.read())
        # time.sleep(1/100000)
        progress.update(1)
    
    tty.close()
    kernel_img.close()

if __name__ == '__main__':
    fd = sys.argv[1]
    main(fd)
import serial
from tqdm import tqdm

def readline():
    ret = b''
    t = ser.read()
    while t != b'\n':
        ret += t
        t = ser.read()
    return ret[:-1]

data = open('./kernel/kernel8.img', 'rb', buffering=0).read()
size = len(data)
ser = serial.Serial('/dev/cu.usbserial-0001', 115200)

# magic
ser.write(b'k87a')
ser.flush()
print(readline().decode())

# size
ser.write(int.to_bytes(size, 4, byteorder='little'))
ser.flush()
print(readline().decode())

# data
for el in tqdm(data):
    ser.write(int.to_bytes(el, length=1, byteorder='little'))
    ser.flush()

print("Sending Complete!")
print(readline().decode())

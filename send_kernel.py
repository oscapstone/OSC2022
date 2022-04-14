import serial

def readline():
    msg = b''
    t = ser.read()
    while t != b'\n':
        msg += t
        t = ser.read()
    return msg[:-1]

data = open('./kernel/kernel8.img', 'rb', buffering=0).read()
size = len(data)
ser = serial.Serial('/dev/cu.usbserial-0001', 115200)

# size
ser.write(int.to_bytes(size, 4, byteorder='little'))
ser.flush()
print(readline().decode())

# data
for d in data :
    ser.write(int.to_bytes(d, length=1, byteorder='little'))
    ser.flush()

print("[-] Successfully send the kernel8.img")
print(readline().decode())

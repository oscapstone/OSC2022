import serial

with open('kernel8.img', 'rb') as kernel:
    with serial.Serial('/dev/ttyUSB0', 115200) as ser:
        raw_data = kernel.read()
        ser.write("kernel_load\n".encode())
        ser.flush()

        raw_len = len(raw_data)
        ser.write((str(raw_len)+"\n").encode())
        ser.flush()
        
        for i in range(raw_len):
            ser.write(raw_data[i: i+1])
            ser.flush()
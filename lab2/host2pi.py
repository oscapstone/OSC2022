#!usr/bin/python3	# defines where the interpreter is located

from serial import Serial

with open("kernel8.img", "rb") as fileobj:		# read only as binary, "with as" make sure file will be close
	with Serial(port = '/dev/ttyUSB0', baudrate = 115200) as serial:
		#first_byte = fileobj.read()
		#length = len(first_byte)
		
		# size of the img
		fileobj.seek(0, 2)		# 2 means move read/write pointer to the end of the file
		length = fileobj.tell()	# position of the pointer
		print("size of kernel8.img :", length)
		fileobj.seek(0, 0)		# 0 means move read/write pointer to the start of the file
		
		# sen size to pi
		serial.write(str(length).encode() + b'\n')		# b convert str to the type: bytes, ref: pySerial doc website
		serial.flush()									# wait until all data is written
	
		print("Send img to pi...")
		for i in range(length):
			serial.write(fileobj[i:i+1])
			serial.flush()
            if i%100==0:
                print("{:>6}/{:>6} bytes".format(i, length))
        print("{:>6}/{:>6} bytes".format(length, length))
        print("Send finished!")


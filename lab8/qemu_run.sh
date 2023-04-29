# sudo chmod 777 /dev/ttyUSB0
# python3 ./script/send_file.py -s bootloader.img
# python3 ./script/send_file.py -s kernel/build/kernel8.img -t /dev/ttyUSB0
python3 ./script/send_file.py -s kernel/build/kernel8.img -t /dev/pts/14

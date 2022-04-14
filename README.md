# Operating Systems Capstone 2022

| GitHub account name | Student ID | Name   |
| ------------------- | ---------- | ------ |
| cl3nn0              | 310551034  | 張逸軍  |

## How to build

```bash
make
```

## How to Run on QEMU

```bash
make run
```

## How to Run on Rpi3

- put files in SD card

    - bootcode.bin

    - fixup.dat

    - start.elf

    - bcm2710-rpi-3-b-plus.dtb

    - bootloader.img

    - config.txt

    - initramfs.cpio

```bash
python3 send_kernel.py
screen /dev/cu.usbserial-0001 115200
```

## Debug with QEMU and GDB

### Run docker and copy directory to container

```bash
docker run --rm --privileged -it -v ~/Downloads/shared-docker:/root/share osc:2022
docker cp <directory> <container ID>:/root
```

### Modify makefile (mac -> ubuntu) & compile

```bash
sh ./cross.sh
make
```

### Run QEMU in docker

```bash
make debug
```

### Debug in docker

```bash
gdb-multiarch ./kernel/kernel8.elf
target remote :1234
```

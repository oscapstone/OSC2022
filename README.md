# OSC 2022

## Author

| 學號 | GitHub 帳號 | 姓名 | Email |
| --- | ----------- | --- | --- |
|`310551086`| `Artis24106` | `杜萬珩` | artis.hh.tu@gmail.com |

## How to build
> I develope on MacOS

### Install Tollchains for MacOS
- [messense/homebrew-macos-cross-toolchains](https://github.com/messense/homebrew-macos-cross-toolchains)

### Build kernel8.img
```
make
```

### Emulate with QEMU
```
make run
```

### Send Files into Container
```
make docker_cp
```

### Debug with QEMU and GDB
Use port 1234 for remote debugging:
```
make debug
```

I use kali container to debug, and [hugsy/gef](https://github.com/hugsy/gef) is installed for better gdb debugging experence.

Take debugging `kernel.img` for example:
```
docker exec -it my_kali fish
gdb-multiarch kernel.elf
gef-remote host.docker.internal:1234
```

## Process
First of all, some files should be put into the SD card:
```
bcm2710-rpi-3-b-plus.dtb
config.txt
initramfs.cpio
bootloader.img
```
- `bcm2710-rpi-3-b-plus.dtb`: device tree
- `config.txt`: rpi3 configuration file
- `initramfs.cpio`: cpio archive 
- `bootloader.img`: bootloader which will receive `kernel.img` via UART then execute the kernel

Send `kernel.img` via UART:
```
python3 send_kernel.py "/dev/cu.usbserial-0001"
```

Connect to the shell:
```
screen /dev/cu.usbserial-0001 115200
```
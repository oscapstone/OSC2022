# OSC Lab2


## Information
| Github      | Student ID | Name   |
| ----------- | ---------- | ------ |
| kksweet8845 | 310552015  | 戴宏諺 |

## Lab1 Project
- How to build
    - it will generate the `kernel8.img` file.
```
> make
```
- How to clean
```
> make clean
```

# Content

| Filename  | Description                    |
| --------- |:------------------------------ |
| sshell.c  | The simple shell               |
| bcm2357.h | Some config of bcm2357         |
| mailbox.c | The basic config of mailbox    |
| reboot.c  | Reboot config                  |
| stdlib.c  | Self impl stdlib               |
| string.c  | Self impl string               |
| uart.c    | Config of uart                 |
| linker.ld | linker script                  |
| Setup.S   | Initialization and entry point |


# Directory
```.

├── dts
│   ├── bcm2710-rpi-3-b-plus.dtb
│   └── bcm2710-rpi-3-b-plus.dts
├── include
│   ├── bcm2357.h
│   ├── cpio_config.h
│   ├── cpio.h
│   ├── ctype.h
│   ├── fdt_config.h
│   ├── fdt.h
│   ├── gpio.h
│   ├── heap.h
│   ├── limits.h
│   ├── list.h
│   ├── mailbox.h
│   ├── mem_armv8.h
│   ├── pfalloc.h
│   ├── printf.h
│   ├── reboot.h
│   ├── slab.h
│   ├── stdlib.h
│   ├── string.h
│   ├── strtol.h
│   ├── tty.h
│   ├── type.h
│   ├── uart_config.h
│   └── uart.h
├── labs
│   └── lab2
│       ├── initramfs.cpio
│       ├── kernel8.dump
│       ├── kernel8.img
│       ├── Makefile
│       ├── raspi3bcom
│       ├── README.md
│       ├── relo_bootloader.elf
│       ├── relo_bootloader.img
│       ├── relo_bootloader.log
│       ├── rootfs
│       │   ├── file1
│       │   └── file2
│       ├── sshell.elf
│       ├── sshell.log
│       └── test_tty.py
├── LICENSE
├── linker-script
│   ├── bootloader.ld
│   └── linker.ld
├── README.md
├── setup
│   └── Setup.S
└── src
    ├── main
    │   ├── app
    │   │   ├── echo.c
    │   │   ├── sshell.c
    │   │   └── testmbox.c
    │   ├── bootloader
    │   │   ├── bootloader.c
    │   │   ├── bootloader.o
    │   │   ├── relo_bootloader.c
    │   │   └── relo_bootloader.o
    │   ├── lib
    │   │   ├── cpio.c
    │   │   ├── ctype.c
    │   │   ├── fdt.c
    │   │   ├── heap.c
    │   │   ├── mailbox.c
    │   │   ├── pfalloc.c
    │   │   ├── reboot.c
    │   │   ├── stdlib.c
    │   │   ├── string.c
    │   │   ├── strtol.c
    │   │   ├── tty.c
    │   │   └── uart.c
    │   └── utils
    │       └── raspi3bcom.c
    └── test
        ├── dtb-test
        │   ├── dtb-test.c
        │   └── dtb-test.o
        └── strtol-test
            ├── strtol-test.c
            └── strtol-test.o
```
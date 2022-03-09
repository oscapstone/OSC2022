# OSC Lab1


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
├── include
│   ├── bcm2357.h
│   ├── gpio.h
│   ├── list.h
│   ├── mailbox.h
│   ├── mem_armv8.h
│   ├── reboot.h
│   ├── stdlib.h
│   ├── string.h
│   ├── type.h
│   └── uart.h
├── LICENSE
├── linker.ld
├── Makefile
├── README.md
├── Setup.S
└── src
    ├── mailbox.c
    ├── main
    │   └── sshell.c
    ├── reboot.c
    ├── stdlib.c
    ├── string.c
    └── uart.c
```





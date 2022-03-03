# My OSC 2022

## Author

| 學號 | GitHub 帳號 | 姓名 | Email |
| --- | ----------- | --- | --- |
|`310551091`| `b4435242` | `林柏均` | b4435242@gmail.com |

## How to build

**make**

compile kernel8.img

## How to run

**make run**

qemu test

## How to burn it into pi3

**make burn**

copy build/kernel8.img to /dev/sdb

## Architecture

**WIP**

## Directory structure


├── src\
│   ├── *.S\
│   ├── *.c\
│   ├── *.h\
├── build\
│   ├── kernel8.img\
│   ├── *.o\
├── Makefile

#!/bin/bash

qemu-system-aarch64 -M raspi3 -kernel $1 -display none -S -s

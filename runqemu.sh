#!/bin/bash

if [[ "$1" == "debug" ]]
then
    qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -s -S # -d in_asm
else
    qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio
fi
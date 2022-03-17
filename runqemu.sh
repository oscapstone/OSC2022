#!/bin/bash



if [[ "$2" == "debug" ]]
then
    qemu-system-aarch64 -M raspi3 -kernel $1 -initrd ./initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -display none -serial null -serial pty -s -S # -d in_asm
else
    qemu-system-aarch64 -M raspi3 -kernel $1 -initrd ./initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -display none -serial null -serial pty
fi
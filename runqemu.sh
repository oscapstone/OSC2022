#!/bin/bash



if [[ "$2" == "debug" ]]
then
    qemu-system-aarch64 -M raspi3 -kernel $1 -initrd ./initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -display vnc=0.0.0.0:0 -serial null -serial pty -s -S # -d in_asm
    #qemu-system-aarch64 -M raspi3 -kernel $1 -initrd ./lab5.cpio -dtb bcm2710-rpi-3-b-plus.dtb -display vnc=0.0.0.0:0 -serial null -serial pty -s -S # -d in_asm
else
    qemu-system-aarch64 -M raspi3 -kernel $1 -initrd ./initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -display vnc=0.0.0.0:0 -serial null -serial pty -vga std
    #qemu-system-aarch64 -M raspi3 -kernel $1 -initrd ./lab5.cpio -dtb bcm2710-rpi-3-b-plus.dtb -display vnc=0.0.0.0:0 -serial null -serial pty -vga std
fi
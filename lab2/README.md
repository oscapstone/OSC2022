# Lab 2

## Goals

- Implement the UART bootloader
- Try the initial ramdisk
- Know the device tree

## Initial Ramdisk

```bash
cd rootfs
find . | cpio -o -H newc > ../initramfs.cpio
cd ..
```

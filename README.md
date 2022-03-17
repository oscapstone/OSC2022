# Operating Systems Capstone 2022

This repository is homework submission for students

## Submission details

- GitHub account name: xdavidwu
- Student ID: 0716022
- Your name: 吳秉澔

### Build instructions

Requires meson, ninja/samurai, aarch64 GCC toolchain

Cross compiling target definition at arch/aarch64/cross.ini assumes toolchain
available under $PATH with triplet prefix `aarch64-none-elf-`, create another
definition base on it if that does not suit you.

```
cd src
meson build --cross-file arch/aarch64/cross.ini
ninja -C build
# kernel8.img will be at (src/)build/kernel8.img
```

For bootloader, replace src with bootloader on commands above. Additional
program for host to send kernel will be at `(bootloader/)build/send-kernel`.

Usage: `send-kernel DEVICE KERNEL INITRD`

Support for passing initrd currently is incomplete as it does not modify address
in fdt yet. The loading address is the same as qemu. If device is using a dtb
and initrd address is different, it won't work. In that case, you can pass
/dev/null to skip loading it, and use whatever is already loaded by device.

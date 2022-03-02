# Lab0 Note
---
## 1. How to write linker file
- https://blog.louie.lu/2016/11/06/10%E5%88%86%E9%90%98%E8%AE%80%E6%87%82-linker-scripts/
    ### Linker File
    ```
    SECTIONS
    {
      . = 0x80000; // location counter, start from 0x80000
      .text : // output section
      { 
        *(.text) // find .text as input file
      } 
    }
    ```

## 2. QEMU 
`qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -d in_asm`

```
-M machine?
-d enable specified debug log
-S freeze CPU at startup (use 'c' to start execution)???
-s shorthand for -gdb tcp::1234, debugger port
```
- output of Lab0
    - see Reference

## 3. How to create your bootable image
1. You need following things to create a bootable image
    - An FAT16/32 partition contains
        - Firmware for GPU.
        - Kernel image.(kernel8.img)
    - The image create by TA is already a bootable img
    - find your sd card
        - `lsblk`
        - `sudo ls /dev/sd` + tab
    - you can mount the SD card with this image to replace the kernel you want
        `sudo mount -t vfat /dev/sdc1 sd-card-mount`
        -   `sdc` is not mountable
    - After mounting you can see
        ```
        mount_dir
        ├── bootcode.bin
        ├── fixup.dat
        ├── kernel8.img
        └── start.elf
        ```
    - replace kernel8.img with yours
    - `sudo umount sd-card-mount`

## Problems
1. why two partition on SD card(sdc, sdc1)?
## Reference
```bash
> qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -d in_asm
----------------
IN: 
0x00000000:  580000c0  ldr      x0, #0x18
0x00000004:  aa1f03e1  mov      x1, xzr
0x00000008:  aa1f03e2  mov      x2, xzr
0x0000000c:  aa1f03e3  mov      x3, xzr
0x00000010:  58000084  ldr      x4, #0x20
0x00000014:  d61f0080  br       x4

----------------
IN: 
0x00000300:  d2801b05  movz     x5, #0xd8
0x00000304:  d53800a6  mrs      x6, mpidr_el1
0x00000308:  924004c6  and      x6, x6, #3
0x0000030c:  d503205f  wfe      
0x00000310:  f86678a4  ldr      x4, [x5, x6, lsl #3]
0x00000314:  b4ffffc4  cbz      x4, #0x30c

----------------
IN: 
0x00000300:  d2801b05  movz     x5, #0xd8
0x00000304:  d53800a6  mrs      x6, mpidr_el1
0x00000308:  924004c6  and      x6, x6, #3
0x0000030c:  d503205f  wfe      
0x00000310:  f86678a4  ldr      x4, [x5, x6, lsl #3]
0x00000314:  b4ffffc4  cbz      x4, #0x30c

----------------
IN: 
0x00000300:  d2801b05  movz     x5, #0xd8
0x00000304:  d53800a6  mrs      x6, mpidr_el1
0x00000308:  924004c6  and      x6, x6, #3
0x0000030c:  d503205f  wfe      
0x00000310:  f86678a4  ldr      x4, [x5, x6, lsl #3]
0x00000314:  b4ffffc4  cbz      x4, #0x30c

----------------
IN: 
0x0000030c:  d503205f  wfe      
0x00000310:  f86678a4  ldr      x4, [x5, x6, lsl #3]
0x00000314:  b4ffffc4  cbz      x4, #0x30c

----------------
IN: 
0x00080000:  d503205f  wfe      
0x00080004:  17ffffff  b        #0x80000

----------------
IN: 
0x0000030c:  d503205f  wfe      
0x00000310:  f86678a4  ldr      x4, [x5, x6, lsl #3]
0x00000314:  b4ffffc4  cbz      x4, #0x30c

----------------
IN: 
0x0000030c:  d503205f  wfe      
0x00000310:  f86678a4  ldr      x4, [x5, x6, lsl #3]
0x00000314:  b4ffffc4  cbz      x4, #0x30c
```
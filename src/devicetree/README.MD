# compile and decompile from/to dts
- sudo apt-get install device-tree-compiler
- dtc -I dtb -O dts bcm2710-rpi-3-b-plus.dtb > bcm2710-rpi-3-b-plus.dts (decompile)
- dtc -I dts -O dtb bcm2710-rpi-3-b-plus.dts > bcm2710-rpi-3-b-plus.dtb (compile)
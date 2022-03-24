all: 
	make -C kernel
	make -C bootloader

run:
	qemu-system-aarch64 -M raspi3b -kernel ./kernel/kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

debug:
	qemu-system-aarch64 -M raspi3b -kernel ./kernel/kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -s -S

cpio:
	cd rootfs && find . | cpio -o -H newc > ../initramfs.cpio

clean:
	make -C kernel clean
	make -C bootloader clean
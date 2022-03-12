.PHONY: clean directories out_elf run docker_cp

all: 
	make -C kernel
	make -C bootloader

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel.img -display none -serial null -serial stdio -initrd initramfs.cpio

run_cpio:
	qemu-system-aarch64 -M raspi3b -kernel kernel.img -display none -serial null -serial stdio -initrd initramfs.cpio

debug:
	qemu-system-aarch64 -M raspi3b -kernel kernel.img -display none -serial null -serial stdio -initrd initramfs.cpio -s -S

docker_cp:
	docker cp kernel/kernel.elf my_kali:/
	docker cp bootloader/bootloader.elf my_kali:/

cpio:
	cd rootfs && find . | cpio -o -H newc > ../initramfs.cpio

clean:
	rm -rf *.elf
	rm -rf $(BUILD_DIR)
	make -C kernel clean
	make -C bootloader clean
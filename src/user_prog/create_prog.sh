aarch64-linux-gnu-gcc -c svc.S
aarch64-linux-gnu-ld -T linker.ld -o svc.elf svc.o
aarch64-linux-gnu-objcopy -O binary svc.elf user_program.img
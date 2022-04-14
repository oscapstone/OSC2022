sed -i 's/raspi3b/raspi3/g' ./makefile
sed -i 's/aarch64-unknown-linux-gnu/aarch64-linux-gnu/g' ./bootloader/makefile
sed -i 's/aarch64-unknown-linux-gnu/aarch64-linux-gnu/g' ./kernel/makefile
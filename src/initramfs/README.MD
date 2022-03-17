# create new ascii format cpio archive
1. create files under rootfs
2. cd rootfs
3. find . | cpio -o -H newc > ../initramfs.cpio
4. cd ..
all:
	gcc -Wall ssd_fuse.c `pkg-config fuse3 --cflags --libs` -D_FILE_OFFSET_BITS=64 -o ssd_fuse
	gcc -Wall ssd_fuse_dut.c -o ssd_fuse_dut
	mkdir /tmp/ssd
	./ssd_fuse -d /tmp/ssd

clean:
	fusermount -u /tmp/ssd
	rm -rf /tmp/ssd ssd_fuse ssd_fuse_dut

test:
	./test.sh test1
	./test.sh test2

# Tb:
# ./ssd_fuse_dut /tmp/ssd/ssd_file l
# 	This show current logical size in ssd_file
# ./ssd_fuse_dut /tmp/ssd/ssd_file p
# 	This show current physical size in ssd_file
# ./ssd_fuse_dut /tmp/ssd/ssd_file r 100 0
# 	This will read 100 Byte at 0 offset
# ./ssd_fuse_dut /tmp/ssd/ssd_file w 200 10
# 	This will write 200 Byte at 10 offset
# echo “hello world” > /tmp/ssd/ssd_file

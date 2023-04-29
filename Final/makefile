all: 
	gcc -Wall ssd_fuse.c `pkg-config fuse3 --cflags --libs` -D_FILE_OFFSET_BITS=64 -o ssd_fuse
	gcc -Wall ssd_fuse_dut.c -o ssd_fuse_dut

clean:
	fusermount -zu /tmp/ssd
	rm -rf /tmp/ssd
	mkdir /tmp/ssd

run:
	./ssd_fuse -d /tmp/ssd


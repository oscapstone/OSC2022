#ifndef CPIO_CONFIG_H
#define CPIO_CONFIG_H



#define CPIO_MAGIC "070701"



#define C_MASK      0170000
#define C_ISSCK     0140000
#define C_ISSLK     0120000
#define C_ISRGF     0100000
#define C_ISBLK     0060000
#define C_ISDIR     0040000
#define C_ISCSD     0020000
#define C_ISFIFO    0010000
#define C_ISSUID    0004000
#define C_ISSGID    0002000
#define C_ISSTKB    0001000
#define C_PERM      00001ff  //* read/write/exec



#define CPIO_ENTRY_POINT 0x20000000



#endif
#ifndef __CPIO__
#define __CPIO__

#define __USE_QEMU__

#ifdef __USE_QEMU__
#define CPIO_ADDR  ((char *)0x8000000) // qemu
#else
#define CPIO_ADDR  ((char *)0x20000000) // raspi3
#endif
#define MAX_INITRAMFS_SIZE 0x100000  // 1M

#define USER_PROGRAM_ADDR 0x900000
#define MAX_USER_PROGRAM_SIZE 0x100000  // 1M

typedef struct
{
    // uses 8-byte	hexadecimal fields for all numbers
    char magic[6];    //determine whether this archive is written with little-endian or big-endian integers.
    char ino[8];      //determine when two entries refer to the same file.
    char mode[8];     //specifies	both the regular permissions and the file type.
    char uid[8];      // numeric user id
    char gid[8];      // numeric group id
    char nlink[8];    // number of links to this file.
    char mtime[8];    // Modification time of the file
    char filesize[8]; // size of the file
    char devmajor[8];
    char devminor[8];
    char rdevmajor[8];
    char rdevminor[8];
    char namesize[8]; // number of bytes in the pathname
    char check[8];    // always set to zero by writers and ignored by	readers.
}  __attribute__((packed)) cpio_header;

void cpio_list();
void cpio_cat(char *filename);
char * findFile(char *name);
void load_program(char *name);

#endif
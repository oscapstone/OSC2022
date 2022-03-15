#include "mini_uart.h"
#include "cpio.h"
#include "string.h"
#include "shell.h"
#define CPIO_HEADER_MAGIC "070701"
#define CPIO_FOOTER_MAGIC "TRAILER!!!"
#define CPIO_ALIGNMENT 4
#define cpio_addr ((volatile unsigned int*)(0x8000000))
typedef struct {
    char c_magic[6];      /* Magic header '070701'. */
    char c_ino[8];        /* "i-node" number. */
    char c_mode[8];       /* Permisions. */
    char c_uid[8];        /* User ID. */
    char c_gid[8];        /* Group ID. */
    char c_nlink[8];      /* Number of hard links. */
    char c_mtime[8];      /* Modification time. */
    char c_filesize[8];   /* File size. */
    char c_devmajor[8];   /* Major dev number. */
    char c_devminor[8];   /* Minor dev number. */
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];   /* Length of filename in bytes. */
    char c_check[8];      /* Checksum. */
}cpio_newc_header;

/* Transform the string type hex to unsigned long.  */
static unsigned long parse_hex_str(char *s, unsigned int max_len)
{
    unsigned long r = 0;
    unsigned long i;

    for (i = 0; i < max_len; i++) {
        r *= 16;
        if (s[i] >= '0' && s[i] <= '9') {
            r += s[i] - '0';
        }  else if (s[i] >= 'a' && s[i] <= 'f') {
            r += s[i] - 'a' + 10;
        }  else if (s[i] >= 'A' && s[i] <= 'F') {
            r += s[i] - 'A' + 10;
        } else {
            return r;
        }
        continue;
    }
    return r;
}

/* The pathname is followed by NUL bytes so that the total size of the fixed
     header plus pathname is a multiple	of four.  Likewise, the	file data is
     padded to a multiple of four bytes. */
/* align to multiple of align */
static unsigned long align_up(unsigned long n, unsigned long align)
{
    return (n + align - 1) & (~(align - 1));
}

int cpio_parse_header(cpio_newc_header *archive,
        char **filename, unsigned long *_filesize, char **data,
        cpio_newc_header **next)
{
    unsigned long file_size;
    unsigned long file_namesize;
    char *file_data;
    /* Ensure magic header exists. */
    if (strncmp(archive->c_magic, CPIO_HEADER_MAGIC,
                sizeof(archive->c_magic)) != 0)
        return -1;

    /* Get filename and file size. */
    file_size = parse_hex_str(archive->c_filesize, sizeof(archive->c_filesize));
    file_namesize = parse_hex_str(archive->c_namesize, sizeof(archive->c_namesize));

    /* filename is in after header. */
    *filename = ((char *)archive) + sizeof(cpio_newc_header);
    
    /* Ensure filename is not the trailer indicating EOF. */
    if (strncmp(*filename, CPIO_FOOTER_MAGIC, sizeof(CPIO_FOOTER_MAGIC)) == 0)
        return 1;

    file_data = (char *)align_up(((unsigned long)archive)
            + sizeof(cpio_newc_header) + file_namesize, CPIO_ALIGNMENT);
    *next = (cpio_newc_header *)align_up(((unsigned long)file_data) + file_size, CPIO_ALIGNMENT);
    *data = file_data;
    *_filesize = file_size;
    return 0;
}
void cpio_ls(){
    cpio_newc_header* cnh = (cpio_newc_header*)cpio_addr;
    cpio_newc_header* next_header;
    char *filename;
    char *filedata;
    unsigned long filesize;
    
    while(1){
        if(cpio_parse_header(cnh,&filename,&filesize,&filedata,&next_header)!=0)
            break;
        writes_n_uart(filename,parse_hex_str(cnh->c_namesize,sizeof(cnh->c_namesize)));
        writes_uart("\r\n");
        //writes_n_uart(filedata,parse_hex_str(cnh->c_filesize,sizeof(cnh->c_filesize)));
        //writes_uart("\r\n");
        cnh = next_header;
        
    }
    
}

void cpio_cat(){
    cpio_newc_header* cnh = (cpio_newc_header*)cpio_addr;
    cpio_newc_header* next_header;
    char *filename;
    char *filedata;
    unsigned long filesize;
    writes_uart("Filename: ");
    char* read_name;
    read_string(&read_name);
    
    while(1){
        if(cpio_parse_header(cnh,&filename,&filesize,&filedata,&next_header)!=0){
            writes_uart("File not found!\r\n");
            break;
        }
        if(strncmp(read_name,".",sizeof(cnh->c_namesize))==0){
            writes_uart(".: Is a directory\r\n");
            break;
        }
        else if(strncmp(read_name,filename,sizeof(cnh->c_namesize))==0){
            writes_uart(filedata);
            writes_uart("\r\n");
            break;
        }
        //writes_n_uart(filename,parse_hex_str(cnh->c_namesize,sizeof(cnh->c_namesize)));
        //writes_uart("\r\n");
        // writes_n_uart(filedata,parse_hex_str(cnh->c_filesize,sizeof(cnh->c_filesize)));
        // writes_uart("\r\n");
        cnh = next_header;
        
    }
}

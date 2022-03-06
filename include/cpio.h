#ifndef CPIO_H
#define CPIO_H

#include "cpio_config.h"

#ifndef WITH_STDLIB
#include "type.h"
#else
#include <stdint.h>
#endif

typedef struct {
    char    c_magic[6];
    char    c_ino[8];
    char    c_mode[8];
    char    c_uid[8];
    char    c_gid[8];
    char    c_nlink[8];
    char    c_mtime[8];
    char    c_filesize[8];
    char    c_devmajor[8];
    char    c_devminor[8];
    char    c_rdevmajor[8];
    char    c_rdevminor[8];
    char    c_namesize[8];
    char    c_check[8];
} cpio_newc_header_t;

typedef cpio_newc_header_t* cpio_newc_header_ptr_t;

typedef struct {
    cpio_newc_header_ptr_t header;
    char* pathname;
    void* data;
    uint32_t namesize;
    uint32_t filesize;
} cpio_newc_t;

typedef cpio_newc_t* cpio_newc_ptr_t;



uint32_t* _get_cpio_start_addr();

int cpio_parse(cpio_newc_ptr_t obj, uint32_t* addr);

int _cpio_list(uint32_t* addr);
void cpio_list();

cpio_newc_t* cpio_find(char* name);




int is_trailer(char* name);


#endif
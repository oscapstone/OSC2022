
#ifndef WITH_STDLIB
#include "type.h"
#else
#include <stdint.h>
#endif
#include "cpio.h"
#include "string.h"
#include "uart.h"
#include "heap.h"
#include "strtol.h"
#include "stdlib.h"


int cpio_parse(cpio_newc_ptr_t obj, uint32_t* addr) {

    obj->header = addr;
    char buf[9];

    if(memcmp(obj->header->c_magic, CPIO_MAGIC, 6) != 0)
        return -1;
    
    obj->pathname = ((char*)obj->header) + sizeof(cpio_newc_header_t);

    if(is_trailer(obj->pathname))
        return -1;

    memset(buf, '\0', 9 * sizeof(char));
    memcpy(buf, obj->header->c_namesize, 8);
    obj->namesize = _strtoul(buf, NULL, 16);

    memset(buf, '\0', 9 * sizeof(char));
    memcpy(buf, obj->header->c_filesize, 8);
    obj->filesize = _strtoul(buf, NULL, 16);
    obj->data = obj->pathname + obj->namesize;

    return 0;

}


int _cpio_list(uint32_t* addr) {

    cpio_newc_t cpio_obj;
    cpio_newc_t* cpio_ptr = &cpio_obj;
    cpio_ptr->header = addr;
    
    int i=0;
    int namesize;

    char buf[9];
    
    while(1) {
        if(memcmp(cpio_ptr->header->c_magic, CPIO_MAGIC, 6) != 0)
            break;

        cpio_ptr->pathname = ((char*)cpio_ptr->header) + sizeof(cpio_newc_header_t);

        if(is_trailer(cpio_ptr->pathname))
            break;

        memset(buf, '\0', 9 * sizeof(char));
        memcpy(buf, cpio_ptr->header->c_namesize, 8);
        uint32_t namesize = _strtoul(buf, NULL, 16);

        memset(buf, '\0', 9 * sizeof(char));
        memcpy(buf, cpio_ptr->header->c_filesize, 8);
        uint32_t filesize = _strtoul(buf, NULL, 16);

        cpio_ptr->data = (uint32_t)cpio_ptr->pathname + namesize;

        uart_write(cpio_ptr->pathname);
        uart_write("\n");

        cpio_ptr->header = (uint32_t)cpio_ptr->data + filesize;
        if(((uint32_t)cpio_ptr->header & 0x3))
            cpio_ptr->header = _ALIGN(cpio_ptr->header, 4);
            
        i++;
    }

    return i;

}

void cpio_list() {

    if(_cpio_list(_get_cpio_start_addr())  == 0) {
        uart_write("No initramdisk\n");
    }

}


cpio_newc_t* cpio_find(char* name) {


    cpio_newc_ptr_t cpio_obj = hmalloc(sizeof(cpio_newc_t));

    if(!cpio_obj) {
        return NULL;
    }

    cpio_obj->header = _get_cpio_start_addr();

    int ret;
    
    for( ret = cpio_parse(cpio_obj, _get_cpio_start_addr()); 
         ret == 0; ret = cpio_parse(cpio_obj, _ALIGN(cpio_obj->data+cpio_obj->filesize, 4))) {

        if(strcmp(cpio_obj->pathname, name) == 0) {
            return cpio_obj;
        }
    }


    return NULL;


}

uint32_t* _get_cpio_start_addr() {
    return (uint32_t*) CPIO_ENTRY_POINT;
}


int is_trailer(char* name) {

    if(strcmp(name, "TRAILER!!!") == 0)
        return 1;
    return 0;

}




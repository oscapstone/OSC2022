#include <stdint.h>
#include <stddef.h>
#include <kmalloc.h>
#include <initrd.h>
#include <string.h>
#include <uart.h>
#include <dtb.h>

char *INITRD_ADDRESS_START;
char *INITRD_ADDRESS_END;
int INITRD_INITIAL;

uint32_t hex2u32_8(char *buf)
{
    uint32_t num = 0;
    for(int i=0;i<8;i++){
        num <<= 4;
        num += (buf[i]>='A'?buf[i]-'A'+10:buf[i]-'0');
    }
    return num;
}

struct initrd_fdt_data{
    int found;
    char *initrd_start;
    char *initrd_end;
};

int initrd_fdt_callback(char* node, const char *name, int depth, void *data)
{
    if(depth != 2 || strcmp(name, "chosen")) return 0;
    struct initrd_fdt_data* ifdtdata = (struct initrd_fdt_data*) data;
    fdt_prop *prop;
    //uart_puts("initrd_fdt_callback() /chosen found");
    while(prop = fdt_getnextprop(node+1, &node)){
        uart_puts(prop->name);
        if(!strcmp(prop->name,"linux,initrd-start")){
            ifdtdata->initrd_start = (char *)(uint64_t)fdt32_ld((uint32_t*)prop->value);
        }
        if(!strcmp(prop->name,"linux,initrd-end")){
            ifdtdata->initrd_end = (char *)(uint64_t)fdt32_ld((uint32_t*)prop->value);
        }
    }
    ifdtdata->found = 1;
    return 1;
}

int initrd_init()
{
    uart_puts("initrd_init()");
    struct initrd_fdt_data ifdtdata;
    ifdtdata.initrd_start = 0;
    ifdtdata.initrd_end = 0;
    ifdtdata.found = 0;
    int fdt_res = fdt_traverse(initrd_fdt_callback, (void *)&ifdtdata);
    if(ifdtdata.found){
        uart_print("/chosen linux,initrd-start: 0x");
        uart_putshex((uint64_t)ifdtdata.initrd_start);
        uart_print("/chosen linux,initrd-end: 0x");
        uart_putshex((uint64_t)ifdtdata.initrd_end);
        INITRD_ADDRESS_START = ifdtdata.initrd_start;
        INITRD_ADDRESS_END = ifdtdata.initrd_end;
        return 0;
    }
    return -1;
}

INITRD_FILE* initrd_parse()
{
    if(!INITRD_INITIAL){
        int initres = initrd_init();
        if(!initres) INITRD_INITIAL = 1;
        else return 0;
    }
    char *current_ptr = (char *)INITRD_ADDRESS_START;
    char *filename = current_ptr + sizeof(struct cpio_newc_header);
    struct cpio_newc_header* cur_cpio;
    INITRD_FILE* prev = 0;
    INITRD_FILE* cur = 0;
    INITRD_FILE* head = 0;
    while((uint64_t)current_ptr < (uint64_t)INITRD_ADDRESS_END){
        cur_cpio = (struct cpio_newc_header*)current_ptr;
        filename = current_ptr + sizeof(struct cpio_newc_header);
        // uart_puts("=== New File ===");
        // uart_puts((char*)cur_cpio);
        // uart_puts((char*)filename);
        if(!strcmp("TRAILER!!!", filename)){
            break;
        }
        cur = (INITRD_FILE*)kmalloc(sizeof(INITRD_FILE));
        if(!head) head = cur;
        cur->nextfile = 0;
        cur->header = current_ptr;
        cur->filename = filename;
        cur->namesize = hex2u32_8(cur_cpio->c_namesize);
        cur->filesize = hex2u32_8(cur_cpio->c_filesize);
        //size_t namesize_pad = ((cur->namesize>>2)+1)<<2;
        //size_t filesize_pad = ((cur->filesize>>2)+1)<<2;
        // uart_print("Filesize: ");
        // uart_puts(cur_cpio->c_filesize);
        // uart_print("Namesize: ");
        // uart_puts(cur_cpio->c_namesize);
        // uart_print("Filesize (parsed): 0x");
        // uart_putshex(cur->filesize);
        // uart_print("Namesize (parsed): 0x");
        // uart_putshex(cur->namesize);
        cur->filecontent = (char *)(((((uint64_t)filename + cur->namesize - 1)>>2)+1)<<2);
        // uart_puts(cur->filecontent);
        current_ptr = (char *)(((((uint64_t)cur->filecontent + cur->filesize - 1)>>2)+1)<<2);
        if(prev){
            prev->nextfile = cur;
        }
        prev = cur;
    }
    return head;
}

INITRD_FILE* initrd_list()
{
    return initrd_parse();
}

INITRD_FILE* initrd_get(const char *filename)
{
    INITRD_FILE* list_head = initrd_parse();
    while(list_head){
        if(!strcmp(list_head->filename, filename)){
            return list_head;
        }
        list_head = list_head->nextfile;
    }
    return 0;
}
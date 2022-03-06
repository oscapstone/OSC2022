#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "fdt.h"
#include "list.h"
#include <stdlib.h>





#define PRINT32HEX(key, var)   printf("%s : 0x%08x\n", key, var)
#define PRINTSTR(key, addr)    printf("%s : %s\n", key, addr)
#define _addr(addr, off)       ((char*)addr + off) 

#define PRINTBYTE(key, ptr, len)                            \
    do {                                                    \
        printf("%s : ", key);                               \
        for(int i=0;i<len;i++) {                            \
            printf("%c", ((char*)ptr)[i]);                 \
        }                                                   \
        printf("\n");                                       \
    } while(0)
    


void dump_chnode(struct list_head *head, fdt_struct_t* tree);



int main(int argc, char* argv[]) {


    char* dtb_path = argv[1];
    struct stat dtb_stat;


    int dtbfd = open(dtb_path, O_RDONLY);


    if(fstat(dtbfd, &dtb_stat)) {
        printf("Error: cannot stat %s, %s\n", dtb_path, strerror(errno));
        return -1;
    }

    printf("The file size %08x\n", dtb_stat.st_size);

    uint32_t* dtb_ptr = (uint32_t*)mmap(NULL, dtb_stat.st_size,  PROT_NONE | PROT_READ, MAP_SHARED, dtbfd, 0);
    if(dtb_ptr == NULL) {
        printf("Map error %s\n", strerror(errno));
    }


    printf("%08x\n", *dtb_ptr);

    
    fdt_struct_t* fdt_tree = malloc(sizeof(fdt_struct_t));

    fdt_parse(dtb_ptr, fdt_tree, malloc);

    //* dump the address

    PRINT32HEX("magic", fdt_tree->header->magic);
    PRINT32HEX("totalsize", fdt_tree->header->totalsize);
    PRINT32HEX("off_dt_struct", fdt_tree->header->off_dt_struct);
    PRINT32HEX("off_dt_strings", fdt_tree->header->off_dt_strings);
    PRINT32HEX("off_mem_rscmap", fdt_tree->header->off_mem_rsvmap);
    PRINT32HEX("version", fdt_tree->header->version);
    PRINT32HEX("last_comp_version", fdt_tree->header->last_comp_version);
    PRINT32HEX("boot_cpuid_phys", fdt_tree->header->boot_cpuid_phys);
    PRINT32HEX("size_dt_strings", fdt_tree->header->size_dt_strings);
    PRINT32HEX("size_dt_struct", fdt_tree->header->size_dt_struct);


    fdt_node_t* nodeitem;


    list_for_each_entry(nodeitem, &fdt_tree->node_head, list) {

        PRINTSTR("Node Name", nodeitem->node_name);
        PRINT32HEX("Addr", nodeitem->addr);

        fdt_prop_accessor_t* propitem;

        list_for_each_entry(propitem, &nodeitem->prop_head, list) {
            PRINTSTR("Prop name", _addr(fdt_tree->strblk_addr, brl(propitem->prop->nameoff)));
            PRINTBYTE("Prop val", _addr(propitem->prop, sizeof(fdt_prop_t)), brl(propitem->prop->len));
        }

        dump_chnode(&nodeitem->chnode_head, fdt_tree);

    }



}


void dump_chnode(struct list_head *head, fdt_struct_t* tree) {



    fdt_node_t* nodeitem;

    list_for_each_entry(nodeitem, head, list) {
        PRINTSTR("Node Name", nodeitem->node_name);
        PRINT32HEX("Addr", nodeitem->addr);

        fdt_prop_accessor_t* propitem;
        list_for_each_entry(propitem, &nodeitem->prop_head, list) {
            PRINTSTR("Prop name", _addr(tree->strblk_addr, brl(propitem->prop->nameoff)));
            PRINTBYTE("Prop val", _addr(propitem->prop, sizeof(fdt_prop_t)), brl(propitem->prop->len));
        }

        dump_chnode(&nodeitem->chnode_head, tree);
    }

}










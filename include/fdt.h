#ifndef FDT_H
#define FDT_H


#include "fdt_config.h"
#include "list.h"

#ifndef WITH_STDLIB
#include "type.h"
#else
#include <stdint.h>
#endif

typedef struct fdt_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
} fdt_header_t;

typedef struct fdt_reserve_entry {
    uint64_t address;
    uint64_t size;
} fdt_reserve_entry_t;

typedef struct fdt_prop {
    uint32_t len;
    uint32_t nameoff;
} fdt_prop_t;





typedef struct fdt_prop_accessor {
    fdt_prop_t*         prop;
    struct list_head    list;
} fdt_prop_accessor_t;



typedef struct fdt_node {
    uint32_t            addr;
    char*               node_name;
    struct fdt_node*    parent_node;
    struct list_head    prop_head;
    struct list_head    chnode_head;
    struct list_head    list;
} fdt_node_t;


typedef struct fdt_struct {
    fdt_header_t*         header;
    fdt_reserve_entry_t*  rsvmap;
    void*                 strblk_addr;
    struct list_head      node_head;
    struct list_head      list;
} fdt_struct_t;




void fdt_init_node(fdt_node_t *node);
void fdt_init_prop(fdt_prop_accessor_t *prop_obj, void* prop_addr);


void fdt_parse(uint32_t *addr, fdt_struct_t* tree, void* (*mf)(size_t));
void fdt_dump(fdt_struct_t* tree);

uint32_t brl(uint32_t netlong);

uint32_t* align_l(uint32_t* addr, uint32_t len);


uint32_t _strlen_null(char* str);


#endif
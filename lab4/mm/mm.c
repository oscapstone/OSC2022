#include "mm/mm.h"

extern int __heap_start;
extern int __kernel_image_start;
extern int __kernel_image_end;

static struct list_head mem_rsvmap;
static struct list_head mem_unusedmap;
static struct mem_block memory_node;

void reserve_memory(uint64_t start, uint64_t end){
    printf("start: %p, end: %p\r\n", (void*)start, (void*)end);
}

void _create_memory_rsvmap(void *dtb){
    INIT_LIST_HEAD(&mem_rsvmap);
    // reseved memory in dtb
    fdt_parse_rsvmap((uint8_t*)dtb, reserve_memory);

    // reverse memroy for kernel image 
    reserve_memory((uint64_t)&__kernel_image_start, (uint64_t)&__kernel_image_end);
}

void _create_memory_unusedmap(){
    INIT_LIST_HEAD(&mem_unusedmap);
}

void* _get_memory_node(uint32_t token, fdt_node* node, fdt_property* prop, int32_t layer){
    static int found_memory_node = 0;
    uint32_t* pw;
    if(strcmp(node->name, "memory@0") == 0){
        found_memory_node = 1;
    }
    if(found_memory_node && strcmp(prop->name, "reg") == 0){
        pw = (uint32_t*)prop->value;
        memory_node.start = bswap32(pw[0]);
        memory_node.end = memory_node.start + bswap32(pw[1]);
        found_memory_node = 0;
        printf("memory@0  start: %p, end: %p\r\n", memory_node.start, memory_node.end);
    }
}
void mm_init(void *dtb){
    fdt_parser((uint8_t*)dtb, _get_memory_node);
    _create_memory_rsvmap(dtb);
    _create_memory_unusedmap();
}

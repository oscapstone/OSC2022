#include "mm/mm.h"

extern int __heap_start;
extern int __kernel_image_start;
extern int __kernel_image_end;
extern int __kernel_image_end;
extern int __EL1_stack_size;
extern int __EL1_stack;

static struct list_head mem_rsvmap;
static struct list_head mem_unusedmap;
static struct mem_node memory_node;
static struct page *page_table;

void reserve_memory(uint64_t start, uint64_t end){
    struct list_head *node;
    struct list_head *head;
    struct mem_block *mb = simple_malloc(sizeof(struct mem_block));
    struct mem_block *tmp_mb;

    end = ALIGN_UP(end, PAGE_SIZE);
    mb->start = start;
    mb->end = end;
    list_for_each(node, &mem_rsvmap){
        tmp_mb = list_entry(node, struct mem_block, list); 
        if(tmp_mb->start > start){
            head = node->prev;
            list_add(&mb->list, head);
            break;
        }
    }

    if(list_is_head(node, &mem_rsvmap)){
        list_add(&mb->list, node->prev);
    }
}

size_t get_reserved_size(){
    uint64_t start, end;
    struct list_head* node;
    struct mem_block* mb;
    size_t size = 0;
    list_for_each(node, &mem_rsvmap){
        mb = list_entry(node, struct mem_block, list);
        start = mb->start;
        end = mb->end;
        size += (end - start);
       // printf("start: %p, end: %p\r\n", start, end);
    }
    return size;
}

size_t get_unused_size(){
    uint64_t start, end;
    struct list_head* node;
    struct mem_block* mb;
    size_t size = 0;
    list_for_each(node, &mem_unusedmap){
        mb = list_entry(node, struct mem_block, list);
        start = mb->start;
        end = mb->end;
        size += (end - start);
        printf("start: %p, end: %p\r\n", start, end);
    }
    return size;
}

uint64_t _reserve_dtb(void *dtb){
    fdt_header header;
    fdt_header *pheader = &header;
    uint64_t start, end;
    size_t size = 0;

    fdt_parse_header((uint8_t*)dtb, pheader);
    size = ALIGN_UP(pheader->totalsize, PAGE_SIZE);

    start = (uint64_t)dtb;
    end = start + size;
    reserve_memory(start, end);
}

void* _reserve_cpio(uint32_t token, fdt_node* node, fdt_property* prop, int32_t layer){
    static uint64_t start, end;
    static uint8_t found_start = 0, found_end = 0;
    if(prop != NULL){
        if(strcmp(prop->name, "linux,initrd-start") == 0){
            start = bswap32(*(uint32_t*)prop->value);
            found_start = 1;
        }   
        if(strcmp(prop->name, "linux,initrd-end") == 0){
            end = bswap32(*(uint32_t*)prop->value);
            found_end = 1; 
        }
        if(found_start && found_end){
            found_start = found_end = 0;
            reserve_memory(start, end);
        }
    }
}

void _create_memory_rsvmap(void *dtb){
    uint64_t start, end;
    // reseved memory in dtb
    fdt_parse_rsvmap((uint8_t*)dtb, reserve_memory);

    // reverse memory for kernel image 
    reserve_memory((uint64_t)&__kernel_image_start, (uint64_t)&__kernel_image_end);

    // reserve memory for stack
    reserve_memory((uint64_t)&__EL1_stack - (uint64_t)&__EL1_stack_size, (uint64_t)&__EL1_stack);

    // reserve memory for device tree
    _reserve_dtb(dtb);

    // reserve memory for cpio
    fdt_parser(dtb, _reserve_cpio);
   
    // Create struct page table for mapping every physical frame to it
    page_table = simple_malloc(sizeof(struct page) * (memory_node.end >> PAGE_SHIFT));

    // reserve memory for simple memory allocator
    start = (uint64_t)&__heap_start;
    end = ALIGN_UP((uint64_t)simple_malloc_get_remainder(), PAGE_SIZE) + 8 * PAGE_SIZE;
    reserve_memory(start, end);
    
}

void _create_memory_unusedmap(){
    uint64_t unused_start = 0;
    struct list_head *node;
    struct mem_block *mb;
    struct mem_block *tmp_mb;

    list_for_each(node, &mem_rsvmap){
        mb = list_entry(node, struct mem_block, list);
        if(unused_start < mb->start){
            tmp_mb = (struct mem_block*)simple_malloc(sizeof(struct mem_block));

            tmp_mb->start = unused_start;
            tmp_mb->end = mb->start;
            list_add_tail(&tmp_mb->list, &mem_unusedmap);
        }

        unused_start = mb->end;
    }

    if(unused_start < memory_node.end){
        tmp_mb = (struct mem_block*)simple_malloc(sizeof(struct mem_block));
        tmp_mb->start = unused_start;
        tmp_mb->end = memory_node.end;
        list_add_tail(&tmp_mb->list, &mem_unusedmap);
    }
}

void* _get_memory_node(uint32_t token, fdt_node* node, fdt_property* prop, int32_t layer){
    static int found_memory_node = 0;
    uint32_t* pw;
    if(strcmp(node->name, "memory@0") == 0){
        memory_node.name = "memory@0";
        found_memory_node = 1;
    }
    if(found_memory_node && strcmp(prop->name, "reg") == 0){
        pw = (uint32_t*)prop->value;
        memory_node.start = bswap32(pw[0]);
        memory_node.end = memory_node.start + bswap32(pw[1]);
        found_memory_node = 0;
    }
}


void mm_init(void *dtb){
    INFO("Init mm subsystem..");
    // initialize global variable
    INIT_LIST_HEAD(&mem_rsvmap);
    INIT_LIST_HEAD(&mem_unusedmap);
    fdt_parser((uint8_t*)dtb, _get_memory_node);

    _create_memory_rsvmap(dtb);
    _create_memory_unusedmap();
    
    INFO("Memory node: %s, size: %p", memory_node.name, memory_node.end - memory_node.start);
    INFO("Memory reserved size: %p", get_reserved_size());
    INFO("Memory unused size  : %p", get_unused_size());
}

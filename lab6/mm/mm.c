#include "mm/mm.h"
#include "mm/mmu.h"

extern int __heap_start;
extern int __kernel_image_start;
extern int __kernel_image_end;
extern int __kernel_image_end;
extern int __EL1_stack_size;
extern int __EL1_stack;

struct list_head mem_rsvmap;
struct list_head mem_unusedmap;
struct mem_node memory_node;
struct page *mem_map;

void reserve_memory(uint64_t start, uint64_t end){
    struct list_head *node;
    struct list_head *head;
    struct mem_block *mb = simple_malloc(sizeof(struct mem_block));
    struct mem_block *tmp_mb;
    
    start = ALIGN_DOWN(start, PAGE_SIZE);
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
        LOG("unused map: start: %p, end: %p", start, end);
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
    INFO("1");
    fdt_parse_rsvmap((uint8_t*)dtb, reserve_memory);

    // reverse memory for kernel image 
    INFO("2");
    reserve_memory((uint64_t)&__kernel_image_start - UPPER_ADDR_SPACE_BASE, (uint64_t)&__kernel_image_end - UPPER_ADDR_SPACE_BASE);

    // reserve memory for stack
    INFO("3");
    reserve_memory((uint64_t)&__EL1_stack - (uint64_t)&__EL1_stack_size - UPPER_ADDR_SPACE_BASE, (uint64_t)&__EL1_stack - UPPER_ADDR_SPACE_BASE);

    // reserve memory for device tree
    INFO("4");
    _reserve_dtb((void*)((uint64_t)dtb - UPPER_ADDR_SPACE_BASE));

    // reserve memory for cpio
    INFO("5");
    fdt_parser(dtb, _reserve_cpio);
   
    // Create struct page table for mapping every physical frame to it
    INFO("6");
    mem_map = simple_malloc(sizeof(struct page) * (memory_node.end >> PAGE_SHIFT));

    // reserve memory for simple memory allocator
    INFO("7");
    start = (uint64_t)&__heap_start - UPPER_ADDR_SPACE_BASE;
    end = ALIGN_UP((uint64_t)simple_malloc_get_remainder() - UPPER_ADDR_SPACE_BASE, PAGE_SIZE) + 8 * PAGE_SIZE;
    reserve_memory(start, end);
    INFO("8");
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
            INFO("unused map: start = %p, end = %p", tmp_mb->start, tmp_mb->end);
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
    if(node != NULL && strcmp(node->name, "memory@0") == 0){
        memory_node.name = "memory@0";
        found_memory_node = 1;
    }
    if(found_memory_node && prop != NULL && strcmp(prop->name, "reg") == 0){
        pw = (uint32_t*)prop->value;
        memory_node.start = bswap32(pw[0]);
        memory_node.end = memory_node.start + bswap32(pw[1]);
        found_memory_node = 0;
    }
}


void mm_init(void *dtb){
    INFO("Init mm subsystem..");
    // initialize mm's global variable
    INIT_LIST_HEAD(&mem_rsvmap);
    INIT_LIST_HEAD(&mem_unusedmap);
    fdt_parser((uint8_t*)dtb, _get_memory_node);

    _create_memory_rsvmap(dtb);
    INFO("_create_memory_unusedmap");
    _create_memory_unusedmap();
    
    INFO("Memory node: %s, size: %p", memory_node.name, memory_node.end - memory_node.start);
    INFO("Memory reserved size: %p", get_reserved_size());
    INFO("Memory unused size  : %p", get_unused_size());
    
    // initialize buddy system 
    buddy_init();

    print_buddy_statistics();

  /*  if(debug)
        debug_slab();*/

    kmalloc_init();

/*    if(debug){
        debug_kmalloc();
        //debug_buddy();
        print_buddy_statistics();
    }*/

}

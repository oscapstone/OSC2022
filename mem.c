#include "mem.h"
#include "devicetree.h"
#include "uart.h"

// #define MEM_DEMO_LOG
#define BUDDY_MAX_ORDER 5
#define BUDDY_MAX_LEN (1 << BUDDY_MAX_ORDER)
#define FRAME_SIZE 4096
#define CHUNK_SIZE 32
#define RESERVED_LEN 32
extern unsigned char _start, _end;

unsigned long mem_position = (unsigned long)&_end;

void* simple_malloc(unsigned long size) {
    void *chunk = (void*)mem_position;
    mem_position += ((size-1) & -FRAME_SIZE) + FRAME_SIZE;
    return chunk;
}

unsigned long BUDDY_BASE;

typedef struct mem_frame{
    unsigned long address;
    unsigned int position;
    unsigned int order;
    unsigned int free;
    struct mem_frame *next;
    struct mem_frame *prev;
} mem_frame;

mem_frame *frame_array;
mem_frame *buddy_system;

typedef struct chunk{
    mem_frame *curr;
    struct chunk *next;
    struct chunk *prev;
    mem_frame chunk_slot[FRAME_SIZE/CHUNK_SIZE];
} chunk;

chunk *chunk_array;
chunk chunk_system;

int reserved_position[BUDDY_MAX_LEN] = {0};
int reserved_address[RESERVED_LEN][2] = {0};

void show_frame() {
#ifdef MEM_DEMO_LOG
    printf("|");
    int position = 0;
    while (position < BUDDY_MAX_LEN) {
        mem_frame *show = &frame_array[position];
        int re = 1 << (show->order);
        re *= 4;
        re--;
        while (re--) {
            if (show->free)
                printf(" ");
            else
                printf("*");
        }
        printf("|");
        position += 1 << show->order;
    }
    printf("\n");
#endif
}

void pop_page(mem_frame *my_frame) {
    my_frame->prev->next = my_frame->next;
    my_frame->next->prev = my_frame->prev;
}

void push_page(mem_frame *my_frame) {
    mem_frame *top = &buddy_system[my_frame->order];
    top->next->prev = my_frame;
    my_frame->next = top->next;
    top->next = my_frame;
    my_frame->prev = top;
}

mem_frame *find_address_frame(unsigned long address) {
    unsigned long position = (address - BUDDY_BASE) / 4096;
    mem_frame *target = &frame_array[(unsigned int)position];
    return target;
};

void split(mem_frame *my_frame) {
    pop_page(my_frame);
    my_frame->order--;

    mem_frame *buddy_frame = &frame_array[my_frame->position + (1 << my_frame->order)];
    buddy_frame->order = my_frame->order;

    my_frame->free = 1;
    buddy_frame->free = 1;

    push_page(buddy_frame);
    push_page(my_frame);
    show_frame();
};

mem_frame *check_merge(mem_frame *my_frame) {
    int master = my_frame->position % (2 << my_frame->order) ? 0 : 1;
    mem_frame *buddy_frame;
    if (master)
        buddy_frame = &frame_array[my_frame->position + (1 << my_frame->order)];
    else
        buddy_frame = &frame_array[my_frame->position - (1 << my_frame->order)];

    if (buddy_frame->free && buddy_frame->order == my_frame->order)
        return master ? my_frame : buddy_frame;
    else
        return 0;
    return 0;
};

void merge(mem_frame *my_frame) {
    mem_frame *merge_frame = check_merge(my_frame);
    if (merge_frame) {
        mem_frame *buddy_frame = &frame_array[merge_frame->position + (1 << merge_frame->order)];
        merge_frame->order++;
        buddy_frame->order++;

        buddy_frame->free = 0;
        
        pop_page(merge_frame);
        pop_page(buddy_frame);
        push_page(merge_frame);
        show_frame();
        if (my_frame->order < BUDDY_MAX_ORDER) merge(merge_frame);
    }
};

mem_frame *ask_mem(unsigned int need_order) {
    unsigned int current_order = need_order + 1;
    while (current_order <= BUDDY_MAX_ORDER) {
        mem_frame *target = buddy_system[current_order].next;
        if (!target) {
            current_order++;
        }
        else {
            while (current_order > need_order) {
                split(target);
                current_order--;
            }
            return target;
        }
    } 
    printf("out-of-memory");
    return 0;
}

mem_frame *buddy_malloc(unsigned int size) {
    // check size
    unsigned int need_order = 0;
    unsigned int tmp_size = size;

    while ((tmp_size-1) / FRAME_SIZE) {
        tmp_size = tmp_size / 2;
        need_order++;
    }

    mem_frame *target = buddy_system[need_order].next;
    if (!target) {
        target = ask_mem(need_order);
    }

    target->free = 0;
    show_frame();
    pop_page(target);

    return target;
}

void buddy_free(unsigned long address) {
    mem_frame *target = find_address_frame(address);
    target->free = 1;
    show_frame();
    push_page(target);
    merge(target);
};

chunk *init_chunk(mem_frame *target) {
    chunk *slot = &chunk_array[target->position];
    slot->curr = &slot->chunk_slot[0];
    slot->next = chunk_system.next;
    slot->next->prev = slot;
    chunk_system.next = slot;
    slot->prev = &chunk_system;
    for (int i=0; i<(FRAME_SIZE/CHUNK_SIZE); i++) {
        slot->chunk_slot[i].address = target->address + CHUNK_SIZE*i;
        slot->chunk_slot[i].free = 0;
        slot->chunk_slot[i].position = i;
    }
    slot->chunk_slot[0].order = FRAME_SIZE/CHUNK_SIZE;
    slot->chunk_slot[0].next = 0;
    slot->chunk_slot[0].prev = slot->curr;
    return slot;
};

mem_frame *slot_malloc(chunk *my_chunk, mem_frame *slot, unsigned int need_len) {
    mem_frame *bound = &my_chunk->chunk_slot[need_len+slot->position];
    bound->next = slot->next;
    slot->next->prev = bound;
    bound->prev = slot;
    bound->free = 1;
    bound->order = slot->order - need_len;
    slot->next = bound;
    slot->order = need_len;

    return slot;    
};

mem_frame *find_slot(unsigned int need_len) {
    chunk *target = chunk_system.next;
    mem_frame *slot;
    while (target) {
        slot = target->curr;
        while (slot) {
            if (slot->free && slot->order >= need_len) {
                slot = slot_malloc(target, slot, need_len);
                return slot;
            }
            slot = slot->next;
        }
        target = target->next;
    }
    return 0;    
};

mem_frame *ask_chunk(unsigned int need_len) {
    mem_frame *target = buddy_malloc(FRAME_SIZE);
    chunk *slot = init_chunk(target);
    target = slot_malloc(slot, slot->curr, need_len);
    return target;
};

mem_frame *chunk_malloc(unsigned int size) {
    unsigned int need_len = (size-1) / CHUNK_SIZE +1;
    mem_frame *target = find_slot(need_len);
    if (!target) {
        target = ask_chunk(need_len);
    }
    target->free = 0;
#ifdef MEM_DEMO_LOG
    uart_hex(target->address);
    uart_send('\n');
#endif
    return target;
}

int slot_merge(mem_frame *slot) {
    // right
    mem_frame *target = slot->next;
    if (target != 0 && target->free) {
        target->free = 0;
        slot->order += target->order;
        target->next->prev = slot;
        slot->next = target->next;
    }
    // left
    if (slot->position != 0) {
        target = slot->prev;
        if (target->free) {
            slot->free = 0;
            target->order += slot->order;
            slot->next->prev = target;
            target->next = slot->next;
        }
    }
    else 
        target = slot;
        
    if (target->order == FRAME_SIZE/CHUNK_SIZE) {
        buddy_free(target->address);
        return 1;
    }
    return 0;
};

void chunk_free(unsigned long address) {
    unsigned int position = (address - BUDDY_BASE) / FRAME_SIZE;
    unsigned int shift = (address - BUDDY_BASE - position*FRAME_SIZE) / CHUNK_SIZE;
    // free slot
    mem_frame *target = &chunk_array[position].chunk_slot[shift];
    target->free = 1;
    if (slot_merge(target)) {
        // free chunk
        chunk *slot = &chunk_array[position];
        slot->prev->next = slot->next;
        slot->next->prev = slot->prev;
    }
};

void* kmalloc(unsigned int size) {
    mem_frame *target;
    if (size > FRAME_SIZE)
        target = buddy_malloc(size);
    else 
        target = chunk_malloc(size);
    return (void*)target->address;
}

void kfree(void *ptr) {
    unsigned long address = (unsigned long) ptr;
    mem_frame *target = find_address_frame(address & -FRAME_SIZE);
    if (target->order)
        buddy_free(address);
    else
        chunk_free(address);
};

void memory_reserve(unsigned long start, unsigned long end, char* message) {
    int i = 0;
    while (i<RESERVED_LEN && reserved_address[i][1]) {
        if ((reserved_address[i][0]<start && start<reserved_address[i][1]) || (reserved_address[i][0]<end && end<reserved_address[i][1])) {
            printf("invalid_memory_address");
            while (1);
        }
        i++;
    }
    reserved_address[i][0] = start;
    reserved_address[i][1] = end;

    int position_start = (start-BUDDY_BASE) / FRAME_SIZE;
    int position_end = (end-BUDDY_BASE) / FRAME_SIZE;
    if (position_start < 0 && position_end >= 0) {
        int j = 0;
        while (j<BUDDY_MAX_LEN && j<=position_end) {
            reserved_position[j] = 1;
            j++;
        }
    }
    else if (0 <= position_start) {
        int j = position_start;
        while (j<BUDDY_MAX_LEN && j<=position_end) {
            reserved_position[j] = 1;
            j++;
        }
    }
#ifdef MEM_DEMO_LOG
    printf("reserve %s:\t%x\t---%x\n", message, start, end);
#endif
};

void startup_allocation() {
    memory_reserve((unsigned long)0x0000, (unsigned long)0x1000, "multicore boot"); // Spin tables for multicore boot
    memory_reserve((unsigned long)&_start, (unsigned long)&_end, "Kernel image");   // Kernel image in the physical memory
    memory_reserve((unsigned long)cpio_start, (unsigned long)cpio_end, "Initramfs");// Initramfs
    memory_reserve((unsigned long)fdt_start, (unsigned long)fdt_end, "Devicetree");  // Devicetree
    memory_reserve((unsigned long)&_end, mem_position-1, "simple alloc");   // simple allocator (startup allocator)
    int last = BUDDY_MAX_LEN - 1;
    while (reserved_position[last] == 0 && last != -1) last--;
    void *init[BUDDY_MAX_LEN];
    for (int i=0; i<=last; i++) {
        init[i] = kmalloc(4096);
    }
    for (int i=last; i>=0; i--) {
        if (reserved_position[i] == 0)
            kfree(init[i]);
        else {
            // free chunk
            chunk *slot = &chunk_array[i];
            slot->prev->next = slot->next;
            slot->next->prev = slot->prev;
        }
    }
};

void init_buddy() {
    frame_array = simple_malloc(BUDDY_MAX_LEN * sizeof(mem_frame));
    buddy_system = simple_malloc((BUDDY_MAX_ORDER+1) * sizeof(mem_frame));
    chunk_array = simple_malloc(sizeof(chunk) * BUDDY_MAX_LEN);
    BUDDY_BASE = (unsigned long)simple_malloc(FRAME_SIZE*BUDDY_MAX_LEN);;
    
    for (int i=0; i<BUDDY_MAX_LEN; i++) {
        frame_array[i].position = i;
        frame_array[i].address = BUDDY_BASE + FRAME_SIZE * i;
        frame_array[i].free = 0;
    }
    for (int i=0; i<BUDDY_MAX_ORDER+1; i++) {
        buddy_system[i].next = 0;
        buddy_system[i].next->prev = &buddy_system[i];
    }
    buddy_system[5].next->prev = &frame_array[0];
    buddy_system[5].next->prev->next = buddy_system[5].next;
    buddy_system[5].next = &frame_array[0];
    buddy_system[5].next->prev = &buddy_system[5];
    buddy_system[5].next->order = BUDDY_MAX_ORDER;
    buddy_system[5].next->free = 1;
    
    // init chunk slots
    chunk_system.next = 0;
    chunk_system.next->prev = &chunk_system;

    mem_position -= FRAME_SIZE*BUDDY_MAX_LEN;
    startup_allocation();
    mem_position += FRAME_SIZE*BUDDY_MAX_LEN;

#ifdef MEM_DEMO_LOG
    show_frame();
    char *a = kmalloc(4096*16);
    char *b = kmalloc(400);
    char *c = kmalloc(4000);
    char *d = kmalloc(4096*8);
    kfree(a);
    char *e = kmalloc(4000);
    char *f = kmalloc(40);
    kfree(b);
    char *g = kmalloc(4096*8);
    kfree(d);
    kfree(c);
    kfree(f);
    kfree(g);
    kfree(e);
#endif
};

void test_malloc() {
    char* string1 = simple_malloc(4);
    string1[0] = 'q';
    string1[1] = 'w';
    string1[2] = 'e';
    string1[3] = 'r';
    string1[4] = '\n';
}

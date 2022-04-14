#include <kmalloc.h>
#include <stdint.h>
#include <stddef.h>
#include <dtb.h>
#include <uart.h>
#define BUDDY_SYSTEM_SIZE 16
#define PAGE_SIZE 12
#define HEAP_SIZE 32 //pages

extern uint32_t _heap_start;
extern uint32_t _heap_end;
extern uint32_t _kernel_reserved_memory_start;
extern uint32_t _kernel_reserved_memory_end;
void *kmalloc_simple_top_chunk;
void *kmalloc_simple_end;

struct struc_usable_memory_{
    uint64_t address;
    uint64_t size;
    struct struc_usable_memory_* next;
};
typedef struct struc_usable_memory_ struc_usable_memory;
struc_usable_memory *usable_memory;

struct struc_buddy_system_element_{
    struct struc_buddy_system_element_* fd;
    //struct struc_buddy_system_element_* bk;
    uint64_t frame_id;
    uint8_t frame_size;
    uint8_t alloc;
};
typedef struct struc_buddy_system_element_ struc_buddy_system_element;

#define buddy_flag_directalloc 0b1 //this buddy shouldn't be split.
#define buddy_flag_alloc 0b10
#define buddy_type_normal 0
#define buddy_type_directalloc 1
#define buddy_readflag(b,f) (((struct struc_buddy_system_*)b)->flags & f)
#define buddy_setflag(b,f) (((struct struc_buddy_system_*)b)->flags | f)
#define buddy_unsetflag(b,f) (((struct struc_buddy_system_*)b)->flags & ~f)

struct struc_buddy_system_{
    uint64_t base_address;
    uint64_t totalsize;
    uint8_t flags;
    struct struc_buddy_system_element_* freelist[BUDDY_SYSTEM_SIZE+1];
    struct struc_buddy_system_element_* frame_array;
    struct struc_buddy_system_* next;
};
typedef struct struc_buddy_system_ struc_buddy_system;
struc_buddy_system *buddy_system;


struct Heap_Chunk_{
    uint64_t prev_size;
    uint64_t size;
    struct Heap_Chunk_* fd;
    struct Heap_Chunk_* bk;
};
typedef struct Heap_Chunk_ Heap_Chunk;

struct Heap_{
    uint64_t base_address;
    uint64_t size;
    Heap_Chunk* top_chunk;
    Heap_Chunk* fastbin[16];
    Heap_Chunk unsorted_bin;
    struct Heap_* next;
};
typedef struct Heap_ Heap;

Heap* kernel_heap;


void *kmalloc_simple(size_t size)
{
    if(!kmalloc_simple_top_chunk){
        kmalloc_simple_top_chunk = (void *)&_heap_start;
        kmalloc_simple_end = (void *)&_heap_end;
    }
    //if(size&0xf)size = ((size>>4) + 1) << 4;
    size = (((size-1)>>4)+1)<<4;
    if((uint64_t)kmalloc_simple_top_chunk+size >= (uint64_t)kmalloc_simple_end)
    {
        return 0;
    }
    void *chunk = kmalloc_simple_top_chunk;
    kmalloc_simple_top_chunk = (void *)((uint64_t)kmalloc_simple_top_chunk+size);
    return chunk;
}

struc_usable_memory *new_usable_memory_entry(uint64_t address, uint64_t size)
{
    struc_usable_memory *usable_memory = kmalloc_simple(sizeof(struc_usable_memory));
    usable_memory->address = address;
    usable_memory->size = size;
    usable_memory->next = 0;
    return usable_memory;
}

void kmalloc_init()
{
    usable_memory = new_usable_memory_entry(0, 0x3c000000);
    struc_usable_memory *head = new_usable_memory_entry(0, 0);
    head->next = usable_memory;
    usable_memory = head;
    kmalloc_memory_reserve(0x0, 0x1000);
    kmalloc_memory_reserve((uint64_t)&_kernel_reserved_memory_start, (uint64_t)&_kernel_reserved_memory_end - (uint64_t)&_kernel_reserved_memory_start);
    if((uint64_t)_DTB_ADDRESS != 0xffffffff){
        uint32_t dtbsize = fdt_totalsize(_DTB_ADDRESS);
        kmalloc_memory_reserve((uint64_t)_DTB_ADDRESS, (uint64_t)dtbsize);
    }
    buddy_system = 0;
    kernel_heap = 0;
}

int kmalloc_memory_reserve(uint64_t address, uint64_t size)
{
    address >>= PAGE_SIZE;
    address <<= PAGE_SIZE;
    size = (uint64_t)(((((int64_t)size-1)>>PAGE_SIZE)+1)<<PAGE_SIZE);
    struc_usable_memory *cur_entry = usable_memory->next;
    struc_usable_memory *prev_entry = usable_memory;
    while(cur_entry){
        if(address < cur_entry->address) break;
        if(address >= cur_entry->address+cur_entry->size){
            prev_entry = cur_entry;
            cur_entry = cur_entry->next;
            continue;
        }
        if(address+size > cur_entry->address+cur_entry->size) break;
        uint64_t start = address;
        uint64_t end = address+size;
        uint64_t old_start = cur_entry->address;
        uint64_t old_end = cur_entry->address + cur_entry->size;
        struc_usable_memory *l_entry, *r_entry;
        l_entry = new_usable_memory_entry(old_start, start-old_start);
        r_entry = new_usable_memory_entry(end, old_end-end);
        if(l_entry->size){
            l_entry->next = prev_entry->next;
            prev_entry->next = l_entry;
            prev_entry = l_entry;
        }
        if(r_entry->size){
            r_entry->next = prev_entry->next;
            prev_entry->next = r_entry;
            prev_entry = r_entry;
        }
        prev_entry->next = cur_entry->next;
        return 0;
    }
    return -1;
}

uint64_t buddy_find_memory(uint64_t size)
{
    struc_usable_memory *cur_entry = usable_memory->next;
    struc_usable_memory *best = 0;
    while(cur_entry){
        if(cur_entry->size < size){
            cur_entry = cur_entry->next;
            continue;
        }
        if(!best){
            best = cur_entry;
        }
        else if(cur_entry->size < best->size){
            best = cur_entry;
        }
        cur_entry = cur_entry->next;
    }
    return best->address;
}

struc_buddy_system_element* buddy_new_frame_array(uint64_t length)
{
    struc_buddy_system_element* frame_array = (struc_buddy_system_element*)kmalloc_simple(sizeof(struc_buddy_system_element)*length);
    for(int i=0;i<length;i++){
        frame_array[i].fd = 0;
        //frame_array[i].bk = 0;
        frame_array[i].frame_id = i;
        frame_array[i].frame_size = BUDDY_SYSTEM_SIZE + 2;
        frame_array[i].alloc = 0;
    }
    return frame_array;
}

struc_buddy_system* buddy_new(uint8_t type, uint64_t size)
{
    uart_print("buddy_new(): call type=0x");
    uart_print_hex((uint64_t)type);
    uart_print(", size=0x");
    uart_putshex((uint64_t)size);

    struc_buddy_system* newbuddy = (struc_buddy_system*)kmalloc_simple(sizeof(struc_buddy_system));
    newbuddy->next = 0;
    for(int i=0;i<BUDDY_SYSTEM_SIZE+1;i++)newbuddy->freelist[i]=0;
    if(type == buddy_type_normal){
        size = 1<<(BUDDY_SYSTEM_SIZE+PAGE_SIZE);
        uart_puts("buddy_new(): finding usable memory.");
        uint64_t addr = buddy_find_memory(size);
        uart_print("buddy_new(): usable memory found 0x");
        uart_putshex((uint64_t)addr);
        if(!addr) return 0;
        if(kmalloc_memory_reserve(addr, size)==-1){
            return 0;
        }
        newbuddy->base_address = addr;
        newbuddy->totalsize = size;
        newbuddy->flags = 0;
        newbuddy->frame_array = buddy_new_frame_array(1<<(BUDDY_SYSTEM_SIZE+1));
        newbuddy->frame_array[0].frame_size = BUDDY_SYSTEM_SIZE;
        newbuddy->freelist[BUDDY_SYSTEM_SIZE] = &(newbuddy->frame_array[0]);
        //newbuddy->frame_array[0].bk = (struc_buddy_system_element *)&(newbuddy->freelist[BUDDY_SYSTEM_SIZE]);
        return newbuddy;
    }
    else if(type == buddy_type_directalloc){
        size = (uint64_t)(((((int64_t)size-1)>>PAGE_SIZE)+1)<<PAGE_SIZE);
        uart_puts("buddy_new(): finding usable memory.");
        uint64_t addr = buddy_find_memory(size);
        uart_print("buddy_new(): usable memory found 0x");
        uart_putshex((uint64_t)addr);
        if(!addr) return 0;
        if(kmalloc_memory_reserve(addr, size)==-1){
            return 0;
        }
        newbuddy->base_address = addr;
        newbuddy->totalsize = size;
        newbuddy->flags = 0;
        buddy_setflag(newbuddy, buddy_flag_directalloc);
        newbuddy->frame_array = 0;
        return newbuddy;
    }
    else{
        return 0;
    }
}

//#define buddy_popfree(f) (*(struct struc_buddy_system_element_**)f = (*f)->fd)
//#define buddy_pushfree(f,n) (n->fd = *(struct struc_buddy_system_element_**)f = (*f)->fd)

void buddy_popfree(struc_buddy_system_element** f)
{
    *f = (*f)->fd;
    //(*f)->fd->bk = (struc_buddy_system_element *)f;
}

void buddy_pushfree(struc_buddy_system_element** f, struc_buddy_system_element* n)
{
    n->fd = *f;
    *f = n;
    //n->fd->bk = n;
    //n->bk = (struc_buddy_system_element *)f;
}

void debug_print_buddy_element(struc_buddy_system_element* e)
{
    uart_print("buddy_system_element - 0x");
    uart_print_hex((uint64_t)e);
    uart_print(", frame_id=0x");
    uart_print_hex((uint64_t)e->frame_id);
    uart_print(", frame_size=0x");
    uart_print_hex((uint64_t)e->frame_size);
    uart_print(", alloc=0x");
    uart_print_hex((uint64_t)e->alloc);
    uart_puts("");
}

void debug_print_buddy_system(struc_buddy_system* buddy)
{
    uart_print("buddy_system - 0x");
    uart_print_hex((uint64_t)buddy);
    uart_print(", base_address=0x");
    uart_print_hex((uint64_t)buddy->base_address);
    uart_print(", flags=0x");
    uart_print_hex((uint64_t)buddy->flags);
    uart_print(", flags=0x");
    uart_print_hex((uint64_t)buddy->flags);
    uart_puts("");
}

int64_t buddy_alloc_(struc_buddy_system *buddy, uint64_t page_num)
{
    uart_print("buddy_alloc_(): call buddy=0x");
    uart_print_hex((uint64_t)buddy);
    uart_print(", page_num=0x");
    uart_putshex((uint64_t)page_num);
    uart_print("buddy_alloc_(): ");
    debug_print_buddy_system(buddy);

    uint32_t page_num_pow=0;
    while(page_num>1){
        page_num_pow+=1;
        page_num>>=1;
    }

    uart_print("buddy_alloc_(): page_num to power of 2: 0x");
    uart_putshex((uint64_t)page_num_pow);

    struc_buddy_system_element* e = 0;
    uart_puts("buddy_alloc_(): finding usable buddy element in freelist.");
    for(int i=page_num_pow;i<BUDDY_SYSTEM_SIZE+1;i++){
        if(!buddy->freelist[i]) continue;
        e = buddy->freelist[i];
        buddy_popfree(&(buddy->freelist[i]));
        break;
    }
    if(e == 0)return 0;

    uart_print("buddy_alloc_(): buddy element found: ");
    debug_print_buddy_element(e);

    for(int i=e->frame_size-1;i>=(int32_t)page_num_pow;i--){
        uart_puts("buddy_alloc_(): split buddy element...");
        struc_buddy_system_element* l = e;
        struc_buddy_system_element* r = &(buddy->frame_array[e->frame_id + (1<<(e->frame_size-1))]);
        if(r->alloc){
            uart_puts("buddy_alloc_(): Buddy System Error.");
            return 0;
        }
        l->frame_size = i;
        r->frame_size = i;
        uart_print("e: ");
        debug_print_buddy_element(e);
        uart_print("l: ");
        debug_print_buddy_element(l);
        uart_print("r: ");
        debug_print_buddy_element(r);
        buddy_pushfree(&(buddy->freelist[i]), r);
        e = l;
    }
    e->alloc = 1;
    uart_print("buddy_alloc_(): buddy element found and splitted: ");
    debug_print_buddy_element(e);
    return e->frame_id;
}

void *buddy_alloc(uint64_t page_num)
{
    uart_print("buddy_alloc(): call page_num=0x");
    uart_putshex((uint64_t)page_num);

    uint64_t new_page_num = 1;
    while(new_page_num<page_num)new_page_num<<=1;
    
    uart_print("buddy_alloc(): pad to pow of 2 new_page_num=0x");
    uart_putshex((uint64_t)new_page_num);
    //page_num = new_page_num;

    if(new_page_num >= (1<<(BUDDY_SYSTEM_SIZE))){
        // TODO
        uart_puts("buddy_alloc(): Too large size, direct alloc a memory without manage by buddy system.");
        struc_buddy_system *new_buddy_dalloc = buddy_new(buddy_type_directalloc, new_page_num * (1<<(PAGE_SIZE)));
        buddy_setflag(new_buddy_dalloc, buddy_flag_alloc);
        return (void *)(new_buddy_dalloc->base_address);
    }
    struc_buddy_system *cur_buddy = buddy_system;
    int64_t frame_id = -1;
    while(cur_buddy){
        uart_print("buddy_alloc(): try to alloc page from buddy_system 0x");
        uart_putshex((uint64_t)cur_buddy);
        //debug_print_buddy_system(cur_buddy);

        frame_id = buddy_alloc_(cur_buddy, new_page_num);
        if(frame_id!=-1){
            uart_print("buddy_alloc(): page allocated address=0x");
            uart_putshex((uint64_t)(cur_buddy->base_address + ((frame_id) * (1<<PAGE_SIZE))));

            return (void *)(cur_buddy->base_address + ((frame_id) * (1<<PAGE_SIZE)));
        }
        cur_buddy = cur_buddy->next;
    }
    uart_puts("buddy_alloc(): Couldn't alloc from existed buddy system, try to create new buddy system!!");

    struc_buddy_system *new_buddy = buddy_new(buddy_type_normal, 0);
    new_buddy->next = buddy_system;
    buddy_system = new_buddy;
    frame_id = buddy_alloc_(new_buddy, new_page_num);
    if(frame_id==-1) return 0;

    uart_print("buddy_alloc(): page allocated address=0x");
    uart_putshex((uint64_t)(new_buddy->base_address + ((frame_id) * (1<<PAGE_SIZE))));
    return (void *)(new_buddy->base_address + ((frame_id) * (1<<PAGE_SIZE)));
}

void buddy_free_unlink(struc_buddy_system *buddy, uint64_t frame_id)
{
    uint64_t frame_size = buddy->frame_array[frame_id].frame_size;
    struc_buddy_system_element *cur_ele;
    cur_ele = buddy->freelist[frame_size];
    if(cur_ele->frame_id == frame_id){
        buddy->freelist[frame_size] = cur_ele->fd;
        return ;
    }
    while(cur_ele){
        if(cur_ele->fd && cur_ele->fd->frame_id == frame_id){
            cur_ele->fd = cur_ele->fd->fd;
            break;
        }
        cur_ele = cur_ele->fd;
    }
    return ;
}

void buddy_free(void *ptr)
{
    uart_print("buddy_free(): call ptr=0x");
    uart_putshex((uint64_t)ptr);

    uint64_t addr = (uint64_t)ptr;
    struc_buddy_system *cur_buddy = buddy_system;
    while(cur_buddy){
        if(addr >= cur_buddy->base_address && addr < (cur_buddy->base_address + cur_buddy->totalsize)){
            break;
        }
    }
    if(cur_buddy == 0)return ;
    uart_print("buddy_free(): buddy_system found: ");
    debug_print_buddy_system(cur_buddy);

    if(buddy_readflag(cur_buddy, buddy_flag_directalloc)){
        uart_print("buddy_free(): This buddy system is direct alloc. Just unset buddy alloc flag.");
        buddy_unsetflag(cur_buddy, buddy_flag_alloc);
        return ;
    }

    uint64_t frame_id = ((addr - cur_buddy->base_address) / (1<<PAGE_SIZE));
    uart_print("buddy_free(): frame_id found: 0x");
    uart_putshex((uint64_t)frame_id);

    uart_puts("buddy_free(): Trying to merge.");

    cur_buddy->frame_array[frame_id].alloc = 0;

    uint64_t frame_size = cur_buddy->frame_array[frame_id].frame_size;
    for(int i=frame_size;i<=BUDDY_SYSTEM_SIZE;i++){
        uart_print("buddy_free(): merging size=0x");
        uart_putshex((uint64_t)i);
        uint64_t l_id = 0, r_id = 0, to_merge_id = 0;
        if(frame_id&(1<<i)){
            r_id = frame_id;
            l_id = frame_id - (1<<i);
            to_merge_id = l_id;
        }
        else{
            r_id = frame_id + (1<<i);
            l_id = frame_id;
            to_merge_id = r_id;
        }
        uart_print("buddy_free(): l_element: "); debug_print_buddy_element(&(cur_buddy->frame_array[l_id]));
        uart_print("buddy_free(): r_element: "); debug_print_buddy_element(&(cur_buddy->frame_array[r_id]));
        uart_print("buddy_free(): to_merge_element: "); debug_print_buddy_element(&(cur_buddy->frame_array[to_merge_id]));
        if(cur_buddy->frame_array[to_merge_id].frame_size != i || cur_buddy->frame_array[to_merge_id].alloc){
            break;
        }
        uart_puts("buddy_free(): These frames could be merge.");
        buddy_free_unlink(cur_buddy, to_merge_id);
        cur_buddy->frame_array[r_id].frame_size = i+1;
        cur_buddy->frame_array[l_id].frame_size = i+1;
        frame_id = l_id;
        frame_size = i+1;
    }
    uart_print("buddy_free(): merged: "); debug_print_buddy_element(&(cur_buddy->frame_array[frame_id]));
    buddy_pushfree((&cur_buddy->freelist[frame_size]), &(cur_buddy->frame_array[frame_id]));
}

void *kmalloc_(size_t size)
{
    return kmalloc_simple(size);
}

#define BIN_ID(x) ((x>>4)-2)
#define chunk2mem(x) ((void *)((int64_t)x + 0x10))
#define mem2chunk(x) ((void *)((int64_t)x - 0x10))
#define SIZE(x) (x->size & 0xfffffffffffffff0)
#define next_chunk(x) ((Heap_Chunk*)((int64_t)x + SIZE(x)))
#define prev_chunk(x) ((Heap_Chunk*)((int64_t)x - x->prev_size))
#define next_size_chunk(c,s) ((Heap_Chunk*)((int64_t)c + s)) 
#define chunk_flag_p 0b1
#define chunk_setflag(x,f) (x->size |= f)
#define chunk_unsetflag(x,f) (x->size &= ~f)
#define chunk_readflag(x,f) (x->size & f)

void debug_print_heap(Heap* heap)
{
    uart_print("===== Heap @ 0x");
    uart_print_hex((uint64_t)heap);
    uart_puts(" =====");
    uart_print("->base_address = 0x");
    uart_putshex((uint64_t)heap->base_address);
    uart_print("->size = 0x");
    uart_putshex((uint64_t)heap->size);
    for(int i=0;i<16;i++){
        uart_print("->fastbin[0x");
        uart_print_hex(i);
        uart_print("] = 0x");
        uart_putshex((uint64_t)heap->fastbin[i]);
    }
    uart_print("->unsorted_bin.fd = 0x");
    uart_putshex((uint64_t)heap->unsorted_bin.fd);
    uart_print("->unsorted_bin.bk = 0x");
    uart_putshex((uint64_t)heap->unsorted_bin.bk);
    uart_print("->top_chunk->size = 0x");
    uart_putshex((uint64_t)heap->top_chunk->size);
    uart_print("SIZE(->top_chunk) = 0x");
    uart_putshex((uint64_t)SIZE(heap->top_chunk));
    uart_print("->top_chunk->prev_size = 0x");
    uart_putshex((uint64_t)heap->top_chunk->prev_size);
    uart_puts("===============");
}

Heap *heap_new(uint64_t heap_size)
{
    Heap *newheap = (Heap *)kmalloc_simple(sizeof(Heap));
    newheap->base_address = (uint64_t)buddy_alloc(heap_size);
    if(newheap->base_address == 0) {
        uart_print("heap_new(): Couldn't get pages from buddy system.");
        return 0;
    }
    newheap->size = ((uint64_t)1<<12) * heap_size;
    newheap->next = 0;
    for(int i=0;i<16;i++){
        newheap->fastbin[i] = 0;
    }
    newheap->unsorted_bin.size = 0;
    newheap->unsorted_bin.fd = 0;
    newheap->unsorted_bin.bk = 0;
    newheap->top_chunk = (Heap_Chunk*)newheap->base_address;
    newheap->top_chunk->prev_size = 0;
    newheap->top_chunk->size = newheap->size - 0x10;
    return newheap;
}

void unlink(Heap_Chunk* chunk)
{
    chunk->bk->fd = chunk->fd;
    if(chunk->fd) chunk->fd->bk = chunk->bk;
    chunk->fd = 0;
    chunk->bk = 0;
}

void heap_free(Heap* heap, void *ptr)
{
    uart_print("heap_free(): ptr=0x");
    uart_putshex((uint64_t)ptr);
    debug_print_heap(heap);

    Heap_Chunk *victim = mem2chunk(ptr);

    uart_print("heap_free(): chunk->size=0x");
    uart_putshex((uint64_t)victim->size);

    if(SIZE(victim)<0x120){
        uart_puts("heap_free(): Put the chunk into fastbin.");
        victim->fd = heap->fastbin[BIN_ID(SIZE(victim))];
        heap->fastbin[BIN_ID(SIZE(victim))] = victim;
        debug_print_heap(heap);
        return ;
    }

    chunk_unsetflag(next_chunk(victim), chunk_flag_p);

    uart_puts("heap_free(): Check if prev chunk could be merged.");
    if(!chunk_readflag(victim, chunk_flag_p)){
        Heap_Chunk *prev = prev_chunk(victim);
        uart_print("heap_free(): prev chunk could be merge: 0x");
        uart_putshex((uint64_t)prev);
        unlink(prev);
        prev->size += victim->size;
        next_chunk(victim)->prev_size = prev->size;
        victim = prev;
    }
    uart_puts("heap_free(): Check if next chunk is top_chunk.");
    if(next_chunk(victim)==heap->top_chunk){
        uart_puts("heap_free(): Merge to top_chunk.");
        victim->size += heap->top_chunk->size;
        heap->top_chunk = victim;
        return ;
    }

    uart_puts("heap_free(): Check if next chunk could be merged.");
    if(!chunk_readflag(next_chunk(next_chunk(victim)), chunk_flag_p)){
        Heap_Chunk *next = next_chunk(victim);
        uart_print("heap_free(): next chunk could be merge: 0x");
        uart_putshex((uint64_t)next);
        victim->size += next->size;
        next_chunk(next)->prev_size = victim->size;
    }

    uart_puts("heap_free(): Chain to unsorted_bin.");
    victim->fd = heap->unsorted_bin.fd;
    victim->bk = &(heap->unsorted_bin);
    heap->unsorted_bin.fd->bk = victim;
    heap->unsorted_bin.fd = victim;
    debug_print_heap(heap);
}

void *heap_malloc(Heap* heap, size_t size)
{
    uart_print("heap_malloc(): size=0x");
    uart_putshex((uint64_t)size);
    debug_print_heap(heap);
    if(size<0x120){
        uart_puts("heap_malloc(): Try to find chunk from fastbin.");
        if(heap->fastbin[BIN_ID(size)]){
            Heap_Chunk* victim = heap->fastbin[BIN_ID(size)];
            uart_print("heap_malloc(): Chunk is found from fastbin: 0x");
            uart_putshex((uint64_t)victim);
            heap->fastbin[BIN_ID(size)] = victim->fd;
            return chunk2mem(victim);
        }
    }
    Heap_Chunk *cur_chunk = heap->unsorted_bin.fd;
    uart_puts("heap_malloc(): Try to find chunk from unsorted_bin.");
    while(cur_chunk){
        if(SIZE(cur_chunk) < size){
            cur_chunk = cur_chunk->fd;
            continue;
        }
        Heap_Chunk *victim = cur_chunk;
        uart_print("heap_malloc(): Chunk is found from unsorted_bin: 0x");
        uart_putshex((uint64_t)victim);
        unlink(victim);
        if(SIZE(victim)-size < 0x20){
            chunk_setflag(next_chunk(victim), chunk_flag_p);
            return chunk2mem(victim);
        }
        Heap_Chunk *remain = next_size_chunk(victim, size);
        uart_print("heap_malloc(): Split remain chunk: 0x");
        uart_putshex((uint64_t)remain);
        remain->size = victim->size - size;
        victim->size = size | (victim->size&0xf);
        remain->prev_size = victim->size;
        chunk_setflag(next_chunk(victim), chunk_flag_p);
        chunk_setflag(next_chunk(remain), chunk_flag_p);
        heap_free(heap, chunk2mem(remain));
        return chunk2mem(victim);
    }

    uart_puts("heap_malloc(): Try to alloc chunk from top_chunk.");

    if(SIZE(heap->top_chunk)<size) return 0;

    Heap_Chunk *victim = heap->top_chunk;


    heap->top_chunk = next_size_chunk(victim, size);
    heap->top_chunk->size = victim->size - size;
    victim->size = size | (victim->size&0xf);
    heap->top_chunk->prev_size = victim->size;
    chunk_setflag(next_chunk(victim), chunk_flag_p);

    uart_print("heap_malloc(): Chunk is allocated from top_chunk: 0x");
    uart_print_hex((uint64_t)victim);
    uart_print(", size=0x");
    uart_print_hex((uint64_t)victim->size);
    uart_print(", nextchunk=0x");
    uart_putshex((uint64_t)next_chunk(victim));

    uart_print("heap_malloc(): top_chunk changed: 0x");
    uart_putshex((uint64_t)heap->top_chunk);

    debug_print_heap(heap);

    return chunk2mem(victim);
}

void *kmalloc(size_t size)
{
    uart_print("kmalloc(): size=0x");
    uart_putshex((uint64_t)size);

    size = ((size>>4)+1)<<4;

    if(size<0x10) size = 0x10;
    size += 0x10;

    if(size >= HEAP_SIZE * (1<<PAGE_SIZE)){
        uart_puts("kmalloc(): size bigger than 128K. Alloc new heap.");
        size = (size_t)(((((int64_t)size-1)>>PAGE_SIZE)+1)<<PAGE_SIZE);
        uart_print("kmalloc(): size align to page: 0x");
        uart_putshex((uint64_t)size);
        Heap *newheap = heap_new(size>>PAGE_SIZE);
        newheap->next = kernel_heap;
        kernel_heap = newheap;
        return heap_malloc(newheap, size);
    }

    Heap *cur_heap = kernel_heap;
    while(cur_heap){
        void *victim = heap_malloc(cur_heap, size);
        if(victim) return victim;
        cur_heap = cur_heap->next;
    }

    Heap *new_heap = heap_new(HEAP_SIZE);
    if(!cur_heap) kernel_heap = new_heap;
    else cur_heap->next = new_heap;
    void *victim = heap_malloc(new_heap, size);
    return victim;
}

void kfree(void *ptr)
{
    Heap *cur_heap = kernel_heap;
    while(cur_heap){
        if((uint64_t)ptr >= (uint64_t)cur_heap->base_address && (uint64_t)ptr < ((uint64_t)cur_heap->base_address + cur_heap->size)){
            heap_free(cur_heap, ptr);
            return ;
        }
    }
    return ;
}
#include <allocator.h>
#include <list.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <uart.h>

Buddy buddy_list[MAX_BUDDY_ORDER+1];
Frame frames[FRAME_NUM];

void allocator_init(){
    for(unsigned int i = 0; i <= MAX_BUDDY_ORDER; i++){
        INIT_LIST_HEAD(&buddy_list[i].list);
        // buddy_list[i].free_num = 0;
    }

    for(unsigned int i = 0; i < FRAME_NUM; i++){
        INIT_LIST_HEAD(&frames[i].list);
        frames[i].free = 1;
        frames[i].idx = i;
        frames[i].order = -1;
    }

    /* 
     * set max val in first frame
     * and add it in buddy_list
     */
    frames[0].order = MAX_BUDDY_ORDER;
    list_add(&frames[0].list, &buddy_list[MAX_BUDDY_ORDER].list);

    char buf[20];

    buddy_alloc(0x1000);
    buddy_alloc(0x2000);
    buddy_alloc(0x2000);
    buddy_alloc(0x1000);

    void *addr1 = buddy_alloc(0x1000);
    uitohex(buf, (unsigned int)addr1);
    uart_puts("Addr1 :");
    uart_puts(buf);
    uart_puts("\n");

}

void *buddy_alloc(unsigned int size){
    unsigned int use_frames = (size % FRAME_SIZE == 0) ? (size / FRAME_SIZE) : (size / FRAME_SIZE) + 1;
    int use_order = (int)log2(use_frames);
    print_use_frame(size, use_frames, use_order);
    for(unsigned int i = use_order; i <= MAX_BUDDY_ORDER; i++){
        if(!list_empty(&buddy_list[i].list)){
            Frame *alloca_frame = (Frame *)buddy_pop(&buddy_list[i], use_order);
            unsigned long long alloca_addr = alloca_frame->idx * FRAME_SIZE + BUDDY_ADDR_START;
            memset((void *)alloca_addr, '\0', (1 << alloca_frame->order) * FRAME_SIZE);
            return (void *)alloca_addr;
        }
    }
    return NULL;
}


void *buddy_pop(Buddy *buddy, int use_order){
    Frame *target_frame = (Frame *)buddy->list.next;
    list_del_first(&buddy->list); // pop the free entry
    return release_redundant(target_frame, use_order);
}

void *release_redundant(Frame *left_frame, int use_order){
    while(left_frame->order > use_order){
        int samll_order = left_frame->order - 1;
        unsigned int right_idx = left_frame->idx + (1 << samll_order);  //e.g. 2^15 + 0 = 32768
        Frame *right_frame = &frames[right_idx];
        right_frame->order = samll_order;
        left_frame->order = samll_order;
        list_add(&frames[right_idx].list, &buddy_list[samll_order].list);
    }
    left_frame->free = 0;
    print_buddy_list();
    return left_frame;
}


void buddy_remove(Buddy *buddy){
    list_del_first(&buddy->list);
}

void print_use_frame(unsigned int size, unsigned int use_frames, int use_order){
    char buf[20];
    uart_puts("[*] Allocate Size: 0x");
    uitohex(buf, size);
    uart_puts(buf);
    uart_puts("\n");

    uart_puts("[*] Use Frames: ");
    uitoa(buf, use_frames);
    uart_puts(buf);
    uart_puts("\n");

    uart_puts("[*] Use Order: ");
    uitoa(buf, use_order);
    uart_puts(buf);
    uart_puts("\n");
}

void print_frame_info(Frame *frame){
    char buf[20];

    uitoa(buf,frame->idx);
    uart_puts("idx: ");
    uart_puts(buf);
    uart_puts(" - ");

    itoa(buf,frame->order);
    uart_puts("order: ");
    uart_puts(buf);
    uart_puts("\n");

}

void print_buddy_list(){
    char buf[20];
    struct list_head *pos;
    for(unsigned int i = 0; i <= MAX_BUDDY_ORDER; i++){
        uitoa(buf, i);
        uart_puts(buf);
        uart_puts("\t:\t");
        list_for_each(pos, &buddy_list[i].list){
            Frame *tmp = (Frame *)pos;
            uitohex(buf, tmp->idx);
            uart_puts(buf);
        }
        uart_puts("\n");
    }
}




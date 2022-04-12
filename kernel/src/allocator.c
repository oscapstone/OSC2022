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
    }

    unsigned long long maxorder_frame_idx = (1 << MAX_BUDDY_ORDER);

    for(unsigned int i = 0; i < FRAME_NUM; i++){
        /*
         * add the max order frame in buddy list
         * if not max order frame, just init the info. 
         */
        if(i % maxorder_frame_idx == 0){
            INIT_LIST_HEAD(&frames[i].list);
            frames[i].free = 0;
            frames[i].idx = i;
            frames[i].order = MAX_BUDDY_ORDER;
            frames[i].chunk_used = 0;
            list_add_tail(&frames[i].list, &buddy_list[MAX_BUDDY_ORDER].list);
        }
        else{
            INIT_LIST_HEAD(&frames[i].list);
            frames[i].free = 1;
            frames[i].idx = i;
            frames[i].order = -1;
            frames[i].chunk_used = 0;
        }
    }
    // buddy_debug();
}


void *buddy_alloc(unsigned int size){
    unsigned int use_frames = (size % FRAME_SIZE == 0) ? (size / FRAME_SIZE) : (size / FRAME_SIZE) + 1;
    int use_order = (int)log2(use_frames);
    for(unsigned int i = use_order; i <= MAX_BUDDY_ORDER; i++){
        if(!list_empty(&buddy_list[i].list)){
            Frame *alloca_frame = (Frame *)buddy_pop(&buddy_list[i], use_order);
            unsigned long long alloca_addr = alloca_frame->idx * FRAME_SIZE + BUDDY_ADDR_START;
            print_use_frame(size, alloca_frame->idx, use_frames, use_order);
            print_buddy_list();
            // memset((void *)alloca_addr, '\0', (1 << alloca_frame->order) * FRAME_SIZE);
            return (void *)alloca_addr;
        }
    }
    print_string(UITOHEX, "[x] Allocate Size: 0x", size, 1);
    uart_puts("[x] No enough memory!!!!\n");
    // print_string(UITOHEX, "[*] No enough memory!!!!", left_frame->idx, 0);
    return NULL;
}


void *buddy_pop(Buddy *buddy, int use_order){
    Frame *target_frame = (Frame *)buddy->list.next;
    list_del(&target_frame->list); // pop the free entry
    return release_redundant(target_frame, use_order);
}

Frame *find_buddy_frame(Frame *me, int order){
    /*
     * 0b11(left_frame->me) xor 0b10(order) = 0b01(buddy_frame)
     * 0b01(right_frame->me) xor 0b10(order) = 0b11(buddy_frame)
     */
    if(order == MAX_BUDDY_ORDER)
        return NULL;
    return &frames[me->idx ^ (1 << (unsigned int)order)];   
}

void *release_redundant(Frame *left_frame, int use_order){
    while(left_frame->order > use_order){
        int samll_order = left_frame->order - 1;
        Frame *right_frame = find_buddy_frame(left_frame, samll_order);
        right_frame->order = samll_order;
        left_frame->order = samll_order;
        list_add(&frames[right_frame->idx].list, &buddy_list[samll_order].list);
        print_string(UITOHEX, "[*] Split: left->idx = 0x", left_frame->idx, 0);
        print_string(UITOHEX, " | right->idx = 0x", right_frame->idx, 0);
        print_string(UITOA, " | order = ", left_frame->order, 1);
    }
    left_frame->free = 0;
    return left_frame;
}


void buddy_free(void *addr){
    unsigned long long offset = (unsigned long long)addr - BUDDY_ADDR_START;
    unsigned int idx = (unsigned int)(offset / FRAME_SIZE);
    Frame *target_frame = &frames[idx];
    Frame *buddy_frame  = find_buddy_frame(target_frame, target_frame->order);
    int first = 1;
    /* 
     * buddy_frame is not free 
     * or buddy_frame's order is not same as target_frame's order 
     * then they are not buddy, cannot merge
     */

    if(buddy_frame == NULL || buddy_frame->free == 0 || buddy_frame->order != target_frame->order){
        print_string(UITOHEX, "[*] Free Addr: 0x", (unsigned int)addr, 1);
        print_string(UITOHEX, "[*] No buddy to merge | target_frame->idx = 0x", target_frame->idx, 0);
        print_string(UITOA, " | order = ", target_frame->order, 1);

        target_frame->free = 1;

        // no buddy_frame = the init status
        if(buddy_frame != NULL) 
            list_add(&frames[target_frame->idx].list, &buddy_list[target_frame->order].list);
        print_buddy_list();
        return;
    }

    while(buddy_frame->free == 1 && buddy_frame->order == target_frame->order){
        if(first){
            print_string(UITOHEX, "[*] Free Addr: 0x", (unsigned int)addr, 1);
            first = 0;
        }
        /* buddy cannot be allocated, it will be merged */
        list_del(&frames[buddy_frame->idx].list);

        if(target_frame->idx > buddy_frame->idx){
            print_string(UITOHEX, "[*] Merge: left->idx = 0x", buddy_frame->idx, 0);
            print_string(UITOHEX, " | right->idx = 0x", target_frame->idx, 0);
            print_string(UITOA, " | order = ", buddy_frame->order+1, 1);
            buddy_frame->order++;
            target_frame->order = -1;
            target_frame = buddy_frame;
        }
        else{
            print_string(UITOHEX, "[*] Merge: left->idx = 0x", target_frame->idx, 0);
            print_string(UITOHEX, " | right->idx = 0x", buddy_frame->idx, 0);
            print_string(UITOA, " | order = ", target_frame->order+1, 1);
            target_frame->order++;
            buddy_frame->order = -1;
        }

        buddy_frame = find_buddy_frame(target_frame, target_frame->order);  
        if(buddy_frame == NULL) break; 
        // print_buddy_list();

    }
    target_frame->free = 1;
    list_add(&frames[target_frame->idx].list, &buddy_list[target_frame->order].list);
    print_buddy_list();
}



void print_use_frame(unsigned int size, unsigned int frame_idx, unsigned int use_frames,int use_order){
    print_string(UITOHEX, "[*] Allocate Size: 0x", size, 0);
    print_string(UITOHEX, " | Frame idx: 0x", frame_idx, 0);
    print_string(UITOA, " | Use Frames: ", use_frames, 0);
    print_string(UITOA, " | Use Order: ", use_order, 1);
}

void print_frame_info(Frame *frame){
    print_string(UITOA, "idx: ", frame->idx, 0);
    print_string(ITOA, " - order: ", frame->order, 1);
}

void print_buddy_list(){
    struct list_head *pos;
    for(unsigned int i = 0; i <= MAX_BUDDY_ORDER; i++){
        print_string(UITOA, "", i, 0);
        uart_puts("\t:\t");
        unsigned int first = 1;
        list_for_each(pos, &buddy_list[i].list){
            Frame *tmp = (Frame *)pos;
            if(first){
                print_string(UITOHEX, "0x", tmp->idx, 0);
                first = 0;
            } 
            else print_string(UITOHEX, " -> 0x", tmp->idx, 0);
        }
        uart_puts("\n");
    }
}

void buddy_debug(){
    void *addr1 = buddy_alloc(0x2000);
    void *addr2 = buddy_alloc(0x1000);
    void *addr3 = buddy_alloc(0x4000);
    void *addr4 = buddy_alloc(0x1000);
    void *addr5 = buddy_alloc(0x8000);
    void *addr6 = buddy_alloc(0x1000);
    void *addr7 = buddy_alloc(0x10000);
    void *addr8 = buddy_alloc(0x1000);
    for(int i = 0; i < 8; i++){
        buddy_alloc(0x2000000);
    }
    buddy_alloc(0x10000000);


    buddy_free(addr2);
    buddy_free(addr4);
    buddy_free(addr6);
    buddy_free(addr8);

    buddy_free(addr1);
    buddy_free(addr3);
    buddy_free(addr5);
    buddy_free(addr7);
}


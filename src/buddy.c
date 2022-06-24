#include "buddy.h"

void init_buddy()
{
    // alloc frame_arr
    frame_arr = simple_alloc(sizeof(frame_t) * BUDDY_PAGE_NUM);
    // frame_arr = (uint64_t) frame_arr | 0xFFFF000000000000;

    // set all status to be zeros
    for (int i = 0; i < BUDDY_PAGE_NUM; i++) {
        frame_arr[i].status = ORDER_VALID;
        frame_arr[i].order = 0;
    }
}

void build_buddy()
{
    struct list_head *addr;
    for (int i = 0; i < BUDDY_PAGE_NUM; i++) {
        if (frame_arr[i].status == ORDER_VALID) {
            addr = PAGE2ADDR(i);
            addr->prev = NULL;
            addr->next = NULL;
        }
    }

    for (int order = 0; order < FREE_LIST_MAX_ORDER - 1; order++) {
        //uart_sdec("order = ", order,"\n");
        for (int page_id = 0; page_id < BUDDY_PAGE_NUM; page_id += 1 << order) {
            //uart_sdec("page_id = ", page_id,"\n");
            merge_buddy(page_id, order, false);
        }
    }
}

int merge_buddy(uint32_t index, uint32_t order, int log_on)
{
    uint32_t sib_id = sibling(index, order);
    uint32_t is_left_child = index < sib_id; // me is left child
    frame_t *me = frame_arr + index;
    frame_t *sib = frame_arr + sib_id;

    // uint32_t order = me->order;
    uint64_t tmp;

    if (me->status != ORDER_VALID || sib->status != ORDER_VALID || index >= (BUDDY_PAGE_NUM) || sib_id >= (BUDDY_PAGE_NUM)) {
        return false;
    }
    //uart_puts("3\n");
    if (me->order == sib->order && index != sib_id) {
        // need to merge

        if (!is_left_child) {
            tmp = sib_id;
            sib_id = index;
            index = tmp;

            tmp = (uint64_t) me;
            me = sib;
            sib = (frame_t *) tmp;
            //uart_puts("5\n");
        }
        //uart_puts("7\n");
        
        list_del(PAGE2ADDR(index));
        //uart_puts("8\n");
        list_del(PAGE2ADDR(sib_id));
        //uart_puts("9\n");

        me->status  = ORDER_VALID;
        sib->status = PG_FREE;
        me->order++;
        //uart_puts("10\n");
        list_add_tail((struct list_head *) (PAGE2ADDR(index)), &free_list[me->order]);
        //uart_puts("11\n");
        if (log_on) {
            // uart_puts("Merge ");
            // uart_dec(index);
            // uart_puts(" and ");
            // uart_dec(sib_id);
            // uart_puts(". Order from ");
            // uart_dec(sib->order);
            // uart_puts(" to ");
            // uart_dec(me->order);
            // uart_puts(".\n");
        }
    }

    return true;
}

uint32_t sibling(uint32_t index, uint32_t order)
{
    uint32_t sib_id = index ^ (1 << order);

    return sib_id;
}

void reserve_memory(uint64_t start, uint64_t end)
{
    uint64_t start_page_idx = (start - BUDDY_BASE) / PAGE_SIZE;
    uint64_t end_page_idx = (end - BUDDY_BASE) / PAGE_SIZE;

    for (int i = start_page_idx; i <= end_page_idx; i++) {
        frame_arr[i].status = PG_RESERVED;
    }
    uart_puts("reserve memory start at 0x");
    uart_hex(start);
    uart_puts("\nreserve memory end   at 0x");
    uart_hex(end);
    uart_puts("\n\n");
}

// allocate 2^request_order pages to caller
void* buddy_alloc(uint32_t request_order)
{
    int i;
    struct list_head* buddy;
    uint32_t index;
    uint32_t order;
    uint32_t sib_id;

    //uart_sdec("Start to find request order ", request_order, "\n");
    for (i = request_order; i < FREE_LIST_MAX_ORDER; i++) {
        if (!list_empty(&free_list[i]))
            break;
    }

    if (i == FREE_LIST_MAX_ORDER) {
        //uart_sdec("Can't find the order ", request_order, "\n");
        exit(); // can't find the order
    }

    buddy = list_pop(&free_list[i]);
    //uart_shex("buddy: 0x", buddy,"\n");
    index = ADDR2PAGE((uint64_t) buddy);
    order = i;
    
    //uart_sdec("Order found ", order, "\n");
    
    while (order > request_order) {
        order--;
        sib_id = sibling(index, order);
        frame_arr[sib_id].status = ORDER_VALID;
        frame_arr[sib_id].order = order;
        list_add_tail(((struct list_head *) PAGE2ADDR(sib_id)), &free_list[order]);

        //uart_sdec("Relase redundant order ", order, "\n");
    }

    if (order != request_order) {
        //uart_puts("Something error\n");
        exit(); // something error
    }

    frame_arr[index].status = PG_ALLOC;
    frame_arr[index].order = order;

    // uart_puts("Haved request 2^");
    // uart_dec(order);
    // uart_puts(" pages in buddy system. \tPage id = ");
    // uart_dec(index);
    // uart_puts("\t is allocated.\n\n");

    return buddy;
}

void buddy_free(void* addr)
{
    uint32_t page_id = ADDR2PAGE((uint64_t) addr);
    void *page_addr = PAGE2ADDR(page_id);
    frame_t* me = &frame_arr[page_id];

    if (me->status == PG_RESERVED) return;
    else if (me->status != PG_ALLOC) exit(); // something error

    me->status = ORDER_VALID;
    list_add_tail((struct list_head *) page_addr, &free_list[me->order]);

    for (int order = me->order; order < FREE_LIST_MAX_ORDER - 1; order++) {
        if (!merge_buddy(page_id, order, true)) {
            // uart_puts("Free page_id = ");
            // uart_dec(page_id);
            // uart_puts(". Final order = ");
            // uart_dec(order);
            // uart_puts("\n\n");
            break;
        }
        else {
            // change page_id to the new id
            page_id = (page_id < sibling(page_id, order))? page_id: sibling(page_id, order);
        }
    }

}

void build_free_list()
{
    frame_t* page;

    for (int i = 0; i < FREE_LIST_MAX_ORDER; i++) {
        INIT_LIST_HEAD(&free_list[i]);
    }

    for (int page_id; page_id < BUDDY_PAGE_NUM; page_id++) {
        page = frame_arr + page_id;
        if (page->status == ORDER_VALID) {
            list_add_tail((struct list_head *)(PAGE2ADDR(page_id)), free_list + page->order);
        }
    }
}

// void print_free_list()
// {
//     struct list_head *addr;
//     for (int i = 0; i < FREE_LIST_MAX_ORDER; i++) {
//         uart_puts("free_list[");
//         uart_dec(i);
//         uart_puts("]\t = ");
//         addr = free_list[i].next;
//         while(addr != &free_list[i]) {
//             uart_dec(ADDR2PAGE((uint32_t) addr));
//             uart_puts("  ");
//             addr = addr->next;
//         }
//         uart_puts("\n");
//     }
//     uart_puts("\n");
// }

// void print_buddy()
// {
//     for (int i = 0; i < BUDDY_PAGE_NUM; i+=10) {

//         uart_puts("\nid\t");
//         for (int j = 0; j < 20; j++) {
//             if (i+j >= BUDDY_PAGE_NUM)
//                 break;
//             uart_dec(i + j);
//             uart_puts("\t");
//         }

//         uart_puts("\n\n\t");
//         for (int j = 0; j < 20; j++) {
//             if (i+j >= BUDDY_PAGE_NUM)
//                 break;
//             if (frame_arr[i+j].status == PG_ALLOC)
//                 uart_puts("X");
//             else if (frame_arr[i+j].status == PG_RESERVED)
//                 uart_puts("R");
//             else if (frame_arr[i+j].status == PG_FREE)
//                 uart_puts("F");
//             else
//                 uart_dec(frame_arr[i+j].order);
//             uart_puts("\t");
//         }
//         uart_puts("\n");
//     }

//     uart_puts("\n\n");
// }
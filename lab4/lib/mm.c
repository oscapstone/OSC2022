#include "mm.h"
#include "math.h"
#include "memory.h"
#include "mini_uart.h"

static unsigned int n_frames = 0;
static unsigned int max_size = 0;
static struct frame* frame_list[MAX_ORDER] = {NULL};
static struct frame* frame_array = NULL;

void *malloc(unsigned int size) {

    if (size > max_size) {
        printf("[error] Request exceeded allocable continuous size %d.\n", (int)max_size);
        return NULL;
    }

    int req_order = 0;
    for(unsigned int i=PAGE_SIZE; i<size; i*=2, req_order++);

    int t;
    for (t=req_order; t<MAX_ORDER; t++) {
        if (frame_list[t] != NULL) break;
    }
    
    if (t >= MAX_ORDER) {
        printf("[error] No memory allocable.\n");
        return NULL;
    }
    
    while (t != req_order) {
        struct frame* l_tmp = frame_list[t];
        frame_list[t] = l_tmp->next;
        frame_list[t]->prev = NULL;
        printf("[info] Split at order %d, new head is 0x%x.\n", t+1, frame_list[t]);

        unsigned int off = pow(2, l_tmp->val-1);
        struct frame* r_tmp = &frame_array[l_tmp->index+off];

        l_tmp->val -= 1;
        l_tmp->state = ALLOCABLE;
        l_tmp->prev = NULL;
        l_tmp->next = r_tmp;

        r_tmp->val = l_tmp->val;
        r_tmp->state = ALLOCABLE;
        r_tmp->prev = l_tmp;
        r_tmp->next = NULL;

        t--;
        if (frame_list[t] != NULL)
            frame_list[t]->prev = r_tmp;
        r_tmp->next = frame_list[t];
        frame_list[t] = l_tmp;
    }

    struct frame* ret = frame_list[req_order];
    frame_list[req_order] = ret->next;
    frame_list[req_order]->prev = NULL;
    
    ret->val = ret->val;
    ret->state = ALLOCATED;
    ret->prev = NULL;
    ret->next = NULL;

    printf("[info] allocated address: 0x%x\n", MEM_REGION_BEGIN+PAGE_SIZE*ret->index);

    return (void*)MEM_REGION_BEGIN+PAGE_SIZE*ret->index;

}

void free(void *address) {

    unsigned int idx = ((unsigned int)address-MEM_REGION_BEGIN) / PAGE_SIZE;
    struct frame* target = &frame_array[idx];

    if (target->state == ALLOCABLE || target->state == C_NALLOCABLE) {
        printf("[error] invalid free of already freed memory.\n");
        return;
    }
    printf("=========================================================\n");
    printf("[info] Now freeing address 0x%x with frame index %d.\n", address, (int)idx);

    for (int i=target->val; i<MAX_ORDER; i++) {
        
        unsigned int buddy = idx ^ pow(2, i);
        struct frame* fr_buddy = &frame_array[buddy];
        printf("[info] Index %d with buddy %d at order %d.\n", (int)idx, (int)buddy, (int)i+1);

        if (i == MAX_ORDER-1 || fr_buddy->state == ALLOCATED) {
            
            target->val = i;
            target->state = ALLOCABLE;
            target->prev = NULL;
            target->next = frame_list[i];
            if (frame_list[i] != NULL)
                frame_list[i]->prev = target;
            frame_list[i] = target;
            printf("[info] Frame index %d pushed to frame list of order %d.\n", 
                (int)target->index, (int)i+1);
            break;

        } else {

            printf("[info] Merging from order %d. Frame indices %d, %d.\n", i+1, (int)buddy, (int)idx);
            
            if (fr_buddy->prev != NULL) {
                fr_buddy->prev->next = fr_buddy->next;
            } else {
                frame_list[fr_buddy->val] = fr_buddy->next;
            }

            if (fr_buddy->next != NULL) {
                fr_buddy->next->prev = fr_buddy->prev;
            }

            fr_buddy->prev = NULL;
            fr_buddy->next = NULL;
            fr_buddy->val = C_NALLOCABLE;
            fr_buddy->state = C_NALLOCABLE;
            target->val = C_NALLOCABLE;
            target->state = C_NALLOCABLE;

            if (fr_buddy->index < target->index) {
                idx = fr_buddy->index;
                target = fr_buddy;
            }
            
            printf("[info] Frame index of next merge target is %d.\n", (int)idx);

        }
    } 

    printf("[info] Free finished.\n");
    for (int i=0; i < MAX_ORDER; i++) {
        if (frame_list[i] != NULL)
            printf("[info] Head of order %d has frame array index %d.\n",i+1,frame_list[i]->index);
        else
            printf("[info] Head of order %d has frame array index null.\n",i+1);
    }

}

void init_mm() {

    n_frames = (MEM_REGION_END-MEM_REGION_BEGIN) / PAGE_SIZE;
    frame_array = simple_malloc(n_frames*sizeof(struct frame));
    unsigned int mul = (unsigned int)pow(2, MAX_ORDER-1);
    printf("[info] Frame array start address 0x%x.\n", frame_array);
    for (unsigned int i=0; i<n_frames; i++) {
        frame_array[i].index = i;
        if (i%mul == 0) {
            frame_array[i].val = MAX_ORDER-1;
            frame_array[i].state = ALLOCABLE;
            frame_array[i].prev = &frame_array[i-mul];
            frame_array[i].next = &frame_array[i+mul];
        } else {
            frame_array[i].val = C_NALLOCABLE;
            frame_array[i].state = C_NALLOCABLE;
            frame_array[i].prev = NULL;
            frame_array[i].next = NULL;
        }
    }
    frame_array[0].prev = NULL;
    frame_array[n_frames-mul].next = NULL;

    for (int i=0; i<MAX_ORDER; i++) {
        frame_list[i] = NULL;
    }

    frame_list[5] = &frame_array[0];
    
    max_size = PAGE_SIZE * pow(2, MAX_ORDER-1);

}
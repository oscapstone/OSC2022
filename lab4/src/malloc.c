#include "malloc.h"
extern char _heap_start;
static char* simple_top = &_heap_start;

// simple_malloc
void *simple_malloc(unsigned int size)
{
    char *r = simple_top + 0x10;
    if (size < 0x18)
        size = 0x18; // minimum size 0x20 //like ptmalloc
    size = size + 0x7;
    size = 0x10 + size - size % 0x10;
    *(unsigned int *)(r - 0x8) = size;
    simple_top += size;
    return r;
}

//buddy system allocator
//https://oscapstone.github.io/labs/lab4.
//page size = 4K
/* For val (-2 ~ 6)
>=1 -> There is an allocable, contiguous memory that starts from the idx’th frame with size = 2**val * 4kb.
-1  -> allocated
-2  -> free, but it belongs to a larger contiguous memory block
*/

static unsigned long long buddysystem_start = 0x10000000L;
static frame_t framearray[0x10000] = {0};
static list_head_t freelist[MAXORDER + 1];

void init_allocator()
{
    // init framearray
    for (int i = 0; i < 0x10000; i++)
    {
        if (i % (1 << MAXORDER) == 0)
        {
            framearray[i].val = 6;
        }
        else
        {
            framearray[i].val = -2;
        }
    }

    //init frame freelist
    for (int i = 0; i <= MAXORDER; i++)
    {
        INIT_LIST_HEAD(&freelist[i]);
    }


    for (int i = 0; i < 0x10000; i++)
    {
        //init listhead for each frame
        INIT_LIST_HEAD(&framearray[i].listhead);
        framearray[i].idx = i;

        //add init frame into freelist
        if (i % (1 << MAXORDER) == 0)
        {
            list_add(&framearray[i].listhead, &freelist[MAXORDER]);
        }
    }
}

// like C malloc
//smallest 4K
void *kmalloc(unsigned int size)
{
    uart_printf("kmalloc size : 0x%x\r\n",size);
    for (int i = 0; i <= MAXORDER; i++)uart_printf("%d : %d\r\n",i, list_size(&freelist[i]));

    // get real val size
    //int allocsize;
    int val;
    for (int i = 0; i <= MAXORDER; i++)
    {

        if (size <= (0x1000 << i))
        {
            //allocsize = (0x1000 << i);
            val = i;
            break;
        }

        if(i==MAXORDER)
        {
            uart_puts("Too large size for kmalloc!!!!\r\n");
            return simple_malloc(size);
        }
    
    }

    disable_interrupt();
    for (int i = val; i <= MAXORDER; i++)
    {
        // find the smallest larger frame in freelist
        if (list_empty(&freelist[i]))
            continue;

        //get the frame
        frame_t *target_frame_ptr = (frame_t*)freelist[i].next;
        list_del_entry((struct list_head *)target_frame_ptr);

        // Release redundant memory block
        for (int j = i; j > val; j--)
        {
            release_redundant(target_frame_ptr);
        }
        target_frame_ptr->val = -1;
        enable_interrupt();
        return (void *)buddysystem_start + (0x1000 * (target_frame_ptr->idx));
    }

    uart_puts("kmalloc ERROR (all lists are empty?)!!!!\r\n");
    return (void*)0;
}

//TODO
void kfree(void* ptr)
{
    uart_printf("kfree 0x%x\r\n", ptr);
    return;
}

frame_t* release_redundant(frame_t *frame)
{
    frame->val -= 1;
    frame_t *buddyptr = get_buddy(frame);
    buddyptr->val = frame->val;
    list_add(&buddyptr->listhead, &freelist[buddyptr->val]);
    return frame;
}

frame_t* get_buddy(frame_t *frame)
{
    return &framearray[frame->idx ^ (1<<frame->val)];
}

void dump_list_info(){
    for (int i = 0; i <= MAXORDER; i++)
        uart_printf("%d : %d\r\n", i, list_size(&freelist[i]));
}
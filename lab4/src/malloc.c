#include "malloc.h"
extern char _heap_start;
static char *simple_top = &_heap_start;

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
// use val(size) and isused with freelist is enough
//page size = 4K
/* For val (-2 ~ 6)
>=0 -> There is an allocable, contiguous memory that starts from the idx’th frame with size = 2**val * 4kb.
-1  -> allocated (deprecated)
-2  -> free, but it belongs to a larger contiguous memory block (deprecated)
*/

static frame_t framearray[0x10000] = {0};
static list_head_t freelist[MAXORDER + 1];       // 4K * (idx**ORDER) (for every 4K) (page)
static list_head_t cachelist[MAXCACHEORDER + 1]; // 32, 64, 128, 256, 512  (for every 32bytes)

void init_allocator()
{
    // init framearray
    for (int i = 0; i < 0x10000; i++)
    {
        if (i % (1 << MAXORDER) == 0)
        {
            framearray[i].isused = 0;
            framearray[i].val = 6;
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
        framearray[i].cacheorder = -1;

        //add init frame into freelist
        if (i % (1 << MAXORDER) == 0)
        {
            list_add(&framearray[i].listhead, &freelist[MAXORDER]);
        }
    }

    //init cachelist
    for (int i = 0; i <= MAXCACHEORDER; i++)
    {
        INIT_LIST_HEAD(&cachelist[i]);
    }
}

//smallest 4K
void *allocpage(unsigned int size)
{
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

        if (i == MAXORDER)
        {
            uart_puts("Too large size for kmalloc!!!!\r\n");
            while (1)
                ;
            return simple_malloc(size);
        }
    }

    int needto_enable_interrupt = 0;
    if (!is_disable_interrupt())
    {
        disable_interrupt();
        needto_enable_interrupt = 1;
    }

    // find the smallest larger frame in freelist
    int target_list_val;
    for (target_list_val = val; target_list_val <= MAXORDER; target_list_val++)
    {
        if (!list_empty(&freelist[target_list_val]))
            break;
    }

    if (target_list_val > MAXORDER)
    {
        uart_puts("kmalloc ERROR (all lists are empty?)!!!!\r\n");
        while (1);
        return simple_malloc(size);
    }

    //get the frame
    frame_t *target_frame_ptr = (frame_t *)freelist[target_list_val].next;
    list_del_entry((struct list_head *)target_frame_ptr);

    // Release redundant memory block
    for (int j = target_list_val; j > val; j--)
    {
        release_redundant(target_frame_ptr);
    }
    target_frame_ptr->isused = 1;
    if (needto_enable_interrupt)
        enable_interrupt();
#ifdef DEBUG
    uart_printf("allocpage ret : 0x%x, val : %d\r\n", BUDDYSYSTEM_START + (0x1000 * (target_frame_ptr->idx)), target_frame_ptr->val);
#endif
    return (void *)BUDDYSYSTEM_START + (0x1000 * (target_frame_ptr->idx));
}

void freepage(void *ptr)
{
    //if (ptr < BUDDYSYSTEM_START)
    //    return;

    frame_t *target_frame_ptr = &framearray[((unsigned long long)ptr - BUDDYSYSTEM_START) >> 12];

    //critical section
    int needto_enable_interrupt = 0;
    if (!is_disable_interrupt())
    {
        disable_interrupt();
        needto_enable_interrupt = 1;
    }

#ifdef DEBUG
    uart_printf("freepage 0x%x, val = %d\r\n", ptr, target_frame_ptr->val);
#endif

    target_frame_ptr->isused = 0;
    while (coalesce(target_frame_ptr) == 0);

    list_add(&target_frame_ptr->listhead, &freelist[target_frame_ptr->val]);

    if (needto_enable_interrupt)
        enable_interrupt();
}

frame_t *release_redundant(frame_t *frame_ptr)
{
    frame_ptr->val -= 1;
    frame_t *buddyptr = get_buddy(frame_ptr);
    buddyptr->val = frame_ptr->val;
    buddyptr->isused = 0;
    list_add(&buddyptr->listhead, &freelist[buddyptr->val]);
#ifdef DEBUG
    uart_printf("release_redundant idx = %d,%d\r\n", frame_ptr->idx, buddyptr->idx);
#endif
    return frame_ptr;
}

frame_t *get_buddy(frame_t *frame)
{
    return &framearray[frame->idx ^ (1 << frame->val)];
}

//return 0  -> success
//return -1 -> cannot coalesce
int coalesce(frame_t *frame_ptr)
{
    frame_t *buddy = get_buddy(frame_ptr);

    // MAXORDER
    if (frame_ptr->val == MAXORDER)
        return -1;

    // val not the same (there is some chunks in the buddy used)
    if (frame_ptr->val != buddy->val)
        return -1;

    //buddy is used
    if (buddy->isused == 1)
        return -1;

    list_del_entry((struct list_head *)buddy);
    frame_ptr->val += 1;

#ifdef DEBUG
    uart_printf("coalesce idx = %d,%d\r\n", frame_ptr->idx, buddy->idx);
#endif
    return 0;
}

void *alloccache(unsigned int size)
{
    //get order
    int order;
    for (int i = 0; i <= MAXCACHEORDER; i++)
    {
        if (size <= (32 << i))
        {
            order = i;
            break;
        }
    }

    if (list_empty(&cachelist[order]))
    {
        page2caches(order);
    }

    list_head_t *r = cachelist[order].next;
    list_del_entry(r);
#ifdef DEBUG
    uart_printf("alloc cache order : %d\r\n", order);
#endif
    return r;
}

void page2caches(int order)
{
    //make caches of the order from a page
    char *page = allocpage(0x1000);
    frame_t *pageframe_ptr = &framearray[((unsigned long long)page - BUDDYSYSTEM_START) >> 12];
    pageframe_ptr->cacheorder = order;

    // split page into a lot of caches and push them into cachelist
    int cachesize = (32 << order);
    for (int i = 0; i < 0x1000; i += cachesize)
    {
        list_head_t *c = (list_head_t *)(page + i);
        list_add(c, &cachelist[order]);
    }
}

void freecache(void *ptr)
{
    list_head_t *c = (list_head_t *)ptr;
    frame_t *pageframe_ptr = &framearray[((unsigned long long)ptr - BUDDYSYSTEM_START) >> 12];
    list_add(c, &cachelist[pageframe_ptr->cacheorder]);
#ifdef DEBUG
    uart_printf("freecache 0x%x, order : %d\r\n", ptr, pageframe_ptr->cacheorder);
#endif
}

void *kmalloc(unsigned int size)
{
#ifdef DEBUG
    uart_printf("kmalloc size: %d\r\n", size);
#endif
    //For page
    if (size > (32 << MAXCACHEORDER))
    {
        void *r = allocpage(size);
#ifdef DEBUG
        dump_freelist_info();
        dump_cachelist_info();
#endif
        return r;
    }

    void *r = alloccache(size);

#ifdef DEBUG
    dump_freelist_info();
    dump_cachelist_info();
    uart_printf("kmalloc ret 0x%x\r\n", r);
#endif

    //For cache
    return r;
}

void kfree(void *ptr)
{
#ifdef DEBUG
    uart_printf("kfree 0x%x\r\n", ptr);
#endif
    //For page
    if ((unsigned long long)ptr % 0x1000 == 0 && framearray[((unsigned long long)ptr - BUDDYSYSTEM_START) >> 12].cacheorder == -1)
    {
        freepage(ptr);
#ifdef DEBUG
        dump_freelist_info();
        dump_cachelist_info();
#endif
        return;
    }

    //For cache
    freecache(ptr);
#ifdef DEBUG
    dump_freelist_info();
    dump_cachelist_info();
#endif
}

void dump_freelist_info()
{
    for (int i = 0; i <= MAXORDER; i++)
        uart_printf("freelist %d : %d\r\n", i, list_size(&freelist[i]));
}

void dump_cachelist_info()
{
    for (int i = 0; i <= MAXCACHEORDER; i++)
        uart_printf("cachelist %d : %d\r\n", i, list_size(&cachelist[i]));
}

void memory_reserve(unsigned long long start, unsigned long long end)
{
    start -= start % 0x1000; // floor (align 0x1000)
    end = end % 0x1000 ? end + 0x1000 - (end % 0x1000) : end; // ceiling (align 0x1000)
    uart_printf("start 0x%x\r\n", start);
    uart_printf("end 0x%x\r\n",end);

    //delete page from freelist
    for (int order = MAXORDER; order >= 0; order--)
    {
        list_head_t *pos;
        list_for_each(pos, &freelist[order])
        {
            unsigned long long pagestart = ((frame_t *)pos)->idx * 0x1000L + BUDDYSYSTEM_START;
            unsigned long long pageend = pagestart + (0x1000L << order);

            if (start <= pagestart && end >= pageend) // if page all in reserved memory -> delete it from freelist
            {
                ((frame_t *)pos)->isused = 1;
                list_del_entry(pos);
                uart_printf("del order %d\r\n",order);
                dump_freelist_info();
            }
            else if (start >= pageend || end <= pagestart) // no intersection
            {
                continue;
            }
            else // partial intersection (or reversed memory all in the page)
            {
                list_del_entry(pos);
                list_head_t *temppos = pos -> prev;
                list_add(&release_redundant((frame_t *)pos)->listhead, &freelist[order - 1]);
                pos = temppos;
                dump_freelist_info();
            }
        }
    }
}

void alloctest()
{
    uart_printf("alloc test\r\n");

    memory_reserve(0x1FFFAddb, 0x1FFFFdda);
    
    char *a = kmalloc(0x10);
    char *b = kmalloc(0x100);
    char *c = kmalloc(0x1000);

    kfree(a);
    kfree(b);
    kfree(c);

    a = kmalloc(32);
    char *aa = kmalloc(50);
    b = kmalloc(64);
    char *bb = kmalloc(64);
    c = kmalloc(128);
    char *cc = kmalloc(129);
    char *d = kmalloc(256);
    char *dd = kmalloc(256);
    char *e = kmalloc(512);
    char *ee = kmalloc(999);

    kfree(a);
    kfree(aa);
    kfree(b);
    kfree(bb);
    kfree(c);
    kfree(cc);
    kfree(dd);
    kfree(d);
    kfree(e);
    kfree(ee);
}
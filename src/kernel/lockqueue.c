#include <lockqueue.h>
#include <kmalloc.h>
#include <interrupt.h>

#define queue_flag_fifo 0b1
#define queue_readflag(q,f) ( ((Queue*)q)->flags & f )
#define queue_setflag(q,f) ( ((Queue*)q)->flags |= f )
#define queue_unsetflag(q,f) ( ((Queue*)q)->flags &= ~f )

void *lockqueue_front(LockQueue *qu)
{
    size_t flags = 0;
    flags = interrupt_disable_save();
    void *retval = 0;
    if(qu->size==0){
        retval = 0;
        goto ret;
    }
    retval = qu->front->entry;
    ret:
    interrupt_enable_restore(flags);
    return retval;
}
void *lockqueue_pop(LockQueue *qu)
{
    size_t flags = 0;
    flags = interrupt_disable_save();
    void *retval = 0;
    if(qu->size==0) goto ret;
    LockQueue_Entry *qe = qu->front;
    retval = qe->entry;
    qu->front = qe->fd;
    qu->size -= 1;
    kfree(qe);

    ret:
    interrupt_enable_restore(flags);
    return retval;
}
void lockqueue_push(LockQueue *qu, void *entry)
{
    size_t flags = 0;
    flags = interrupt_disable_save();
    LockQueue_Entry *qe = (LockQueue_Entry *)kmalloc(sizeof(LockQueue_Entry));
    qe->entry = entry;
    qe->fd = 0;
    if(qu->size==0){
        qu->front = qu->back = qe;
    }
    else{
        qu->back->fd = qe;
        qu->back = qe;
    }
    qu->size += 1;
    interrupt_enable_restore(flags);
}

LockQueue *lockqueue_new()
{
    LockQueue *qu = (LockQueue *)kmalloc(sizeof(LockQueue));
    qu->front = qu->back = 0;
    qu->size = 0;
    qu->flags = 0;
    return qu;
}
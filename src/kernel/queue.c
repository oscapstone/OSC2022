#include <queue.h>
#include <kmalloc.h>
#include <interrupt.h>
#include <lock.h>

#define queue_flag_fifo 0b1
#define queue_readflag(q,f) ( ((Queue*)q)->flags & f )
#define queue_setflag(q,f) ( ((Queue*)q)->flags |= f )
#define queue_unsetflag(q,f) ( ((Queue*)q)->flags &= ~f )


void *queue_front(Queue *qu)
{
    //if(qu->lock)lock_get(qu->lock);
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
    //if(qu->lock)lock_release(qu->lock);
    return retval;
}
void *queue_pop(Queue *qu)
{
    //if(qu->lock)lock_get(qu->lock);
    size_t flags = 0;
    flags = interrupt_disable_save();
    void *retval = 0;
    if(qu->size==0) goto ret;
    Queue_Entry *qe = qu->front;
    retval = qe->entry;
    qu->front = qe->fd;
    qu->size -= 1;
    kfree(qe);

    ret:
    //if(qu->lock)lock_release(qu->lock);
    interrupt_enable_restore(flags);
    return retval;
}
void queue_push(Queue *qu, void *entry)
{
    //if(qu->lock)lock_get(qu->lock);
    size_t flags = 0;
    flags = interrupt_disable_save();
    Queue_Entry *qe = (Queue_Entry *)kmalloc(sizeof(Queue_Entry));
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
    //if(qu->lock)lock_release(qu->lock);
}

Queue *queue_new()
{
    Queue *qu = (Queue *)kmalloc(sizeof(Queue));
    qu->front = qu->back = 0;
    qu->size = 0;
    qu->flags = 0;
    //qu->lock = lock_new();
    return qu;
}
#include <sched.h>
#include <list.h>
#include <malloc.h>
#include <string.h>
#include <stddef.h>
#include <allocator.h>

Thread *thread_pool;
Thread *thread_head;

void init_thread_pool_and_head(){
    thread_pool = (Thread*)kmalloc(sizeof(Thread) * MAX_THREAD);
    memset((char *)thread_pool, 0, sizeof(Thread) * MAX_THREAD);
    for(unsigned int i = 0; i < MAX_THREAD; i++){
        INIT_LIST_HEAD(&thread_pool[i].list);
        thread_pool[i].state = NOUSE;
        thread_pool[i].id = i;
        thread_pool[i].ustack_addr = NULL;
        thread_pool[i].kstack_addr = NULL;
    }

    thread_head = (Thread *)kmalloc(sizeof(Thread));
    memset((char *)thread_head, 0, sizeof(Thread));
    INIT_LIST_HEAD(&thread_head->list);
}

Thread *thread_create(void(*func)()){
    unsigned int idx;
    for(idx = 0; idx < MAX_THREAD; idx++){
        if(thread_pool[idx].state == NOUSE)
            break;
    }
    if(idx == MAX_THREAD) return NULL;

    Thread *new_thread = &thread_pool[idx];
    new_thread->state = WAIT;
    new_thread->ustack_addr = kmalloc(STACT_SIZE);
    new_thread->kstack_addr = kmalloc(STACT_SIZE);
    new_thread->ctx.fp = (unsigned long)new_thread->kstack_addr + STACT_SIZE;
    new_thread->ctx.sp = (unsigned long)new_thread->kstack_addr + STACT_SIZE;
    new_thread->ctx.lr = (unsigned long)func;


    list_add_tail(&new_thread->list, &thread_head->list);

    return new_thread;
}
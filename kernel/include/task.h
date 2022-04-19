#ifndef	TASK_H_
#define TASK_H_
#include <stddef.h>
#include <list.h>
typedef void (*Handler)();

// typedef struct _Task{
//     Handler handler;
//     unsigned int priority;
//     struct _Task *next;
//     struct _Task *prev;
// }Task;

void init_task_head();

typedef struct _Task{
    struct list_head list;
    Handler handler;
    unsigned int priority;
}Task;


void add_task(Handler, unsigned int);
void do_task();

#endif
#include "utils.h"
#include "mini_uart.h"
#include "stdlib.h"
struct taskq
{
    struct taskq *next;
    void (*handler)();
    int privilege;
};
typedef struct taskq taskq;
taskq *taskq_head;

void init_taskq();
void add_task(void (*handler)(),int privilege);
bool is_taskq_empty();
void do_task(int* last_privilege);
#include "timer.h"

#include "uart.h"
#include "exception.h"
#include "mem.h"

timer_list first;

void cb_message(char* message) {
    // top
    printf("%s\n# ", message);
};

void cb_message_delay(char* message) {
    // top
    printf("start\n");
    unsigned long dtime = 0;
    int i = 0;
    while (message[i]) {
        dtime = dtime*10 + (int)message[i] - (int)'0';
        i++;
    }
    // bottom
    enable_current_interrupt();
    unsigned long time = get_time10();
    int delay[10] = {0};
    int count = 1;
    while (get_time10() < time + dtime*10) {
        if ((get_time10() - time) % 10 == 0 && delay[(get_time10() - time) / 10] == 0) {
            delay[(get_time10() - time) / 10] = 1;
            printf("# %d second\n", count);
            count++;
        }
    }
    printf("# finish\n# ");
};

void timer_init() {
    first.next = 0;
    first.next->prev = &first;
    first.priority = 10000;
};

void add_timer(timer_list *task) {
    task->next = first.next;
    task->next->prev = task;
    first.next = task;
    task->prev = &first;
    if (task->next == 0) {
        set_time(1);
        enable_timer_interrupt();
    }
    
};

void handle_timer_list() {
    timer_list *find = first.next;
    if (find == 0) disable_timer_interrupt();
    timer_list *target = 0;
    while (find) {
        if (find->priority == 1) {
            target = find;
            find->next->prev = find->prev;
            find->prev->next = find->next;
            
        }
        find->priority -= 1;
        find = find->next;
    }
    set_time(1);
    if (target != 0) {
        target->callback(target->argv);
        kfree(target);
    }
};
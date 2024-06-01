#include "utils.h"
struct timer
{
    struct timer *next;
    void (*callback)(char* s);
    char *message;
    unsigned int value;
};
typedef struct timer timer;
timer *head;

void init_timer();
void add_timer(void (*callback)(char* s),char *message,int after);
bool timer_is_empty();
void itr_timer_queue();
void disable_timer_interrupt();
void enable_timer_interrupt();
bool is_timerq_empty();
void set_expired_time(int );
void get_current_time(unsigned long long *time_count,unsigned long long *time_freq);
timer* to_next_timer();
timer* get_head_timer();
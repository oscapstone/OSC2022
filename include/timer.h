#ifndef TIMER
#define TIMER

typedef struct timer_event 
{
    unsigned long long timeout_tick;
    unsigned long long begin_tick;
    char* message;
    void* callback;
    struct timer_event* next_event;
} timer_event_t;

#define is_empty(x) (x) == NULL
#define is_not_empty(x) (x) != NULL

#define STR(x) #x
#define XSTR(s) STR(s)
#define CORE0_TIMER_IRQ_CTRL 0x40000040

void add_timer(void (*callback)(char* message), unsigned long long timeout_tick, char *message);
void core_timer_handler();
void enable_core_timer();
void disable_core_timer();
unsigned long long get_current_tick();
void set_core_timer_by_tick(unsigned long long tick);
void set_core_timer_by_second(unsigned long long second);
unsigned long long tick2second(unsigned long long tick);
unsigned long long second2tick(unsigned long long second); 
void show_message(char* message);
void increment_timeout_2_seconds(char* message);

#endif
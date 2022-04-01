#define CORE0_TIMER_IRQ_CTRL 0x40000040
extern void core_timer_enable (void);
extern void core_timer_disable (void);
void el0_timer_irq();
void el1_timer_irq();
unsigned long get_current_time();
void set_expired_time(unsigned int duration);
void add_timer(void (*callback)(char *), char *args, unsigned int duration);

typedef struct timeout_event{
    unsigned int register_time;
    unsigned int duration;
    void (*callback)(char *);
    char args[20];
    struct timeout_event *prev, *next;
}timeout_event;
timeout_event *timeout_queue_head, *timeout_queue_tail;

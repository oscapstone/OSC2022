#ifndef INTERRUPT
#define INTERRUPT

#define UART_INTERRUPT_TX_PRIORITY 2
#define UART_INTERRUPT_RX_PRIORITY 3
#define TIMER_INTERRUPT_PRIORITY 4
#define WAIT_INTERRUPT_PRIORITY 999

typedef struct interrupt_event 
{
    unsigned long long priority;
    void* handler;
    struct interrupt_event* next_event;
} interrupt_event_t;

#define is_empty(x) (x) == NULL
#define is_not_empty(x) (x) != NULL

void add_interrupt(void (*handler)(), unsigned long long priority);
void run_interrupt();
void run_preemption();
void exec_handler(interrupt_event_t* event_p);

static inline void enable_interrupt()
{
    __asm__ __volatile__("msr daifclr, 0xf");
}

static inline void disable_interrupt()
{
    __asm__ __volatile__("msr daifset, 0xf");
}

unsigned long long is_disable_interrupt();
void lock_interrupt();
void unlock_interrupt();

void wait_loop();

#endif
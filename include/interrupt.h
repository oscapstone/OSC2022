#ifndef INTERRUPT
#define INTERRUPT

#define UART_INTERRUPT_TX_PRIORITY 2
#define UART_INTERRUPT_RX_PRIORITY 3
#define TIMER_INTERRUPT_PRIORITY 5
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

void enable_interrupt();
void disable_interrupt();

void wait_loop();

#endif
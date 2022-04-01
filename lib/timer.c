#include "timer.h"
#include "type.h"
#include "uart.h"
#include "string.h"
#include "memory.h"

volatile timer_event_t* timer_event_pointer = NULL;

unsigned long long get_end_tick(timer_event_t* event_p)
{
    return event_p->timeout_tick + event_p->begin_tick;
}

unsigned long long get_current_tick()
{
    unsigned long long current_tick;
    asm volatile(
        "mrs %0, cntpct_el0\n\t"
        : "=r"(current_tick)
    ); 
    return current_tick;
}

void set_first_timer()
{
    unsigned long long tick = get_end_tick(timer_event_pointer) - get_current_tick();
    set_core_timer_by_tick(tick);
}

int set_next_timer()
{
    /* TODO
    free executed timer event
    */

    timer_event_pointer = timer_event_pointer->next_event;
    if(is_not_empty(timer_event_pointer))
    {
        unsigned long long tick = get_end_tick(timer_event_pointer) - get_current_tick();
        set_core_timer_by_tick(tick);
        return 1;
    }
    else
    {
        return 0;
    }
}

void init_timer_event(timer_event_t* event_p, void (*callback)(char* message), unsigned long long timeout_tick, char *message)
{
    event_p->timeout_tick = timeout_tick;
    event_p->begin_tick = get_current_tick();
    if(is_not_empty(message))
    {
        // uart_puts("Create Message\n");
        event_p->message = (char*)simple_malloc((strlen(message) + 1) * sizeof(char));
        // uart_puts("Copy Message\n");
        strcpy(event_p->message, message);
    }
    else
    {
        // uart_puts("NULL message\n");
        event_p->message = NULL;
    }
        
    event_p->callback = callback;
    event_p->next_event = NULL;
}

void insert_after_it(timer_event_t* prev, timer_event_t* next)
{
    next->next_event = prev->next_event;
    prev->next_event = next;
}

void stack_head(timer_event_t* event_p)
{
    event_p->next_event = timer_event_pointer;
    timer_event_pointer = event_p;
    set_first_timer();
}

void insert_timer_event(timer_event_t* event_p)
{
    unsigned long long end_tick = get_end_tick(event_p);
    if(end_tick < get_end_tick(timer_event_pointer)) // less than head 
    {
        stack_head(event_p);
        return;
    }

    timer_event_t* current_event_p = timer_event_pointer;
    while(is_not_empty(current_event_p->next_event))
    {
        if(end_tick < get_end_tick(current_event_p->next_event)) // insert here
        {
            insert_after_it(current_event_p, event_p);
            return;
        }

        current_event_p = current_event_p->next_event; // check next
    }

    current_event_p->next_event = event_p; // greater than tail
}

void add_timer(void (*callback)(char* message), unsigned long long timeout_tick, char *message)
{
    // uart_puts("Create new event\n");
    timer_event_t* new_timer_event = (timer_event_t*)simple_malloc(sizeof(timer_event_t));
    // uart_puts("Init events\n");
    init_timer_event(new_timer_event, callback, timeout_tick, message);

    if(is_empty(timer_event_pointer))
    {
        // uart_puts("Add first events\n");
        timer_event_pointer = new_timer_event;  
    }
    else
    {
        // uart_puts("Insert events\n");
        insert_timer_event(new_timer_event);
    }
}

void exec_timer_callback(timer_event_t* event_p)
{
    char second_string[24];
    unsigned long long current_tick = get_current_tick();
    
    uart_dem();

    unsigned long long second = tick2second(current_tick);
    uart_puts("Current time(s): "); uart_num(second); uart_newline();

    // second = tick2second(current_tick - event_p->begin_tick);
    second = tick2second(event_p->timeout_tick);
    uart_puts("Executed time(s): "); uart_num(second); uart_newline();
    ((void (*)(char *))event_p->callback)(event_p->message);

    uart_dem();
    uart_prefix();
}

void core_timer_handler()
{
    static int wait = 1;

    if(is_not_empty(timer_event_pointer))
    {
        if(wait == 0)
        {
            exec_timer_callback(timer_event_pointer);
            int have_next = set_next_timer();

            if(have_next == 0)
            {
                wait = 1;
                // uart_puts("Empty events\n");
                set_core_timer_by_second(1); // wait 1s for next interrupt
            }
        }
        else
        {
            // uart_puts("Set first events\n");
            wait = 0;
            set_first_timer();
        }
    }
    else // nothing to do
    {
        // uart_puts("Nothing to do\n");
        wait = 1;    
        set_core_timer_by_second(1); // wait 1s for next interrupt
    }

    enable_core_timer();
}

void enable_core_timer()
{
    asm volatile(
        "mov x1, 1\n\t"
        "msr cntp_ctl_el0, x1\n\t" // enable
        "mov x2, 2\n\t"
        "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
        "str w2, [x1]\n\t" // unmask timer interrupt
    );
}

void disable_core_timer()
{
    asm volatile(
        "mov x2, 0\n\t"
        "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
        "str w2, [x1]\n\t" // mask timer interrupt
    );
}

void set_core_timer_by_tick(unsigned long long tick)
{
    asm volatile(
        "msr cntp_tval_el0, %0\n\t"
        ::"r" (tick)
    );
}

void set_core_timer_by_second(unsigned long long second)
{
    asm volatile(
        "mrs x1, cntfrq_el0\n\t"
        "mul x1, x1, %0\n\t"
        "msr cntp_tval_el0, x1\n\t"
        ::"r" (second)
    );
}

unsigned long long get_timer_frequency() 
{
    unsigned long long freq;
    asm volatile(
        "mrs %0, cntfrq_el0\n\t"
        :"=r" (freq)
    );

    return freq;
}

unsigned long long tick2second(unsigned long long tick) 
{
    unsigned long long freq = get_timer_frequency();
    unsigned long long second = tick / freq;
    return second;
}

unsigned long long second2tick(unsigned long long second) 
{
    unsigned long long freq = get_timer_frequency();
    unsigned long long tick = second * freq;
    return tick;
}

void show_message(char* message)
{
    uart_puts("MESSAGE: ");
    uart_puts(message);
    uart_newline();
}

void increment_timeout_2_seconds(char* message)
{
    static unsigned long seconds = 0;
    static unsigned long next_val = 1;
    unsigned long tick;

    seconds += next_val;
    next_val += 2;
    tick = second2tick(next_val);

    add_timer(increment_timeout_2_seconds, tick, NULL);

    uart_puts("Pass seconds = "); uart_num(seconds); uart_newline();
}

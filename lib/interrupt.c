#include "interrupt.h"
#include "type.h"
#include "uart.h"
#include "string.h"
#include "memory.h"
#include "utils.h"

volatile interrupt_event_t* interrupt_event_pointer = NULL;
volatile interrupt_event_t* preemption_event_pointer = NULL;

volatile int flag = 0; // 0 - wait loop, 1 - preemption wait loop

void init_interrupt_event(interrupt_event_t* event_p, void (*handler)(), unsigned long long priority)
{
    event_p->priority = priority;        
    event_p->handler = handler;
    event_p->next_event = NULL;
}

void int_insert_after_it(interrupt_event_t* prev, interrupt_event_t* next)
{
    next->next_event = prev->next_event;
    prev->next_event = next;
}

void int_stack_head(interrupt_event_t* event_p)
{
    event_p->next_event = interrupt_event_pointer;
    interrupt_event_pointer = event_p;
}

void insert_preemption_event(interrupt_event_t* event_p)
{
    unsigned long long event_priority = event_p->priority;
    if(event_priority < preemption_event_pointer->priority) // less than head 
    {
        // uart_puts("Preemption to preemption\n");
        exec_handler(event_p);    
        return;
    }

    interrupt_event_t* current_event_p = preemption_event_pointer;
    while(is_not_empty(current_event_p->next_event))
    {
        if(event_priority < current_event_p->next_event->priority) // insert here
        {
            int_insert_after_it(current_event_p, event_p);
            return;
        }

        current_event_p = current_event_p->next_event; // check next
    }

    current_event_p->next_event = event_p; // greater than tail
}

void insert_interrupt_event(interrupt_event_t* event_p)
{
    unsigned long long event_priority = event_p->priority;
    if(event_priority < interrupt_event_pointer->priority) // less than head 
    {
        // add preemption to preemption (another linking list for preemptions)
        lock_interrupt();
        if(is_empty(preemption_event_pointer))
        {
            // uart_puts("First preemption\n");
            preemption_event_pointer = event_p; 
            run_preemption();
        }
        else
        {
            // uart_puts("Insert preemption\n");
            insert_preemption_event(event_p);
        }
        unlock_interrupt();
            
        return;
    }

    interrupt_event_t* current_event_p = interrupt_event_pointer;
    while(is_not_empty(current_event_p->next_event))
    {
        if(event_priority < current_event_p->next_event->priority) // insert here
        {
            int_insert_after_it(current_event_p, event_p);
            return;
        }

        current_event_p = current_event_p->next_event; // check next
    }

    current_event_p->next_event = event_p; // greater than tail
}

void add_interrupt(void (*callback)(), unsigned long long priority)
{
    interrupt_event_t* new_interrupt_event = (interrupt_event_t*)malloc(sizeof(interrupt_event_t));
    init_interrupt_event(new_interrupt_event, callback, priority);

    lock_interrupt();
    // uart_puts("TAR interrupt: "); uart_hex(interrupt_event_pointer); uart_newline();
    // uart_puts("New interrupt: "); uart_hex(new_interrupt_event); uart_newline();
   
    if(is_empty(interrupt_event_pointer))
    {
        // uart_puts("First interrupt\n");
        interrupt_event_pointer = new_interrupt_event;  
        run_interrupt();
    }
    else
    {
        // uart_puts("Insert interrupt\n");
        insert_interrupt_event(new_interrupt_event);
    }
    unlock_interrupt();
}

void run_preemption()
{
    while(is_not_empty(preemption_event_pointer))
    {
        unlock_interrupt();
        exec_handler(preemption_event_pointer);
        lock_interrupt();
        preemption_event_pointer = preemption_event_pointer->next_event;
    }
}

void run_interrupt()
{
    while(is_not_empty(interrupt_event_pointer))
    {
        // uart_puts("Start run interrupt\n");
        interrupt_event_t* next_event_p = interrupt_event_pointer->next_event;
        unlock_interrupt();
        exec_handler(interrupt_event_pointer);
        lock_interrupt();
        interrupt_event_pointer = next_event_p;
    }
}

void exec_handler(interrupt_event_t* event_p)
{
    // uart_puts("Exec Handler: "); uart_num(event_p->priority); uart_newline();
    ((void (*)())event_p->handler)();
    free(event_p);
}

unsigned long long is_disable_interrupt()
{
    unsigned long long daif;
    __asm__ __volatile__("mrs %0, daif\n\t"
                         : "=r"(daif));

    return daif != 0;  //enable -> daif == 0 (no mask)
}

int64 lock_count = 0;
void lock_interrupt() {
    disable_interrupt();
    lock_count++;
    // uart_puts("add lock cou: "); uart_num(lock_count);
    // uart_puts(", is disable? "); uart_num(is_disable_interrupt()); uart_newline();
}

void unlock_interrupt() {
    lock_count--;
    // uart_puts("reduce lock cou: "); uart_num(lock_count);
    
    if(lock_count == 0) {
        enable_interrupt();
    }
    else if(lock_count < 0) {
        raiseError("interrupt unlock before lock\n");
    }

    // uart_puts(", is disable? "); uart_num(is_disable_interrupt()); uart_newline();
}

void wait_loop()
{
    char c = NULL;
    while(is_empty(c))
    {
        c = uart_async_getc();

        if(flag == 0)
        {
            if(c == 'n')
            {
                flag = 1;
                c = NULL;
                uart_async_puts("Another wait loop with higher priority\n");
                add_interrupt(wait_loop, WAIT_INTERRUPT_PRIORITY - 1);
            }
            uart_async_puts("Please enter n to preemption loop, or other key to continue...\n"); 
        }
        else
        {
            uart_async_puts("Please enter any key to continue (preemption)...\n"); 
            if(is_not_empty(c))
            {
                flag = 0;
                uart_async_puts("Preemption done...\n"); 
            }
        }
            
        delay_ms(1000);
    }
}
#include "timer.h"

timer_event_t* timer_event_pointer = NULL;

uint64 get_end_tick(timer_event_t* event_p) {
    return event_p->timeout_tick + event_p->begin_tick;
}

void set_first_timer() {
    uint64 tick = get_end_tick(timer_event_pointer) - get_current_tick();

    if(get_end_tick(timer_event_pointer) < get_current_tick()) {
        // uart_puts("Timer is too slow\n");
        set_core_timer_by_tick(6000);
    }
    else {
        // uart_printf("Set tick: %d\n", tick);
        set_core_timer_by_tick(tick);
    }
}

int set_next_timer() {
    timer_event_t* next_event_p = timer_event_pointer->next_event;
    free(timer_event_pointer->message);
    free(timer_event_pointer);
    timer_event_pointer = next_event_p;
    if(is_not_empty(timer_event_pointer)) {
        set_first_timer();
        return 1;
    }
    
    return 0;
}

void init_timer_event(timer_event_t* event_p, void (*callback)(char* message), uint64 timeout_tick, char *message)
{
    event_p->timeout_tick = timeout_tick;
    event_p->begin_tick = get_current_tick();
    if(is_not_empty(message)) {
        event_p->message = (char*)malloc((strlen(message) + 1) * sizeof(char));
        strcpy(event_p->message, message);
    }
    else {
        event_p->message = NULL;
    }
        
    event_p->callback = callback;
    event_p->next_event = NULL;
}

void insert_after_it(timer_event_t* prev, timer_event_t* next) {
    next->next_event = prev->next_event;
    prev->next_event = next;
}

void stack_head(timer_event_t* event_p) {
    event_p->next_event = timer_event_pointer;
    timer_event_pointer = event_p;
    set_first_timer();
}

void insert_timer_event(timer_event_t* event_p) {
    uint64 end_tick = get_end_tick(event_p);

    // less than head
    if(end_tick < get_end_tick(timer_event_pointer)) {
        stack_head(event_p);
        return;
    }

    // middle
    timer_event_t* current_event_p = timer_event_pointer;
    while(is_not_empty(current_event_p->next_event)) {
        // insert here
        if(end_tick < get_end_tick(current_event_p->next_event)) {
            insert_after_it(current_event_p, event_p);
            return;
        }
        
        current_event_p = current_event_p->next_event; // check next
    }

    current_event_p->next_event = event_p; // greater than tail
}

void add_timer(void (*callback)(char* message), uint64 timeout_tick, char *message) {
    lock_interrupt();
    timer_event_t* new_timer_event = (timer_event_t*)malloc(sizeof(timer_event_t));
    init_timer_event(new_timer_event, callback, timeout_tick, message);

    if(is_empty(timer_event_pointer)) {
        timer_event_pointer = new_timer_event;  
    }
    else {
        insert_timer_event(new_timer_event);
    }
    unlock_interrupt();
}

void exec_timer_callback(timer_event_t* event_p) {
    // uart_printf("Exec Timer Callback at 0x%x\n", event_p->callback);
    ((void (*)(char *))event_p->callback)(event_p->message);
}

static int wait = 1;
void core_timer_handler() {
    // uart_printf("Timer Handler\n");
    lock_interrupt();
    if(is_not_empty(timer_event_pointer)) {
        // uart_printf("Start Handler\n");
        if(wait == 0) {
            // uart_printf("Start Exec\n");
            exec_timer_callback(timer_event_pointer);
            int have_next = set_next_timer();
            
            if(have_next == 0) {
                wait = 1;
                set_core_timer_by_second(1); // wait 1s for next interrupt
            }
        }
        else {
            // uart_printf("Start First\n");
            wait = 0;
            set_first_timer();
        }
    }
    else {
        // uart_printf("1s Done\n");
        wait = 1;    
        set_core_timer_by_second(1); // wait 1s for next interrupt
    }
    
    unlock_interrupt();
    enable_core_timer();
    // uart_printf("Timer Handler Done\n");
}

bool hasTimerEvent() {
    return is_not_empty(timer_event_pointer);
}

void enable_core_timer() {
    asm volatile(
        "mov x1, 1\n\t"
        "msr cntp_ctl_el0, x1\n\t" // enable
        "mov x2, 2\n\t"
        "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
        "str w2, [x1]\n\t" // unmask timer interrupt
    );
}

void disable_core_timer() {
    asm volatile(
        "mov x2, 0\n\t"
        "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
        "str w2, [x1]\n\t" // mask timer interrupt
    );
}

void set_core_timer_by_tick(uint64 tick) {
    asm volatile(
        "msr cntp_tval_el0, %0\n\t"
        ::"r" (tick)
    );
}

void set_core_timer_by_second(uint64 second) {
    asm volatile(
        "mrs x1, cntfrq_el0\n\t"
        "mul x1, x1, %0\n\t"
        "msr cntp_tval_el0, x1\n\t"
        ::"r" (second)
    );
}

uint64 get_current_tick() {
    uint64 current_tick;
    asm volatile(
        "mrs %0, cntpct_el0\n\t"
        : "=r"(current_tick)
    ); 
    return current_tick;
}

uint64 get_timer_frequency() {
    uint64 freq;
    asm volatile(
        "mrs %0, cntfrq_el0\n\t"
        :"=r" (freq)
    );

    return freq;
}

uint64 tick2second(uint64 tick) {
    uint64 freq = get_timer_frequency();
    uint64 second = tick / freq;
    return second;
}

uint64 second2tick(uint64 second) {
    uint64 freq = get_timer_frequency();
    uint64 tick = second * freq;
    return tick;
}

void show_message(char* message) {
    uart_puts("MESSAGE: ");
    uart_puts(message);
    uart_newline();
}

void increment_timeout_2_seconds(char* message) {
    static unsigned long seconds = 0;
    static unsigned long next_val = 1;
    unsigned long tick;

    seconds += next_val;
    next_val += 2;
    tick = second2tick(next_val);

    add_timer(increment_timeout_2_seconds, tick, NULL);
    uart_puts("Pass seconds = "); uart_num(seconds); uart_newline();
}

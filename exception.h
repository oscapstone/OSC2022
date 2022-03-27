void enable_interrupt();
void disable_interrupt();

void enable_timer_interrupt();
void disable_timer_interrupt();

void set_time(unsigned int duration);
unsigned long get_time10();

void handle_timer0_irq();
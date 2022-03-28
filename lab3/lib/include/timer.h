#define CORE0_TIMER_IRQ_CTRL 0x40000040

#define STR(x) #x
#define XSTR(s) STR(s)


void core_timer_enable(void);
void clock_alert(void);
void set_core_timer_interrupt(unsigned long long expired_time);

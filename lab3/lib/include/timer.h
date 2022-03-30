#define CORE0_TIMER_IRQ_CTRL 0x40000040

#define STR(x) #x
#define XSTR(s) STR(s)

typedef void (*callback_typ)(char *);

void core_timer_enable(void);
void clock_alert(void);
void set_core_timer_interrupt(unsigned long long expired_time);
void add_timer(callback_typ callback, int time, char *msg);

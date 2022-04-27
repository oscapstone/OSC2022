#include "stdint.h"

#define CORE0_TIMER_IRQ_CTRL 0x40000040

#define STR(x) #x
#define XSTR(s) STR(s)

typedef void (*callback_typ)(char *);

void core_timer_enable(void);
void core_timer_interrupt_enable(void);
void core_timer_interrupt_disable(void);
unsigned long clock_time(void);
void clock_alert(char *str);
void timeout_print(char *str);
void print_time(void);
void set_core_timer_interrupt(uint64_t expired_time);
void add_timer(callback_typ callback, char *msg, int time);
void pop_timer(void);
void add_task(callback_typ callback, char *msg, int piority);
void pop_task(void);

typedef struct timer_list timer_list;

struct timer_list{
  uint64_t expired_time;
  callback_typ call_back;
  char msg[100];
  struct timer_list *next;
};

typedef struct task_list task_list;
struct task_list{
  callback_typ task_call_back;
  char *arg;
  int piority;
  struct task_list *next;
};


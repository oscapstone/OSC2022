#include "timer.h"
#include "mini_uart.h"
#include <stdint.h>

#define MAX_TASK_NUM 256

struct timer_task {
  void (*callback)(void*);
  void* data;
  uint64_t expire_time;
  int valid;
};


static struct timer_task tasks[MAX_TASK_NUM];

static int first_free() {
  for (int i = 0; i < MAX_TASK_NUM; i++) {
    if (tasks[i].valid == 0) return i;
  }
  return -1;
}

void add_timer(void (*callback)(void*), void *data, unsigned int after) {

  uint64_t cur_time = get_current_time();
  uint64_t expire_time = cur_time + after;
  uint64_t min_exp = expire_time + 1;

  for (int i = 0; i < MAX_TASK_NUM; i++) {
    if (tasks[i].valid && tasks[i].expire_time < min_exp)
      min_exp = tasks[i].expire_time;
  }
  
  int idx = first_free();
  if (idx < 0) {
    print("Cannot set more timer\n");
    return;
  }
  
  tasks[idx].callback = callback;
  tasks[idx].data = data;
  tasks[idx].valid = 1;
  tasks[idx].expire_time = expire_time;

  if (min_exp > expire_time) {
    asm volatile("msr cntp_cval_el0, %0"
                 :
                 : "r" (expire_time));
  }
}

unsigned long get_current_time() {
  volatile unsigned long cur;
  asm volatile("mrs %0, cntpct_el0"
               : "=r" (cur));
  return cur;
}

unsigned long get_cpu_freq() {
  volatile unsigned long frq;
  asm volatile("mrs %0, cntfrq_el0"
               : "=r" (frq));
  return frq;
}

void handle_timer_interrupt() {
  uint64_t cur = get_current_time();
  uint64_t min_expire = (uint64_t)-1;
  for (int i = 0; i < MAX_TASK_NUM; i++) {
    if (tasks[i].valid) {
      if (tasks[i].expire_time < cur) {
        tasks[i].callback(tasks[i].data);
        tasks[i].valid = 0;
      } else if (tasks[i].expire_time < min_expire) {
        min_expire = tasks[i].expire_time;
      }
    }
  }
  asm volatile("msr cntp_cval_el0, %0"
               :
               : "r" (min_expire));
  return;
}

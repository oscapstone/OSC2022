#include "stdint.h"

enum state{RUNNING, EXIT, ZOMBIE};

typedef struct task{
  uint64_t x19;
  uint64_t x20;
  uint64_t x21;
  uint64_t x22;
  uint64_t x23;
  uint64_t x24;
  uint64_t x25;
  uint64_t x26;
  uint64_t x27;
  uint64_t x28;
  uint64_t fp;  // x29
  uint64_t lr;  // x30
  uint64_t sp;
  uint64_t sp_addr;
  uint32_t pid;
  enum state state;
  struct task *next;
} task;

typedef void (*tread_func)(void);

extern void *get_current();
extern void switch_to(task *pre, task *next);
extern void write_current(uint64_t x0);

void thread_init(void);
void idle_thread(void);
void task_create(tread_func func);
void schedule();
void kill_zombies();

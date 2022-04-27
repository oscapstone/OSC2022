#pragma once
#include <stdint.h>
#include <stddef.h>
#include "utils.h"

#define STACK_SIZE 4096
#define USER_PROGRAM_BASE 0x30000000
#define USER_PROGRAM_SIZE (1 * mb)

#define THREAD_DEAD 1
#define THREAD_FORK 2
#define THREAD_READY 4

typedef struct {
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
  uint64_t fp;
  uint64_t lr;
  uint64_t sp;
} cpu_context;

//typedef enum {
//  ALIVE,
//  ACTIVE,
//  DEAD,
//  WAIT,
//} thread_status;

typedef struct thread_info {
  cpu_context context;
  uint32_t pid;
  uint32_t child_pid;
  int status;
  uint64_t trap_frame_addr;
  uint64_t kernel_stack_base;
  uint64_t user_stack_base;
  uint64_t user_program_base;
  uint32_t user_program_size;
  struct thread_info *next;
} thread_info;

typedef struct {
  thread_info *head, *tail;
} thread_queue;

thread_queue run_queue;
uint32_t thread_cnt;

extern thread_info *get_current();
extern void switch_to(uint64_t, uint64_t);

typedef struct {
  uint64_t x[31];
} trap_frame_t;

void foo();
void thread_test();
void thread_init();
thread_info *thread_create(void (*func)());
void run_queue_push(thread_info *thread);
void schedule();
void idle();
void exit();
void kill_zombies();
thread_info *current_thread();
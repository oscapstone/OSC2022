#ifndef __SCHEDULE__H__
#define __SCHEDULE__H__

#include "uart.h"
#include "buddy.h"
#include "allocator.h"
#include "stddef.h"
#include "cpio.h"
#include "timer.h"

#define THREAD_SIZE	0x1000
#define USER_PROGRAM_BASE 0x5000000
#define USER_PROGRAM_ADDR 0x60000
#define USER_SP 0x60000

typedef struct
{
	// context to kernel-stack (x19 to x28, fp(x29), lr(x30), sp)
	// only use x19 to x28: Callee-saved register
	unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long fp; // x29
	unsigned long lr; // x30
	unsigned long sp; // sp contain x0-x18
} cpu_context;

typedef enum 
{
    THREAD_RUNNING,
    THREAD_DEAD,
	THREAD_FORK,
} thread_state;

typedef struct _thread
{
	cpu_context context;
	unsigned int pid;
	thread_state thread_status;
	struct _thread *next;
	
	char *program_addr; // not necessary
	unsigned long program_size;
	unsigned long childID;
} thread;

/* defined in kernel/boot.S */
void switch_to(thread* prev, thread* next);
thread* get_current();
void set_current(thread* current);

/* defined in schedule.c */
void thread_init();
void thread_test();
void foo();
thread* thread_create(void* func);
void add_to_run_queue(thread* new_thread);
void schedule();
void kill_zombies();
void idle();
void delay(int count);
void exit();

// int exec(char *name, char* argv[]);
// unsigned long get_user_addr();
// void load_user_program_with_args(const char* name, char *const argv[], unsigned long addr, thread* current_thread);
// int get_pid();
// int fork();
// void switch_to_el0();

#endif
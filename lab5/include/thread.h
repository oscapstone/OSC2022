#include "uint.h"
#pragma once
int Thread(void *func(void), ...);
void idle();
void init_thread();
void set_first_thread();
void clear_threads();
void handle_child(tid_t tid);
void push_first_thread();
void printf_thread();
void record_mem(void* addr);
int getpid();
int set_fork(void *t,void* sp);
void kill(pid_t pid);
void move_last_mem(tid_t tid);

#define thread_numbers 65536

struct thread_sibling{
    struct thread *self;
    struct thread_sibling *next;
};

struct thread{
    unsigned long long registers[2*7];
    struct thread* next;
    void* malloc_table[256];
    int priority;
    tid_t tid, ptid;
    enum STATUS{
        starting,
        running,
        stop,
        dead
    } status;
    struct thread_sibling *childs;
    unsigned char stack[0x10000];
};


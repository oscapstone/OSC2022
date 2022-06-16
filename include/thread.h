#ifndef THREAD_H 
#define THREAD_H

#include "type.h"
#include "list.h"
#include "signal.h"
#include "exception.h"
#include "mmu.h"
#include "vfs.h"

#define PIDMAX 16
#define SIGMAX 16
#define USER_SIG_WRAPPER_VIRT_ADDR_ALIGNED 0xffffffff9000L

typedef struct signal_pair {
    uint64 spsr_el1;
    int pid;
    int code; 
} signal_pair_t;

extern void switch_to(void *curr_context, void *next_context);
extern void store_context(void *context);
extern void load_context(void *context);
extern void *get_current();
extern void switch_pgd(uint64 pgd);
extern uint64 *get_pgd();

typedef struct threadContext {
    uint64 x19;
    uint64 x20;
    uint64 x21;
    uint64 x22;
    uint64 x23;
    uint64 x24;
    uint64 x25;
    uint64 x26;
    uint64 x27;
    uint64 x28;
    uint64 fp;
    uint64 lr;
    uint64 sp;
    uint64 *pgd; // for mmu
} threadContext_t;

typedef struct vm_cell {
    list_head_t listHead;
    uint64 vir_addr;
    uint64 phy_addr;
    uint64 size;
} vm_cell_t;


typedef struct thread {
    list_head_t listHead;
    threadContext_t context;
    char *data;
    uint64 datasize;
    int pid;
    state_t state;
    char *stackPtr;
    char *kernel_stackPtr;
    void (*signal_handlers[SIGMAX + 1])();
    signal_pair_t signal_pair;
    bool has_signal;
    threadContext_t signal_context;
    list_head_t used_vm;
    char pwd[MAX_PATHNAME];
    file_t *file_descriptor_table[MAX_FD];
} thread_t;


extern thread_t *currThread;
extern thread_t threads[PIDMAX + 1];
extern list_head_t *run_queue, *wait_queue;

void initThreads();
thread_t *createThread(void *program, uint64 datasize);
void idle();
void kill_zombies();
void free_file_descriptor_table_for_thread(thread_t *thread);
void schedule();
void execThread(char *pathname);
void testThread();
void thread_exit();
void set_schedule_timer();
void schedule_callback(char *message);
#endif

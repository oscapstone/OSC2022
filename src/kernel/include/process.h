#ifndef _DEF_PROCESS
#define _DEF_PROCESS
#include <stdint.h>
#include <syscall.h>
#include <signal.h>

typedef struct ProcMem_{
    void *addr;
    uint64_t size; //page
    struct ProcMem_* next;
} ProcMem;

typedef enum ProcessStatus_{
    ProcessStatus_init = 0,
    ProcessStatus_running = 1,
    ProcessStatus_wait = 2,
    ProcessStatus_exit = 3,
    ProcessStatus_zombie = 4,
} ProcessStatus;

typedef struct Process_{
    uint64_t pid; // equal to tid
    uint64_t tid; // map to a kernel thread
    uint64_t signal_tid; // signal handler thread
    ProcessStatus status;
    char *filename;
    struct Process_ *next;
    int exit_code;
    void *entry;
    void *sp;
    void *exec;
    void *stack;
    void *sigstack;
    uint8_t signal[SIGNAL_NUM+1];
    uint8_t signal_handling;
    SignalHandler *signal_handlers;
    ProcMem *process_memory;
} Process;

void process_init();
int32_t process_exec(const char* name);
int process_getpid();
void process_free(Process *process);
void process_exit(int status);
void process_kill(int pid);
int process_fork(TrapFrame *trapframe);
Process *get_process(uint64_t pid);

#endif
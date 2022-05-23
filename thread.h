#ifndef THREAD_h
#define THREAD_h

#include "exception.h"
#define NULLPTR ((void*)0xFFFF000000000000)
#define THREAD_SIZE 0x1000
#define USER_STACK_SIZE 0x1000
#define KERNEL_STACK_SIZE 0x1000
#define size_t unsigned long

extern void switch_to(void *curr_context, void *next_context);
extern void store_context(void *curr_context);
extern void load_context(void *curr_context);
extern unsigned long store_pgd();
extern void load_pgd(unsigned long next_pgd);
extern void *get_current();
extern int get_el();

typedef struct thread_context
{
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
    unsigned long fp;
    unsigned long lr;
    unsigned long sp;
    unsigned long spsr_el1;
} thread_context;

typedef struct Thread{
	thread_context context;
	int pid, parent, iszombie;
    char *user_stack, *kernel_stack;
    char *program;
	unsigned int program_size;
    unsigned long pgd;
	struct Thread* next;
} Thread;

typedef struct Thread_List{
	Thread *beg;
	Thread *end;
	int pid_cnt;
} Thread_List;

void schedule();
void init_schedule();
int getpid(Trap_Frame* tpf);
size_t uartread(Trap_Frame *tpf,char buf[], size_t size);
size_t uartwrite(Trap_Frame *tpf,const char buf[], size_t size);
int exec(Trap_Frame *tpf,const char* name, char *const argv[]);
int fork(Trap_Frame *tpf);
void exit(Trap_Frame *tpf,int status);
void exec_thread(char *data, unsigned int filesize);
void jump_thread(char *data, unsigned int filesize);
int syscall_mbox_call(Trap_Frame *tpf, unsigned char ch, unsigned int *mbox);
void kill(Trap_Frame *tpf, int pid);
void fork_test();
void run();
void foo();
#endif
#ifndef THREAD_h
#define THREAD_h

#include "exception.h"
#include "vfs.h"

#define THREAD_SIZE 0x1000
#define USER_STACK_SIZE 0x1000
#define KERNEL_STACK_SIZE 0x1000
#define FILE_DESCRIPTOR_LEN 16
#define size_t unsigned long

extern struct mount root_fs;

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
    unsigned long demand[16];
    vnode *curr_dir;
    struct file *fd_table[FILE_DESCRIPTOR_LEN]; 
	struct Thread* next;
} Thread;

typedef struct Thread_List{
	Thread *beg;
	Thread *end;
	int pid_cnt;
} Thread_List;

Thread_List thread_list;

void schedule();
void init_schedule();
int alloc_fd(Thread *user_thread);
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
int open(Trap_Frame *tpf, const char *pathname, int flags);
int close(Trap_Frame *tpf, int fd);
long write(Trap_Frame *tpf, int fd, const void *buf, unsigned long count);
long read(Trap_Frame *tpf, int fd, void *buf, unsigned long count);
int mkdir(Trap_Frame *tpf, const char *pathname, unsigned mode);
int mount(Trap_Frame *tpf, const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data);
int chdir(Trap_Frame *tpf, const char *path);
void fork_test();
void run();
void foo();
#endif
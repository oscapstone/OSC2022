#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include "types.h"
#include "lib/list.h"

#define SIG_SIGHUP       1
#define SIG_SIGINT       2  
#define SIG_SIGQUIT      3
#define SIG_SIGILL       4 
#define SIG_SIGTRAP      5
#define SIG_SIGABRT      6
#define SIG_SIGBUS       7
#define SIG_SIGFPE       8
#define SIG_SIGKILL      9
#define SIG_SIGUSR1      10
#define SIG_SIGSEGV      11
#define SIG_SIGUSR2      12
#define SIG_SIGPIPE      13
#define SIG_SIGALRM      14
#define SIG_SIGTERM      15
#define SIG_SIGSTKFLT    16
#define SIG_SIGCHLD      17
#define SIG_SIGCONT      18
#define SIG_SIGSTOP      19
#define SIG_SIGTSTP      20
#define SIG_SIGTTIN      21
#define SIG_SIGTTOU      22
#define SIG_SIGURG       23
#define SIG_SIGXCPU      24
#define SIG_SIGXFSZ      25
#define SIG_SIGVTALRM    26
#define SIG_SIGPROF      27
#define SIG_SIGWINCH     28
#define SIG_SIGIO        29
#define SIG_SIGPWR       30
#define SIG_SIGSYS       31

#define SIG_FLAG_KERNEL  1

#define SIG_MAX_NUM 32

typedef void (*sig_handler)();

struct signal_queue{
    struct list_head list;
    int32_t signal;
};

struct sigpending{
    struct list_head pending;
    int32_t cur_signal;
};

struct sigaction{
    uint32_t sa_flags;
    sig_handler sa_handler;
};

struct sighand_struct{
    struct sigaction actions[SIG_MAX_NUM];
};

extern void sigpending_init(struct sigpending*);
extern void default_sighand_init(struct sighand_struct*);
extern int send_signal(uint64_t, int);
extern int register_signal(int, sig_handler);
extern void handle_sigreturn();
extern void handle_signal();
extern void sigreturn_frame_save(sig_handler);
extern void sigreturn_frame_restore();
extern void sys_sigreturn();
extern void sys_signal(int, sig_handler);
extern int sys_sigkill(uint64_t, int);
extern void sig_default();
extern void sig_terminate();
extern void free_sigpendings(struct sigpending*);


#endif

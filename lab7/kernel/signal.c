#include "kernel/sched/task.h"
#include "kernel/signal.h"
#include "kernel/signal_inner.h"

extern void sigreturn_trampoline();

int is_valid_signal(int signal){
    return (signal > 0 && signal < SIG_MAX_NUM);
}
void sigpending_init(struct sigpending* sigpending){
    INIT_LIST_HEAD(&sigpending->pending);
    sigpending->cur_signal = 0;
}

void default_sighand_init(struct sighand_struct* sighandler){
    memcpy(sighandler->actions, default_actions, sizeof(default_actions));
} 

int send_signal(uint64_t pid, int signal){
    LOG("enter send_signal, pid: %l signal %d", pid, signal);
    struct task_struct* target = find_task_by_pid(pid);
    struct signal_queue* sigq;
    volatile uint64_t daif;
    
    daif = local_irq_disable_save();
    if(!is_valid_signal(signal) || target == NULL) goto end;
    if(target->thread_info.state == TASK_DEAD) goto end;

    LOG("target %p", target);
    sigq = kmalloc(sizeof(struct signal_queue));  

    
    sigq->signal = signal;
    list_add_tail(&sigq->list, &target->sigpending.pending);
    
end:
    local_irq_restore(daif);
    LOG("end send_signal");
    return 0;
}

int register_signal(int signal, sig_handler handler){
    LOG("enter register_signal");
    uint64_t daif;
    int ret;
    struct task_struct* current = get_current();
    
    if(current == NULL) return -1;

    LOG("signal: %d, handler: %p", signal, handler);
    daif = local_irq_disable_save();
    // install signal handler from user space 
    if(is_valid_signal(signal)){
        current->sighandler.actions[signal].sa_flags &= ~(SIG_FLAG_KERNEL);
        current->sighandler.actions[signal].sa_handler = handler;

        local_irq_restore(daif);
        return 0;
    }    

    local_irq_restore(daif);
    return -1;
}

void handle_sigreturn(){
    sigreturn_frame_restore();
}

int get_pending_signal(struct task_struct *task){
    struct signal_queue* sigq;
    uint64_t signal = 0;
    if(!list_empty(&task->sigpending.pending)){
        sigq = list_first_entry(&task->sigpending.pending, struct signal_queue, list);
        signal = sigq->signal;
        list_del(&sigq->list);
        kfree(sigq);
        LOG("get pending signal %d", signal);
    }
    return signal;
}

void handle_signal(){
    int cur_signal;
    volatile uint64_t daif;
    struct sigaction* action;
    struct task_struct* current = get_current();
    struct trap_frame* trap_frame = get_trap_frame(current);
    if(current == NULL) return;

    daif = local_irq_disable_save(); 
    
    cur_signal = get_pending_signal(current);
    if(!is_valid_signal(cur_signal)) return;
    LOG("handle signal %d", cur_signal);

    action = &current->sighandler.actions[cur_signal];
    current->sigpending.cur_signal = cur_signal; 
    
    if(action->sa_flags & SIG_FLAG_KERNEL){
        // if signal handler is in kernel space, just directly execute it
        LOG("execute kernel signal handler");
        action->sa_handler();
    }else{
        // if signal handler is in user space, execute it in user space by modify trap frame
        LOG("execute user signal handler");
        sigreturn_frame_save(action->sa_handler);
    }

    local_irq_restore(daif); 
}

void sigreturn_frame_save(sig_handler handler){
    LOG("enter sigreturn_frame_save");
    struct task_struct* current = get_current();
    struct trap_frame* trap_frame = get_trap_frame(current);
    uint64_t old_trap_frame;
    volatile uint64_t daif;

    daif = local_irq_disable_save(); 
    // store a copy of current trap frame onto user stack
    old_trap_frame = trap_frame->sp_el0 - sizeof(struct trap_frame);
    memcpy((void*)old_trap_frame, trap_frame, sizeof(struct trap_frame));

    // change sp to point to this copy 
    trap_frame->sp_el0 = old_trap_frame;

    // set elr_el1 to handler
    // set lr to jump to our sigreturn_trampoline 
    // so that when task return to user from kernel mode, it will back to sigreturn_trampoline
    trap_frame->x30 = (uint64_t)sigreturn_trampoline; // in entry.S
    trap_frame->elr_el1 = (uint64_t)handler; // in entry.S
    
    local_irq_restore(daif);
    LOG("end sigreturn_frame_save");
}

void sigreturn_frame_restore(){
    LOG("enter sigreturn_frame_restore");
    struct task_struct* current = get_current();
    struct trap_frame* trap_frame = get_trap_frame(current);
    struct trap_frame* old_trap_frame;
    volatile uint64_t daif;
    
    daif = local_irq_disable_save(); 
    // restore old trap frame
    old_trap_frame = (struct trap_frame*)trap_frame->sp_el0;
    memcpy(trap_frame, old_trap_frame, sizeof(struct trap_frame));
    
    local_irq_restore(daif);
    LOG("end sigreturn_frame_restore");
}

void sig_terminate(){
    struct task_struct *current = get_current();
    uint64_t pid;
    volatile uint64_t daif;
    LOG("enter signal_terminate");

    daif = local_irq_disable_save();
    
    pid = current->thread_info.pid;
    printf("pid: %l, Receive signal: %s\r\n",pid ,sig_to_str[current->sigpending.cur_signal]);
    local_irq_restore(daif);

    task_exit();
}

void sig_default(){
    struct task_struct *current = get_current();
    uint64_t pid;
    volatile uint64_t daif;
    LOG("enter signal_default");

    daif = local_irq_disable_save();

    pid = current->thread_info.pid;
    printf("pid: %l, Receive signal: %s\r\n",pid , sig_to_str[current->sigpending.cur_signal]);
    local_irq_restore(daif);

    LOG("end signal_default");
}

void free_sigpendings(struct sigpending* sigp){
    struct signal_queue* sigq;
    while(!list_empty(&sigp->pending)){
        sigq = list_first_entry(&sigp->pending, struct signal_queue, list);
        list_del(&sigq->list); 
        kfree(sigq);
    }
}

void sys_sigreturn(){
    handle_sigreturn();
}

void sys_signal(int signal, sig_handler handler){
    register_signal(signal, handler);
}

int sys_sigkill(uint64_t pid, int signal){
    return send_signal(pid, signal);
}

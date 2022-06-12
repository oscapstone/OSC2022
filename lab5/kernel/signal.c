#include "kernel/signal.h"

int send_signal(uint64_t pid, int signal){

}

int register_signal(int signal, sig_handler handler){

}

void handle_sigreturn(){

}

int check_sigpending(){

}

void handle_signal(){

}

void* sigreturn_stack_create(struct trap_frame* trap_frame){

}

void sigreturn_stack_destroy(struct trap_frame* trap_frame){

}

void sys_sigreturn(){
    handle_sigreturn();
}

void sys_signal(int signal, sig_handler handler){
    register_signal(signal, handler);
}

int sys_sigkill(uint64_t pid, int signal){
    send_signal(pid, signal);
}

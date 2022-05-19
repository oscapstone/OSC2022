#include <signal.h>
#include <sched.h>
#include <syscall.h>


void sig_default_handler(){
    TrapFrame fake_tf;
    fake_tf.x[0] = get_current()->id;
    sys_kill(&fake_tf);
}
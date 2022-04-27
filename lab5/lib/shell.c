#include "shell.h"
#include "mini_uart.h"
#include "utils.h"
#include "mailbox.h"
#include "reboot.h"
#include "string.h"
#include "cpio.h"
#include "memory.h"
#include "timer.h"
#include "exception.h"
#include "math.h"
#include "mm.h"
#include "../include/sched.h"
#include "fork.h"


#define MAX_BUFFER_SIZE 256u

static char buffer[MAX_BUFFER_SIZE];

void foo() {
    for(int i = 0; i < 10; ++i) {
        printf("Thread id: %d %d\n", current->id, i);
        delay(1000000);
        schedule();
    }
    current->state = TASK_ZOMBIE;
    while(1);
}

void user_foo() {
    for(int i = 0; i < 10; ++i) {
        printf("User thread id: %d %d\n", current->id, i);
        delay(1000000);
    }
    current->state = TASK_ZOMBIE;
    while(1);
}

void kernel_process(){
	printf("Kernel process started.\n");
	int err = move_to_user_mode((unsigned long)&user_foo);
	if (err < 0){
		printf("Error while moving process to user mode\n\r");
	} 
}

void read_cmd()
{
    unsigned int idx = 0;
    char c = '\0';
    
    while (1) {
        c = uart_recv();
        if (c == '\r' || c == '\n') {
            uart_send_string("\n");
            
            if (idx < MAX_BUFFER_SIZE) buffer[idx] = '\0';
            else buffer[MAX_BUFFER_SIZE-1] = '\0';
            
            break;
        } else {
            uart_send(c);
            buffer[idx++] = c;
        } 
    }

}

void parse_cmd()
{

    if (stringcmp(buffer, "\0") == 0) 
        uart_send_string("\n");
    else if (stringcmp(buffer, "hello") == 0)
        uart_send_string("Hello World!\n");
    else if (stringcmp(buffer, "reboot") == 0) {
        uart_send_string("rebooting...\n");
        reset(100);
    }
    else if (stringcmp(buffer, "hwinfo") == 0) {
        get_board_revision();
        get_arm_memory();
    }
    else if (stringcmp(buffer, "ls") == 0) {
        cpio_ls();
    }
    else if (stringcmp(buffer, "cat") == 0) {
        cpio_cat();
    }
    else if (stringcmp(buffer, "execute") == 0) {
        cpio_exec();
    }
    else if (stringcmp(buffer, "thread_test") == 0) {
        for (int i=0; i<10; i++) {
            copy_process(PF_KTHREAD, (unsigned long)&foo, 0, 0);
        }
    }
    else if (stringcmp(buffer, "to_user") == 0) {
        copy_process(PF_KTHREAD, (unsigned long)&kernel_process, 0, 0);
    }
    else if (stringcmp(buffer, "help") == 0) {
        uart_send_string("help:\t\tprint list of available commands\n");
        uart_send_string("hello:\t\tprint Hello World!\n");
        uart_send_string("reboot:\t\treboot device\n");
        uart_send_string("hwinfo:\t\tprint hardware information\n");
        uart_send_string("ls:\t\tlist initramfs files\n");
        uart_send_string("cat:\t\tprint file content in initramfs\n");
        uart_send_string("execute:\trun program from cpio\n");
    }
    else 
        uart_send_string("Command not found! Type help for commands.\n");

}

void shell_loop() 
{
    while (1) {
        // kill zombies
        // schedule
        uart_send_string("% ");
        read_cmd();
        parse_cmd();
    }
}
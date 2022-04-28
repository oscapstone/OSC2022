#include "mini_uart.h"
#include "shell.h"
#include "mailbox.h"
#include "utils.h"
#include "sched.h"

void kernel_main(void){

	system_init();
	delay(10000000);
   	uart_send_string("# Hello I'm raspi3 B+ model\r\n");
    
   	mailbox_board_revision();
  	mailbox_vc_memory();
	int el = get_el();
	uart_printf("Exception level %d \n",el);   
	
	scheduler_init();
	
	/*while(1){
		shell_standby();
	}*/
	
	char *str = "syscall.img";
	void * user_program = get_usr_program_address(str);
	int filesize = get_usr_program_size(str);
	exec_thread(user_program,filesize);
	
	uart_printf("thread create done\n");
	uart_printf("run queue list size: %d\n", run_queue_list_size());
	uart_printf("zombie queue list size: %d\n", zombie_queue_list_size());
	idle();
}

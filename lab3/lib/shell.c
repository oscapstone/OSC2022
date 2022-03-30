#include "uart.h"
#include "string.h"
#include "reboot.h"
#include "mailbox.h"
#include "mmio.h"
#include "printf.h"
#include "cpio.h"
#include "timer.h"
#include "shell.h"
#include "malloc.h"

void shell(){
  char *input = simple_malloc(sizeof(char) * BUF_MAX_SIZE);
  char **args = simple_malloc(sizeof(char*) * BUF_ARG_SIZE);
  int args_num = 0;
  char read = 0;
  while(1){
    read = async_uart_getc();
    if(read != '\n' && read != 0x7f){
      append_str(input, read);
      printf("%c", read);
      read = 0;
    }else if(read == 0x7f){
      if(strlen(input) > 0){
        pop_str(input);
        printf("\b \b");
      }
    }else{
      args_num = spilt_strings(args, input, " ");
      if(args_num != 0){
        printf("%c", read);
        if(!strcmp(args[0], "help")){
          printf("help      : print this help menu\n\r");
          printf("hello     : print Hello World!\n\r");
          printf("reboot    : reboot the device\n\r");
          printf("sysinfo   : print the system information\n\r");
          printf("ls        : list the file\n\r");
          printf("cat <file>: print the content of the file\n\r");
        }else if(!strcmp(args[0], "hello")){
          printf("Hello World!\n\r");
        }else if(!strcmp(args[0], "reboot")){
          printf("Bye bye~\n\r");
          reset(300);
        }else if(!strcmp(args[0], "sysinfo")){
          if(mailbox_property(MAILBOX_TAG_BOARD_VISION, 4, MBOX_CH_PROP)){
            printf("Board version: 0x%08x\n\r", mbox[5]);
          }else{
            printf("Board version: querry error.\n\r");
          }
          if(mailbox_property(MAILBOX_TAG_MEMORY, 8, MBOX_CH_PROP)){
            printf("ARM memory base address: 0x%08x\n\r", mbox[5]);
            printf("ARM memory size: 0x%08x\n\r", mbox[6]);
          }else{
            printf("ARM memory base address: querry error.\n\r");
            printf("ARM memory size: querry error.\n\r");
          }
        }else if(!strcmp(args[0], "ls")){
          cpio_ls();
        }else if(!strcmp(args[0], "cat")){
          if(args_num == 2)
            cpio_cat(args[1]);
        }else if(!strcmp(args[0], "exec")){
          if(args_num == 2)
            cpio_exec(args[1]);
        }else if(!strcmp(input, "clock")){
          core_timer_enable();
          // asm(
          //   "bl from_el1_to_el0\n\t"
          // );
        }else if(!strcmp(input, "setTimeout")){
          if (args_num == 3)
            add_timer(uart_puts, atoi(args[1]), args[2]);
        }else{
          printf("Please use \"help\" to get information.\n\r");
        }
        printf("raspberryPi: ");
        input[0] = 0;
      }else{
        printf("\n\rraspberryPi: ");
      }  
    }
  }
}
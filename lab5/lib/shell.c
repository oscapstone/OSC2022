#include "uart.h"
#include "string.h"
#include "reboot.h"
#include "printf.h"
#include "cpio.h"
#include "timer.h"
#include "shell.h"
#include "malloc.h"
#include "scheduler.h"
#include "system.h"
#include "delay.h"

void shell(){
  char *input = malloc(sizeof(char) * BUF_MAX_SIZE);
  char **args = malloc(sizeof(char*) * BUF_ARG_SIZE);
  *input = 0;
  int args_num = 0;
  char read = 0;
  printf("\n\r\n\rWelcome!!!\n\r");
  printf("raspberryPi: ");
  printf("\n\r\n\r");
  while(1){
    read = uart_getc(); // async_uart_getc()
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
        printf("\n\r");
        if(!strcmp(args[0], "help")){
          printf("reboot    : reboot the device\n\r");
          printf("exec <file>: exec the content of the file\n\r");
        }else if(!strcmp(args[0], "reboot")){
          printf("Bye bye~\n\r");
          reset(300);
        }else if(!strcmp(args[0], "ls")){
          cpio_ls();
        }else if(!strcmp(args[0], "cat")){
          if(args_num == 2)
            cpio_cat(args[1]);
        }else if(!strcmp(args[0], "exec")){
          if(args_num == 2)
            cpio_exec(args[1]);
        }else if(!strcmp(args[0], "d")){
          show_page_list();
        }else if(!strcmp(args[0], "m")){
          if (args_num == 2)
            printf("alloc the memory form: 0x%x\n\r", malloc(atoi(args[1])));
        }else if(!strcmp(args[0], "e")){
          char *addr = load_program(args[1]);
          task_create((thread_func)addr, USER);
        }
        printf("raspberryPi: ");
        input[0] = 0;
      }else{
        printf("\n\rraspberryPi: ");
      }
    }
  }
}


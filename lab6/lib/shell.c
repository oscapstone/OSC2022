#include "shell.h"
#include "uart.h"
#include "string.h"
#include "reboot.h"
#include "printf.h"
#include "cpio.h"
#include "malloc.h"
#include "scheduler.h"
#include "timer.h"
#include "mailbox.h"
#include "mmu.h"


void test(){
  printf("test\n\r");
}

void shell(){
  char *input = malloc(sizeof(char) * BUF_MAX_SIZE);
  char **args = malloc(sizeof(char*) * BUF_ARG_SIZE);
  *input = 0;
  int args_num = 0;
  char read = 0;
  printf("pi#  ");
  // task *cur = task_create((thread_func)USER_PROGRAM_ADDR, USER);
  // load_program("syscall.img", cur->page_table);
  // idle_thread();
  while(1){
    read = uart_getc();
    if(read != '\n' && read != 0x7f){
      append_str(input, read);
      printf("%c", read);
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
        }else if(!strcmp(args[0], "reboot")){
          printf("Bye bye~\n\r");
          reset(300);
        }else if(!strcmp(args[0], "ls")){
          cpio_ls();
        }else if(!strcmp(args[0], "run")){
          task *cur = task_create((thread_func)USER_PROGRAM_ADDR, USER);
          // load_program(args[1], cur->page_table);
          load_program("syscall.img", cur->page_table);
          // printf_pt(cur->page_table);
          // uint64_t *dup;
          // init_PT(&dup);
          // printf("-----------\n\r");
          // duplicate_PT(cur->page_table, dup);
          // printf_pt(dup);
          idle_thread();
        }else if(!strcmp(input, "T")){
          add_timer(test, "Test", atoi(args[1])*get_timer_freq());
        }
        printf("pi#  ");
        input[0] = 0;
      }
    }
  }
}


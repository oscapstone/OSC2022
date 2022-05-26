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

// void printf_pt(uint64_t *page_table){
//   for(int i=0; i<512; i++){
//     page_table = PA2VA(page_table);
//     if((page_table[i]) != 0){
//       printf("%d: 0x%llx\n\r", i, page_table[i]);
//       page_table = page_table[i] - 3;
//       for(int j=0; j<512; j++){
//         if(page_table[j] != 0){
//           printf("  %d: 0x%llx\n\r", j, page_table[j]);
//           page_table = page_table[j] - 3;
//           for(int k=0; k<512; k++){
//             if(page_table[k] != 0){
//               printf("    %d: 0x%llx\n\r", k, page_table[k]);
//               page_table = page_table[k] - 3;
//               for(int l=0; l<512; l++){
//                 if(page_table[l] != 0){
//                   printf("      %d: 0x%llx\n\r", l, page_table[l]);
//                 }
//               }
//             }
//           }
//         }
//       }
//     }
//   }
// }


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
          load_program("syscall.img", cur->page_table);
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


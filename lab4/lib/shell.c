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
    // read = uart_getc();
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
        printf("\n\r");
        if(!strcmp(args[0], "help")){
          printf("help      : print this help menu\n\r");
          printf("reboot    : reboot the device\n\r");
          printf("sysinfo   : print the system information\n\r");
          printf("ls        : list the file\n\r");
          printf("cat <file>: print the content of the file\n\r");
          printf("exec <file>: exec the content of the file\n\r");
          printf("clock      : print the core timer time every 2 seconds\n\r");
          printf("setTimeout <MESSAGE> <TIME>: Print the message when timeout\n\r");
          printf("dump_page  : dump the free frame list\n\r");
          printf("allocp <size>  : alloc the page\n\r");
          printf("freep <size>   : free the page\n\r");
          printf("malloc <size>  : alloc the memory\n\r");
          printf("free <size>    : free the memory\n\r");
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
          add_timer(clock_alert, args[1], 2);
        }else if(!strcmp(input, "setTimeout")){
          if (args_num == 3)
            add_timer(timeout_print, args[1], atoi(args[2]));
        }else if(!strcmp(args[0], "dump_page")){
          print_free_frame_list();
        }else if(!strcmp(args[0], "allocp")){
          if (args_num == 2)
            printf("allocte page index: %d\n\r", page_allocate(atoi(args[1]) * 0x1000));
        }else if(!strcmp(args[0], "freep")){
          if (args_num == 2)
            page_free(atoi(args[1]));
        }else if(!strcmp(args[0], "malloc")){
          if (args_num == 2)
            printf("alloc the memory form: 0x%x\n\r", malloc(atoi(args[1])));
        }else if(!strcmp(args[0], "free")){
          if (args_num == 2){
            char *addr = (char *)((unsigned long)myHex2Int(args[1]));
            free(addr);
          }
        }
        printf("raspberryPi: ");
        input[0] = 0;
      }else{
        printf("\n\rraspberryPi: ");
      }  
    }
  }
}
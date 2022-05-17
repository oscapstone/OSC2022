#include "uart.h"
#include "string.h"
#include "power.h"
#include "mbox.h"
#include "cpio.h"
#include "timer.h"
#include "exception.h"
#include "alloc.h"
#include "utils.h"
#include "thread.h"
#include "printf.h"

void clean_buffer(char * buffer, int buffer_len);
void command_help();
void command_hello();
void command_not_found(char * buffer);
void command_ls();
void command_cat(char* pathname);
void command_mailbox();
void command_test();
void command_load_user_program(const char *program_name);
void command_set_timeout(char *args);
void command_buddy_test();
void command_dma_test();
void command_thread_test1() ;
void command_thread_test2() ;
void command_thread_test3() ;
void parse_command(char * buffer);
void run_shell();

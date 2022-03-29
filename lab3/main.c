
#include "uart.h"
#include "mbox.h"
#include "general.h"
#include "diy_sscanf.h"
#include "diy_string.h"
#include "cpio.h"
#include "diy_malloc.h"
#include "fdtb.h"
#include <stdint.h>

#define MACHINE_NAME "rpi3-baremetal-lab3$ "
#define CMD_HELP     "help"
#define CMD_HELLO    "hello"
#define CMD_REBOOT   "reboot"
#define CMD_LSHW     "lshw"
#define CMD_LS       "ls"
#define CMD_CAT      "cat"
#define CMD_LSDEV    "lsdev"
#define CMD_EL0      "el0"
#define CMD_EXEC     "exec"

#define ADDR_IMAGE_START 0x80000

static void show_hardware_info();
static int spilt_strings(char** str_arr, char* str, char* deli);
extern uint64_t _start;
extern void from_el1_to_el0(); // defined in start.S
extern void from_el1_to_el0_remote(uint64_t args, uint64_t addr, uint64_t u_sp); // defined in start.S
void main(void *dtb_addr)
{
  char *input_s;
  char **args;
  int args_cnt = 0;

  input_s = simple_malloc(sizeof(char) * 32);
  args = simple_malloc(sizeof(char*) * 10);

  // set up serial console
  uart_init();

  // say hello
  uart_printf("\r\n\r\n");
  uart_printf("Welcome------------------------ lab 3\r\n");

  uart_printf("_start=0x%p, dtb_addr=0x%p\r\n", &_start, dtb_addr);

  fdtb_parse(dtb_addr, 0, cpio_parse);

  while(1) {

    // Read cmd
    uart_printf(MACHINE_NAME);
    uart_gets_n(32, input_s, 1);
    args_cnt = spilt_strings(args, input_s, " ");

    // Execute cmd
    if(strlen_(args[0]) > 0){
      if     (strcmp_(args[0], CMD_HELP) == 0){
        uart_printf(CMD_HELP   "\t\t: print this help menu\r\n");
        uart_printf(CMD_HELLO  "\t\t: print Hello World!\r\n");
        uart_printf(CMD_REBOOT "\t\t: reboot the device\r\n");
        uart_printf(CMD_LSHW   "\t\t: print hardware info acquired from mailbox\r\n");
        uart_printf(CMD_LS     "\t\t: List files and dirs\r\n");
        uart_printf(CMD_CAT    "\t\t: Print file content\r\n");
        uart_printf(CMD_LSDEV  "\t\t: Print all the nodes and propperties parsed from dtb.\r\n");
        uart_printf(CMD_EL0    "\t\t: Switch from exception level el1 to el0 (user mode)\r\n");
        uart_printf(CMD_EXEC " <file> \t: Switch to el0, reallocate the file (img) and jumps to it.\r\n");
      }
      else if(strcmp_(args[0], CMD_HELLO) == 0){
        uart_printf("Hello World!\r\n");
      }
      else if(strcmp_(args[0], CMD_REBOOT) == 0){
        uart_printf("Rebooting...\r\n");
        reset(1000);
        while(1);
      }
      else if(strcmp_(args[0], CMD_LSHW) == 0){
        show_hardware_info();
      }
      else if(strcmp_(args[0], CMD_LS) == 0){
        cpio_ls();
      }
      else if(strcmp_(args[0], CMD_CAT) == 0){
        if(args_cnt > 1)
          cpio_cat(args[1]);
      }
      else if(strcmp_(args[0], CMD_LSDEV) == 0){
        fdtb_parse(dtb_addr, 1, NULL);
      }
      else if(strcmp_(args[0], CMD_EL0) == 0){
        from_el1_to_el0();
        uart_printf("Now in el0 user mode.\r\n");
      }
      else if(strcmp_(args[0], CMD_EXEC) == 0){
        if(args_cnt > 1){
          // Reallocate file to 0x50000
          if(cpio_copy(args[1], (uint8_t*) 0x50000) == 0)
            from_el1_to_el0_remote((uint64_t)dtb_addr, 0x50000, 0x50000); // switch to el0 and jumps to 0x50000
        }
        else
          uart_printf("Usage: " CMD_EXEC " <file>\r\n");
      }
      else
        uart_printf("Unknown cmd \"%s\".\r\n", input_s);
    }
  }
}

static int spilt_strings(char** str_arr, char* str, char* deli){
  int count = 0;
  // Spilt str by specified delimeter
  str_arr[0] = strtok_(str, deli);
  count = 0;
  while(str_arr[count] != NULL){
    count++;
    str_arr[count] = strtok_(NULL, deli);
  }
  return count;
}

static void show_hardware_info(){
  uint32_t board_rev=0;
  uint32_t *mem_start_addr = 0;
  uint32_t mem_size = 0;
  mbox_board_rev(&board_rev);
  mbox_arm_mem_info(&mem_start_addr, &mem_size);
  uart_printf("board_rev=0x%08X\r\n", board_rev);
  uart_printf("mem_start_addr=0x%p\r\n", mem_start_addr);
  uart_printf("mem_size=0x%08X\r\n", mem_size);
}

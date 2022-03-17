#include "system.h"
#include "dtb_parser.h"

struct cmd{
    char input[20];
    char description[30];
};


struct cmd cmd_list[SYS_CMD_NUM] = {
    {.input = "help", .description="List commands."},
    {.input = "hello", .description="Print hello world."},
    {.input = "reset", .description="Reset raspi3."},
    {.input = "loadimg", .description="Load new kernel."},
    {.input = "ls", .description="List rootfs."},
    {.input = "cat", .description="Show content of file."},
    {.input = "dtb_ls",. description="List device tree."},
    {.input = "dtb_cat",. description="Parse device tree."}
};

void system_command(char* buf){
    char *args = buf;
    int arg_idx = 0;
    for(; buf[arg_idx] != '\0'; ++arg_idx, ++args){
        if(buf[arg_idx] == ' '){
            buf[arg_idx] = '\0';
            args = buf + arg_idx + 1;
            break;
        }
    }
	
	
	if     (strcmp(buf, "help" ) == 0) 		help();
	else if(strcmp(buf, "hello") == 0)		hello();
	else if(strcmp(buf, "reset") == 0)		reset(100);
    else if(strcmp(buf, "loadimg") == 0)	loadimg();
    else if(strcmp(buf, "ls") == 0)   		cpio_list();
	else if(strcmp(buf, "cat") == 0)   		cpio_cat(args);
	else if(strcmp(buf, "dtb_ls") == 0)   	print_dt_info();
	else if(strcmp(buf, "dtb_cat") == 0)   	cat_dt();
	else if(strcmp(buf, "malloc") == 0)   	my_malloc();
	else if(strcmp(buf, "dtb_init") == 0)   dtb_init();
    else uart_puts("Unknown command\r\n");
}

void help(){
    uart_puts("\n\r###########################################\r\n");
    uart_puts("[Command]\t\t[Description]\r\n");
    for(int i = 0; i < SYS_CMD_NUM; ++i){
        uart_puts(cmd_list[i].input);
        uart_puts("\t\t\t");
        uart_puts(cmd_list[i].description);
        uart_puts("\r\n");
    }
	uart_puts("###########################################\r\n\n\r");
    
}

void hello(){
    uart_puts("Hello World !\r\n");
}

void my_malloc(){
    char *str_a = simple_malloc(8000);
    char *str_b = simple_malloc(8000);
    uart_put_int((unsigned long)str_a);
    uart_puts("\n");
    uart_put_int((unsigned long)str_b);
    uart_puts("\n");

    str_a[0] = 'a';
    str_a[1] = 'p';
    str_a[2] = '\n';
    str_a[2] = '\0';
    uart_puts(str_a);

    str_a = "apple\n\0";
    uart_puts(str_a);
    uart_put_int((unsigned long)str_a);
    uart_puts("\n");
}


void reset(int tick){ // reboot after watchdog timer expire
  uart_puts("Start Reset...\n");
  *PM_RSTC = PM_PASSWORD | 0x20; // full reset
  *PM_WDOG = PM_PASSWORD | tick; // number of watchdog tick
  while(1);
}

void cancel_reset() {
  uart_puts("Cancel Rebooting...\n");
  *PM_RSTC = PM_PASSWORD | 0; // full reset
  *PM_WDOG = PM_PASSWORD | 0; // number of watchdog tick
}



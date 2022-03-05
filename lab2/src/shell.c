#include "uart.h"
#include "string.h"
#include "shell.h"
#include "mbox.h"
#include "system.h"
#include "cpio.h"

void shell()
{
    char cmd[MAX_BUF_SIZE];
    print_system_messages();
    while(1)
    {
        uart_printf("# ");
        uart_gets(cmd);
        do_cmd(cmd);
    }
}

void do_cmd(char* cmd)
{
    if(strcmp(cmd,"help")==0)
    {
        uart_puts("help       : print this help menu");
        uart_puts("hello      : print Hello World!");
        uart_puts("reboot     : reboot the device");
        uart_puts("ls         : list current directory");
    }
    else if(strcmp(cmd,"hello")==0)
    {
        uart_puts("Hello World!");
    }
    else if(strcmp(cmd,"reboot")==0)
    {
        reboot();
    }else if(strcmp(cmd,"cat")==0)
    {
        uart_printf("Filename: ");
        char filepath[MAX_BUF_SIZE];
        uart_gets(filepath);
        cat(filepath);

    }else if(strcmp(cmd,"ls")==0)
    {
        ls(".");
    }else
    {
        uart_puts("Unknown command!");
    }
}

int cat(char* thefilepath)
{
    char* filepath;
    char* filedata;
    unsigned int filesize;
    struct cpio_newc_header *header_pointer = CPIO_DEFAULT_PLACE;

    while(header_pointer!=0)
    {
        int error = cpio_newc_parse_header(header_pointer,&filepath,&filesize,&filedata,&header_pointer);
        //if parse header error
        if(error)
        {
            uart_puts("error");
            break;
        }

        if(strcmp(thefilepath,filepath)==0)
        {
            uart_printf("%s",filedata);
            break;
        }

        //if this is TRAILER!!! (last of file)
        if(header_pointer==0)uart_printf("cat: %s: No such file or directory\n",thefilepath);
    }
    return 0;
}

//working_dir doesn't implemented
int ls(char* working_dir)
{
    char* filepath;
    char* filedata;
    unsigned int filesize;
    struct cpio_newc_header *header_pointer = CPIO_DEFAULT_PLACE;

    while(header_pointer!=0)
    {
        int error = cpio_newc_parse_header(header_pointer,&filepath,&filesize,&filedata,&header_pointer);
        //if parse header error
        if(error)
        {
            uart_puts("error");
            break;
        }

        //if this is not TRAILER!!! (last of file)
        if(header_pointer!=0)uart_printf("%s\n",filepath);
    }
    return 0;
}

void print_system_messages()
{
    unsigned int board_revision;
    get_board_revision(&board_revision);
    uart_printf("Board revision is : 0x");
    uart_hex(board_revision);
    uart_puts("");
    
    unsigned int arm_mem_base_addr;
    unsigned int arm_mem_size;

    get_arm_memory_info(&arm_mem_base_addr,&arm_mem_size);
    uart_printf("ARM memory base address in bytes : 0x");
    uart_hex(arm_mem_base_addr);
    uart_puts("");
    uart_printf("ARM memory size in bytes : 0x");
    uart_hex(arm_mem_size);
    uart_puts("");
}
#include "power.h"
#include "mbox.h"
#include "stdint.h"
#include "string.h"
#include "mini_uart.h"

typedef struct commads
{
    char* cmd;
    char* help;
    void (*func)(void);

} commads;

void shell_help();
void shell_hello();
void shell_mailbox();
void shell_reboot();
void shell_ls();
commads cmd_list[]={
        {.cmd="help", .help="print this help menu", .func=shell_help},
        {.cmd="hello", .help="print Hello World!", .func=shell_hello},
        {.cmd="reboot", .help="reboot the device", .func=shell_reboot},
        {.cmd="mailbox", .help="get mailbox information", .func=shell_mailbox},
        {.cmd="ls", .help="list directory", .func=shell_ls}
};

// https://www.freebsd.org/cgi/man.cgi?query=cpio&sektion=5
typedef struct {
    char    c_magic[6];
    char    c_ino[8];
    char    c_mode[8];
    char    c_uid[8];
    char    c_gid[8];
    char    c_nlink[8];
    char    c_mtime[8];
    char    c_filesize[8];
    char    c_devmajor[8];
    char    c_devminor[8];
    char    c_rdevmajor[8];
    char    c_rdevminor[8];
    char    c_namesize[8];
    char    c_check[8];
} cpio_newc_header;

void shell_ls(){
    uint64_t cur_addr = 0x8000000;
    cpio_newc_header* cpio_ptr;
    uint64_t name_size, file_size;
    char *file_name;
    char *file_content;

    // cpio packet: |header|file_name|data|

    while(1){
        // read the memory section as header
        cpio_ptr = (cpio_newc_header*)cur_addr;
        name_size = hex_to_int64(cpio_ptr->c_namesize);
        file_size = hex_to_int64(cpio_ptr->c_filesize);

        // the file name is after the header file, padded with NUL bytes
        // so the totol size of the header+filename is multiple of 4
        cur_addr += sizeof(cpio_newc_header);
        file_name = (char*)cur_addr;
        if(compare(file_name, "TRAILER!!!")) // special string indicates end of archive
            break;            

        file_content = file_name + name_size; // move pointer to file data location

        uart_puts("File Name: ");
        uart_puts(file_name);
        uart_puts("\r\n");

        for(uint64_t i=0; i<file_size; i++){
            if(file_content[i] == '\n')
                uart_putc('\r');
            uart_putc(file_content[i]);
        }
            
        // uart_puts(file_content);
        uart_puts("\r\n");
        uart_puts("File Size: ");
        uart_puts(itoa(file_size, 10));
        uart_puts(" bytes");    
        uart_puts("\r\n");
        uart_puts("==========\r\n");

        // file_name is padded with NUL butes, making it multiple of 4
        // add 3 for max possible padding, then and ~3=1111..1100 to cut the last two bits.
        cur_addr = (uint64_t)((cur_addr + name_size + 3) & (~3));
        cur_addr = (uint64_t)((cur_addr + file_size + 3) & (~3));
    }
}

void shell_help(){
    uart_puts("===============================================");
    uart_puts("\r\n");
    uart_puts("Command Name");
    uart_puts("\t");
    uart_puts("Description");
    uart_puts("\r\n");
    uart_puts("===============================================");
    uart_puts("\r\n");

    int cmd_len = sizeof(cmd_list)/sizeof(commads);
    for(int cmd_idx=0; cmd_idx<cmd_len; cmd_idx+=1){
        uart_puts(cmd_list[cmd_idx].cmd);
        uart_puts("\t\t");
        uart_puts(cmd_list[cmd_idx].help);
        uart_puts("\r\n");
    }
    uart_puts("===============================================");
    uart_puts("\r\n");
}

void shell_hello(){
    uart_puts("Hello World!");
}

void shell_reboot(){
    uart_puts("reset");
    reset();
}

void shell_mailbox(){

    // see mailbox detail in https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
    // get the board's unique serial number with a mailbox call
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETSERIAL;   // get serial number command
    mbox[3] = 8;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("My serial number is: ");
        uart_hex(mbox[6]);
        uart_hex(mbox[5]);
        uart_puts("\r\n");
    } else {
        uart_puts("Unable to query serial!\n");
    }
    
    /* ===== get the board's revison ===== */
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_BOARD_REVISION;   // get serial number command
    mbox[3] = 4;                    // buffer size
    mbox[4] = 4;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("My board revision is: ");
        uart_hex(mbox[6]);
        uart_hex(mbox[5]);
        uart_puts("\r\n");
    } else {
        uart_puts("Unable to query serial!\n");
    }
    /* ===== get the board's memory info ===== */
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_BOARD_MEMORY;   // get serial number command
    mbox[3] = 8;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("My ARM memory base address in bytes is: ");
        uart_hex(mbox[5]);
        uart_puts("\r\n");
        uart_puts("My ARM memory size in bytes is: ");
        uart_hex(mbox[6]);
        uart_puts("\r\n");
    } else {
        uart_puts("Unable to query serial!\n");
    }
    // echo everything back
    // uart_send(uart_getc());
}
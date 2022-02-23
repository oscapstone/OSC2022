#include "uart.h"
#include "utils.h"
#include "reset.h"

void print_gpu_info () {
    
}

int main() {

    unsigned int board_revision;
    unsigned int board_serial_msb, board_serial_lsb;
    unsigned int mem_base;
    unsigned int mem_size;
    // char start_prompt[7] = {'A', 'H', 'O', 'Y', '!', '\n', '\0'};
    char start_prompt[8] = {'A', 'H', 'O', 'Y', '!', '!', '\n', '\0'};
    char data;

    uart_init();
    uart_flush();

    uart_puts(start_prompt);

    get_board_revision(&board_revision);
    uart_puts("board_revision : 0x");
    uart_puth(board_revision);
    get_board_serial(&board_serial_msb, &board_serial_lsb);
    uart_puts("\nboard serial : 0x");
    uart_puth(board_serial_msb);
    uart_puth(board_serial_lsb);
    get_memory_info(&mem_base, &mem_size);
    uart_puts("\nmem base     : 0x");
    uart_puth(mem_base);
    uart_puts("\nmem size     : 0x");
    uart_puth(mem_size);


    while (1) {
        data = uart_get();
        uart_putc(data);
        if (data == '\n') uart_putc('\r');
        if (data == 'r') reset(10);
    }

    return 0;
}
#include "exec.h"

extern void from_EL1_to_EL0(unsigned long prog, unsigned long sp);

void exec_old (cpio_new_header *header, char *file_name, int enable_timer)
// void exec_old (cpio_new_header *header, char *file_name)
{
    char *prog;
    // void (*prog)();
    unsigned int current_el;
    char *stack_top;
    
    stack_top = malloc(0x2000);
    stack_top = stack_top + 0x2000;

    prog = cpio_load(header, file_name);

    if (prog == 0)
    {
        uart_puts("User program not found!\n");
        return;
    }

    /* lab5 */

    unsigned long file_size = 0x2000;
    char *file_addr = prog;
    char *new_addr = (char *)0x30000;
	char *new_addr_start = (char *)0x30000;
	uart_puts("\nmoving user prog...\n");
	while(file_size--)
    {
		*new_addr = *file_addr;
		new_addr++;
		file_addr++;
	}
    unsigned long user_stack_top = 0x48000;
    /* lab5 */

    // Get current EL
    asm volatile ("mrs %0, CurrentEL" : "=r" (current_el));
    current_el = current_el >> 2;

    // get current exception level
    uart_puts("Current EL: 0x");
    uart_hex(current_el);
  
    uart_puts("\n");
    uart_puts("-----------------Entering user program-----------------\n");

    if (enable_timer) 
    {
        core_timer_enable();
    }

    from_EL1_to_EL0((unsigned long)new_addr_start, (unsigned long)user_stack_top);
    
    // from_EL1_to_EL0((unsigned long)prog, (unsigned long)stack_top);

    // uart_puts("\r\nCurrent EL: 0x");
    // uart_hex(current_el);
    return;
}
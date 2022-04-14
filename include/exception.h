#define GPU_INTERRUPTS_ROUTING ((volatile uint32_t *)(0x4000000C))
#define core0_interrupt_source ((volatile unsigned int*)(0x40000060))

#define INTERRUPT_PRIVILEGE_TIMER 2
#define INTERRUPT_PRIVILEGE_READ 3
#define INTERRUPT_PRIVILEGE_WRITE 1

void enable_interrupt();
void disable_interrupt();
void exception_entry();
void irq_entry();
void GPU_interrupt_handler();
void Timer_interrupt_handler();
void rx_interrupt_handler();
void tx_interrupt_handler();


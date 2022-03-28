#ifndef	EXC_H_
#define	EXC_H_

void exception_entry(unsigned long long, 
                    unsigned long long, 
                    unsigned long long, 
                    unsigned long long, 
                    unsigned long long);
void Sync_exception(unsigned long long , unsigned long long , unsigned long long);

void Time_interrupt(unsigned long long);

void GPU_interrupt();

void core_timer_handler();

#define TRANSMIT_HOLDING 0b10
#define RECEIVE_VALID 0b100



#endif  
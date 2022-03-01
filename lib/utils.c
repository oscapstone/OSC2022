void delay_cycle(unsigned int n) {
    while(n--) { 
        asm volatile("nop"); 
    } 
}

void delay_ms(unsigned int n) {
    register unsigned long f, t, r;
    // get the current counter frequency
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    // read the current counter
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));
    
    t += ((f / 1000) * n) / 1000;
    do {
        asm volatile ("mrs %0, cntpct_el0" : "=r"(r));
    } while(r < t);
}
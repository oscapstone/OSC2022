extern int getpid();
extern unsigned int uart_read(char buf[],unsigned int size);
extern unsigned int uart_write(const char* name,unsigned int size);
extern int exec(const char* name, char *const argv[]);
extern int fork();
extern void exit();
extern int mbox_call(unsigned char ch, unsigned int *mbox);
extern void kill(int pid);
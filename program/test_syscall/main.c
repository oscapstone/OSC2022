#include <stdint.h>
#include <stddef.h>

int getpid()
{
    asm(
        "mov x8, #0\t\n"
        "svc 0"
    );
    int ret;
    asm("mov %0, x0":"=r"(ret));
    return ret;
}

size_t uartread(const char buf[], size_t size)
{
    asm(
        "mov x8, #1\t\n"
        "svc 0"
    );
    size_t ret;
    asm("mov %0, x0":"=r"(ret));
    return ret;
}

size_t uartwrite(const char buf[], size_t size)
{
    asm(
        "mov x8, #2\t\n"
        "svc 0"
    );
    size_t ret;
    asm("mov %0, x0":"=r"(ret));
    return ret;
}

int exec(const char *name, char *const argv[])
{
    asm(
        "mov x8, #3\t\n"
        "svc 0"
    );
    size_t ret;
    asm("mov %0, x0":"=r"(ret));
    return ret;
}

int fork()
{
    asm(
        "mov x8, #4\t\n"
        "svc 0"
    );
    int ret;
    asm("mov %0, x0":"=r"(ret));
    return ret;
}

void exit(int status)
{
    asm(
        "mov x8, #5\t\n"
        "svc 0"
    );
}

int mbox_call(unsigned char ch, unsigned int *mbox)
{
    asm(
        "mov x8, #6\t\n"
        "svc 0"
    );
    int ret;
    asm("mov %0, x0":"=r"(ret));
    return ret;
}

void kill(int pid)
{
    asm(
        "mov x8, #7\t\n"
        "svc 0"
    );
}

void signal(int SIGNAL, void (*handler)())
{
    asm(
        "mov x8, #8\t\n"
        "svc 0"
    );
}

void sigkill(int pid, int SIGNAL)
{
    asm(
        "mov x8, #9\t\n"
        "svc 0"
    );
}

size_t strlen(const char *str)
{
    int l = 0;
    while(*str){
        l++;
        str++;
    }
    return l;
}

char u42hex(uint32_t num)
{
    if(num<=9) return '0'+num;
    if(num>9 && num<16) return 'a'+num-10;
    return '?';
}

void u322hex(uint32_t num, char* buf, size_t len)
{
    len--;
    buf[0] = '0';
    buf[1] = 0;
    size_t i = 0;
    while(num && i<len){
        buf[i++] = u42hex(num&0xf);
        num = num>>4;
    }
    for(int j=0;j<i/2;j++){
        char tmp = buf[j];
        buf[j] = buf[i-j-1];
        buf[i-j-1] = tmp;
    }
    buf[(i>0?i:1)] = 0;
}

void u642hex(uint64_t num, char* buf, size_t len)
{
    len--;
    buf[0] = '0';
    buf[1] = 0;
    size_t i = 0;
    while(num && i<len){
        buf[i++] = u42hex(num&0xf);
        num = num>>4;
    }
    for(int j=0;j<i/2;j++){
        char tmp = buf[j];
        buf[j] = buf[i-j-1];
        buf[i-j-1] = tmp;
    }
    buf[(i>0?i:1)] = 0;
}

int atoi(const char* buf)
{
    int num = 0;
    int i=0;
    while(buf[i]){
        num *= 10;
        num += buf[i]-'0';
        i++;
    }
    return num;
}

void uart_print(const char* buf)
{
    int i=0;
    while(buf[i]){
        uartwrite(&buf[i], 1);
        i++;
    }
}

void uart_puts(const char* buf)
{
    uart_print(buf);
    uartwrite("\n\r", 2);
}

size_t uart_gets(char* buf)
{
    int i=0;
    while(uartread(&buf[i], 1)){
        if(buf[i]=='\n'){
            buf[i] = 0;
            break;
        }
        i++;
    }
    return i;
}


void uart_print_hex(uint64_t num)
{
    char buf[0x10];
    u642hex(num, buf, 0x10);
    uart_print(buf);
}

void uart_putshex(uint64_t num)
{
    uart_print_hex(num);
    uart_puts("");
}

#define GET_BOARD_REVISION_TAG  0x00010002
#define GET_ARM_MEMORY_TAG      0x00010005
#define REQUEST_CODE            0x00000000
#define REQUEST_SUCCEED         0x80000000
#define REQUEST_FAILED          0x80000001
#define TAG_REQUEST_CODE        0x00000000
#define END_TAG                 0x00000000

void get_board_revision(unsigned int* revision){
    unsigned int pad1[3];
    unsigned int mailbox[7];
    unsigned int pad2[3];
    uart_print("mailbox message buffer: 0x");
    uart_putshex((uint64_t)mailbox);
    mailbox[0] = 7 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_BOARD_REVISION_TAG; // tag identifier
    mailbox[3] = 4; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    // tags end
    mailbox[6] = END_TAG;

    int ret = mbox_call(8, mailbox);
    *revision = mailbox[5];
    if(ret!=0){
        uart_puts("mbox_call error");
    }
    return ;
}

void sighandle8()
{
    uart_puts("run signal handler 8.");
}

void init()
{
    //uartwrite("User process test!!", 19);
    uart_puts("User process test!!");
    int pid = getpid();
    uart_print("Pid: ");
    uart_putshex(pid);
    char buf[0x10];
    uart_print("Input: ");
    size_t len = uartread(buf, 5);
    uart_print("Test Input: ");
    uart_print(buf);
    uart_print(" (length: 0x");
    uart_print_hex(len);
    uart_puts(")");
    // uart_puts("Test Exec");
    // int ret = exec("test_syscall", 0);
    // uart_puts("exec error QQ");
    //exit(0);
    //uart_puts("exit error QQ");
    //for(int i=0;i<10000000;i++);
    unsigned int board=0;
    get_board_revision(&board);
    uart_print("Board Revision: 0x");
    uart_putshex((uint64_t)board);
    int cpid = fork();
    if(cpid == -1){
        uart_puts("fork error");
        exit(-1);
    }
    if(cpid){
        uart_print("Parent!!! Child Pid: 0x");
        uart_putshex(cpid);
        while(1){
            uart_print("Kill process: ");
            uart_gets(buf);
            //kill(atoi(buf));
            int pid = atoi(buf);
            uart_print("Kill process signal: ");
            uart_gets(buf);
            sigkill(pid, atoi(buf));
        }
    }
    else{
        pid = getpid();
        cpid = fork();
        if(cpid){
            signal(8, sighandle8);
            while(1){
                uart_print("Child!!! Pid: 0x");
                uart_putshex(pid);
                for(int i=0;i<10000000;i++);
            }
        }
        else{
            pid = getpid();
            while(1){
                uart_print("Second Child!!! Pid: 0x");
                uart_putshex(pid);
                for(int i=0;i<10000000;i++);
            }
        }
        
    }
    /*while(1){
        uart_print("Kill process: ");
        uart_gets(buf);
        kill(atoi(buf));
    }*/
    exit(0);
}
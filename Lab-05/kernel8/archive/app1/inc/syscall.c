#include "syscall.h"
#include "printf.h"

int getpid(){
	long ret;
	asm volatile("\
		svc 0\n\
		mov %0, x0\n\
	":"=r"(ret):);
	return ret;
}

int uart_read(char* buf,int size){
	long ret;
	asm volatile("\
		svc 1\n\
		mov %0, x0\n\
	":"=r"(ret):);
	return ret;
}

int uart_write(char* buf,int size){
	long ret;
	asm volatile("\
		svc 2\n\
		mov %0, x0\n\
	":"=r"(ret):);
	return ret;
}

unsigned int uart_printf(char* fmt,...){
	char dst[100];
    //__builtin_va_start(args, fmt): "..." is pointed by args
    //__builtin_va_arg(args,int): ret=(int)*args;args++;return ret;
    __builtin_va_list args;
    __builtin_va_start(args,fmt);
    unsigned int ret=vsprintf(dst,fmt,args);
    uart_write(dst,ret);
    return ret;
}

int exec(char* name,char** argv){
	long ret;
	asm volatile("\
		svc 3\n\
		mov %0, x0\n\
	":"=r"(ret):);
	return ret;
}

void exit(){
	asm volatile("svc 5\n"::);
	while(1){}
}

int fork(){
	long ret;
	asm volatile("\
		svc 4\n\
		mov %0, x0\n\
	":"=r"(ret):);
	return ret;
}

int mbox_call(unsigned char ch, unsigned int *mbox){
	long ret;
	asm volatile("\
		svc 6\n\
		mov %0, x0\n\
	":"=r"(ret):);
	return ret;
}

int getBoardRevision(unsigned int* dst){//it should be 0xa020d3 for rpi3 b+
    
	mbox[0]=7*4;
	mbox[1]=MBOX_REQUEST;
	mbox[2]=MBOX_TAG_GETREVISION;
	mbox[3]=4;
	mbox[4]=TAG_REQUEST_CODE;
	mbox[5]=0;
	mbox[6]=MBOX_TAG_LAST;

	int success=mbox_call(MBOX_CH_PROP, mbox);
	if(success){
		dst[0]=mbox[5];
	}
	return success;
}
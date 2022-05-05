#include "inc/syscall.h"

int main(int argc, char **argv){
	uart_write("app1\n", 100);
    uart_printf("pid %d\n", getpid());
//    for (int i = 0; i < argc; ++i) {
//        uart_printf("%s\n",argv[i]);
//   }
    unsigned int ret;
    getBoardRevision(&ret);
    uart_printf("getBoardRevision: %d\n", ret);

    char *fork_argv[] = {"fork_test", 0};
    exec("app2", fork_argv);
}


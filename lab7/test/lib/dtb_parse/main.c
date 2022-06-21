#include "fdt_parse.h"
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(void){
    int fd = open("./bcm2710-rpi-3-b-plus.dtb", O_RDONLY);
    if(fd < 0){
        printf("failed to open file\n");
    }
    struct stat statbuf;
    fstat(fd, &statbuf);
    char *ptr = mmap(NULL,statbuf.st_size,PROT_READ,MAP_PRIVATE,fd,0);

    fdt_parser(ptr, fdt_print_callback); 
}

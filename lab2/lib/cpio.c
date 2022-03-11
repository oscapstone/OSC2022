#include "cpio.h"
#include "printf.h"
#include "string.h"

void read_header(char *str, int size);

char* cpio_ls(){
  cpio_header *cpio = (cpio_header *)CPIO_ADDR;
  read_header(cpio->c_namesize, 8);
  printf("%s\n", cpio->c_check+8);
  return (char *)cpio->c_ino;
}

void read_header(char *str, int size){
  char *a = "";
  for(int i=0; i<size; i++){
    // *a += *(str+i);
    append_str(a, *(str+i));
    printf("%s\n\r", a);
  }
  
}

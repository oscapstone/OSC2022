#include "cpio.h"
#include "printf.h"
#include "string.h"
#include "malloc.h"

static int read_header(char *str, int size);

void cpio_ls(){
  cpio_header *cpio = (cpio_header *)CPIO_DEFAULT_PLACE;
  char *header = cpio->c_magic;
  int namesize = 0;
  int filesize = 0;
  while(strcmp(header+110, "TRAILER!!!")){
    namesize = read_header(header + 94, 8);
    filesize = read_header(header + 54, 8);
    if((namesize+110)%4){
      namesize += (4 - (namesize+110)%4);
    }
    if(filesize%4){
      filesize += (4 - filesize%4);
    }
    printf("%s\n\r", header+110);
    header = header + namesize + filesize + 110;
  }
}

int cpio_cat(char *str){
  cpio_header *cpio = (cpio_header *)CPIO_DEFAULT_PLACE;
  char *header = cpio->c_magic;
  int namesize = 0;
  int filesize = 0;
  while(strcmp(header+110, "TRAILER!!!")){
    namesize = read_header(header + 94, 8);
    filesize = read_header(header + 54, 8);
    namesize = namesize + namesize%2;
    if((namesize+110)%4){
      namesize += (4 - (namesize+110)%4);
    }
    if(filesize%4){
      filesize += (4 - filesize%4);
    }
    if(!strcmp(header+110, str) && filesize != 0){
      printf("%s\n\r", header+110+namesize);
      return 1;
    }
    header = header + namesize + filesize + 110;
  }
  return 0;
}

int cpio_exec(char *str){
  cpio_header *cpio = (cpio_header *)CPIO_DEFAULT_PLACE;
  char *header = cpio->c_magic;
  int namesize = 0;
  int filesize = 0;
  while(strcmp(header+110, "TRAILER!!!")){
    namesize = read_header(header + 94, 8);
    filesize = read_header(header + 54, 8);
    namesize += namesize%2;
    if((namesize+110)%4){
      namesize += (4 - (namesize+110)%4);
    }
    if(filesize%4){
      filesize += (4 - filesize%4);
    }
    if(!strcmp(header+110, str) && filesize != 0){
      char *data = header+110+namesize;
      char * sp = simple_malloc(0x20);
      asm("mov x1, 0x3c0\n\t"
          "msr spsr_el1, x1\n\t"
          "msr elr_el1, %[input0]\n\t"
          "msr sp_el0, %[input1]\n\t"
          "eret\n\t"
          :
          :[input0] "r" (data), [input1] "r" (sp)
          :);
      return 1;
    }
    header = header + namesize + filesize + 110;
  }
  return 0;
}

static int read_header(char *str, int size){
  char a[size + 1];
  a[1] = 0;
  for(int i=0; i<size; i++){
    append_str(a, *(str+i));
  }
  return myHex2Int(a);
}

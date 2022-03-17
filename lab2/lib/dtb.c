#include "dtb.h"
#include "printf.h"
#include "utilities.h"
#include "string.h"

void fdt_traverse(dtb_callback callback){
  struct fdt_header* header = (struct fdt_header*) dtb_place;

  if(little_2_big_u32(header->magic) != 0xD00DFEED){
    printf("fdt_traverse : wrong magic in fdt_traverse\n\r");
    return;
  }

  uint32_t struct_size = little_2_big_u32(header->size_dt_struct);
  char* dt_struct_ptr = (char*)((char*)header + little_2_big_u32(header->off_dt_struct));
  char* dt_strings_ptr = (char*)((char*)header + little_2_big_u32(header->off_dt_strings));
  char* end = (char*)dt_struct_ptr + struct_size;
  char* pointer = dt_struct_ptr;
  struct fdt_prod *prod_ptr;

  while(pointer < end){
    uint32_t token_type = little_2_big_u32(*(uint32_t*)pointer);
    pointer += 4;
    if(token_type == FDT_BEGIN_NODE) { 
      callback(token_type, pointer, 0, 0);
      pointer += strlen(pointer);
      pointer += 4 - (unsigned long long)pointer%4;           //alignment 4 byte
    }else if(token_type == FDT_PROP) {
      prod_ptr = (struct fdt_prod *) pointer;
      pointer += 8;  // add the fdt_prod len
      uint32_t len = little_2_big_u32(prod_ptr->len);
      char* name = (char*)dt_strings_ptr + little_2_big_u32(prod_ptr->nameoff);
      callback(token_type, name, pointer, len);
      pointer += len;
      if((unsigned long long)pointer % 4 !=0){
        pointer += 4 - (unsigned long long)pointer%4;   //alignment 4 byte
      }
    }
  }
}

void initramfs_callback(uint32_t node_type, char *name, void *value, uint32_t name_size) {
  if(node_type==FDT_PROP && strcmp(name,"linux,initrd-start")==0){
    CPIO_DEFAULT_PLACE = (void *)(unsigned long long)little_2_big_u32(*(uint32_t*)value);
  }
}

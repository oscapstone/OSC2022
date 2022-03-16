#include "devtree.h"
#include "mini_uart.h"

uint32_t fdt32_to_cpu(uint32_t fdt_num) {
  uint8_t *part = (uint8_t*)&fdt_num;
  return (part[0] << 24) | (part[1] << 16) | (part[2] << 8) | (part[3]);
}

uint64_t fdt64_to_cpu(uint64_t fdt_num) {
  uint8_t *part = (uint8_t*)&fdt_num;
  uint64_t val = (part[0] << 24) | (part[1] << 16) | (part[2] << 8) | (part[3]);
  val <<= 32;
  val |= (part[4] << 24) | (part[5] << 16) | (part[6] << 8) | (part[7]);
  return val;
}


static uint32_t fdt_iter_tag(const void *mem, uint32_t off, uint32_t *nextoff) {
  uint32_t *tagp;
  uint32_t tag, len;
  uint32_t offset = off;
  char *node_name;
  uint32_t name_size;
  struct fdt_prop_header *prop_ptr;

  tagp = (uint32_t*)(mem + off);
  tag = fdt32_to_cpu(*tagp);

  switch (tag) {
  case FDT_BEGIN_NODE:
    node_name = (char*)(mem + off + sizeof(tag));
    for (name_size = 1; node_name[name_size-1] != '\0'; name_size++);
    if (name_size & 3) name_size += 4 - (name_size & 3);
    offset += sizeof(tag) + name_size;
    break;
    
  case FDT_PROP:
    prop_ptr = (struct fdt_prop_header*)(mem+off+sizeof(tag));
    len = fdt32_to_cpu(prop_ptr->len);
    if (len & 3) len += 4 - (len & 3);
    offset += sizeof(tag) + sizeof(struct fdt_prop_header) + len;
    break;
    
  case FDT_END_NODE:
  case FDT_NOP:
  case FDT_END:
    offset += sizeof(tag);
    break;
  }

  *nextoff = offset;
  return tag;
}


static uint32_t fdt_iter_node(const void *mem, uint32_t off, uint32_t *nextoff) {
  uint32_t nxtoff = off;
  uint32_t tag;
  do {
    off = nxtoff;
    tag = fdt_iter_tag(mem, off, &nxtoff);
    if (tag == FDT_END) return -1;
  } while (tag != FDT_BEGIN_NODE);
  *nextoff = nxtoff;
  return off;
}

static struct fdt_prop_header* fdt_iter_prop(const void *mem, uint32_t off, uint32_t *nextoff) {
  uint32_t tag;
  uint32_t nxtoff = off;
  do {
    off = nxtoff;
    tag = fdt_iter_tag(mem, off, &nxtoff);
    if (tag == FDT_END_NODE) {
      return 0;
    }
  } while (tag != FDT_PROP);
  *nextoff = nxtoff;
  struct fdt_prop_header *prop_ptr = (struct fdt_prop_header*)(mem + off + sizeof(uint32_t));
  return prop_ptr;
}

inline static char* fdt_prop_getname(struct fdt_prop_header *prop_ptr, void *mem, uint32_t strings_off) {
  return (char*)mem + strings_off + fdt32_to_cpu(prop_ptr->nameoff);
}


void fdt_traverse(void (*func)(const char* nd, const char *prop, void* value, uint32_t size)) {
  struct fdt_header *header = (struct fdt_header*)(*DTB_ADDR);
  uint32_t struct_off = fdt32_to_cpu(header->off_dt_struct);
  uint32_t strings_off = fdt32_to_cpu(header->off_dt_strings);
  char *node_name;
  char *prop_name;

  uint32_t node_off;
  uint32_t nextnode_off = struct_off;
  print_hex((uint32_t)(header));
  print(" dtb base addr\n");
  while (1) {
    node_off = nextnode_off;
    node_off = fdt_iter_node(header, node_off, &nextnode_off);
    if (node_off == -1) break;
    node_name = ((char*)header) + node_off + sizeof(uint32_t);

    /*
    print("node_name: ");
    print(node_name);
    print_char('\n');
    print("    ");
    */

    
    uint32_t prop_off = node_off;
    struct fdt_prop_header *prop_ptr;
    while(1) {
      prop_ptr = fdt_iter_prop(header, prop_off, &prop_off);
      if (prop_ptr == 0) {
        break;
      }
      prop_name = fdt_prop_getname(prop_ptr, header, strings_off);
      func(node_name, prop_name,
           ((char*)prop_ptr) + sizeof(struct fdt_prop_header),
           fdt32_to_cpu(prop_ptr->len));
      /*
      print(prop_name);
      print("|");
      */
    }
  }
}


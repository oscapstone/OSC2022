#ifndef DEVTREE_H
#define DEVTREE_H

#include "stdint.h"

#define DTB_ADDR ((volatile uint64_t *)(0x5FFF8))

struct fdt_header {
  uint32_t magic;
  uint32_t totalsize;
  uint32_t off_dt_struct;
  uint32_t off_dt_strings;
  uint32_t off_mem_rsvmap;
  uint32_t version;
  uint32_t last_comp_version;
  uint32_t boot_cpuid_phys;
  uint32_t size_dt_strings;
  uint32_t size_dt_struct;
};

struct fdt_prop_header {
  uint32_t len;
  uint32_t nameoff;
};

#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

void fdt_traverse(void (*func)(const char* nd, const char *prop, void* value, uint32_t size));

uint32_t fdt32_to_cpu(uint32_t fdt_num);

uint64_t fdt64_to_cpu(uint64_t fdt_num);

#endif

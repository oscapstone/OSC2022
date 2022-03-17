#include "utils.h"
void fdt_traverse(void (*f)());

#define FDT_HEADER_MAGIC 0xd00dfeed

#define FDT_BEGIN_NODE (0x00000001)
#define FDT_END_NODE (0x00000002)
#define FDT_PROPERTY (0x00000003)
#define FDT_NOP (0x00000004)
#define FDT_END (0x00000009)



typedef struct fdt_header {
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
}fdt_header;
typedef struct fdt_reserve_entry{
    uint64_t address;
    uint64_t size;
}fdt_reserve_entry;
typedef struct fdt_prop
{
    uint32_t len;
    uint32_t nameoff;
}fdt_prop;
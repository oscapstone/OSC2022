#ifndef FDT
#define FDT

#include "type.h"

#define FDT_BEGIN_NODE  (0x00000001)
#define FDT_END_NODE    (0x00000002)
#define FDT_PROP        (0x00000003)
#define FDT_NOP         (0x00000004)
#define FDT_END         (0x00000009)

extern uint32* initramfs;

struct fdt_header {
    uint32 magic;                 // 0xd00dfeed
    uint32 totalsize;             // total size of devicetree in bytes
    uint32 off_dt_struct;         // offset in bytes of the structure block
    uint32 off_dt_strings;        // offset in bytes of the strings block
    uint32 off_mem_rsvmap;        // offset in bytes of the memory reservation block
    uint32 version;               // version of devicetree
    uint32 last_comp_version;     // lowest version of the devicetree with which is backwards compatible.
    uint32 boot_cpuid_phys;       // physical ID of the system’s boot CPU
    uint32 size_dt_strings;       // length in bytes of the strings block section
    uint32 size_dt_struct;        // length in bytes of the structure block section
};

typedef struct fdt_reserve_entry_ {
    uint64 address;
    uint64 size;
} fdt_ent;

typedef struct fdt_prop_ {
    uint32 len;
    uint32 nameoff;
} fdt_prop;

typedef struct memory_node {
    struct fdt_reserve_entry_ entry;
    struct memory_node* next;
} m_node;

int initramfs_callback(char* data, char* name, char* prop_name);
void fdt_traverse(uint32* addr, int (*callback)(char* prop, char* name, char* prop_name));

#endif
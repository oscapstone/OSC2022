#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SWAP32(x) ((x >> 24) | ((x & 0x00FF0000) >> 8) | \
                   (x << 24) | ((x & 0x0000FF00) << 8)) // for big endian
#define SWAP64(x) ((x >> 56) | ((x & 0x00FF000000000000ULL) >> 40) | ((x & 0x0000FF0000000000ULL) >> 24) | ((x & 0x000000FF00000000ULL) >> 8) | \
                   (x << 56) | ((x & 0x000000000000FF00ULL) << 40) | ((x & 0x0000000000FF0000ULL) << 24) | ((x & 0x00000000FF000000ULL) << 8)) // for big endian

#define FDT_BEGIN_NODE  (0x00000001)
#define FDT_END_NODE    (0x00000002)
#define FDT_PROP        (0x00000003)
#define FDT_NOP         (0x00000004)
#define FDT_END         (0x00000009)

typedef unsigned int uint32;
typedef unsigned long long uint64;

uint32* initramfs;

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

void get_mem_resblocks(m_node* node, uint64* addr) {
    uint64 address = 0;
    uint64 size = 0;
    m_node* curr = node;

    while(1) {
        address = *(addr++); address = SWAP64(address);
        size = *(addr++); size = SWAP64(size);
        printf("address: 0x%016llx, size: %llu\n", address, size);
        if(address == 0 && size == 0) {
            break;
        }
        else {
            if(curr->entry.address != 0 || curr->entry.size != 0) {
                curr->next = (m_node*)malloc(sizeof(m_node));
                curr = curr->next;
            }
           
            curr->entry.address = address;
            curr->entry.size = size;
            curr->next = NULL;   
        }
    }  
}

int initramfs_callback(char* data, char* name, char* prop_name) {
    if(strcmp(name, "chosen") == 0 && strcmp(prop_name, "linux,initrd-start") == 0) {
        uint32 value = *((uint32*)data);
        value = SWAP32(value);
        initramfs = (volatile uint32*)value;
        printf("Get initramfs\n");
        return 1;
    }
    else {
        return 0;
    }
}

void fdt_traverse(uint32* addr, int (*callback)(char* prop, char* name, char* prop_name)) {
    struct fdt_header head = {
        SWAP32(addr[0]), SWAP32(addr[1]), SWAP32(addr[2]), SWAP32(addr[3]), SWAP32(addr[4]), 
        SWAP32(addr[5]), SWAP32(addr[6]), SWAP32(addr[7]), SWAP32(addr[8]), SWAP32(addr[9])
    };

    printf("magic: 0x%x\n", head.magic);
    printf("totalsize: 0x%x\n", head.totalsize);
    printf("off_dt_struct: 0x%x\n", head.off_dt_struct);
    printf("off_dt_strings: 0x%x\n", head.off_dt_strings);
    printf("off_mem_rsvmap: 0x%x\n", head.off_mem_rsvmap);
    printf("version: %d\n", head.version);
    printf("last_comp_version: %d\n", head.last_comp_version);
    printf("boot_cpuid_phys: 0x%x\n", head.boot_cpuid_phys);
    printf("size_dt_strings: 0x%x\n", head.size_dt_strings);
    printf("size_dt_struct: 0x%x\n", head.size_dt_struct);
    
    char* name, * prop_name, * cnode;
    fdt_prop* prop;
    char* struct_addr = (char*)addr + head.off_dt_struct;
    uint32* node = (uint32*)struct_addr;
    uint32 value, alice;
    int finish = 0;
    while(finish == 0) {
        value = *(node++);
        cnode = (char*)node;
        // printf("NODE: 0x%08x at 0x%08x\n", SWAP32(value), cnode);
        switch(SWAP32(value)) {
            case FDT_BEGIN_NODE:
                name = cnode;
                cnode += strlen(name) + 1; // include '\0'
                printf("name = %s(%02ld)\n", name, strlen(name));
                break;
            case FDT_END_NODE:
                break;
            case FDT_PROP:
                prop = (fdt_prop*)cnode;
                prop->len = SWAP32(prop->len);
                prop->nameoff = SWAP32(prop->nameoff);
                prop_name = (char*)addr + head.off_dt_strings + prop->nameoff;
                
                printf("prop name = %s\n", prop_name);
                cnode += sizeof(fdt_prop);
                finish = callback(cnode, name, prop_name);
                cnode += prop->len;
                break;   
            case FDT_NOP:
                break;
            case FDT_END:
                finish = 1;
                break;
            default:
                break;
        }

        // 4 bytes alignment
        alice = (cnode - struct_addr) % 4;
        if(alice != 0) {
            cnode += 4 - alice;
        }

        node = (uint32*)cnode;
    }

}

int main(void) {
    FILE *fp = fopen("./bcm2710-rpi-3-b-plus.dtb", "rb");

    char buffer[31790];
    fread(buffer, sizeof(char), 31790, fp);

    fdt_traverse((uint32*)buffer, initramfs_callback);
    // uint32* buf32 = (uint32*)buffer;
    // struct fdt_header head = {
    //     SWAP32(buf32[0]), SWAP32(buf32[1]), SWAP32(buf32[2]), SWAP32(buf32[3]), SWAP32(buf32[4]), 
    //     SWAP32(buf32[5]), SWAP32(buf32[6]), SWAP32(buf32[7]), SWAP32(buf32[8]), SWAP32(buf32[9])
    // };

    // printf("magic: 0x%x\n", head.magic);
    // printf("totalsize: 0x%x\n", head.totalsize);
    // printf("off_dt_struct: 0x%x\n", head.off_dt_struct);
    // printf("off_dt_strings: 0x%x\n", head.off_dt_strings);
    // printf("off_mem_rsvmap: 0x%x\n", head.off_mem_rsvmap);
    // printf("version: %d\n", head.version);
    // printf("last_comp_version: %d\n", head.last_comp_version);
    // printf("boot_cpuid_phys: 0x%x\n", head.boot_cpuid_phys);
    // printf("size_dt_strings: 0x%x\n", head.size_dt_strings);
    // printf("size_dt_struct: 0x%x\n", head.size_dt_struct);

    // uint64* buf64 = (uint64*)(buffer + head.off_mem_rsvmap);
    
    // m_node node;
    // node.entry.address = 0;
    // node.entry.size = 0;
    // node.next = NULL;
    // get_mem_resblocks(&node, buf64);

    fclose(fp);
    return 0;
}
#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)
#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1
#define MAIR_CONFIG_DEFAULT ((MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)))

#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_PAGE 0b11
#define PD_ACCESS (1 << 10)
#define USER_RW ((1 << 6) | (0 << 7))
#define USER_RO ((1 << 6) | (1 << 7))
#define BOOT_PGD_ATTR PD_TABLE
#define BOOT_PUD_ATTR PD_TABLE

#define PD_RAM_ATTR (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK)
#define USER_READ_WRITE (PD_ACCESS | USER_RW | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK)
#define USER_READ_ONLY (PD_ACCESS | USER_RO | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK)
#define PD_PERIPHERAL_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)

#define PD_ADDRESS_MASK 0x0000FFFFFFFFF000
#define PAGE_OFFSET_MASK 0xFFF

#define VIRTUAL_USER_STACK 0xFFFFFFFFD000
#define VIRTUAL_KERNEL_STACK 0xFFFFFFFFE000
#define VIRTUAL_USER_PROGRAM 0x0
#define PHYSICAL_USER_PROGRAM 0x07000000

extern unsigned long store_pgd();

unsigned long get_high_pa(void *ptr);
unsigned long get_low_pa(void *ptr);
void page_table_init(unsigned long* table);
unsigned long* create_page_table();
void page_table_alloc(unsigned long table, unsigned long next, unsigned long attribute, unsigned int offset);
void* page_alloc(unsigned long pgd, unsigned long virtual_addr, unsigned long physical_addr, unsigned long attribute);
void page_free(unsigned long pgd, unsigned long virtual_addr);
void change_attribute(unsigned long virtual_addr, unsigned long attribute);
void video_paging(unsigned long pgd, unsigned long pud, unsigned long pmd);
void user_default_paging();
void lower_data_abort_handler();
unsigned long demand_log(unsigned long* list, unsigned long virtual_addr);
int demand_find(unsigned long* list, unsigned long virtual_addr);
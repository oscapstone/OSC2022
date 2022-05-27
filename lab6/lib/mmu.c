#include "mmu.h"
#include "malloc.h"
#include "printf.h"

void map_pages(uint64_t* page_table, uint64_t va, uint64_t alloc){
  uint32_t index[4]; // index of each table
  va >>= 12;
  index[3] = va & 0x1ff;
  va >>= 9;
  index[2] = va & 0x1ff;
  va >>= 9;
  index[1] = va & 0x1ff;
  va >>= 9;
  index[0] = va & 0x1ff;
  uint64_t* table = (uint64_t *)PA2VA(page_table);
  for (int i = 0; i < 3; i++) {
    if (table[index[i]] == 0) {
      init_PT((uint64_t **)&table[index[i]]);
      table[index[i]] |= PD_TABLE;
    }
    uint64_t entry = table[index[i]];
    entry = entry - (entry & 0xfff);
    table = (uint64_t*)PA2VA(entry);
  }
  if (table[index[3]])
		printf("[ERROR][map_pages] the VA: %x has already been mapped!\n\r", va);
  table[index[3]] = (alloc) | (1<<10) | (1<<6) | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_TABLE;
}

void init_PT(uint64_t** page_table){
  char *table = (char *)page_allocate_addr(0x1000);
  for (int i = 0; i < 4096; ++i)
		table[i] = 0;
  *page_table = (void*)VA2PA(table);
}

void duplicate_PT(void *page_table_src, void *page_table_dst){
  if (page_table_src == 0 || page_table_dst == 0)
		printf("[ERROR][dupPT] invalid table!\n\r");
  uint64_t *PGD_src = (uint64_t *)PA2VA(page_table_src);
	uint64_t *PGD_dst = (uint64_t *)PA2VA(page_table_dst);
  for(int i=0; i<512; i++){
    if(PGD_src[i] != 0){
      if(i != 511){
        init_PT((uint64_t **)&PGD_dst[i]);  // alloc PUD
        PGD_dst[i] |= PGD_src[i] & 0xfff;
        uint64_t *PUD_src = (uint64_t *)PA2VA(PGD_src[i]-(PGD_src[i]&0xfff));
        uint64_t *PUD_dst = (uint64_t *)PA2VA(PGD_dst[i]-(PGD_dst[i]&0xfff));
        for(int j=0; j<512; j++){
          if(PUD_src[j] != 0){
            init_PT((uint64_t **)&PUD_dst[j]);      // alloc PMD
            PUD_dst[j] |= PUD_src[j] & 0xfff;
            uint64_t *PMD_src = (uint64_t *)PA2VA(PUD_src[j]-(PUD_src[j]&0xfff));
            uint64_t *PMD_dst = (uint64_t *)PA2VA(PUD_dst[j]-(PUD_dst[j]&0xfff));
            for(int k=0; k<512; k++){
              if(PMD_src[k] != 0){
                init_PT((uint64_t **)&PMD_dst[k]); // alloc pte
                PMD_dst[k] |= PMD_src[k] & 0xfff;
                uint64_t *PTE_src = (uint64_t *)PA2VA(PMD_src[k]-(PMD_src[k]&0xfff));
                uint64_t *PTE_dst = (uint64_t *)PA2VA(PMD_dst[k]-(PMD_dst[k]&0xfff));
                for(int l=0; l<512; l++){
                  if(PTE_src[l] != 0){
                    PTE_dst[l] = PTE_src[l];
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

void mmu_init(){
  // Set up TCR_EL1
  asm volatile("msr tcr_el1, %[output]\n"::[output]"r"(TCR_CONFIG_DEFAULT));
  /*
		1000 0000 0001 0000 0000 0000 0001 0000
		[5:0] the number of the most significant bits that must be all 0s // TTBR0_EL1
		[21:16] the number of the most significant bits that must be all 1s // TTBR1_EL1
		[15:14] granule size for user space	(00=4KB, 01=16KB, 11=64KB) // TTBR0_EL1
		[31:30] granule size for kernel space // TTBR1_EL1
	*/
  // Set up mair_el1
  asm volatile("msr mair_el1, %[output]\n"::[output]"r"(MAIR_CONFIG_DEFAULT));
  /*
		0100 0100 0000 0000
		[7:0] device memory nGnRnE
		[15:8] normal memory without cache
	*/
  // L0 table init
	asm volatile("str %0, [%1]\n"::"r"(0x2000|BOOT_PGD_ATTR),"r"(0x1000));             // 0x1000 for the PGD's frame
  // L1 table init
  asm volatile("str %0, [%1]\n"::"r"(0x3000|BOOT_PGD_ATTR),"r"(0x2000));        // finer granularity for different memory type
	asm volatile("str %0, [%1]\n"::"r"(0x40000000|BOOT_PUD_ATTR),"r"(0x2000+8));  // 1G block for ARM peripherals
  // Set Identity Paging
  // 0x00000000 ~ 0x3f000000: Normal
  // 0x3f000000 ~ 0x40000000: Device
  // 0x40000000 ~ 0x80000000: Device
  asm volatile("\
		mov x10, %0\n\
		mov x11, %1\n\
		mov x0, #0x3000\n\
		mov x1, #512\n\
		mov x2, #0\n\
  beg1:\n\
      cbz x1, end1\n\
      ldr x3, =0x3F000000\n\
      cmp x2, x3\n\
      blt normalmem\n\
  peripheralsmem:\n\
      orr x3, x2, x10\n\
      b end2\n\
  normalmem:\n\
      orr x3, x2, x11\n\
      b end2\n\
  end2:\n\
      str x3, [x0]\n\
      add x0, x0, #8\n\
      sub x1, x1, #1\n\
      add x2, x2, #0x200000\n\
      b beg1\n\
  end1:\n\
    "::"r"(BOOT_L2D_ATTR),"r"(BOOT_L2N_ATTR));
  // load the PGD to the bottom translation-based register
  asm volatile("msr ttbr0_el1, %0\n"::"r"(0x1000));
  asm volatile("msr ttbr1_el1, %0\n"::"r"(0x1000));
  // enable the mmu
  asm volatile("mrs x2, sctlr_el1\n");
  asm volatile("orr x2 , x2, 1\n");
  asm volatile("msr sctlr_el1, x2\n");
}


// for debug

void printf_pt(uint64_t *page_table){
  for(int i=0; i<512; i++){
    page_table = (uint64_t *)PA2VA(page_table);
    if((page_table[i]) != 0){
      printf("PGD[%03d]: 0x%llx\n\r", i, page_table[i]);
      uint64_t *PUD = (uint64_t *)PA2VA(page_table[i] - (page_table[i]&0xfff));
      for(int j=0; j<512; j++){
        if(PUD[j] != 0){
          printf("  PUD[%03d]: 0x%llx\n\r", j, PUD[j]);
          uint64_t *PMD = (uint64_t *)PA2VA(PUD[j] - (PUD[j]&0xfff));
          for(int k=0; k<512; k++){
            if(PMD[k] != 0){
              printf("    PMD[%03d]: 0x%llx\n\r", k, PMD[k]);
              uint64_t *PTE = (uint64_t *)PA2VA(PMD[k] - (PMD[k]&0xfff));
              for(int l=0; l<512; l++){
                if(PTE[l] != 0){
                  printf("      PTE[%03d]: 0x%llx", l, PTE[l]);
                  if(l %4 == 0)
                    printf("\n\r");
                }
                
              }
            }
          }
        }
      }
    }
  }
}

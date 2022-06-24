#include <mmu-def.h>

.section ".text.mmu"
.global enable_kernel_mmu
enable_kernel_mmu:
    mov x4, x0
    ldr x0, =TCR_CONFIG_DEFAULT
    msr tcr_el1, x0
    ldr x0, =( \
    (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
    (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
    )
    msr mair_el1, x0

    mov x0, 0x1000 // PGD's page frame at 0x1000
    mov x1, 0x2000 // PUD's page frame at 0x2000
    mov x5, 0x3000 // PMD's page frame at 0x3000

    ldr x2, = BOOT_PGD_ATTR
    orr x2, x1, x2 // combine the physical address of next level page with attribute.
    str x2, [x0]

    ldr x2, = BOOT_PUD_TABLE
    orr x3, x5, x2
    str x3, [x1] // 1st 1GB mapped by the 1st entry of PUD
    ldr x2, = BOOT_PMD_ATTR_NORMAL
    mov x6, 0x0
    mov x7, 0x0
    mov x8, 0x0
    .loop:
        orr x3, x6, x2
        str x3, [x5], 8
        add x6, x6, 0x200000
        add x7, x7, 1
        cmp x7, 480
        blt .loop
    ldr x2, = BOOT_PMD_ATTR_DEVICE
    .loop1:
        orr x3, x6, x2
        str x3, [x5], 8
        add x6, x6, 0x200000
        add x7, x7, 1
        cmp x7, 512
        blt .loop1
    ldr x2, = BOOT_PUD_ATTR
    mov x3, 0x40000000
    orr x3, x2, x3
    str x3, [x1, 8] // 2nd 1GB mapped by the 2nd entry of PUD
    msr ttbr0_el1, x0 // load PGD to the bottom translation-based register.
    msr ttbr1_el1, x0

    mrs x2, sctlr_el1
    orr x2 , x2, 1
    msr sctlr_el1, x2 // enable MMU, cache remains disabled

    br x4
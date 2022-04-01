.section ".text.start"
.global _start
_start:
    mov x0, 0
loop:
    add x0, x0, 1
//    svc 0
    mov x1, 0
//    .sleep:
//        add x1, x1, 1
//        cmp x1, 0x10000
//        b.lt .sleep
    b loop
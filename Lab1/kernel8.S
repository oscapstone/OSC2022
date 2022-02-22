
build/kernel8.elf:     file format elf64-littleaarch64


Disassembly of section .text.boot:

0000000000000000 <_start>:
   0:	d53800a0 	mrs	x0, mpidr_el1
   4:	92401c00 	and	x0, x0, #0xff
   8:	b4000060 	cbz	x0, 14 <master>
   c:	14000001 	b	10 <proc_hang>

0000000000000010 <proc_hang>:
  10:	14000000 	b	10 <proc_hang>

0000000000000014 <master>:
  14:	10001960 	adr	x0, 340 <bss_begin>
  18:	10001941 	adr	x1, 340 <bss_begin>
  1c:	cb000021 	sub	x1, x1, x0
  20:	94000093 	bl	26c <memzero>
  24:	b26a03ff 	mov	sp, #0x400000              	// #4194304
  28:	94000080 	bl	228 <kernel_main>
  2c:	17fffff9 	b	10 <proc_hang>

Disassembly of section .text:

0000000000000030 <uart_send>:
  30:	a9be7bfd 	stp	x29, x30, [sp, #-32]!
  34:	910003fd 	mov	x29, sp
  38:	39007fe0 	strb	w0, [sp, #31]
  3c:	d28a0a80 	mov	x0, #0x5054                	// #20564
  40:	f2a7e420 	movk	x0, #0x3f21, lsl #16
  44:	94000085 	bl	258 <get32>
  48:	121b0000 	and	w0, w0, #0x20
  4c:	7100001f 	cmp	w0, #0x0
  50:	54000041 	b.ne	58 <uart_send+0x28>  // b.any
  54:	17fffffa 	b	3c <uart_send+0xc>
  58:	d503201f 	nop
  5c:	528007e1 	mov	w1, #0x3f                  	// #63
  60:	d28a0800 	mov	x0, #0x5040                	// #20544
  64:	f2a7e420 	movk	x0, #0x3f21, lsl #16
  68:	9400007a 	bl	250 <put32>
  6c:	d503201f 	nop
  70:	a8c27bfd 	ldp	x29, x30, [sp], #32
  74:	d65f03c0 	ret

0000000000000078 <uart_recv>:
  78:	a9bf7bfd 	stp	x29, x30, [sp, #-16]!
  7c:	910003fd 	mov	x29, sp
  80:	d28a0a80 	mov	x0, #0x5054                	// #20564
  84:	f2a7e420 	movk	x0, #0x3f21, lsl #16
  88:	94000074 	bl	258 <get32>
  8c:	12000000 	and	w0, w0, #0x1
  90:	7100001f 	cmp	w0, #0x0
  94:	54000041 	b.ne	9c <uart_recv+0x24>  // b.any
  98:	17fffffa 	b	80 <uart_recv+0x8>
  9c:	d503201f 	nop
  a0:	d28a0800 	mov	x0, #0x5040                	// #20544
  a4:	f2a7e420 	movk	x0, #0x3f21, lsl #16
  a8:	9400006c 	bl	258 <get32>
  ac:	12001c00 	and	w0, w0, #0xff
  b0:	a8c17bfd 	ldp	x29, x30, [sp], #16
  b4:	d65f03c0 	ret

00000000000000b8 <uart_send_string>:
  b8:	a9bd7bfd 	stp	x29, x30, [sp, #-48]!
  bc:	910003fd 	mov	x29, sp
  c0:	f9000fe0 	str	x0, [sp, #24]
  c4:	b9002fff 	str	wzr, [sp, #44]
  c8:	14000009 	b	ec <uart_send_string+0x34>
  cc:	b9802fe0 	ldrsw	x0, [sp, #44]
  d0:	f9400fe1 	ldr	x1, [sp, #24]
  d4:	8b000020 	add	x0, x1, x0
  d8:	39400000 	ldrb	w0, [x0]
  dc:	97ffffd5 	bl	30 <uart_send>
  e0:	b9402fe0 	ldr	w0, [sp, #44]
  e4:	11000400 	add	w0, w0, #0x1
  e8:	b9002fe0 	str	w0, [sp, #44]
  ec:	b9802fe0 	ldrsw	x0, [sp, #44]
  f0:	f9400fe1 	ldr	x1, [sp, #24]
  f4:	8b000020 	add	x0, x1, x0
  f8:	39400000 	ldrb	w0, [x0]
  fc:	7100001f 	cmp	w0, #0x0
 100:	54fffe61 	b.ne	cc <uart_send_string+0x14>  // b.any
 104:	d503201f 	nop
 108:	d503201f 	nop
 10c:	a8c37bfd 	ldp	x29, x30, [sp], #48
 110:	d65f03c0 	ret

0000000000000114 <uart_init>:
 114:	a9be7bfd 	stp	x29, x30, [sp, #-32]!
 118:	910003fd 	mov	x29, sp
 11c:	d2800080 	mov	x0, #0x4                   	// #4
 120:	f2a7e400 	movk	x0, #0x3f20, lsl #16
 124:	9400004d 	bl	258 <get32>
 128:	b9001fe0 	str	w0, [sp, #28]
 12c:	b9401fe0 	ldr	w0, [sp, #28]
 130:	12117000 	and	w0, w0, #0xffff8fff
 134:	b9001fe0 	str	w0, [sp, #28]
 138:	b9401fe0 	ldr	w0, [sp, #28]
 13c:	32130000 	orr	w0, w0, #0x2000
 140:	b9001fe0 	str	w0, [sp, #28]
 144:	b9401fe0 	ldr	w0, [sp, #28]
 148:	120e7000 	and	w0, w0, #0xfffc7fff
 14c:	b9001fe0 	str	w0, [sp, #28]
 150:	b9401fe0 	ldr	w0, [sp, #28]
 154:	32100000 	orr	w0, w0, #0x10000
 158:	b9001fe0 	str	w0, [sp, #28]
 15c:	b9401fe1 	ldr	w1, [sp, #28]
 160:	d2800080 	mov	x0, #0x4                   	// #4
 164:	f2a7e400 	movk	x0, #0x3f20, lsl #16
 168:	9400003a 	bl	250 <put32>
 16c:	52800001 	mov	w1, #0x0                   	// #0
 170:	d2801280 	mov	x0, #0x94                  	// #148
 174:	f2a7e400 	movk	x0, #0x3f20, lsl #16
 178:	94000036 	bl	250 <put32>
 17c:	d28012c0 	mov	x0, #0x96                  	// #150
 180:	94000038 	bl	260 <delay>
 184:	52980001 	mov	w1, #0xc000                	// #49152
 188:	d2801300 	mov	x0, #0x98                  	// #152
 18c:	f2a7e400 	movk	x0, #0x3f20, lsl #16
 190:	94000030 	bl	250 <put32>
 194:	d28012c0 	mov	x0, #0x96                  	// #150
 198:	94000032 	bl	260 <delay>
 19c:	52800001 	mov	w1, #0x0                   	// #0
 1a0:	d2801300 	mov	x0, #0x98                  	// #152
 1a4:	f2a7e400 	movk	x0, #0x3f20, lsl #16
 1a8:	9400002a 	bl	250 <put32>
 1ac:	52800021 	mov	w1, #0x1                   	// #1
 1b0:	d28a0080 	mov	x0, #0x5004                	// #20484
 1b4:	f2a7e420 	movk	x0, #0x3f21, lsl #16
 1b8:	94000026 	bl	250 <put32>
 1bc:	52800001 	mov	w1, #0x0                   	// #0
 1c0:	d28a0c00 	mov	x0, #0x5060                	// #20576
 1c4:	f2a7e420 	movk	x0, #0x3f21, lsl #16
 1c8:	94000022 	bl	250 <put32>
 1cc:	52800001 	mov	w1, #0x0                   	// #0
 1d0:	d28a0880 	mov	x0, #0x5044                	// #20548
 1d4:	f2a7e420 	movk	x0, #0x3f21, lsl #16
 1d8:	9400001e 	bl	250 <put32>
 1dc:	52800061 	mov	w1, #0x3                   	// #3
 1e0:	d28a0980 	mov	x0, #0x504c                	// #20556
 1e4:	f2a7e420 	movk	x0, #0x3f21, lsl #16
 1e8:	9400001a 	bl	250 <put32>
 1ec:	52800001 	mov	w1, #0x0                   	// #0
 1f0:	d28a0a00 	mov	x0, #0x5050                	// #20560
 1f4:	f2a7e420 	movk	x0, #0x3f21, lsl #16
 1f8:	94000016 	bl	250 <put32>
 1fc:	528021c1 	mov	w1, #0x10e                 	// #270
 200:	d28a0d00 	mov	x0, #0x5068                	// #20584
 204:	f2a7e420 	movk	x0, #0x3f21, lsl #16
 208:	94000012 	bl	250 <put32>
 20c:	52800061 	mov	w1, #0x3                   	// #3
 210:	d28a0c00 	mov	x0, #0x5060                	// #20576
 214:	f2a7e420 	movk	x0, #0x3f21, lsl #16
 218:	9400000e 	bl	250 <put32>
 21c:	d503201f 	nop
 220:	a8c27bfd 	ldp	x29, x30, [sp], #32
 224:	d65f03c0 	ret

0000000000000228 <kernel_main>:
 228:	a9bf7bfd 	stp	x29, x30, [sp, #-16]!
 22c:	910003fd 	mov	x29, sp
 230:	97ffffb9 	bl	114 <uart_init>
 234:	90000000 	adrp	x0, 0 <_start>
 238:	910a0000 	add	x0, x0, #0x280
 23c:	97ffff9f 	bl	b8 <uart_send_string>
 240:	97ffff8e 	bl	78 <uart_recv>
 244:	12001c00 	and	w0, w0, #0xff
 248:	97ffff7a 	bl	30 <uart_send>
 24c:	17fffffd 	b	240 <kernel_main+0x18>

0000000000000250 <put32>:
 250:	b9000001 	str	w1, [x0]
 254:	d65f03c0 	ret

0000000000000258 <get32>:
 258:	b9400000 	ldr	w0, [x0]
 25c:	d65f03c0 	ret

0000000000000260 <delay>:
 260:	f1000400 	subs	x0, x0, #0x1
 264:	54ffffe1 	b.ne	260 <delay>  // b.any
 268:	d65f03c0 	ret

000000000000026c <memzero>:
 26c:	f800841f 	str	xzr, [x0], #8
 270:	f1002021 	subs	x1, x1, #0x8
 274:	54ffffcc 	b.gt	26c <memzero>
 278:	d65f03c0 	ret

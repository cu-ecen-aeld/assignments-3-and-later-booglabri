<pre>
./buildroot/output/target/lib/modules/5.15.18/extra/faulty.ko:     file format elf64-littleaarch64


Disassembly of section .text:

0000000000000000 <faulty_write>:
   0:	d503245f 	bti	c
   4:	d2800001 	mov	x1, #0x0                   	// #0
   8:	d2800000 	mov	x0, #0x0                   	// #0
   c:	d503233f 	paciasp
  10:	d50323bf 	autiasp
  14:	b900003f 	str	wzr, [x1]
  18:	d65f03c0 	ret
  1c:	d503201f 	nop

0000000000000020 <faulty_read>:
  20:	d503233f 	paciasp
  24:	a9bd7bfd 	stp	x29, x30, [sp, #-48]!
  28:	d5384100 	mrs	x0, sp_el0
  2c:	910003fd 	mov	x29, sp
  30:	a90153f3 	stp	x19, x20, [sp, #16]
  34:	aa0103f4 	mov	x20, x1
  38:	aa0203f3 	mov	x19, x2
  3c:	f941f801 	ldr	x1, [x0, #1008]
  40:	f90017e1 	str	x1, [sp, #40]
  44:	d2800001 	mov	x1, #0x0                   	// #0
  48:	d2800282 	mov	x2, #0x14                  	// #20
  4c:	52801fe1 	mov	w1, #0xff                  	// #255
  50:	910093e0 	add	x0, sp, #0x24
  54:	94000000 	bl	0 <memset>
  58:	d5384100 	mrs	x0, sp_el0
  5c:	b9402401 	ldr	w1, [x0, #36]
  60:	f100127f 	cmp	x19, #0x4
  64:	d2800082 	mov	x2, #0x4                   	// #4
  68:	9a829273 	csel	x19, x19, x2, ls  // ls = plast
  6c:	37a80361 	tbnz	w1, #21, d8 <faulty_read+0xb8>
  70:	f9400000 	ldr	x0, [x0]
  74:	aa1403e2 	mov	x2, x20
  78:	7206001f 	tst	w0, #0x4000000
  7c:	540002e1 	b.ne	d8 <faulty_read+0xb8>  // b.any
  80:	b2409be1 	mov	x1, #0x7fffffffff          	// #549755813887
  84:	aa0103e0 	mov	x0, x1
  88:	ab130042 	adds	x2, x2, x19
  8c:	9a8083e0 	csel	x0, xzr, x0, hi  // hi = pmore
  90:	da9f3042 	csinv	x2, x2, xzr, cc  // cc = lo, ul, last
  94:	fa00005f 	sbcs	xzr, x2, x0
  98:	9a9f87e2 	cset	x2, ls  // ls = plast
  9c:	aa1303e0 	mov	x0, x19
  a0:	b5000222 	cbnz	x2, e4 <faulty_read+0xc4>
  a4:	7100001f 	cmp	w0, #0x0
  a8:	d5384101 	mrs	x1, sp_el0
  ac:	93407c00 	sxtw	x0, w0
  b0:	9a931000 	csel	x0, x0, x19, ne  // ne = any
  b4:	f94017e3 	ldr	x3, [sp, #40]
  b8:	f941f822 	ldr	x2, [x1, #1008]
  bc:	eb020063 	subs	x3, x3, x2
  c0:	d2800002 	mov	x2, #0x0                   	// #0
  c4:	54000221 	b.ne	108 <faulty_read+0xe8>  // b.any
  c8:	a94153f3 	ldp	x19, x20, [sp, #16]
  cc:	a8c37bfd 	ldp	x29, x30, [sp], #48
  d0:	d50323bf 	autiasp
  d4:	d65f03c0 	ret
  d8:	9340de82 	sbfx	x2, x20, #0, #56
  dc:	8a020282 	and	x2, x20, x2
  e0:	17ffffe8 	b	80 <faulty_read+0x60>
  e4:	9340de82 	sbfx	x2, x20, #0, #56
  e8:	8a020282 	and	x2, x20, x2
  ec:	ea21005f 	bics	xzr, x2, x1
  f0:	9a9f0280 	csel	x0, x20, xzr, eq  // eq = none
  f4:	d503229f 	csdb
  f8:	910093e1 	add	x1, sp, #0x24
  fc:	aa1303e2 	mov	x2, x19
 100:	94000000 	bl	0 <__arch_copy_to_user>
 104:	17ffffe8 	b	a4 <faulty_read+0x84>
 108:	94000000 	bl	0 <__stack_chk_fail>
 10c:	d503201f 	nop

0000000000000110 <faulty_init>:
 110:	d503233f 	paciasp
 114:	a9be7bfd 	stp	x29, x30, [sp, #-32]!
 118:	90000004 	adrp	x4, 0 <faulty_write>
 11c:	910003fd 	mov	x29, sp
 120:	f9000bf3 	str	x19, [sp, #16]
 124:	90000013 	adrp	x19, 0 <faulty_write>
 128:	b9400260 	ldr	w0, [x19]
 12c:	90000003 	adrp	x3, 0 <faulty_write>
 130:	91000084 	add	x4, x4, #0x0
 134:	91000063 	add	x3, x3, #0x0
 138:	52802002 	mov	w2, #0x100                 	// #256
 13c:	52800001 	mov	w1, #0x0                   	// #0
 140:	94000000 	bl	0 <__register_chrdev>
 144:	37f800a0 	tbnz	w0, #31, 158 <faulty_init+0x48>
 148:	b9400261 	ldr	w1, [x19]
 14c:	350000e1 	cbnz	w1, 168 <faulty_init+0x58>
 150:	b9000260 	str	w0, [x19]
 154:	52800000 	mov	w0, #0x0                   	// #0
 158:	f9400bf3 	ldr	x19, [sp, #16]
 15c:	a8c27bfd 	ldp	x29, x30, [sp], #32
 160:	d50323bf 	autiasp
 164:	d65f03c0 	ret
 168:	52800000 	mov	w0, #0x0                   	// #0
 16c:	f9400bf3 	ldr	x19, [sp, #16]
 170:	a8c27bfd 	ldp	x29, x30, [sp], #32
 174:	d50323bf 	autiasp
 178:	d65f03c0 	ret
 17c:	d503201f 	nop

0000000000000180 <cleanup_module>:
 180:	d503233f 	paciasp
 184:	90000000 	adrp	x0, 0 <faulty_write>
 188:	a9bf7bfd 	stp	x29, x30, [sp, #-16]!
 18c:	52802002 	mov	w2, #0x100                 	// #256
 190:	52800001 	mov	w1, #0x0                   	// #0
 194:	910003fd 	mov	x29, sp
 198:	b9400000 	ldr	w0, [x0]
 19c:	90000003 	adrp	x3, 0 <faulty_write>
 1a0:	91000063 	add	x3, x3, #0x0
 1a4:	94000000 	bl	0 <__unregister_chrdev>
 1a8:	a8c17bfd 	ldp	x29, x30, [sp], #16
 1ac:	d50323bf 	autiasp
 1b0:	d65f03c0 	ret

Disassembly of section .plt:

0000000000000000 <.plt>:
	...

Disassembly of section .text.ftrace_trampoline:

0000000000000000 <.text.ftrace_trampoline>:
	...
</pre>
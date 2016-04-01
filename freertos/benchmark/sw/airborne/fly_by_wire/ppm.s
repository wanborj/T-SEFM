	.file	"ppm.c"
	.text
	.p2align 4,,15
	.globl	__vector_34
	.type	__vector_34, @function
__vector_34:
.LFB5:
	.cfi_startproc
	movzbl	state.1282, %eax
	movzwl	sfr_offset+38, %edx
	movzwl	last.1279, %ecx
	testb	%al, %al
	movw	%dx, last.1279
	jne	.L2
	movzbl	sfr_offset+36, %eax
	movl	%eax, %edx
	subb	sync_start.1283, %dl
	movb	%al, sync_start.1283
	cmpb	$109, %dl
	jbe	.L1
	movb	$1, state.1282
	ret
	.p2align 4,,7
	.p2align 3
.L2:
	subw	%cx, %dx
	leal	-11200(%edx), %ecx
	cmpw	$25600, %cx
	ja	.L6
	movzbl	%al, %ecx
	cmpb	$8, %al
	movw	%dx, ppm_pulses-2(%ecx,%ecx)
	ja	.L8
	addl	$1, %eax
	movb	%al, state.1282
.L1:
	rep
	ret
	.p2align 4,,7
	.p2align 3
.L8:
	movb	$1, ppm_valid
.L6:
	movzbl	sfr_offset+36, %eax
	movb	$0, state.1282
	movb	%al, sync_start.1283
	ret
	.cfi_endproc
.LFE5:
	.size	__vector_34, .-__vector_34
	.p2align 4,,15
	.globl	last_radio_from_ppm
	.type	last_radio_from_ppm, @function
last_radio_from_ppm:
.LFB6:
	.cfi_startproc
	subl	$8, %esp
	.cfi_def_cfa_offset 12
	movzwl	ppm_pulses, %eax
	fnstcw	4(%esp)
	subw	$16000, %ax
	movw	%ax, 6(%esp)
	movzwl	4(%esp), %eax
	filds	6(%esp)
	fmuls	.LC0
	movb	$12, %ah
	movw	%ax, 2(%esp)
	fldcw	2(%esp)
	fistps	6(%esp)
	fldcw	4(%esp)
	movzwl	6(%esp), %eax
	cmpw	$9600, %ax
	movw	%ax, last_radio
	jle	.L10
	movw	$9600, last_radio
.L11:
	movzwl	ppm_pulses+2, %eax
	subw	$25600, %ax
	movw	%ax, 6(%esp)
	filds	6(%esp)
	fldcw	2(%esp)
	fistps	6(%esp)
	fldcw	4(%esp)
	movzwl	6(%esp), %eax
	cmpw	$9600, %ax
	movw	%ax, last_radio+2
	jle	.L12
	movw	$9600, last_radio+2
.L13:
	movzwl	ppm_pulses+4, %eax
	subw	$25600, %ax
	movw	%ax, 6(%esp)
	filds	6(%esp)
	fldcw	2(%esp)
	fistps	6(%esp)
	fldcw	4(%esp)
	movzwl	6(%esp), %eax
	cmpw	$9600, %ax
	movw	%ax, last_radio+4
	jle	.L14
	movw	$9600, last_radio+4
.L15:
	movzwl	ppm_pulses+6, %eax
	subw	$25600, %ax
	movw	%ax, 6(%esp)
	filds	6(%esp)
	fldcw	2(%esp)
	fistps	6(%esp)
	fldcw	4(%esp)
	movzwl	6(%esp), %eax
	cmpw	$9600, %ax
	movw	%ax, last_radio+6
	jle	.L16
	movw	$9600, last_radio+6
.L17:
	movzwl	ppm_pulses+8, %eax
	filds	avg_last_radio+8
	subw	$25600, %ax
	movw	%ax, 6(%esp)
	movzwl	ppm_pulses+10, %eax
	filds	6(%esp)
	flds	.LC1
	fmul	%st, %st(1)
	fxch	%st(2)
	subw	$25600, %ax
	faddp	%st, %st(1)
	fldcw	2(%esp)
	fistps	6(%esp)
	fldcw	4(%esp)
	movzwl	6(%esp), %ecx
	movw	%ax, 6(%esp)
	movzwl	ppm_pulses+12, %eax
	filds	avg_last_radio+10
	filds	6(%esp)
	fmul	%st(2), %st
	subw	$25600, %ax
	movw	%cx, avg_last_radio+8
	faddp	%st, %st(1)
	fldcw	2(%esp)
	fistps	6(%esp)
	fldcw	4(%esp)
	movzwl	6(%esp), %edx
	movw	%ax, 6(%esp)
	movzwl	ppm_pulses+14, %eax
	filds	avg_last_radio+12
	filds	6(%esp)
	fmul	%st(2), %st
	subw	$25600, %ax
	movw	%ax, 6(%esp)
	movzwl	ppm_pulses+16, %eax
	movw	%dx, avg_last_radio+10
	faddp	%st, %st(1)
	subw	$25600, %ax
	fldcw	2(%esp)
	fistps	avg_last_radio+12
	fldcw	4(%esp)
	filds	avg_last_radio+14
	filds	6(%esp)
	fmul	%st(2), %st
	faddp	%st, %st(1)
	fldcw	2(%esp)
	fistps	avg_last_radio+14
	fldcw	4(%esp)
	filds	avg_last_radio+16
	movw	%ax, 6(%esp)
	filds	6(%esp)
	fmulp	%st, %st(2)
	movzbl	avg_cpt.1288, %eax
	addl	$1, %eax
	faddp	%st, %st(1)
	cmpb	$10, %al
	movb	%al, avg_cpt.1288
	fldcw	2(%esp)
	fistps	avg_last_radio+16
	fldcw	4(%esp)
	je	.L29
	addl	$8, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
.L10:
	.cfi_restore_state
	testw	%ax, %ax
	jns	.L11
	movw	$0, last_radio
	jmp	.L11
	.p2align 4,,7
	.p2align 3
.L16:
	cmpw	$-9600, %ax
	jge	.L17
	movw	$-9600, last_radio+6
	jmp	.L17
	.p2align 4,,7
	.p2align 3
.L14:
	cmpw	$-9600, %ax
	jge	.L15
	movw	$-9600, last_radio+4
	jmp	.L15
	.p2align 4,,7
	.p2align 3
.L12:
	cmpw	$-9600, %ax
	jge	.L13
	movw	$-9600, last_radio+2
	jmp	.L13
	.p2align 4,,7
	.p2align 3
.L29:
	cmpw	$9600, %cx
	movb	$0, avg_cpt.1288
	movw	%cx, last_radio+8
	movw	$0, avg_last_radio+8
	jg	.L30
	cmpw	$-9600, %cx
	jge	.L20
	movw	$-9600, last_radio+8
.L20:
	cmpw	$9600, %dx
	movw	%dx, last_radio+10
	movw	$0, avg_last_radio+10
	jle	.L21
	movw	$9600, last_radio+10
.L22:
	movzwl	avg_last_radio+12, %eax
	movw	$0, avg_last_radio+12
	cmpw	$9600, %ax
	movw	%ax, last_radio+12
	jle	.L23
	movw	$9600, last_radio+12
.L24:
	movzwl	avg_last_radio+14, %eax
	movw	$0, avg_last_radio+14
	cmpw	$9600, %ax
	movw	%ax, last_radio+14
	jle	.L25
	movw	$9600, last_radio+14
.L26:
	movzwl	avg_last_radio+16, %eax
	movw	$0, avg_last_radio+16
	cmpw	$9600, %ax
	movw	%ax, last_radio+16
	jle	.L27
	movw	$9600, last_radio+16
.L28:
	movb	$1, last_radio_contains_avg_channels
	addl	$8, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
.L30:
	.cfi_restore_state
	movw	$9600, last_radio+8
	jmp	.L20
	.p2align 4,,7
	.p2align 3
.L27:
	cmpw	$-9600, %ax
	jge	.L28
	movw	$-9600, last_radio+16
	jmp	.L28
	.p2align 4,,7
	.p2align 3
.L25:
	cmpw	$-9600, %ax
	jge	.L26
	movw	$-9600, last_radio+14
	jmp	.L26
	.p2align 4,,7
	.p2align 3
.L23:
	cmpw	$-9600, %ax
	jge	.L24
	movw	$-9600, last_radio+12
	jmp	.L24
	.p2align 4,,7
	.p2align 3
.L21:
	cmpw	$-9600, %dx
	jge	.L22
	movw	$-9600, last_radio+10
	jmp	.L22
	.cfi_endproc
.LFE6:
	.size	last_radio_from_ppm, .-last_radio_from_ppm
	.comm	ppm_valid,1,1
	.globl	last_radio_contains_avg_channels
	.bss
	.type	last_radio_contains_avg_channels, @object
	.size	last_radio_contains_avg_channels, 1
last_radio_contains_avg_channels:
	.zero	1
	.comm	avg_last_radio,18,2
	.comm	last_radio,18,2
	.comm	ppm_pulses,18,2
	.comm	sfr_offset,128,32
	.local	last.1279
	.comm	last.1279,2,2
	.local	state.1282
	.comm	state.1282,1,1
	.local	sync_start.1283
	.comm	sync_start.1283,1,1
	.local	avg_cpt.1288
	.comm	avg_cpt.1288,1,1
	.section	.rodata.cst4,"aM",@progbits,4
	.align 4
.LC0:
	.long	1056964608
	.align 4
.LC1:
	.long	1036831949
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits

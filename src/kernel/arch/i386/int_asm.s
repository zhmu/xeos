/*
 * int_asm.s - XeOS i386 assembly interrupts code
 * (c) 2002 Rink Springer, BSD licensed
 *
 * This code is heavily based on Yoctix (src/arch/i386/interrupts_asm.s),
 * (c) 1999 Anders Gavare.
 *
 * This code will actually handle IRQ's and Exceptions.
 *
 */
.text
.global exc0_asm
.global exc1_asm
.global exc2_asm
.global exc3_asm
.global exc4_asm
.global exc5_asm
.global exc6_asm
.global exc7_asm
.global exc8_asm
.global exc9_asm
.global exca_asm
.global excb_asm
.global excc_asm
.global excd_asm
.global exce_asm
.global excf_asm

.global irq0_asm
.global irq1_asm
.global irq2_asm
.global irq3_asm
.global irq4_asm
.global irq5_asm
.global irq6_asm
.global irq7_asm
.global irq8_asm
.global irq9_asm
.global irqa_asm
.global irqb_asm
.global irqc_asm
.global irqd_asm
.global irqe_asm
.global irqf_asm

exc0_asm:
	push	%eax				/* dummy error code */
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs
	push	%ss

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	pushl	$0x0
	call	exception_handler
	addl	$8, %esp			/* skip ss */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	addl	$4, %esp			/* ditch dummy error code */
	iret

exc1_asm:
	push	%eax				/* dummy error code */
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs
	push	%ss

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	pushl	$0x1
	call	exception_handler
	addl	$8, %esp			/* skip ss */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	addl	$4, %esp			/* ditch dummy error code */
	iret

exc2_asm:
	push	%eax				/* dummy error code */
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs
	push	%ss

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	pushl	$0x2
	call	exception_handler
	addl	$8, %esp			/* skip ss */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	addl	$4, %esp			/* ditch dummy error code */
	iret

exc3_asm:
	push	%eax				/* dummy error code */
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs
	push	%ss

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	pushl	$0x3
	call	exception_handler
	addl	$8, %esp			/* skip ss */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	addl	$4, %esp			/* ditch dummy error code */
	iret

exc4_asm:
	push	%eax				/* dummy error code */
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs
	push	%ss

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	pushl	$0x4
	call	exception_handler
	addl	$8, %esp			/* skip ss */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	addl	$4, %esp			/* ditch dummy error code */
	iret

exc5_asm:
	push	%eax				/* dummy error code */
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs
	push	%ss

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	pushl	$0x5
	call	exception_handler
	addl	$8, %esp			/* skip ss */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	addl	$4, %esp			/* ditch dummy error code */
	iret

exc6_asm:
	push	%eax				/* dummy error code */
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs
	push	%ss

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	pushl	$0x6
	call	exception_handler
	addl	$8, %esp			/* skip ss */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	addl	$4, %esp			/* ditch dummy error code */
	iret

exc7_asm:
	push	%eax				/* dummy error code */
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs
	push	%ss

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	pushl	$0x7
	call	exception_handler
	addl	$8, %esp			/* skip ss */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	addl	$4, %esp			/* ditch dummy error code */
	iret

exc8_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs
	push	%ss

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	pushl	$0x8
	call	exception_handler
	addl	$8, %esp			/* skip ss */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

exc9_asm:
	push	%eax				/* dummy error code */
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%ss

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	pushl	$0x9
	call	exception_handler
	addl	$8, %esp			/* skip ss */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	addl	$4, %esp			/* ditch dummy error code */
	iret

exca_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs
	push	%ss

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	pushl	$0xa
	call	exception_handler
	addl	$8, %esp			/* skip ss */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

excb_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs
	push	%ss

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	pushl	$0xb
	call	exception_handler
	addl	$8, %esp			/* skip ss */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

excc_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs
	push	%ss

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	pushl	$0xc
	call	exception_handler
	addl	$8, %esp			/* skip ss */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

excd_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs
	push	%ss

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	pushl	$0xd
	call	exception_handler
	addl	$8, %esp			/* skip ss */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

/*
 * exception #e (pagefaults) are handeled differently
 *
 */
exce_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	mov	%cr2, %eax
	push	%eax				/* store cr2 */

	call	pagefault_handler
	addl	$4, %esp			/* ditch saved cr2 */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal

	addl	$4, %esp			/* ditch error code */
	iret

excf_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs
	push	%ss

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	pushl	$0xf
	call	exception_handler
	addl	$8, %esp			/* skip ss */

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irq0_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20

	pushl	$0x0
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irq1_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20

	pushl	$0x1
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irq2_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20

	pushl	$0x2
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irq3_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20

	pushl	$0x3
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irq4_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20

	pushl	$0x4
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irq5_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20

	pushl	$0x5
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irq6_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20

	pushl	$0x6
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irq7_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20

	pushl	$0x7
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irq8_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20
	outb	%al, $0xa0

	pushl	$0x8
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irq9_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20
	outb	%al, $0xa0

	pushl	$0x9
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irqa_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20
	outb	%al, $0xa0

	pushl	$0xa
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irqb_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20
	outb	%al, $0xa0

	pushl	$0xb
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irqc_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20
	outb	%al, $0xa0

	pushl	$0xc
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irqd_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20
	outb	%al, $0xa0

	pushl	$0xd
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irqe_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20
	outb	%al, $0xa0

	pushl	$0xe
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

irqf_asm:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es

	movb	$0x20, %al
	outb	%al, $0x20
	outb	%al, $0xa0

	pushl	$0xf
	call	irq_handler
	addl	$4, %esp

	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	iret

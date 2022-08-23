/*
 * config.h - XeOS i386 Specific Configuration
 * (c) 2002, 2003 Rink Springer, BSD
 *
 * This file contains i386 specific configuration options.
 *
 */
#ifndef __CONFIG_I386_H__
#define __CONFIG_I386_H__

/* PAGEDIR_ADDRESS is the address where we store our page directory  */
#define PAGEDIR_ADDRESS		(0x1000)

/* GDT_ADDRESS is the address where we store our GDT */
#define GDT_ADDRESS		(PAGEDIR_ADDRESS + PAGESIZE)

/* IDT_ADDRESS is the address where we store our IDT */
#define IDT_ADDRESS		(GDT_ADDRESS + (8 * (8 + (MAX_THREADS))))

/* THREAD_ADDRESS is the address where we store the process info */
#define THREAD_ADDRESS		((IDT_ADDRESS + (8 * 256)))

/* KSTACK_SIZE is the size of the kernel stack */
#define KSTACK_SIZE		1024

/* KSTACK_ADDRESS is the address where we store the process kernel stack */
#define KSTACK_ADDRESS		((THREAD_ADDRESS + KSTACK_SIZE + \
                                 (MAX_THREADS * sizeof (struct THREAD))) + \
				 (MAX_THREADS * KSTACK_SIZE))

/* END_ADDRESS is the end eddress of the kernel information */
#define END_ADDRESS		(KSTACK_ADDRESS)

/* KCODE32_SEL is the selector where kernel code resides in */
#define KCODE32_SEL		0x08

/* KDATA32_SEL is the selector where kernel data resides in */
#define KDATA32_SEL		0x10

/* UCODE32_SEL is the selector where user code resides in */
#define UCODE32_SEL		0x18

/* UDATA32_SEL is the selector where user data resides in */
#define UDATA32_SEL		0x20

/* KERNELTSS_SEL is the kernel's TSS selector */
#define KERNEL_TSS_SEL		5

/* TSS_SEL is the selector where our TSS-es begin */
#define TSS_SEL			(KERNEL_TSS_SEL + 1)

/* KERNEL_LOAD_ADDR is the kernel loading address */
#define KERNEL_LOAD_ADDR	0x100000

/* OSRESERVED_ADDR is the OS reserved data */
#define OSRESERVED_ADDR		0xf800000

/* OSRESERVE_END is the end of the OS reserved data */
#define OSRESERVED_END 		0xffff000

/* PAGESIZE is the pagesize for this architecture */
#define PAGESIZE		4096

/* KSTACK is the kernel stack size */
#define KSTACK			(PAGESIZE*2)

/* PSTACK is the process stack */
#define PSTACK			(PAGESIZE*2)

/* HZ is the frequency of the timer */
#define HZ			1024

/* SWITCH_HZ is the switching frequency */
#define SWITCH_HZ		50

/* SYSCALL_INT is the interrupt we use for system calls */
#define SYSCALL_INT		0x30

#endif

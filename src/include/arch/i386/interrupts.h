/*
 * interrupts.h - XeOS i386 Interrupt Code
 * (c) 2002, 2003 Rink Springer, BSD
 *
 * This include file 
 *
 */
#include <sys/types.h>

#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

/* I386_INT_xxx define the interrupt type */
#define I386_INT_GATE		0x0E

#ifdef __KERNEL
#define	ENABLE 1
#define	DISABLE 1

int	arch_interrupts (int enable);

#endif /* __KERNEL */

#endif /* __INTERRUPTS_H__ */

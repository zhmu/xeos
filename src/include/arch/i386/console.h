/*
 * console.h - XeOS i386 console declarations
 * (c) 2002 Rink Springer, BSD
 *
 * This is a generic include file which describes the i386 console.
 *
 */
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <sys/types.h>

#ifdef __KERNEL
void arch_console_init();
void arch_console_putchar (uint8_t ch);
#endif /* __KERNEL */

#endif /* __CONSOLE_H__ */

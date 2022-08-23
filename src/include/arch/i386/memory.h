/*
 * memory.h - XeOS Memory Probe Stuff
 * (c) 2002, 2003 Rink Springer, BSD
 *
 * This is a generic include file which describes the memory amount.
 *
 */
#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <sys/types.h>

#ifdef __KERNEL
extern size_t mapped_kernel;
extern size_t highest_addressable_byte;
extern size_t first_addressable_high_byte;
extern size_t lowest_addressable_free_byte;

void arch_add_all_memory();
#endif /* __KERNEL */

#define MAP_MEMORY(x) ((x)+mapped_kernel)

#endif

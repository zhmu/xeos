/*
 * init.h - XeOS Machine Dependant Initialization Code
 * (c) 2003 Rink Springer, BSD
 *
 * This include file is for the machine dependant initialization code.
 *
 */
#include <sys/types.h>

#ifndef __MD_INIT_H__
#define __MD_INIT_H__

#ifdef __KERNEL
void arch_init();
#endif /* __KERNEL */

#endif /* __MD_INIT_H__ */

/*
 * paging.h - XeOS Virtual Memory Manager
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This is the include file for the paging stuff.
 *
 */
#include <sys/types.h>

#ifndef __ARCH_VM_H__
#define __ARCH_VM_H__

#ifdef __KERNEL
void arch_vm_setup_thread (struct TSS*, char* pagedir);
int arch_vm_map (struct THREAD* t, addr_t log, addr_t phys, size_t len);
addr_t arch_vm_getmap();
void arch_vm_setmap(addr_t addr);
void arch_vm_setthreadmap(struct THREAD* t);
#endif /* __KERNEL */

#endif /* __ARCH_VM_H__ */

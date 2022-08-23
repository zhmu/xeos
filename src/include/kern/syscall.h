/*
 * syscall.h - XeOS Syscalls include file
 * (c) 2002, 2003 Rink Springer, BSD
 *
 * This include file has the syscall declarations.
 *
 */
#include <sys/mailbox.h>
#include <sys/types.h>

#ifndef __SYSCALL_H__
#define __SYSCALL_H__

/* syscall categories */
#define SYS_thread			0
#define SYS_io				1
#define SYS_module			2
#define SYS_mbox			3
#define SYS_network			4
#define SYS_filesys			5
#define SYS_debug			0xFFFFFFFF
#define SYS_user			8192

/* thread services */
#define SYS_thread_exit			0
#define SYS_thread_suspend		1
#define SYS_thread_getcurthread		2
#define SYS_thread_kill			3
#define SYS_thread_switch		4
#define SYS_thread_find_syscall		5

/* i/o services */
#define SYS_io_putchar			0
#define SYS_io_getchar			1
#define SYS_io_devread			2

/* module services */
#define SYS_module_register_irq		0
#define SYS_module_register_dev		1
#define SYS_module_register_pio		2
#define SYS_module_register_lowmem	3
#define SYS_module_register_tty		4
#define SYS_module_register_syscall	5
#define SYS_module_register_filesystem	6

/* mailbox services */
#define SYS_mbox_add			0
#define SYS_mbox_register		1
#define SYS_mbox_fetch			2

/* networking services */
#define SYS_network_add_packet		0
#define SYS_network_register		1
#define SYS_network_send_packet		2

/* filesystem services */
#define SYS_filesys_mount		0

/* debugging services */
#define SYS_debug_argtest		65535
#define SYS_debug_dumpthreads		0
#define SYS_debug_dumpdevices		1
#define SYS_debug_dumpio		2
#define SYS_debug_dumpnetwork		3
#define SYS_debug_dumpsyscall		4
#define SYS_debug_dumpfilesys		5

/* IPv4 syscalls */
#define MOD_ipv4_debug			0

/* user-installable system calls */
struct USERSYSCALL {
	uint32_t	no;
	char*		name;
	struct THREAD*	thread;
  	size_t (*func)(struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10);
	struct	USERSYSCALL*	next;
};

#ifdef __KERNEL
int  syscall_register (struct THREAD* t, char* name, void* func);
void syscall_dump();

struct USERSYSCALL* syscall_search_by_name (char* name);
#endif /* __KERNEL */

#ifdef __LIBC
uint32_t	__SYSCALL (uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6);
#endif /* __LIBC */

#endif /* __SYSCALL_H__ */

/* vim:set ts=8 sw=8: */

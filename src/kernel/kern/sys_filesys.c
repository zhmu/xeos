/*
 * sys_filesys.c - XeOS Filesystem Syscalls
 * (c) 2003 Rink Springer, BSD licensed
 *
 * This code will handle XeOS filesystem calls.
 *
 */
#include <kern/syscall.h>
#include <sys/device.h>
#include <sys/thread.h>
#include <sys/filesys.h>
#include <sys/types.h>

/*
 * filesys_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10)
 *
 * This will handle networking syscalls.
 *
 */
uint32_t
filesys_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10) {
	switch (r1) {
		case SYS_filesys_mount: /* mount a filesystem */
		                        return fs_mount ((char*)r2, (char*)r3, (char*)r4, r5);
	}

	/* ??? */
	return 0;
}

/* vim:set ts=2 sw=2: */

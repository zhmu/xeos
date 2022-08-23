/*
 * sys_debug.c - XeOS Debugging Syscalls
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will handle XeOS mailbox syscalls.
 *
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/filesys.h>
#include <sys/thread.h>
#include <sys/pio.h>
#include <sys/mailbox.h>
#include <sys/network.h>
#include <kern/syscall.h>
#include <lib/lib.h>

/*
 * debug_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10)
 *
 * This will handle debugging sscalls.
 *
 */
uint32_t
debug_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10) {
	switch (r1) {
			  case SYS_debug_argtest:	kprintf ("debug_syscall(): ARGTEST: r1=%x, r2=%x, r3=%x, r4=%x, r5=%x, r6=%x\n", r1, r2, r3, r4, r5, r6);
		                            return 0x29b;
		case SYS_debug_dumpthreads: /* dump threads */
		                            thread_dump();
		                            return 1;
		case SYS_debug_dumpdevices: /* dump devices */
		                            device_dump();
		                            return 1;
		     case SYS_debug_dumpio: /* dump io */
		                            pio_dump();
		                            return 1;
	  case SYS_debug_dumpnetwork: /* dump network */
		                            network_dump();
		                            return 1;
	  case SYS_debug_dumpsyscall: /* dump syscall */
		                            syscall_dump();
		                            return 1;
	  case SYS_debug_dumpfilesys: /* dump filesystems */
		                            fs_dump();
		                            return 1;
	}

	/* ??? */
	return 0;
}

/* vim:set ts=2 sw=2: */

/*
 * sys_mbox.c - XeOS Mailbox Syscalls
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will handle XeOS mailbox syscalls.
 *
 */
#include <kern/syscall.h>
#include <sys/thread.h>
#include <sys/mailbox.h>
#include <sys/types.h>

/*
 * mbox_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10)
 *
 * This will handle mailbox syscalls.
 *
 */
uint32_t
mbox_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10) {
	switch (r1) {
			case SYS_mbox_add: /* add data to a mailbox */
	 	  	                 mailbox_add ((MAILBOX*)r2, (void*)r3, (size_t)r4);
												 return 1;
 case SYS_mbox_register: /* register this thread as mailbox handler */
	 	  	                 mailbox_register ((MAILBOX*)r2, t);
												 return 1;
		case SYS_mbox_fetch: /* fetch data from the mailbox */
	 	  	                 return mailbox_fetch ((MAILBOX*)r2, (void*)r3, (size_t*)r4);
	}

	/* ??? */
	return 0;
}

/* vim:set ts=2 sw=2: */

/*
 * sys_proc.c - XeOS Process Syscalls
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will handle XeOS thread syscalls.
 *
 */
#include <kern/syscall.h>
#include <sys/thread.h>
#include <sys/irq.h>
#include <sys/types.h>

/*
 * thread_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10)
 *
 * This will handle thread syscalls.
 *
 */
uint32_t
thread_syscall (struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10) {
	struct USERSYSCALL* us;

	switch (r1) {
		    case SYS_thread_exit: /* terminate the thread */
		                          thread_exit (t);
		                          return 1;
		 case SYS_thread_suspend: /* suspend the thread */
		                          thread_suspend (t);
															return 1;
case SYS_thread_getcurthread: /* return the current thread */
		                          return (uint32_t)t;
		    case SYS_thread_kill: /* kill a thread. is it valid? */
		                          if (thread_isvalid ((struct THREAD*)r2)) {
																/* yes. zap it */
		                          	thread_exit ((struct THREAD*)r2);
																return 1;
															}

															/* sorry */
															return 0;
		  case SYS_thread_switch: /* switch to another thread */
															thread_switch (NULL);
															return 1;
case SYS_thread_find_syscall: /* searches a syscall by name */
															us = syscall_search_by_name ((char*)r2);
															if (us == NULL)
																/* this failed. too bad */
																return 0;

															/* return the number */
															return us->no;
	}

	/* ??? */
	return 0;
}

/* vim:set ts=2 sw=2: */

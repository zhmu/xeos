/*
 * syscall.c - XeOSs Syscalls
 * (c) 2002 Rink Springer, BSD licensed
 *
 * This code was inspired by Yoctix, OpenBSD, FreeBSD and Yoctix.
 *
 * This code will handle XeOS syscalls.
 *
 */
#include <sys/mailbox.h>
#include <sys/thread.h>
#include <sys/kmalloc.h>
#include <sys/tty.h>
#include <kern/syscall.h>
#include <lib/lib.h>
#include <md/vm.h>

void syscall_asm();
struct USERSYSCALL* u_rootsyscall = NULL;

#define xSYSCALL_DEBUG

/* macros for readability */
#define DEF_SYSCALL_PROC(x) uint32_t (x)(struct THREAD*,uint32_t,uint32_t, \
		uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
#define SYSCALL_PROC(x) (x)(t_current,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10)

/* declare all syscalls */
DEF_SYSCALL_PROC(thread_syscall)
DEF_SYSCALL_PROC(io_syscall)
DEF_SYSCALL_PROC(module_syscall)
DEF_SYSCALL_PROC(mbox_syscall)
DEF_SYSCALL_PROC(network_syscall)
DEF_SYSCALL_PROC(filesys_syscall)
DEF_SYSCALL_PROC(debug_syscall)

/*
 *
 * syscall_check_addr (addr_t* addr)
 *
 * This will check and possible fix address [addr]. It will return zero
 * on an invalid address or non-zero on success.
 *
 */
int
syscall_check_addr (addr_t* addr) {
/**addr += mapped_kernel;*/
	return 1;
}

/*
 * syscall_search_by_name (char* name)
 * 
 * This will scan all registered user system calls for [name]. It will return
 * a pointer to the entry on success or NULL on failure.
 *
 */
struct USERSYSCALL*
syscall_search_by_name (char* name) {
	struct USERSYSCALL* us = u_rootsyscall;

	/* keep scanning */
	while (us) {
		/* match? */
		if (!kstrcmp (us->name, name))
			/* yes! return it */
			return us;

		/* next */
		us = us->next;
	}

	/* no match */
	return NULL;
}

/*
 * syscall_search_by_number (uint32_t no)
 * 
 * This will scan all registered user system calls for [no]. It will return
 * a pointer to the entry on success or NULL on failure.
 *
 */
struct USERSYSCALL*
syscall_search_by_number (uint32_t no) {
	struct USERSYSCALL* us = u_rootsyscall;

	/* keep scanning */
	while (us) {
		/* match? */
		if (us->no == no)
			/* yes! return it */
			return us;

		/* next */
		us = us->next;
	}

	/* no match */
	return NULL;
}

/*
 * syscall_getfreeno()
 *
 * This will return an available systemcall number.
 *
 */
uint32_t
syscall_getfreeno() {
	uint32_t no = SYS_user;
	struct USERSYSCALL* uc;
	int ok;

	while (1) {
		/* see if someone owns this */
		uc = u_rootsyscall; ok = 1;
		while (uc) {
			/* owned? */
			if (uc->no == no)
				/* yes. damn */
				ok = 0;

			/* next */
			uc = uc->next;
		}

		/* ok? */
		if (ok)
			/* yes. leave the loop */
			break;
		
		/* next */
		no++;
	}

	/* return the number we found */
	return no;
}

/*
 * syscall_register (struct THREAD* t, char* name, void* func)
 *
 * This will register user syscall [name] for thread [t] with function [func].
 *
 */
int
syscall_register (struct THREAD* t, char* name, void* func) {
	struct USERSYSCALL* ucall;
	struct USERSYSCALL* us = u_rootsyscall;

	/* is [name] unique? */
	if (syscall_search_by_name (name) != NULL)
		/* no. too bad */
		return 0;

	/* create the structure and fill it out */
	ucall = (struct USERSYSCALL*)kmalloc (t, sizeof (struct USERSYSCALL), 0);
	ucall->no = syscall_getfreeno();
	ucall->name = kstrdup (name);
	ucall->thread = t;
	ucall->func = func;
	ucall->next = NULL;

	/* got a chain? */
	if (u_rootsyscall == NULL) {
		/* no. now, we do! */
		u_rootsyscall = ucall;
	} else {
		/* yes. find the end of the chain */
		while (us->next) us = us->next;

		/* append it */
		us->next = ucall;
	}

	/* all done */
	return 1;
}

/*
 * syscall (uint32_t no, ...)
 *
 * This is the actual system call handler.
 *
 */
int
syscall (uint32_t no, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10) {
	struct USERSYSCALL* us;
	addr_t addr;
	int i;
  size_t (*ucall)(struct THREAD* t, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7, uint32_t r8, uint32_t r9, uint32_t r10);

	/* debugging */
	#ifdef SYSCALL_DEBUG
	kprintf ("syscall(): no=0x%x,r1=0x%x,r2=0x%x,r3=0x%x,r4=0x%x,r5=0x%x,r6=0x%x,r7=0x%x,r8=0x%x,r9=0x%x,r10=0x%x\n", no, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10);
	#endif

	/* see what it is */
	switch (no) {
		case SYS_thread: /* thread services */
		                 return SYSCALL_PROC (thread_syscall);
		    case SYS_io: /* io services */
		                 return SYSCALL_PROC (io_syscall);
	 case SYS_module: /* module services */
		                 return SYSCALL_PROC (module_syscall);
		  case SYS_mbox: /* mailbox services */
		                 return SYSCALL_PROC (mbox_syscall);
	 case SYS_network: /* network services */
		                 return SYSCALL_PROC (network_syscall);
	 case SYS_filesys: /* filesystem services */
		                 return SYSCALL_PROC (filesys_syscall);
	 	 case SYS_debug: /* debugging services */
		                 return SYSCALL_PROC (debug_syscall);
	}

	/* humm, not something we know. maybe an user syscall? */
	us = syscall_search_by_number (no);
	if (us == NULL) {
		/* no! complain */
		kprintf ("Thread %x tried to call nonexistant syscall %x, exiting\n", t_current, no);
		thread_exit (t_current);
		return 0;
	}

	/* store the function address */
	ucall = us->func;

	/* store our memory map and activate the new one */
	addr = arch_vm_getmap();
	arch_vm_setthreadmap (us->thread);

	/* call the syscall */
	i = SYSCALL_PROC (ucall);

	/* restore the memory map */
	arch_vm_setmap (addr);

	/* all done */
	return i;
}

/*
 * syscall_dump()
 *
 * This will dump all available syscalls.
 *
 */
void
syscall_dump() {
	struct USERSYSCALL* us = u_rootsyscall;

	kprintf ("User system call dump:\n");

	/* dump them all */
	while (us) {
		/* display it */
		kprintf ("%x: %s (thread 0x%x, func 0x%x)\n", us->no, us->name, us->thread, us->func);

		/* next */
		us = us->next;
	}
}

/* vim:set ts=2 sw=2: */

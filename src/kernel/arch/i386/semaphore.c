/*
 * semaphore.c - XeOS i386 Semaphore Manager
 * (c) 2003 Rink Springer, BSD licensed
 *
 * This code will handle semaphores.
 *
 */
#include <sys/types.h>
#include <sys/semaphore.h>
#include <sys/tty.h>

/*
 * arch_sem_create (SEMAPHORE* sem)
 *
 * This will initialize semaphore [sem].
 *
 */
void
arch_sem_create (SEMAPHORE* sem) {
	/* not busy */
	sem->value = 0xFF;
}

/*
 * arch_sem_get (SEMAPHORE* sem)
 *
 * This will get semaphore [sem].
 *
 */
void
arch_sem_get (SEMAPHORE* sem) {
	asm ("semg:\n");
	asm ("mov %%eax,%%esi\n"
	     "mov	$0x1, %%ecx\n"
	     "lock btrl %%ecx,(%%esi)\n"
	     "jc semg2\n"
	     :
	     : "eax" (&sem->value));
	thread_switch ();
	asm ("jmp semg\n");
	asm ("semg2:\n");
}

/*
 * arch_sem_release (SEMAPHORE* sem)
 *
 * This will release semaphore [sem].
 *
 */
void
arch_sem_release (SEMAPHORE* sem) {
	asm ("mov %%eax,%%esi\n"
	     "mov	$0x1, %%ecx\n"
			 "lock btsl %%ecx,(%%esi)\n"
	     :
	     : "eax" (&sem->value));
}

/* vim:set ts=2: */

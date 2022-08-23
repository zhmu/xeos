/*
 * sempahore.h - XeOS Semaphores
 * (c) 2003 Rink Springer, BSD
 *
 * This describes the semaphroes
 *
 */
#include <sys/types.h>

#ifndef __ARCH_SEMAPHORE_H__
#define __ARCH_SEMAPHORE_H__

typedef struct {
	uint8_t	value;
} SEMAPHORE;

void arch_sem_create (SEMAPHORE* sem);
void arch_sem_get (SEMAPHORE* sem);
void arch_sem_release (SEMAPHORE* sem);

#endif /* __ARCH_SEMAPHORE_H__ */

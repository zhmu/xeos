/*
 * sempahore.h - XeOS Semaphores
 * (c) 2003 Rink Springer, BSD
 *
 * This describes the semaphroes
 *
 */
#include <sys/types.h>

#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <md/semaphore.h>

#ifdef __KERNEL

/* semaphores are completely machine dependant by nature, so just add neat
 * defines to use normal names */
#define sem_create arch_sem_create
#define sem_get arch_sem_get
#define sem_release arch_sem_release

#endif /* __KERNEL */

#endif

/* vim:set ts=2: */

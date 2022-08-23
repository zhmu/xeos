/*
 * irq.h - XeOS Interrupt Manager include file
 * (c) 2002, 2003 Rink Springer, BSD
 *
 * This include file has the interrupt manager declarations.
 *
 */
#include <config.h>
#include <sys/types.h>

#ifndef __IRQ_H__
#define __IRQ_H__

/* IRQ_ENOUGH_STRAY is the number of stray IRQ's you need in order to disable
 * logging them */
#define IRQ_ENOUGH_STRAY	5

/* THREAD is a structure, more is irrelevant now */
struct THREAD;

typedef struct {
	char*          name;
	void*          handler;
	struct THREAD* thread; 
	uint32_t*      var;
	int            stray_count;
} IRQ_HANDLER;

typedef void (IRQ_PROC) ();

void irq_init();
int  irq_register(uint8_t,char*,void*,struct THREAD*,uint32_t*);
void irq_handler(unsigned int);

extern IRQ_HANDLER irq_handlers[MAX_IRQS + 1];

#endif

/* vim:set ts=2: */

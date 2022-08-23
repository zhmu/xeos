/*
 * task.h - XeOS i386 task declarations
 * (c) 2002 Rink Springer, BSD
 *
 * This is a generic include file which describes the i386 tasks.
 *
 */
#ifndef __TASK_H__
#define __TASK_H__

#include <sys/types.h>
#include <sys/thread.h>

#define TSS_EFLAGS_CF      0x00001   /*  0: carry */
#define TSS_EFLAGS_RF1     0x00002   /*  1: reserved, must be 1 */
#define TSS_EFLAGS_PF      0x00004   /*  2: parity */
#define TSS_EFLAGS_RF2     0x00008   /*  3: reserved, must be 0 */
#define TSS_EFLAGS_AF      0x00010   /*  4: aux parity */
#define TSS_EFLAGS_RF3     0x00020   /*  5: reserved, must be 0 */
#define TSS_EFLAGS_ZF      0x00040   /*  6: zero */
#define TSS_EFLAGS_SF      0x00080   /*  7: sign */
#define TSS_EFLAGS_TF      0x00100   /*  8: trap flag */
#define TSS_EFLAGS_IF      0x00200   /*  9: interrupt */
#define TSS_EFLAGS_DF      0x00400   /* 10: direction */
#define TSS_EFLAGS_OF      0x00800   /* 11: overflow */
#define TSS_EFLAGS_IOPL_L  0x01000   /* 12: iopl, low */
#define TSS_EFLAGS_IOPL_H  0x02000   /* 13: iopl, hi */
#define TSS_EFLAGS_NT      0x04000   /* 14: nested task */
#define TSS_EFLAGS_RF4     0x08000   /* 15: reserved, must be 0 */
#define TSS_EFLAGS_RF      0x10000   /* 16: restart flag */
#define TSS_EFLAGS_VM      0x20000   /* 17: virtual mode */

struct TSS {
	uint16_t	backlink;
	uint16_t	res0;

	uint32_t	esp0;
	uint16_t	ss0;
	uint16_t	res1;

	uint32_t	esp1;
	uint16_t	ss1;
	uint16_t	res2;

	uint32_t	esp2;
	uint16_t	ss2;
	uint16_t	res3;

	uint32_t	cr3;
	uint32_t	eip;
	uint32_t	eflags;
	uint32_t	eax;
	uint32_t	ecx;
	uint32_t	edx;
	uint32_t	ebx;
	uint32_t	esp;
	uint32_t	ebp;
	uint32_t	esi;
	uint32_t	edi;

	uint16_t	es;
	uint16_t	res4;
	uint16_t	cs;
	uint16_t	res5;
	uint16_t	ss;
	uint16_t	res6;
	uint16_t	ds;
	uint16_t	res7;
	uint16_t	fs;
	uint16_t	res8;
	uint16_t	gs;
	uint16_t	res9;

	uint16_t	ldt;
	uint16_t	res10;
	uint16_t	trap;
	uint16_t	iobase;
	uint32_t	iobitmap;
} __attribute__((packed));

#ifdef __KERNEL
void	arch_thread_init (uint32_t taskno, struct THREAD* t);
void	arch_task_init();
void	arch_thread_launch (struct THREAD* t);
void	arch_thread_seteip (struct THREAD* t, addr_t eip);
void	arch_thread_cleanup (struct THREAD* t);
void  arch_thread_pio_enable (struct THREAD* t, addr_t start, size_t len);
void  arch_thread_push (struct THREAD* t, uint32_t v);
#endif /* __KERNEL*/

#endif /* __TASK_H__ */

/* vim:set ts=2: */

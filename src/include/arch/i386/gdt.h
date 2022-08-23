/*
 * gdt.h - XeOS i386 GDT Code
 * (c) 2002 Rink Springer, BSD
 *
 * This is a generic include file which describes the GDT stuff.
 *
 */
#ifndef __GDT_H__
#define __GDT_H__

#include <sys/types.h>

#define GDT_DESCTYPE_SYSTEM		0
#define GDT_DESCTYPE_CODEDATA		1

#define GDT_SEGTYPE_EXEC		8
#define GDT_SEGTYPE_EXPDOWNCONFORMING	4
#define GDT_SEGTYPE_READ_CODE		2
#define GDT_SEGTYPE_WRITE_DATA		2
#define GDT_SEGTYPE_ACCESSED		1
#define GDT_SEGTYPE_AVAILABLETSS	9

#ifdef __KERNEL
void gdt_set_entry (uint8_t no, uint32_t limit, uint32_t addr, uint8_t segtype, uint8_t desctype, uint8_t dpl, uint8_t pbit, uint8_t avl, uint8_t opsize, uint8_t dbit, uint8_t gran);
#endif /* __KERNEL */

#endif

/*
 * memory.c - XeOS i386 Memory Add code
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code is heavily based on YCX2 (src/arch/i386/machdep_main_startup.c),
 * (c) 1999 Anders Gavare.
 *
 * This code will add the memory we have.
 *
 */
#include <sys/types.h>
#include <sys/kmalloc.h>
#include <md/config.h>
#include <md/memory.h>

/*
 *  arch_add_all_memory():
 *
 *  Add all memory regions using kmalloc_addregion().  On PC, we have some
 *  memory below 640KB, and then some memory above 1MB.  The memory above
 *  1MB is divided into DMA-capable memory (<16MB) and non-DMA-memory.
 *  To complicate things even further, there can be memory holes at 14-15MB
 *  and 15-16MB.
 *
 *  (NOTE:  We are already running in high memory now (paged), so we cannot
 *  actually detect memory.  We have to trust highest_addressable_byte etc
 *  set by code in jumpmain.c.)
 */
void arch_add_all_memory() {
	size_t first_addr, last_addr;

	first_addr = first_addressable_high_byte;
	last_addr = highest_addressable_byte + 1;

#if 0
	/* add the memory *before* the kernel 
	 * FIXME: 0xA0000 shouldn't be hardcoded ... ???? */
	kmalloc_addregion (PAGESIZE, 0xA0000 - PAGESIZE, 0);

	/* XXX: The memory manager isn't smart enough to understand that a single 
   * memory block can't handle a request because there's a gap between it and
   * the next block. Therefore, we just bluntly forget the lower memory for
   * now ... this is evil and should be fixed. Some day :-)
   */
#endif

	/* add memory after the kernel */
	kmalloc_addregion (first_addr, (last_addr - first_addr), 0);

}

/* vim:set ts=2 sw=2: */

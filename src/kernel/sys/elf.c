/*
 * elf.c - XeOS ELF Loader
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * ELF specification from Christopher Giese,
 * http://www.execpc.com/~geezer/osd/exec/elf.txt
 *
 * This code will handle ELF binary loading.
 *
 */
#include <sys/types.h>
#include <sys/thread.h>
#include <sys/elf.h>
#include <sys/kmalloc.h>
#include <lib/lib.h>
#include <md/config.h>
#include <md/vm.h>
#include <md/memory.h>

#define xELF_DEBUG

#if 0
/*
 * elf_handle_pagefault(struct THREAD* t, addr_t addr)
 *
 * This will handle pagefaults.
 *
 */
int
elf_handle_pagefault(struct THREAD* t, addr_t addr) {
#ifdef ELF_DEBUG
			kprintf ("elf_handle_pagefault(): thread=0x%x, addr=0x%x\n", t, addr);
#endif

	/* wade through all sections */
	for (i = 0; i < hdr->sh_num_entries; i++) {
		/* must be allocated? */
		if (sh->flags & 2 /* TODO: #define me! */) {
			/* yes. does this section match the address we are looking for? */
			if ((addr >= sh->addr) && (addr <= (sh->addr + sh->size))) {
				/* yes. round the addresses */
				section_start  = (sh->addr & ~(PAGESIZE - 1));
				section_offset = (sh->offset & ~(PAGESIZE - 1));
				section_len    = ((sh->size - 1) | (PAGESIZE - 1)) + 1;
#ifdef ELF_DEBUG
				kprintf ("addr %x is in {%x,%x} becomes {%x,%x}\n", addr, sh->addr, sh->addr + sh->size, section_start, (section_start+section_len));
#endif

				/* build the mapping flags (TODO: #define me!) */
#if 0
				mapflags = VM_READ;
				if (flags & 1)
					mapflags |= VM_WRITE;
				if (flags & 4)
					mapflags |= VM_EXEC;
#endif

				/* allocate memory and move the data */
				new_page = kmalloc (t, section_len, 0);
				kmemcpy (new_page, t->program + section_offset, section_len);

				/* build a mapping */
#if 0
				kprintf ("elf_parse(): cr2=0x%x mapping from %x to %x, %x bytes\n", addr, (new_page - mapped_kernel), section_start, section_len);
#endif
				if (!paging_map (t, section_start, (new_page - mapped_kernel), section_len))
					panic ("paging_map(): failed!");

				/* done! */
				return 1;
			}
		}
		
		/* next */
		sh++;
	}

	/* no section today... */
	return 0;
}
#endif

/*
 * elf_parse (uint8_t* data, size_t len, struct THREAD* t)
 *
 * This will attempt to parse [len] bytes of ELF executable [data]. On success,
 * [t] will be set up for execution. This function returns ELF_ERROR_xxx on
 * failures or ELF_OK on success.
 *
 */
int
elf_parse (uint8_t* data, size_t len, struct THREAD* t) {
	struct ELF_HEADER*  hdr = (struct ELF_HEADER*)data;
	struct ELF_SECTION* sh  = (struct ELF_SECTION*)(data + hdr->sh_offs);
	int i;
	addr_t map_addr;
	addr_t map_offs;
	addr_t map_size;
	char* new_page;

	/* is the data long enough? */
	if (len < sizeof (struct ELF_HEADER))
		/* no. complain */
		return ELF_ERROR_BADLEN;

	/* is this an ELF file? */
	if (hdr->magic != ELF_MAGIC)
		/* no. complain */
		return ELF_ERROR_NOTELF;

	/* store the program's entry point  */
	arch_thread_seteip (t, hdr->entry);

	/* map all sections */
	for (i = 0; i < hdr->sh_num_entries; i++) {
		/* must be allocated? */
		if (sh->flags & 2 /* TODO: #define me! */) {
			/* yes. calculate the true addresses to which we map */
			map_addr = (sh->addr   & ~(PAGESIZE - 1));
			map_offs = (sh->offset & ~(PAGESIZE - 1));

			/* calculate the number of bytes to map */
			map_size  = (sh->addr - map_addr) + sh->size;
			map_size += (PAGESIZE - (map_size % PAGESIZE));

#ifdef ELF_DEBUG
			kprintf ("%x: mapping %x-%x to %x-%x [offset from %x to %x]\n", t,
					sh->addr, (sh->addr + sh->size),
					map_addr, (map_addr + map_size),
					sh->offset, map_offs);
#endif

			new_page = kmalloc (t, map_size, 0);
			kmemcpy (new_page, data + map_offs, map_size);

			/* build a mapping */
			if (!arch_vm_map (t, map_addr, ((addr_t)new_page - mapped_kernel), map_size))
				panic ("elf_parse(): couldn't map!");
		}

		/* next */
		sh++;
	}

	/* all fine */
	return ELF_OK;
}

/* vim:set ts=2 sw=2: */

/*
 * XeOS Kernel Memory Allocator - (c) 2003 Rink Springer
 *
 * This code is inspired by Yoctix, (c) 1999 Anders Gavare.
 *
 */
#include <sys/types.h>
#include <sys/kmalloc.h>
#include <md/memory.h>
#include <lib/lib.h>
#include <assert.h>

#define xKMALLOC_DEBUG
#define xKMALLOC_DEBUG_BITMAP

struct KMALLOC_REGION* root_region;
#define FIX_ADDR(x) ((x)+mapped_kernel)

/*
 * kmalloc_init()
 *
 * This will initialize the kernel-side memory allocator.
 *
 */
void
kmalloc_init() {
	/* no root region yet */
	root_region = NULL;

	/* ensure PAGESIZE % KMALLOC_BITMAPSIZE == 0 */
	ASSERT ((PAGESIZE % KMALLOC_BITMAPSIZE) == 0);
}

/*
 * kmalloc_addregion (addr_t addr, size_t size, uint32_t flags)
 *
 * This will add [size] bytes of memory, starting at address [addr] with flags
 * [flags] to the region map.
 *
 */
void
kmalloc_addregion (addr_t addr, size_t size, uint32_t flags) {
	struct KMALLOC_REGION* reg;
	struct KMALLOC_REGION* tmpregion;
	struct KMALLOC_CHUNK* chunk;
	size_t i;
	addr_t xaddr;

	/* is addr page-aligned? */
	if ((addr % PAGESIZE) != 0)
		/* no. complain! */
		panic ("kmalloc_addregion(): address 0x%x isn't page aligned!", addr);

	/* calculate the size in pages */
	size /= PAGESIZE;

	/* place the new region at the very beginning of the memory */
	reg = (struct KMALLOC_REGION*)FIX_ADDR (addr);
	reg->address = addr;
	reg->size = size;
	reg->avail = size;
	reg->flags = flags;
	reg->numchunks = size;
	reg->next = NULL;

	/* determine the new address */
	xaddr = addr + sizeof (struct KMALLOC_REGION) +
	        reg->numchunks * sizeof (struct KMALLOC_CHUNK);

	/* round it at pages */
	xaddr += (PAGESIZE - (xaddr % PAGESIZE));

	/* build the chunks */
	for (i = 0; i < reg->numchunks; i++) {
		/* set the chunk up */
		chunk = (struct KMALLOC_CHUNK*)FIX_ADDR (addr +
                                             sizeof (struct KMALLOC_REGION) +
		                                         (i * sizeof (struct KMALLOC_CHUNK)));
		chunk->address = xaddr;
		chunk->thread = (struct THREAD*)NULL;
		chunk->flags = 0;

		/* next */
		xaddr += PAGESIZE;
	}

	/* do we have a root region? */
	if (root_region) {
		/* yes. find the final region */
		tmpregion = root_region;
		while (tmpregion->next)
			tmpregion = tmpregion->next;

		/* append this region */
		tmpregion->next = reg;
	} else {
		/* no. now, we _do_ have a root region */
		root_region = reg;
	}

#ifdef KMALLOC_DEBUG
	if (size > ((1024 * 1024) / PAGESIZE))
		kprintf ("kmalloc_addregion(): addr=0x%x size=%u MB flags=0x%x\n", addr, (size * PAGESIZE) / (1024 * 1024), flags);
	else
		kprintf ("kmalloc_addregion(): addr=0x%x size=%u KB flags=0x%x\n", addr, (size * PAGESIZE) / 1024, flags);
#endif /* KMALLOC_DEBUG */
}

/*
 * kmemstats(size_t* total, size_t* avail)
 *
 * This will return the total amount of memory in [total], and the available
 * memory in [avail]. All results are in bytes.
 *
 */
void
kmemstats(size_t* total, size_t* avail) {
	size_t sztotal, szavail;
	struct KMALLOC_REGION* reg = root_region;

	/* walk through all regions */
	sztotal = 0; szavail = 0;
	while (reg) {
		/* add the sizes */
		sztotal += reg->size;
		szavail += reg->avail;
		
		/* next */
		reg = reg->next;
	}

	/* return the sizes */
	*total = (sztotal * PAGESIZE);
	*avail = (szavail * PAGESIZE);
}

/*
 * kmalloc_check_best_fit (size_t size)
 *
 * This will return the region of memory which has the best fit for [size]
 * pages, or NULL if there isn't any room left.
 *
 * FIXME: make this truly best fit, not first fit.
 *
 */
struct KMALLOC_REGION*
kmalloc_check_best_fit (size_t size) {
	struct KMALLOC_REGION* reg = root_region;

	/* wade through all regions */
	while (reg) {
		/* enough space here? */
		if (reg->avail >= size)
			/* yes. return it */
			return reg;

		/* next */
		reg = reg->next;
	}

	/* no space today... */
	return NULL;
}

/*
 * kmalloc_bitmap_check_best_fit (struct THREAD* t, size_t size)
 *
 * This will return the chunk of memory which has the best fit for [size]
 * _BYTES_ for thread [t], or NULL if there are no bitmaps with that much
 * space left.
 *
 * FIXME: make this truly best fit, not first fit.
 *
 */
struct KMALLOC_CHUNK*
kmalloc_bitmap_check_best_fit (struct THREAD* t, size_t size) {
	struct KMALLOC_REGION* reg = root_region;
	struct KMALLOC_CHUNK* chunk;
	addr_t i;

	/* wade through all regions */
	while (reg) {
		/* scan all chunks */
		for (i = 0; i < reg->numchunks; i++) {
			/* fetch the chunk */
			chunk = (struct KMALLOC_CHUNK*)FIX_ADDR (reg->address +
																							 sizeof (struct KMALLOC_REGION) +
																							 (i * sizeof (struct KMALLOC_CHUNK)));

			/* is this a bitmap chunk and in our thread? */
			if ((chunk->thread == t) && (chunk->flags & KMALLOC_CFLAGS_BITMAP)) {
				/* yes. got enough space left? */
				if (chunk->bitmap_left >= size) {
					/* yes. return this chunk */
					return chunk;
				}
			}
		}

		/* next */
		reg = reg->next;
	}

	/* no space today... */
	return NULL;
}

/*
 * kmalloc_bitmap (struct THREAD* t, size_t size, uint32_t flags)
 *
 * This will allocate [size] bytes for thread [t] with flags [flags]. It will
 * return a pointer to the memory on success or NULL on failure.
 *
 * This function will handle
 *
 */
void*
kmalloc_bitmap (struct THREAD* t, size_t size, uint32_t flags) {
	struct KMALLOC_REGION* reg;
	struct KMALLOC_CHUNK* chunk;
	addr_t i, numavail, j;
	size_t sz;
	uint8_t* ptr;
	uint8_t* tmptr;

	/* ensure we have less than a page */
	ASSERT (size >= PAGESIZE);

	/* calculate space in bitmap-chunks */
	sz = (size / KMALLOC_BITMAPSIZE);
	if (size % KMALLOC_BITMAPSIZE)
		sz++;

#ifdef KMALLOC_DEBUG_BITMAP
	kprintf ("kmalloc_bitmap(): size=%u sz=%u\n", size, sz);
#endif /* KMALLOC_DEBUG_BITMAP */

	/* scan for a bitmap with enough space */
	chunk = kmalloc_bitmap_check_best_fit (t, sz);
	if (chunk == NULL) {
		/* no chunk. build a new one */
		reg = kmalloc_check_best_fit (1);
		if (reg == NULL) {
			/* out of memory! */
			return NULL;
		}

		/* scan for a free chunk
		 * TODO: make this best-fit too! */
		for (i = 0; i < reg->numchunks; i++) {
			/* fetch the chunk */
			chunk = (struct KMALLOC_CHUNK*)FIX_ADDR (reg->address +
																							 sizeof (struct KMALLOC_REGION) +
																							 (i * sizeof (struct KMALLOC_CHUNK)));

			/* available chunk? */
			if (!(chunk->flags & KMALLOC_CFLAGS_USED))
				/* yes. got it */
				break;
		}

		/* ensure the chunk is unused */
		ASSERT ((chunk->flags & KMALLOC_CFLAGS_USED) == 0);

		/* set the chunk up */
		chunk->flags = KMALLOC_CFLAGS_USED | KMALLOC_CFLAGS_BITMAP;
		chunk->thread = t;
		chunk->bitmap_left  =  (PAGESIZE / KMALLOC_BITMAPSIZE);
		chunk->bitmap_left -=  (PAGESIZE / KMALLOC_BITMAPSIZE) / KMALLOC_BITMAPSIZE;

		/* zap the chunk contents, so the bitmap is nuked */
		kmemset ((void*)FIX_ADDR (chunk->address), 0, PAGESIZE);

		/* update the statistics */
		reg->avail--;
	} else {
#ifdef KMALLOC_DEBUG_BITMAP
		kprintf ("kmalloc_bitmap(): appending to chunk 0x%x!\n", chunk);
#endif /* KMALLOC_DEBUG_BITMAP */
	}

	/* the current chunk has space left for us. find it
	 * TODO: make this a best-fit, not first-fit */
	for (i = 0; i < ((PAGESIZE / KMALLOC_BITMAPSIZE) - sz); i++) {
		/* build a pointer */
		ptr = (uint8_t*)FIX_ADDR (chunk->address + i);

		/* got space here? */
		if (!*ptr) {
			/* yes. check for more space */
			numavail = 1;
			for (j = 1; j < sz; j++) {
				/* build a pointer */
				tmptr = (uint8_t*)FIX_ADDR (chunk->address + i + (j / 8));

				/* available? */
				if (!*tmptr)
					/* yes. increment the count */
					numavail++;
			}

			/* got enough space? */
			if (numavail == sz)
				/* yes. use it */
				break;
		}
	}

	/* found a match? */
	if (numavail != sz) {
		/* no. too bad XXX: shouldn't panic? */
		panic ("kmalloc_bitmap(): out of memory!\n");
	}

	/* mark all pages as used */
	tmptr = ptr;
#ifdef KMALLOC_DEBUG_BITMAP
	kprintf ("kmalloc_bitmap(): placing at chunk 0x%x, pos %x\n", chunk, i);
#endif /* KMALLOC_DEBUG_BITMAP */
	for (j = 0; j < sz; j++) {
		/* build a pointer */
		ptr = (uint8_t*)FIX_ADDR (chunk->address + i);

		/* mark it as used */
		*ptr = (sz - j);

		/* next */
		i++;
	}
	i -= j;

	/* update statistics */
	chunk->bitmap_left -= sz;

	/* return the pointer */
#ifdef KMALLOC_DEBUG_BITMAP
	kprintf ("kmalloc_bitmap(): size=%u sz=%u -> 0x%x\n", size, sz,
	         FIX_ADDR (chunk->address +
	                   (PAGESIZE / KMALLOC_BITMAPSIZE) + (i * KMALLOC_BITMAPSIZE)
						        ));
#endif /* KMALLOC_DEBUG_BITMAP */
	return (void*) FIX_ADDR (chunk->address +
	                   (PAGESIZE / KMALLOC_BITMAPSIZE) + (i * KMALLOC_BITMAPSIZE)
						        );
	
/* XXX?
		FIX_ADDR (chunk->address + ((1 + i - sz) * KMALLOC_BITMAPSIZE));
		*/
}

/*
 * kmalloc (struct THREAD* t, size_t size, uint32_t flags)
 *
 * This will allocate [size] bytes for thread [t] with flags [flags]. It will
 * return a pointer to the memory on success or NULL on failure.
 *
 */
void*
kmalloc (struct THREAD* t, size_t size, uint32_t flags) {
	struct KMALLOC_REGION* reg;
	struct KMALLOC_CHUNK* chunk;
	struct KMALLOC_CHUNK* tmpchunk;
	size_t sz, i, j, numavail;

	/* need to allocate zero bytes? */
	if (!size)
		/* yes. return a NULL pointer, it has room for 0 bytes :) */
		return NULL;

	/* do we have at least a full page? */
	if (size < PAGESIZE) {
		/* yes. handle it */
		return kmalloc_bitmap (t, size, flags);
	}

	/* calculate the number of pages needed */
	sz = (size / PAGESIZE);
	if (size % PAGESIZE)
		sz++;

	/* find the best fit region */
	reg = kmalloc_check_best_fit (sz);
	if (reg == NULL)
		/* no space. return */
		return NULL;

	/* there is space here. wade through the chunks */
	for (i = 0; i < (reg->numchunks - sz); i++) {
		/* fetch the chunk */
		chunk = (struct KMALLOC_CHUNK*)FIX_ADDR (reg->address +
                                             sizeof (struct KMALLOC_REGION) +
		                                         (i * sizeof (struct KMALLOC_CHUNK)));

		/* available chunk? */
		if (!(chunk->flags & KMALLOC_CFLAGS_USED)) {
			/* yes. ensure the chunk fits here */
			tmpchunk = chunk; tmpchunk++; numavail = 1;
			for (j = 0; j < (sz - 1); j++) {
				/* count the number of chunks needed */
				if (!(tmpchunk->flags & KMALLOC_CFLAGS_USED))
					numavail++;

				/* next chunk */
				tmpchunk++;
			}
			
			/* enough available chunks? */
			if (numavail == sz)
				/* yes. bail out */
				break;
		}
	}

	/* got a row of chunks? */
	if (numavail != sz) {
		/* no. too bad */
		/* XXX */ panic ("kmalloc(): out of memory\n"); /* XXX */
		return NULL;
	}

	/* mark the head chunk */
	chunk->flags = KMALLOC_CFLAGS_USED;
	chunk->thread = t;
	chunk->chain = sz;

	/* mark other blocks as used too */
	tmpchunk = chunk; tmpchunk++;
	for (i = 0; i < sz; i++) {
#ifdef KMALLOC_DEBUG
		/*kprintf ("kmalloc(): got chunk 0x%x\n", tmpchunk);*/
#endif /* KMALLOC_DEBUG */

		/* mark this block as used */
		tmpchunk->flags = KMALLOC_CFLAGS_USED;
		tmpchunk->thread = t;
		tmpchunk->chain = -1;

		/* next */
		tmpchunk++;
	}

	/* update statistics */
	reg->avail -= sz;

	/* return the address of the first block */
#ifdef KMALLOC_DEBUG
	kprintf ("kmalloc(): size=0x%x, sz=0x%x -> %x\n", size, sz, FIX_ADDR (chunk->address));
#endif /* KMALLOC_DEBUG */

	return (void*)FIX_ADDR (chunk->address);
}

/*
 * kfindptr (addr_t addr, struct KMALLOC_CHUNK** ochunk,
 *           struct KMALLOC_REGION** oreg)
 *
 * This will return pointers to the chunk and region in which [addr] resides.
 * This will return 0 on failure or 1 on success.
 *
 */
int
kfindptr (addr_t addr, struct KMALLOC_CHUNK** ochunk, struct KMALLOC_REGION** oreg) {
	struct KMALLOC_CHUNK* chunk;
	struct KMALLOC_REGION* region = root_region;
	size_t i;

	/* scan all regions */
	while (region) {
		/* fetch the chunk */
		for (i = 0; i < region->numchunks; i++) {
			/* build a pointer */
			chunk = (struct KMALLOC_CHUNK*)FIX_ADDR (region->address +
																							 sizeof (struct KMALLOC_REGION) +
																							 (i * sizeof (struct KMALLOC_CHUNK)));

			/* is the memory within this chunk? */
			if ((addr >= FIX_ADDR (chunk->address)) && (addr <= (FIX_ADDR (chunk->address) + PAGESIZE - 1))) {
				/* yes. return the region and chunk */
				*oreg = region;
				*ochunk = chunk;
				return 1;
			}
		}

		/* next */
		region = region->next;
		return 0;
	}

	/* sorry */
	return 0;
}

/*
 * kfree_bitmap (addr_t addr, struct KMALLOC_REGION* region,
 *               struct KMALLOC_CHUNK* chunk)
 *
 * This will free address [addr] in chunk [chunk] of region [region].
 *
 */
void
kfree_bitmap (addr_t addr, struct KMALLOC_REGION* region, struct KMALLOC_CHUNK* chunk) {
	addr_t bitmap_no = (addr - FIX_ADDR (chunk->address) - (PAGESIZE / KMALLOC_BITMAPSIZE)) / KMALLOC_BITMAPSIZE;
	size_t count = 1;
	uint8_t* ptr;

#ifdef KMALLOC_DEBUG_BITMAP
	kprintf ("kfree_bitmap(): addr = %x, chunk->addr = 0x%x, bitmap_no = %x\n", addr, chunk->address, bitmap_no);
#endif /* KMALLOC_DEBUG_BITMAP */

	/* walk through the bitmap */
	while (1) {
		/* build a pointer */
		ptr = (uint8_t*)FIX_ADDR (chunk->address + bitmap_no);

		/* final one? */
		if (*ptr == 1)
			/* yes. leave */
			break;

		/* clear it */
		*ptr = 0;

		/* next */
		count++; bitmap_no++;
	}

	/* zap the final one */
	*ptr = 0;

	/* update the statistics */
	chunk->bitmap_left += count;

	/* freed an entire chunk? */
	if (chunk->bitmap_left == (PAGESIZE / KMALLOC_BITMAPSIZE) - ((PAGESIZE / KMALLOC_BITMAPSIZE) / KMALLOC_BITMAPSIZE)) {
		/* yes. free the chunk as well */
#ifdef KMALLOC_DEBUG_BITMAP
	kprintf ("kfree_bitmap(): chunk %x is unused, freeing\n", chunk);
#endif /* KMALLOC_DEBUG_BITMAP */
		chunk->flags = 0;

		/* update the statistics */
		region->avail++;
	}
}

/*
 * kfree (void* ptr)
 *
 * This will free the memory pointed at by [ptr].
 *
 */
void
kfree (void* ptr) {
	addr_t addr = (addr_t)ptr;
	struct KMALLOC_CHUNK* chunk;
	struct KMALLOC_REGION* region;
	size_t numleft;

	/* scan for the region in which this block resides */
	if (!kfindptr (addr, &chunk, &region)) {
		/* this failed */
		panic ("kfree(): invalid address 0x%x\n", addr);
	}

	/* is this chunk marked as available? */
	if (!(chunk->flags & KMALLOC_CFLAGS_USED)) {
		/* no. moan */
		kprintf ("kfree(): warning: attempt to free already freed address 0x%x\n", addr);
		return;
	}

	/* is this chunk a bitmap? */
	if (chunk->flags & KMALLOC_CFLAGS_BITMAP) {
		/* yes. pass it through */
		kfree_bitmap (addr, region, chunk);
		return;
	}
	
#ifdef KMALLOC_DEBUG
	kprintf ("kfree(): freeing chunk %x, %x total\n", chunk, chunk->chain);
#endif /* KMALLOC_DEBUG */
	
	/* is this the first block in the chain? */
	if (chunk->chain == -1) {
		/* no. complain */
		kprintf ("kfree(): attempt to free chained block %x!\n", addr);
		return;
	}

	/* just free this chunk and all linked blocks */
	numleft = chunk->chain;
	while (numleft--) {
		/* ensure this chunk is allocated */
		ASSERT ((chunk->flags & KMALLOC_CFLAGS_USED) != 0);

		/* mark it as available */
		chunk->flags = 0;

		/* update the statistics */
		region->avail++;

		/* next */
		chunk++;
	}
}

/* vim:set ts=2 sw=2: */

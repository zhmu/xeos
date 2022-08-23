/*
 * console.c - XeOS i386 Console Driver
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This will handle the internal i386-only console. Only output is supported
 * at this time.
 *
 */
#include <sys/tty.h>
#include <sys/types.h>
#include <sys/device.h>
#include <lib/lib.h>
#include <md/memory.h>

addr_t tty_videobase = 0xb8000;
uint32_t offs = 0;

/*
 * arch_console_init()
 *
 * This will initialize the i386 console.
 *
 */
void
arch_console_init() {
	uint8_t data;

	/* figure out the video type */
	asm ("inb %w1,%0" : "=a" (data) : "d" (0x3cc)); /* data = inb (0x3cc) */
	if ((data & 1)) {
		tty_videobase = 0xb8000;
	} else {
		tty_videobase = 0xb0000;
	}
	tty_videobase = MAP_MEMORY (tty_videobase);
}

/*
 * arch_console_putchar (uint8_t ch)
 *
 * This will put [ch] on the console. Newlines and scrolling is supported.
 *
 */
void
arch_console_putchar (uint8_t ch) {
	uint16_t* w = (uint16_t*)(tty_videobase + offs);

	/* got a lf? */
	if (ch == '\n') {
		/* yes. go the the beginning of the next line */
		offs = offs - (offs % 160);
		offs += 160;
	} else {
		/* just write it in the videomemory */
		*w = 0x1e00 | ch;
		offs += 2;
	}

	/* need to scroll? */
	if (offs >= 4000) {
		/* handle the scrolling */
		kmemcpy ((void*)(tty_videobase),
		         (void*)(tty_videobase + 160),
		 				 (24 * 160));

		/* blank out the bottom line */
		kmemset ((void*)(tty_videobase + (24 * 160)), 0x0, 160);

		/* one line less */
		offs -= 160;
	}
}

/* vim:set ts=2 sw=2: */

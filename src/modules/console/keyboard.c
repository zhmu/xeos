/*
 * console.c - XeOS Console Driver
 *
 * (c) 2002, 2003 Rink Springer, BSD
 *
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/module.h>

#define KEYBUF_SIZE	16

struct DEVICE dev;
char keybuf[KEYBUF_SIZE];
size_t	keybuf_curpos; /* XXX: =0 assignments don't work yet! */

uint8_t console_keymap[128] = {
	/* 00-07 */    0, 0x1b,  '1',  '2',  '3',  '4',  '5',  '6',
  /* 08-0f */  '7',  '8',  '9',  '0',  '-',  '+',    8,    9,
  /* 10-17 */  'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
	/* 18-1f */  'o',  'p',  '[',  ']', 0x0d,    0,  'a',  's',
	/* 20-27 */  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
	/* 28-2f */  ':',  '`',    0, 0x27,  'z',  'x',  'c',  'v',
	/* 30-37 */  'b',  'n',  'm',  ',',  '.',  '/',  '?',  '*',
	/* 38-3f */    0,  ' ',    0,    0,    0,    0,    0,    0,
	/* 40-47 */    0,    0,    0,    0,    0,    0,    0,  '7',
	/* 48-4f */  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
	/* 50-57 */  '2',  '3',  '0',  '.',    0,    0,  'Z',    0,
	/* 57-5f */    0,    0,   0,     0,    0,    0,    0,    0,
	/* 60-67 */    0,    0,   0,     0,    0,    0,    0,    0,
	/* 68-6f */    0,    0,   0,     0,    0,    0,    0,    0,
	/* 70-76 */    0,    0,   0,     0,    0,    0,    0,    0,
	/* 77-7f */    0,    0,   0,     0,    0,    0,    0,    0
};

/*
 * kbd_readch ()
 *
 * This will read a single char from the keyboard buffer. It will return the
 * char on success or 0 on failure.
 *
 */
uint8_t
kbd_readch() {
	uint8_t ch;

	/* got any chars in the buffer? */
	if (keybuf_curpos == 0)
		/* no. return zero */
		return 0;

	/* fetch the first char */
	ch = keybuf[0];

	/* shift everything */
	memcpy (keybuf, keybuf + 1, KEYBUF_SIZE);
	keybuf_curpos--;

	/* return the char */
	return ch;
}

/*
 * kbd_do_scancode (uint8_t sc)
 *
 * This will handle scancode [sc].
 *
 */
void
kbd_do_scancode (uint8_t sc) {
	uint8_t ch;

	/* break? */
	if (sc & 0x80)
		/* yes. bail out */
		return;

	/* fetch the char */
	ch = console_keymap[sc];

	/* did we have something? */
	if (ch) {
		/* yes. put in into the keyboard buffer */
		keybuf[keybuf_curpos] = ch;
		keybuf_curpos++;
		
		/* about to overrun? */
		if (keybuf_curpos == KEYBUF_SIZE)
			/* yes. discard a char */
			(void)kbd_readch();
	}
}

/*
 * kbd_read (struct DEVICE* dev, addr_t offs, uint8_t* buf, size_t len) {
 *
 * This will copy up to [len] bytes from the keyboard buffer to [buf]. It will
 * return the number of charachters read.
*
 */
size_t
kbd_read (struct DEVICE* dev, addr_t offs, uint8_t* buf, size_t len) {
	size_t i = 0;
	uint8_t ch;

	/* keep fetching */
	while (len--) {
		/* fetch a char */
		ch = kbd_readch();

		/* success? */
		if (!ch)
			/* no. bail out */
			break;
		
		/* next */
		buf[i] = ch;
		i++;
	}

	/* return the number of bytes read */
	return i;
}

/*
 * kbd_init()
 *
 * This will initialize the keyboard. It will return 0 on failure and non-zero
 * on success.
 *
 */
int
kbd_init (MODULECONFIG* cf) {
	/* register the IO port */
	if (!register_pio (cf->resources->port, 4)) {
		printf ("%s: unable to register io ports 0x%x-0x%x\n", cf->name, cf->resources->port, cf->resources->port + 4);
		return 0;
	}

	/* register our irq */
	if (!register_irq (cf->resources->irq, NULL, NULL)) {
		printf ("%s: unable to register irq %x\n", cf->name, cf->resources->irq);
		return 0;
	}

	/* read and discard all current chars in the buffer */
	while (inb (cf->resources->port + 4) & 1)
		(void)inb (cf->resources->port);

	/* nothing in the buffer just yet */
	keybuf_curpos = 0;

	/* yeppee! */
	return 1;
}

/*
 * kbd_loop (MODULECONFIG* cf)
 *
 * This will loop for keyboard events and handle them.
 *
 */
int
kbd_loop (MODULECONFIG* cf) {
	/*
	 * the loop below will suspend itself. whenever an irq arrives, the kernel
	 * will resume this thread so input can be handeled
	 *
	 */
	while (1) {
		/* zzzz.... */
		thread_suspend();

		/* fetch the input */
		while (inb (cf->resources->port + 4) & 1)
			/* handle scancode */
			kbd_do_scancode (inb (cf->resources->port));
	}
}

/* vim:set ts=2 sw=2: */

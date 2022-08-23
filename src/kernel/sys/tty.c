/*
 * tty.c - XeOS TTY Driver
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will handle TTY in and output. They are usually bound to the
 * video and keyboard.
 *
 */
#include <sys/tty.h>
#include <sys/types.h>
#include <sys/device.h>
#include <lib/lib.h>
#include <md/console.h>

struct DEVICE* tty_device = NULL;

/*
 * tty_init()
 *
 * This will initialize the TTY in and output.
 *
 */
void
tty_init() {
	/* chain through to arch_console_init() */
	arch_console_init();
}

/*
 * tty_puts (char* msg)
 *
 * This will print [msg] using the TTY driver.
 *
 */
void
tty_puts (char* msg) {
	/* got a device? */
	if (tty_device != NULL) {
		/* yes. let it handle the request */
		device_write (tty_device, 0, msg, kstrlen (msg));
		return;
	}

	while (*msg)
		tty_putchar (*msg++);
}

/*
 * tty_putchar (uint8_t ch)
 *
 * This will print [ch] using the TTY driver.
 *
 */
void
tty_putchar (uint8_t ch) {
	/* got a device? */
	if (tty_device != NULL) {
		/* yes. let it handle the request */
		device_write (tty_device, 0, &ch, 1);
		return;
	}

	/* call the internal handler */
	arch_console_putchar (ch);
}

/*
 * tty_getcursorpos (uint8_t* x, uint8_t* y)
 *
 * This will return the current cursor position.
 *
 */
void
tty_getcursorpos (uint8_t* x, uint8_t* y) {
	/* FIXME */
	*x = 0; *y = 10;
}

/*
 * tty_setdevice (struct DEVICE* dev)
 *
 * This will activate [dev] as TTY device. It will return zero on failure or
 * non-zero on success.
 *
 */
int
tty_setdevice (struct DEVICE* dev) {
	/* trying to activate NULL? */
	if (dev == NULL) {
		/* yes. this is fine (NULL means internal driver) */
		tty_device = NULL;
		return 1;
	}

	/* is this a charachter device? */
	if (dev->type != DEVICE_TYPE_CHAR)
		/* no. complain */
		return 0;

	/* seems fine. activate it */
	tty_device = dev;

	/* all done */
	return 1;
}

/*
 * tty_getdevice()
 *
 * This will fetch a pointer to the TTY device. NULL means the device is
 * internal.
 *
 */
struct DEVICE* tty_getdevice() { return tty_device; }

/*
 * tty_getchar ()
 *
 * This will print [ch] using the TTY driver.
 *
 */
uint8_t
tty_getchar() {
	uint8_t ch = 0;

	/* got a device? */
	if (tty_device != NULL) {
		/* yes. let it handle the request */
		device_read (tty_device, 0, &ch, 1);
		return ch;
	}

	/* sorry */
	return 0;
}

/* vim:set ts=2 sw=2: */

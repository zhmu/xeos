/*
 * console.c - XeOS Console Driver
 *
 * (c) 2002, 2003 Rink Springer, BSD
 *
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/module.h>

struct DEVICE dev;

extern size_t kbd_read    (struct DEVICE*, addr_t, uint8_t*, size_t);
extern size_t video_write (struct DEVICE*, addr_t, uint8_t*, size_t);

/*
 * main (MODULECONFIG* cf)
 *
 * This is the start of the module.
 *
 */
int
main (MODULECONFIG* cf) {
	struct DEVICE* console_dev;

	/* handle keyboard initialization */
	if (!kbd_init (cf))
		exit (0);

	/* handle video intialization */
	if (!video_init (cf))
		exit (0);

	/* register our device */
	memset (&dev, 0, sizeof (struct DEVICE));
	dev.type = DEVICE_TYPE_CHAR;
	dev.name = cf->name;
	dev.config = cf;
	dev.read = &kbd_read;
	dev.write = &video_write;
	console_dev = register_device (&dev);

	/* active our device as the TTY device */
	register_tty (console_dev);

	/* do the keyboard loop */
	kbd_loop (cf);

	/* NOTREACHED */
}

/* vim:set ts=2 sw=2: */

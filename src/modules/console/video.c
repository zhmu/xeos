/*
 * console.c - XeOS Console Driver
 *
 * (c) 2002, 2003 Rink Springer, BSD
 *
 * ANSI specification from
 * http://astronomy.swin.edu.au/~pbourke/dataformats/ansi.html.
 *
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/module.h>

addr_t  video_base;
addr_t  video_pos = 0;
addr_t  video_crtcbase;
uint8_t video_attr = 0x07;
uint8_t	ansi = 0;

/*
 * video_ansi (uint8_t ch)
 *
 * This will handle ANSI char [ch].
 *
 */
int
video_ansi (uint8_t ch) {
	/* have we done more than 3 chars? */
	if (ansi == 4)
		/* yes. auto-disable ANSI mode */
		return 0;

	/* second char not a '[' ? */
	if ((ansi == 1) && (ch != '['))
		/* yes. bail out */
		return 0;

	/* WRITEME */

	/* extra char handeled */
	ansi++;
	return 1;
}

/*
 * video_putch (uint8_t ch)
 *
 * This will write char [ch] to the video memory.
 *
 */
void
video_putch (uint8_t ch) {
	/* if we are in ANSI mode, handle that */
	if (ansi)
		if (video_ansi (ch))
			return;

	switch (ch) {
		case 10: /* newline */
		         video_pos -= (video_pos % 160); video_pos += 160;
		         break;
		 case 8: /* backspace */
						 video_pos -= 2;
						 break;
		case 27: /* escape. are we in ANSI mode? */
						 if (ansi) {
							 /* yes. escape from it */
							 ansi = 0;
							 return;
						 }
						 /* we're in ANSI mode now! */
						 ansi = 1;
						 break;
		default: /* fetch a char */
		         *(uint16_t*)(video_base + video_pos) = ((video_attr << 8) | ch);
						 video_pos += 2;
	}

	/* need to scroll? */
	if (video_pos >= 4000) {
		/* handle the scrolling */
		memcpy ((void*)(video_base),
		        (void*)(video_base + 160),
						(24 * 160));

		/* blank out the bottom line */
		memset ((void*)(video_base + (24 * 160)), 0x0, 160);

		/* one line less */
		video_pos -= 160;
	}
}

/*
 * video_write (struct DEVICE* dev, addr_t offs, uint8_t* buf, size_t len) {
 *
 * This will copy up to [len] bytes from [buf] to the video memory. It will
 * return the number of charachters written.
 *
 */
size_t
video_write (struct DEVICE* dev, addr_t offs, uint8_t* buf, size_t len) {
	size_t i = 0;

	/* keep moving */
	while (len--) {
		/* handle this char */
		video_putch (buf[i]);

		/* next */
		i++;
	}

	/* return the number of bytes copied */
	return i;
}

/*
 * video_init()
 *
 * This will initialize the video card. It will return 0 on failure and non-zero
 * on success.
 *
 */
int
video_init (MODULECONFIG* cf) {
	uint8_t x, y;

	/* register the IO port */
	if (!register_pio (0x3c0 /* cf->resources->port */, 0x1f)) {
		printf ("%s: unable to register io ports 0x%x-0x%x\n", 0x3c0, 0x3df);
		return 0;
	}

	/* check whether we have color or mono video */
	if (inb (0x3cc) & 1) {
		/* color */
		video_crtcbase = 0x3d4; video_base = 0xb8000;
	} else {
		/* mono */
		video_crtcbase = 0x3b4; video_base = 0xb0000;
	}

	/* allocate the address */
	video_base = register_lowmem (video_base, 4000);
	if (!video_base) {
		printf ("%s: unable to register memory 0x%x-0x%x\n", video_base, video_base + 4000);
		return 0;
	}

	/* set the current cursor position */
	video_pos = (10 * 160);

	/* yeppee! */
	return 1;
}

/* vim:set ts=2 sw=2: */

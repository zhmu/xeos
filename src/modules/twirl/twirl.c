/*
 * twirl.c - Twirling thingy
 *
 * (c) 2002, 2003 Rink Springer, BSD
 *
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/module.h>

char twirl[] = "/-\\|/-\\|";

int
main (MODULECONFIG* cf) {
	int tpos = 0;

	uint32_t base = register_lowmem (0xb8000, 4000) + 0x9e;
	while (1) {
		(*(uint8_t*)(base)) = twirl[tpos++];
		tpos %= 8;
		thread_switch();
	}
}

/* vim:set ts=2 sw=2: */

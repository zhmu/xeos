/*
 * kprintf.c - XeOS kprintf() call.
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will handle the kprintf() call.
 *
 */
#include <sys/types.h>
#include <sys/tty.h>
#include <stdarg.h>

uint8_t hextab[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                       'A', 'B', 'C', 'D', 'E', 'F' };

uint8_t kprintf_tmp[40];

/*
 * printf_uint (uint32_t i)
 *
 * This will put unsigned integer [i] into [kprintf_tmp]. 
 *
 */
void
printf_uint (uint32_t i) {
	int j;
	int sz = sizeof (kprintf_tmp) - 1;

	for (j = 0; j < sz; j++)
		kprintf_tmp[j] = ' ';
	kprintf_tmp[sizeof (kprintf_tmp)] = 0;

	while (i > 0 && sz >= 0) {
		kprintf_tmp[sz] = hextab[i % 10];
		sz--;
		i /= 10;
	}	
}

/*
 * vaprintf (char* fmt, va_list ap)
 *
 * This will handle writing to the console from the kernel.
 *
 * NOTICE! This code is _NOT_ re-entrant!
 *
 */
void
vaprintf (char* fmt, va_list ap) {
	uint32_t i;
	char* s;

	/* keep going till the end */
	while (*fmt) {
		switch (*fmt) {
			case '%': /* formatted output */
								fmt++;
								/* skip numbers, they are unsupported */
								while ((*fmt >= '0') && (*fmt <= '9'))
									fmt++;
								switch (*fmt) {
										case 's': /* string */
															s = va_arg (ap, char*);
															if (s)
																tty_puts (s);
															else
																tty_puts ("(null)");
															break;
										case 'c': /* char */
															tty_putchar (va_arg (ap, char));
															break;
										case 'x': /* hex int */
															i = va_arg (ap, unsigned int);
															if (i >= 0x10000000)
																tty_putchar (hextab[(i >> 28) & 0xf]);
															if (i >= 0x1000000)
																tty_putchar (hextab[(i >> 24) & 0xf]);
															if (i >= 0x100000)
																tty_putchar (hextab[(i >> 20) & 0xf]);
															if (i >= 0x10000)
																tty_putchar (hextab[(i >> 16) & 0xf]);
															if (i >= 0x1000)
																tty_putchar (hextab[(i >> 12) & 0xf]);
															if (i >= 0x100)
																tty_putchar (hextab[(i >>  8) & 0xf]);
															if (i >= 0x10)
																tty_putchar (hextab[(i >>  4) & 0xf]);
															tty_putchar (hextab[i & 0xf]);
															break;
										case 'u': /* unsigned int */
															i = va_arg (ap, unsigned int);
															printf_uint (i); i = 0;
															while (kprintf_tmp[i]) {
																if (kprintf_tmp[i] != ' ')
																	tty_putchar (kprintf_tmp[i]);

																i++;
															}
															break;
										 default: /* unknown */
															tty_putchar (*fmt);
															break;
								}
								break;
			 default: /* just show the character */
								tty_putchar (*fmt);
		}

		/* next */
		fmt++;
	}
}

/*
 * kprintf (char* fmt, ...)
 *
 * This will handle writing to the console from the kernel.
 *
 */
void
kprintf (char* fmt, ...) {
	va_list ap;

	va_start (ap, fmt);
	vaprintf (fmt, ap);
	va_end (va);
}

/* vim:set ts=2: */

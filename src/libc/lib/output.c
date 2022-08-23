/*
 * output.c - XeOS printf() call.
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will handle output.
 *
 */
#include <sys/types.h>
#include <stdarg.h>

const uint8_t hextab[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                       'A', 'B', 'C', 'D', 'E', 'F' };

uint8_t printf_tmp[40];

/*
 * printf_uint (uint32_t i)
 *
 * This will put unsigned integer [i] into [printf_tmp]. 
 *
 */
void
printf_uint (uint32_t i) {
	int j;
	int sz = sizeof (printf_tmp) - 1;

	for (j = 0; j < sz; j++)
		printf_tmp[j] = ' ';
	printf_tmp[sizeof (printf_tmp)] = 0;

	while (i > 0 && sz >= 0) {
		printf_tmp[sz] = hextab[i % 10];
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
								/* skip any numbers [they are unsupported at this time] */
								while ((*fmt >= '0') && (*fmt <= '9'))
									fmt++;
								switch (*fmt) {
										case 's': /* string */
															s = va_arg (ap, char*);
															if (s)
																puts (s);
															else
																puts ("(null)");
															break;
										case 'c': /* char */
															putchar (va_arg (ap, char));
															break;
										case 'x': /* hex int */
															i = va_arg (ap, unsigned int);
															if (i >= 0x10000000)
																putchar (hextab[(i >> 28) & 0xf]);
															if (i >= 0x1000000)
																putchar (hextab[(i >> 24) & 0xf]);
															if (i >= 0x100000)
																putchar (hextab[(i >> 20) & 0xf]);
															if (i >= 0x10000)
																putchar (hextab[(i >> 16) & 0xf]);
															if (i >= 0x1000)
																putchar (hextab[(i >> 12) & 0xf]);
															if (i >= 0x100)
																putchar (hextab[(i >>  8) & 0xf]);
															if (i >= 0x10)
																putchar (hextab[(i >>  4) & 0xf]);
															putchar (hextab[i & 0xf]);
															break;
										case 'u': /* unsigned int */
															i = va_arg (ap, unsigned int);
															printf_uint (i); i = 0;
															while (printf_tmp[i]) {
																if (printf_tmp[i] != ' ')
																	putchar (printf_tmp[i]);

																i++;
															}
															break;
										 default: /* unknown */
															putchar (*fmt);
															break;
								}
								break;
			 default: /* just show the character */
								putchar (*fmt);
		}

		/* next */
		fmt++;
	}
}

/*
 * printf (char* fmt, ...)
 *
 * This will handle writing to the console from userland applications.
 *
 */
void
printf (char* fmt, ...) {
	va_list ap;

	va_start (ap, fmt);
	vaprintf (fmt, ap);
	va_end (va);
}

/*
 * puts (const char* s)
 *
 * This will put string [s] on the screen.
 *
 */
int
puts (const char* s) {
	while (*s)
		putchar (*s++);
	return 0;
}

/* vim:set ts=2 sw=2: */

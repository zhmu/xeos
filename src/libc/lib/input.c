/*
 * input.c - XeOS input calls.
 * (c) 2003 Rink Springer, BSD licensed
 *
 * This code will handle input.
 *
 */
#include <sys/types.h>
#include <stdarg.h>

#define INPUT_LEN	64
char input_tmp[INPUT_LEN];

const char*
gets() {
	int i = 0;
	int ch = 0;

	/* keep going */
	while (ch != 13) {
		/* fetch a char */
		while (1) {
			ch = getchar();
			if (ch) break;
			thread_switch();
		}
		
		switch (ch) {
			case 8: /* backspace. got chars to burn? */
							if (i) {
			        	/* yes. zap the last char */
			        	input_tmp[--i] = ch;
			        	putchar (8); putchar (32); putchar (8);
							}
							break;
		 case 13: /* return. leave */
		          putchar ('\n');
							break;
		 default: /* dunno. just add it */
		          input_tmp[i++] = ch;
		          putchar (ch);
		}
	}

	/* null-terminate it and leave */
	input_tmp[i] = 0;
	return input_tmp;
}

/* vim:set ts=2 sw=2: */

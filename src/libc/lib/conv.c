/*
 * conv.c - XeOS conversation calls.
 * (c) 2003 Rink Springer, BSD licensed
 *
 * This code will handle stuff that converts integers and such.
 *
 */
#include <sys/types.h>
#include <stdarg.h>

/*
 * get_digit (char ch, int base)
 *
 * This will retrieve the value of [ch] in [base], or -1 on failure.
 *
 */
int
get_digit (char ch, int base) {
	if ((ch >= '0') && (ch <= '9')) return (ch - '0');

	if (base == 16) {
		if ((ch >= 'a') && (ch <= 'f')) return (ch - 'a' + 10);
		if ((ch >= 'A') && (ch <= 'F')) return (ch - 'A' + 10);
	}

	return -1;
}

/*
 * strtol(const char *nptr, char **endptr, int base)
 * 
 * (man strtol)
 *
 */
long int
strtol(const char *nptr, char **endptr, int base) {
	long int li = 0;
	long int lj;
	int count = 0;
	char* ptr;
	int i, j;

	/* first, we must see how much charachters we have */
	ptr = (char*)nptr;
	while (get_digit (*ptr, base) != -1) {
		ptr++; count++;
	}
	
	/* store the final position */
	if (endptr)
		*endptr = ptr;

	/* handle all digits */
	ptr = (char*)nptr;
	for (i = 0; i < count; i++) {
		/* calculate the power */
		lj = 1;
		for (j = 0; j < (count - i - 1); j++)
			lj *= (long int)base;
		li += ((long int)get_digit (*ptr, base) * (long int)lj);

		/* next */
		ptr++;
	}

	return li;
}

/* vim:set ts=2 sw=2: */

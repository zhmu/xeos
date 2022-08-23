/*
 * string.c - XeOS string stuff.
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will handle the string stuff.
 *
 */
#include <sys/types.h>
#include <stdarg.h>

#if 0
/*
 * memcpy (void* dst, void* src, unsigned int len)
 *
 * This will copy [len] bytes from [src] to [dest].
 *
 */
void*
memcpy (void* dst, const void* src, unsigned int len) {
	uint8_t* sptr = (uint8_t*)src;
	uint8_t* dptr = (uint8_t*)dst;
	while (len--) *dptr++ = *sptr++;
	return dst;
}

/*
 * memset (void* b, int c, unsigned int len)
 *
 * This will fill [len] bytes of [b] with [c].
 *
 */
void*
memset (void* b, int c, unsigned int len) {
	uint8_t* ptr = (uint8_t*)b;
	while (len--) *ptr++ = c;
	return b;
}

/*
 * strlen (const char* s)
 *
 * This will return the length of zero-terminated string [s].
 *
 */
int
strlen (const char* s) {
	int len = 0;
	while (*s++) len++;
	return len;
}

#if 0
/*
 * strdup (const char* s)
 *
 * This will duplicate string [s] in a freshly kmalloc()-ed piece of memory. The
 * result will be returned.
 *
 */
char*
strdup (const char* s) {
	char* ptr;
	ptr = (char*)kmalloc (strlen (s) + 1);
	memcpy (ptr, s, strlen (s) + 1);
	return ptr;
}
#endif

/*
 * strcmp (const char* s1, const char* s2)
 *
 * This will return zero if [s1] and [s2] are equal, or 1 if they are not.
 *
 */
int
strcmp (const char* s1, const char* s2) {
	while (*s1)
		if (*s1++ != *s2++)
			return 1;
	return (*s1 == *s2) ? 0 : 1;
}

/*
 * strcpy (char* dest, const char* src)
 *
 * This will copy string [src] to [dst].
 *
 */
char*
strcpy (char* dest, const char* src) {
	memcpy (dest, src, strlen (src) + 1);
}

/*
 * memcmp (const char* s1, const char* s2, size_t len)
 *
 * This will compare up to [len] bytes of [s1] and [s2]. It will return 0 on
 * success or non-zero on failure.
 *
 */
int
memcmp (const char* s1, const char* s2, size_t len) {
	while (len--)
		/* match? */
		if (*s1 != *s2)
			/* no. too bad */
			return 1;

	/* match */
	return 0;
}

/*
 * strchr (const char* s1, char ch)
 *
 * This will search for [ch] in [s1]. It will return a pointer to [ch] in [s1]
 * on success or NULL if it cannot be found.
 *
 */
char*
strchr (const char* s1, char ch) {
	while (1) {
		/* match? */
		if (*s1 == ch)
			/* yes. return it */
			return s1;

		/* null? */
		if (!*s1)
			/* yes. bail out */
			break;

		/* next */
		s1++;
	}

	/* no match */
	return NULL;
}

/*
 * strncmp (const char* s1, const char* s2, size_t len)
 *
 * This will compare [s1] and [s2], up to [len] chars.
 *
 */
int
strncmp (const char* s1, const char* s2, size_t len) {
	if (!len)
		return 1;
	while (--len)
		if (*s1++ != *s2++)
			return 1;
	return (*s1 == *s2) ? 0 : 1;
}
#endif

/* vim:set ts=2 sw=2: */

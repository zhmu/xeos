/*
 * string.c - XeOS string stuff.
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will handle the string stuff.
 *
 */
#include <sys/types.h>
#include <sys/kmalloc.h>
#include <stdarg.h>

/*
 * kmemcpy (void* dst, void* src, size_t len)
 *
 * This will copy [len] bytes from [src] to [dest].
 *
 */
void
kmemcpy (void* dst, const void* src, size_t len) {
	uint8_t* sptr = (uint8_t*)src;
	uint8_t* dptr = (uint8_t*)dst;
	while (len--) *dptr++ = *sptr++;
}

/*
 * kmemset (void* b, int c, size_t len)
 *
 * This will fill [len] bytes of [b] with [c].
 *
 */
void
kmemset (void* b, int c, size_t len) {
	uint8_t* ptr = (uint8_t*)b;
	while (len--) *ptr++ = c;
}

/*
 * kstrlen (const char* s)
 *
 * This will return the length of zero-terminated string [s].
 *
 */
size_t
kstrlen (const char* s) {
	size_t len = 0;
	if (s == NULL)
		return 0;

	while (*s++) len++;
	return len;
}

/*
 * kstrdup (const char* s)
 *
 * This will duplicate string [s] in a freshly kmalloc()-ed piece of memory. The
 * result will be returned.
 *
 */
char*
kstrdup (const char* s) {
	char* ptr;

	if (s == NULL)
		return NULL;

	ptr = (char*)kmalloc (NULL, kstrlen (s) + 1, 0);
	kmemcpy (ptr, s, kstrlen (s) + 1);
	return ptr;
}

/*
 * kstrcmp (const char* s1, const char* s2)
 *
 * This will return zero if [s1] and [s2] are equal, or 1 if they are not.
 *
 */
int
kstrcmp (const char* s1, const char* s2) {
	while (*s1)
		if (*s1++ != *s2++)
			return 1;
	return (*s1 == *s2) ? 0 : 1;
}

/*
 * kstrncmp (const char* s1, const char* s2, size_t len)
 *
 * This will compare [s1] and [s2], up to [len] chars.
 *
 */
int
kstrncmp (const char* s1, const char* s2, size_t len) {
	while (len--)
		if (*s1++ != *s2++)
			return 1;
	return (*s1 == *s2) ? 0 : 1;
}

/* vim:set ts=2 sw=2: */

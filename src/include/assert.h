/*
 * assert.h - XeOS Debugging Stuff
 * (c) 2002, 2003 Rink Springer, BSD
 *
 * This file contains the ASSERT macro!
 *
 */
#ifndef __ASSERT_H__
#define __ASSERT_H__

#define ASSERT(x) { \
	if (!x) { panic ("Assertion failed! File %s Line %u Function %s", __FILE__, __LINE__, __PRETTY_FUNCTION__); } \
}

#define assert ASSERT

#endif

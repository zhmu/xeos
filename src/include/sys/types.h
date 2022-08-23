/*
 * types.h - XeOS Type Declarations
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This file declares all types used within XeOS.
 *
 */
#ifndef __TYPES_H__
#define __TYPES_H__

#define NULL 0

/* basic types */
typedef	char		int8_t;
typedef unsigned char	uint8_t;
typedef	short		int16_t;
typedef unsigned short	uint16_t;
typedef	int		int32_t;
typedef unsigned int	uint32_t;
typedef	long		int64_t;
typedef	unsigned long	uint64_t;

/* size, address: depends on platform */
typedef uint32_t	size_t;
typedef uint32_t	ssize_t;
typedef uint32_t	addr_t;

#endif

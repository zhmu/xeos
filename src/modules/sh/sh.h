/*
 * sh.h - XeOS Shell include file
 *
 * (c) 2002, 2003 Rink Springer, BSD
 *
 */
#ifndef __SH_H__
#define __SH_H__

/* shell command structure */
struct SHELLCOMMAND {
	char*	cmd;
	char*	help;
	int	(*func)(char* args);
};

#endif /* __SH_H__ */

/*
 * cmd.c - XeOS Shell commands
 *
 * (c) 2002, 2003 Rink Springer, BSD
 *
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/module.h>
#include "sh.h"

extern struct SHELLCOMMAND shell_commands[];

/*
 * cmd_help (char* arg)
 *
 * This will display help.
 *
 */
int
cmd_help (char* arg) {
	struct SHELLCOMMAND* scmd = shell_commands;

	/* display all commands */
	while (scmd->cmd) {
		/* display them */
		printf ("%s - %s\n", scmd->cmd, scmd->help);

		/* next */
		scmd++;
	}
	
	return 0;
}

/*
 * cmd_threads (char* arg)
 *
 * This will dump all threads.
 *
 */
int
cmd_threads (char* arg) {
	dump_threads();
	return 0;
}

/*
 * cmd_devices (char* arg)
 *
 * This will dump all devices.
 *
 */
int
cmd_devices (char* arg) {
	dump_devices();
	return 0;
}

/*
 * cmd_kill (char* arg)
 *
 * This will kill a specified thread.
 *
 */
int
cmd_kill (char* arg) {
	long int li = strtol (arg, NULL, 16);

	printf ("attempting to kill thread 0x%x\n", li);
	if (!thread_kill (li))
		printf ("unable to kill thread 0x%x\n", li);
	else
		printf ("killed thread 0x%x\n", li);
	return 0;
}

/*
 * cmd_network (char* arg)
 *
 * This will dump networking info.
 *
 */
int
cmd_network (char* arg) {
	dump_network();
	return 0;
}

/*
 * cmd_syscall (char* arg)
 *
 * This will dump syscall info.
 *
 */
int
cmd_syscall (char* arg) {
	dump_syscall();
	return 0;
}

/*
 * cmd_ipv4()
 *
 * IPv4 debugging command!
 *
 */
int
cmd_ipv4() {
	ipv4_debug();
	return 0;
}

/*
 * cmd_argtest()
 *
 * Argument testing stuff.
 *
 */
int
cmd_argtest() {
	debug_argtest (0x1234, 0x5678, 0xabcd, 0xef00);
	return 0;
}

/*
 * cmd_fs()
 *
 * This will dump filesystem info.
 *
 */
int
cmd_fs() {
	dump_filesys();
	return 0;
}

/* vim:set ts=2 sw=2: */

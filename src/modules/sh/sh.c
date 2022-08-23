/*
 * sh.c - XeOS Shell!
 *
 * (c) 2002, 2003 Rink Springer, BSD
 *
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/module.h>
#include "sh.h"

/* macros to ease our pain */
#define DECL_CMD(x) int (x)(char* args);

/* forward declarations for all commands */
DECL_CMD(cmd_help)
DECL_CMD(cmd_threads)
DECL_CMD(cmd_devices)
DECL_CMD(cmd_kill)
DECL_CMD(cmd_network)
DECL_CMD(cmd_syscall)
DECL_CMD(cmd_ipv4)
DECL_CMD(cmd_argtest)
DECL_CMD(cmd_fs)

/* supported commands */
struct SHELLCOMMAND shell_commands[] = {
	{  "help",      "Shows this help",   &cmd_help },
	{  "threads",   "Dumps the threads", &cmd_threads },
	{  "devices",   "Dumps the devices", &cmd_devices },
	{  "kill",      "Kill a thread",     &cmd_kill },
	{  "network",   "Network info",			 &cmd_network },
	{  "syscall",   "Syscall info",			 &cmd_syscall },
	{  "ipv4",  	  "IPv4 info",				 &cmd_ipv4 },
	{  "argtest", 	"Argument test",		 &cmd_argtest },
	{  "fs",				"Filesystems info",	 &cmd_fs },
	{  NULL,        NULL,                NULL }
};

char prompt[] = "sh$ ";

/*
 * lookup_cmd (char* cmd)
 *
 * This will look command [cmd] up and return a pointer to the command, or
 * NULL if it could not be found.
 *
 */
struct SHELLCOMMAND*
lookup_cmd (char* cmd) {
	struct SHELLCOMMAND* scmd = shell_commands;

	/* scan them all */
	while (scmd->cmd) {
		/* match? */
		if (!strcmp (scmd->cmd, cmd))
			/* yes. got it */
			return scmd;

		/* next */
		scmd++;
	}

	/* no match */
	return NULL;
}

/*
 * main (MODULECONFIG* cf)
 *
 * This is the start of the module.
 *
 */
int
main (MODULECONFIG* cf) {
	char* s;
	char* arg;
	struct SHELLCOMMAND* cmd;

	libmain (NULL);

	int i = debug_argtest (0x68, 0x42, 0x80, 0x97);
	printf ("ARGTEST=%u!!!\n", i); 

	/* testing */
	mount ("hd0a", "/", "fatfs", 0);

	while (1) {
		/* print the prompt and fetch the command */
		printf (prompt);
		s = gets();

		/* do we have a space? */
		arg = strchr (s, ' ');
		if (arg != NULL) {
			/* yes. isolate the arguments */
			*arg = 0; arg++;
			while (*arg == ' ')
				arg++;
		} else {
			/* no arguments */
			arg = "";
		}

		/* got a command? */
		if (*s) {
			/* yes. look the command up */
			cmd = lookup_cmd (s);
			if (cmd == NULL) {
				/* this failed. complain */
				printf ("unknown command\n");
			} else {
				/* execute the command */
				cmd->func (arg);
			}
		}
	}

	/* NOTREACHED */
}

/* vim:set ts=2 sw=2: */

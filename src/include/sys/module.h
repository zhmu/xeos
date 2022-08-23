/*
 * module.h - XeOS Kernel Modules
 * (c) 2002, 2003 Rink Springer, BSD
 *
 * This is an include file for the modules.
 *
 */
#ifndef __MODULE_H__
#define __MODULE_H__

typedef struct sMODULERESOURCES {
	uint16_t port;
	uint8_t  irq;
	uint8_t  drq;
  uint16_t flags;
} MODULERESOURCES;

struct sMODULECONFIG;

typedef struct sMODULEDATA {
	char* name;																	/* name */
	int (*init)(struct sMODULECONFIG* cf);			/* initializer */
	int (*done)(struct sMODULECONFIG* cf);			/* deinitializer */
	int (*preinit)(struct sMODULECONFIG* cf);		/* pre initializer */
	int (*vfs)(struct sMODULECONFIG* cf);				/* vfs services */
} MODULEDATA;

typedef struct sMODULECONFIG {
	char*							name;
	MODULEDATA*				module;
	MODULERESOURCES*	resources;
} MODULECONFIG;

void modules_init();
MODULECONFIG* module_find (char* name);

#endif

/* vim:set ts=2: */

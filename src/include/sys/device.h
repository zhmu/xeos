/*
 * device.h - XeOS Device Manager
 * (c) 2002, 2003 Rink Springer, BSD
 *
 * This is a generic include file which describes the devices.
 *
 */
#include <sys/types.h>
#include <sys/semaphore.h>
#include <sys/module.h>
#include <sys/thread.h>
#include <md/config.h>

#ifndef __DEVICE_H__
#define __DEVICE_H__

#define DEVICE_TYPE_CHAR    1
#define DEVICE_TYPE_BLOCK   2 
#define DEVICE_TYPE_NETWORK 3

#define DEVICE_STATE_IDLE   0
#define DEVICE_STATE_LOCKED 1

struct DEVICE;

/* DEVICE_BUFSIZE is the size we use to store internals transfer buffers */
#define DEVICE_BUF_SIZE		(PAGESIZE)

/*
 * DEVICE_NETDATA is data belonging to a network device
 *
 */
struct DEVICE_NETDATA {
	uint8_t	  	   hw_addr[6];	/* hardware address */

	void*					 data;				/* more data */
};

/*
 * DEVICE is a structure which covers about any device in the system.
 *
 */
struct DEVICE {
	uint32_t	     type;      /* device type */
	char*          name;      /* device name */
	uint16_t       state;     /* current state */

	struct THREAD* thread;    /* thread */
	struct DEVICE* parent;    /* owner */
	struct DEVICE* next;      /* next device */

	SEMAPHORE      readsem;   /* device read semaphore */
	SEMAPHORE      writesem;  /* device write semaphore */
	MODULECONFIG*  config;    /* configuration */

	size_t         datasize;  /* device specific data length */
	void*          data;      /* device specific data */

	uint8_t*	     readbuf;		/* read buffer */
	uint8_t*	     writebuf;	/* write buffer */

  size_t (*read)(struct DEVICE* dev, addr_t offs,uint8_t* buf,size_t len);
  size_t (*write)(struct DEVICE* dev, addr_t offs,uint8_t* buf,size_t len);
};

#ifdef __KERNEL
extern struct DEVICE* coredevice;

void           device_init();
struct DEVICE* device_register(struct THREAD*,struct DEVICE*);
int            device_unregister (struct DEVICE* dev);
void           device_dump();
struct         DEVICE* device_find (char*);
size_t         device_read (struct DEVICE* dev, size_t offs, void* buffer, size_t count);
size_t         device_write (struct DEVICE* dev, size_t offs, void* buffer, size_t count);
#endif /* __KERNEL */
#endif

/* vim:set ts=2 sw=2: */

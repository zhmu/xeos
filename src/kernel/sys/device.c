/*
 * device.c - XeOS Device Manager
 * (c) 2002, 2003 Rink Springer, BSD licensed
 *
 * This code will handle the allocation of devices.
 *
 */
#include <sys/types.h>
#include <sys/kmalloc.h>
#include <sys/device.h>
#include <sys/thread.h>
#include <sys/tty.h>
#include <sys/semaphore.h>
#include <lib/lib.h>
#include <md/vm.h>

struct DEVICE* coredevice = NULL;

/*
 * device_register (struct THREAD* t, struct DEVICE* dev)
 *
 * This will register a new device for thread [t], based on [dev]. It will
 * return a pointer to the new device, or NULL on failure.
 *
 */
struct DEVICE*
device_register (struct THREAD* t, struct DEVICE* dev) {
	struct DEVICE* device = coredevice;
	struct DEVICE* newdevice;

	/* initialize the new device */
	newdevice = (struct DEVICE*)kmalloc (NULL, sizeof (struct DEVICE), 0);
	kmemcpy (newdevice, dev, sizeof (struct DEVICE));
	newdevice->name = (char*)kstrdup (dev->name);
	newdevice->state = DEVICE_STATE_IDLE;
	sem_create (&newdevice->readsem);
	sem_create (&newdevice->writesem);
	newdevice->readbuf = (uint8_t*)kmalloc (NULL, DEVICE_BUF_SIZE, 0);
	newdevice->writebuf = (uint8_t*)kmalloc (NULL, DEVICE_BUF_SIZE, 0);
	newdevice->thread = t;

	/* the thread now is a device thread */
	thread_mark_device (t);

	/* find the end of the chain */
	if (device)
		while (device->next)
			device = device->next;

	/* got a core device? */
	if (coredevice == NULL)
		/* no. replace it */
		coredevice = newdevice;
	else
		device->next = newdevice;

	/* is this device owned by a parent device? */
	if (newdevice->parent == NULL) {
		/* no. got an address? */
		if (newdevice->config->resources != NULL) {
			/* yes. display the address */
			kprintf ("%s at", newdevice->name);
			if (newdevice->config->resources->port)
				kprintf (" io 0x%x", newdevice->config->resources->port);
			if (newdevice->config->resources->irq)
				kprintf (" irq 0x%x", newdevice->config->resources->irq);
			if (newdevice->config->resources->drq)
				kprintf (" drq 0x%x", newdevice->config->resources->drq);
		} else {
			/* no. just be generic */
			kprintf ("%s detected", newdevice->name);
		}
	} else {
		/* yes. display the physical address */
		kprintf ("%s at %s", newdevice->name, newdevice->parent->name);
	}
	kprintf ("\n");

	/* got data to allocate? */
	if (newdevice->datasize)
		newdevice->data = (void*)kmalloc (NULL, newdevice->datasize, 0);

	/* all set */
	return newdevice;
}

/*
 * device_init()
 *
 * This will initialize the device manager.
 *
 */
void
device_init() {
	/* nothing to do! */
}

/*
 * device_dump()
 *
 * This will show a listing of all available devices.
 *
 */
void
device_dump() {
	struct DEVICE* dev = coredevice;

	kprintf ("Device dump\n");

	/* list them all */
	while (dev) {
		kprintf ("  %x: device '%s' owned by 0x%x\n", dev, dev->name, dev->thread);
		dev = dev->next;
	}
}

/*
 * device_unregister (struct DEVICE* dev)
 *
 * This will unregister device [dev]. It will return zero on success or -1 on
 * failure.
 *
 */
int
device_unregister (struct DEVICE* dev) {
	struct DEVICE* device = coredevice;
	struct DEVICE* prevdevice = NULL;

	/* is this thread the tty thread? */
	if (tty_getdevice() == dev)
		/* yes. switch to the internal one */
		tty_setdevice ((struct DEVICE*)NULL);

	/* scan for the device */
	while (device) {
		/* match? */
		if (device == dev) {
			/* yes. do we have a previous device? */
			if (prevdevice != NULL)
				/* yes. update the previous' device's next field */
				prevdevice->next = device->next;
			else
				/* no. we were the core device. update that then */
				coredevice = device->next;

			/* remove the buffers */
			kfree (dev->readbuf);
			kfree (dev->writebuf);

			/* all set */
			kfree (dev);
			return 0;
		}

		/* next */
		prevdevice = device; device = device->next;
	}

	/* device not found */
	return -1;
}

/*
 * device_read (struct DEVICE* dev, size_t offs, void* buffer, size_t count) {
 *
 * This will read [count] bytes into [buffer] from device [dev] at offset
 * [offs]. It will return the number of bytes read.
 *
 */
size_t
device_read (struct DEVICE* dev, size_t offs, void* buffer, size_t count) {
	size_t i;
  size_t (*_read)(struct DEVICE* dev, addr_t offs,uint8_t* buf,size_t len);
	uint8_t* buf = dev->readbuf;
	addr_t addr = arch_vm_getmap();

	/* wait for the device to become idle */
	sem_get (&dev->readsem);
	_read = dev->read;

	/* switch to the device's memory table */
	arch_vm_setthreadmap (dev->thread);

	/* fetch the data */
	i = _read (dev, offs, (void*)buf, count);

	/* switch back to the original thread's memory table */
	arch_vm_setmap (addr);

	/* copy data over XXX */
	kmemcpy (buffer, dev->readbuf, i);

	/* release the device */
	sem_release (&dev->readsem);

	/* that's all */
	return i;
}

/*
 * device_write (struct DEVICE* dev, size_t offs, void* buffer, size_t count)
 *
 * This will write [count] bytes from [buffer] to device [dev] at offset [offs].
 * It will return the number of bytes written.
 *
 */
size_t
device_write (struct DEVICE* dev, size_t offs, void* buffer, size_t count) {
	size_t i;
  size_t (*_write)(struct DEVICE* dev, addr_t offs,uint8_t* buf,size_t len);
	uint8_t* buf = dev->writebuf;
	addr_t addr = arch_vm_getmap();

	/* wait for the device to become idle */
	sem_get (&dev->writesem);
	_write = dev->write;

	/* copy data over XXX */
	kmemcpy (dev->writebuf, buffer, count);

	/* switch to the device's memory table */
	arch_vm_setthreadmap (dev->thread);

	/* fetch the data */
	i = _write (dev, offs, (void*)buf, count);

	/* switch back to the original thread's memory table */
	arch_vm_setmap (addr);

	/* release the device */
	sem_release (&dev->writesem);

	/* that's all */
	return i;
}

/*
 * device_find (char* devname)
 *
 * This will scan the device chain for device [devname]. It will return a
 * pointer to the device if found or NULL on failure.
 */
struct DEVICE*
device_find (char* devname) {
	struct DEVICE* dev = coredevice;

	/* scan them all */
	while (dev) {
		/* match? */
		if (!kstrcmp (dev->name, devname))
			/* yes. got it */
			return dev;

		/* next */
		dev = dev->next;
	}
	return NULL;
}

/* vim:set ts=2 sw=2: */

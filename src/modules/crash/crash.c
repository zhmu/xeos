/*
 * crash.c - XeOS Crash Driver
 *
 * (c) 2002, 2003 Rink Springer, BSD
 *
 */
#include <sys/types.h>
#include <sys/device.h>
#include <sys/module.h>

void main() {
	asm ("mov $0x01, %eax\n");
	asm ("mov $0x02, %ebx\n");
	asm ("mov $0x03, %ecx\n");
	asm ("mov $0x04, %edx\n");
	asm ("mov $0x05, %esi\n");
	asm ("mov $0x06, %edi\n");
	asm ("mov $0x07, %ebp\n");

	/* BYE! */
#if 1
	asm ("xor %edx,%edx\n");
	asm ("xor %eax,%eax\n");
	asm ("div %edx,%eax\n");
#else
	*(uint32_t*)NULL = 0;
#endif
	
	printf ("???");

	while (1);
}

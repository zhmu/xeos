TARGET	=	syscalls.o
GENSYSCALLS =	./generate_syscalls.pl
DEPTH	=	../
CFLAGS	=	-nostdinc -I../../include -D__LIBC

include 	../../mk/defs.mk

syscalls.o:	syscalls.c
		$(CC) $(CFLAGS) -c -o syscalls.o syscalls.c

syscalls.c:	SYSCALLS
		$(GENSYSCALLS)

clean:
		$(RM) -f syscalls.c syscalls.o

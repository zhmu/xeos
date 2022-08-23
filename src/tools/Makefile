TARGET	= sys2c
CC	= cc

sys2c:	sys2c.o
	      $(CC) -o sys2c sys2c.o

.c.o:	      
	      $(CC) $(CFLAGS) -c -o $@ $<

base-grub.flp:	build-grub.sh
						    ./build-grub.sh

clean:
				rm -rf sys2c sys2c.o

# vim: set ts=2:

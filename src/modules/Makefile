TARGET = modules.c

include		../mk/defs.mk

MODULES = console twirl sh ne2k ipv4 ide fatfs

modules.c:	$(SYS2C) ../libc/libc.a
		echo > modules.c.tmp
		@for i in $(MODULES); do \
		echo "==> Building $$i/$$i.sys"; \
		cd $$i && $(MAKE) $$i.sys && cd ..; \
		$(SYS2C) $$i/$$i.sys ELF_$$i >> modules.c.tmp; \
		done
		$(MV) modules.c.tmp modules.c

../libc/libc.a:	../libc/Makefile
		cd ../libc && $(MAKE)

$(SYS2C):	../tools/Makefile
		cd ../tools && $(MAKE)

clean:
		@for i in $(MODULES); do \
		echo "==> Cleaning $$i"; \
		cd $$i && rm -f $$i.sys $$i.sys.debug;  cd ..; \
		done
		rm -f modules.c modules.c.tmp

# vim:set ts=8 sw=8:

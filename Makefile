all:	Makefile.tmp
	$(MAKE) -f Makefile.mk

Makefile.tmp:
	./configure.pl

clean:
	$(MAKE) -f Makefile.mk clean

debug:
	$(MAKE) -f Makefile.mk debug

distclean:
	$(MAKE) -f Makefile.mk distclean

install:
	$(MAKE) -f Makefile.mk install

purge:
	$(MAKE) -f Makefile.mk purge

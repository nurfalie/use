standard:	Makefile.tmp
		$(MAKE) -f Makefile.mk

Makefile.tmp:
	./configure.pl

clean:
	$(MAKE) -f Makefile.mk clean

purge:
	$(MAKE) -f Makefile.mk purge

distclean:
	$(MAKE) -f Makefile.mk distclean

install:
	$(MAKE) -f Makefile.mk install

-include ./Makefile.tmp
CC_OPTIONS	= -Wall -Wconversion -Werror -Wextra -Wformat=2 \
		  -Wpointer-arith -Wstack-protector \
                  -Wstrict-overflow=5 -Wstrict-prototypes \
                  -fPIE -fstack-protector-all -pedantic
INCLUDES	= flags.h \
		  use.h \
		  use_tmp.h
INCLUDE_PATH	= -I. -I/usr/include
LIBS		=
SRC		= use.c validate.c

all:	Makefile.tmp $(INCLUDES) $(SRC)
	$(CC) $(CC_OPTIONS) $(DEBUG) $(INCLUDE_PATH) -o use.bin \
	$(SRC) $(LIBS)
	chmod g+rx,o+rx,u+rx use.bin

clean:
	rm -f core use.bin use.core

debug:
	$(MAKE) -e DEBUG=-DDEBUG

distclean: clean purge
	rm -f Makefile.tmp use_tmp.h

install:
	$(MAKE) -f Makefile
	cp -f use.1 ${INSTALL_MANPATH}/. && \
	cp -f use.bash use.bin use.csh use.ksh use.sh use.tcsh \
	${INSTALL_PATH}/. && cp -f use.table ${INSTALL_TABLEFULLPATH}

purge:
	rm -f *~

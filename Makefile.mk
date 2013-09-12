-include ./Makefile.tmp
GCC		=	gcc
GCC_OPTIONS	=	-Wall -Werror -Wextra -Wstack-protector \
			-fPIE -fstack-protector-all -pedantic
INCLUDES	=	flags.h \
			use.h \
			use_tmp.h
INCLUDE_PATH	=	-I/usr/include -I.
SRC		=	use.c validate.c
LIBS		=	-lc -lm

standard:	Makefile.tmp $(SRC) $(INCLUDES)
		$(GCC) $(GCC_OPTIONS) $(DEBUG) $(INCLUDE_PATH) \
		-o use.bin $(SRC) $(LIBS)
		chmod g+rx,o+rx,u+rx use.bin

clean:
	rm -f use.bin use.core core

debug:
	$(MAKE) -e DEBUG=-DDEBUG

purge:
	rm -f *~

distclean: clean purge
	rm -f use_tmp.h Makefile.tmp

install:
	$(MAKE) -f Makefile
	cp -f use.bin use.sh use.csh use.ksh use.bash use.tcsh \
		${INSTALL_PATH}/.
	cp -f use.1 ${INSTALL_MANPATH}/.
	cp -f use.table ${INSTALL_TABLEFULLPATH}

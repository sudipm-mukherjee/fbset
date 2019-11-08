#
# Linux Frame Buffer Device Configuration
#

srcdir = .

CC = gcc
CFLAGS = -Wall -O2
BISON = bison -d
FLEX = flex
INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -m 755
INSTALL_DATA = $(INSTALL) -m 644
RM = rm -f

all: fbset con2fbmap

fbset: fbset.o modes.tab.o lex.yy.o

fbset.o: fbset.c fbset.h fb.h
modes.tab.o: modes.tab.c fbset.h fb.h
lex.yy.o: lex.yy.c fbset.h modes.tab.h

lex.yy.c: modes.l
	$(FLEX) $<

modes.tab.c modes.tab.h: modes.y
	$(BISON) $<

con2fbmap: con2fbmap.o
con2fbmap.o: con2fbmap.c

install: fbset
	$(INSTALL) -d $(DESTDIR)/etc
	$(INSTALL) -d $(DESTDIR)/bin
	$(INSTALL) -d $(DESTDIR)/usr/share/man/man1
	$(INSTALL) -d $(DESTDIR)/usr/share/man/man5
	$(INSTALL_DATA) $(srcdir)/etc/fb.modes.ATI $(DESTDIR)/etc/fb.modes
	$(INSTALL_DATA) $(srcdir)/fb.modes.5 $(DESTDIR)/usr/share/man/man5
	$(INSTALL_PROGRAM) fbset $(DESTDIR)/bin
	$(INSTALL_DATA) $(srcdir)/fbset.8 $(DESTDIR)/usr/share/man/man1/fbset.1
	$(INSTALL_PROGRAM) $(srcdir)/modeline2fb $(DESTDIR)/bin
	$(INSTALL_DATA) $(srcdir)/modeline2fb.1 $(DESTDIR)/usr/share/man/man1
	$(INSTALL_PROGRAM) con2fbmap $(DESTDIR)/bin
	$(INSTALL_DATA) $(srcdir)/con2fbmap.1 $(DESTDIR)/usr/share/man/man1

install-devices:
	if [ ! -c /dev/fb0 ]; then mknod $(DESTDIR)/dev/fb0 c 29 0; fi
	if [ ! -c /dev/fb1 ]; then mknod $(DESTDIR)/dev/fb1 c 29 32; fi
	if [ ! -c /dev/fb2 ]; then mknod $(DESTDIR)/dev/fb2 c 29 64; fi
	if [ ! -c /dev/fb3 ]; then mknod $(DESTDIR)/dev/fb3 c 29 96; fi
	if [ ! -c /dev/fb4 ]; then mknod $(DESTDIR)/dev/fb4 c 29 128; fi
	if [ ! -c /dev/fb5 ]; then mknod $(DESTDIR)/dev/fb5 c 29 160; fi
	if [ ! -c /dev/fb6 ]; then mknod $(DESTDIR)/dev/fb6 c 29 192; fi
	if [ ! -c /dev/fb7 ]; then mknod $(DESTDIR)/dev/fb7 c 29 224; fi

clean:
	$(RM) *.o fbset con2fbmap lex.yy.c modes.tab.c modes.tab.h


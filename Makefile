#
# Linux/m68k Frame Buffer Device Configuration
#

CC =		gcc -Wall -O2 -g
BISON =		bison -d
FLEX =		flex
INSTALL =	install
RM =		rm -f

All:		fbset


fbset:		fbset.o modes.tab.o lex.yy.o

fbset.o:	fbset.c fbset.h
modes.tab.o:	modes.tab.c fbset.h
lex.yy.o:	lex.yy.c fbset.h modes.tab.h

lex.yy.c:	modes.l
		$(FLEX) modes.l

modes.tab.c:	modes.y
		$(BISON) modes.y

install:	fbset
		if [ -f /sbin/fbset ]; then rm /sbin/fbset; fi
		$(INSTALL) fbset /usr/sbin
		$(INSTALL) fbset.8 /usr/man/man8
		$(INSTALL) fb.modes.5 /usr/man/man5
		if [ ! -c /dev/fb0current ]; then mknod /dev/fb0current c 29 0; fi
		if [ ! -c /dev/fb0autodetect ]; then mknod /dev/fb0autodetect c 29 1; fi
		if [ ! -c /dev/fb0user0 ]; then mknod /dev/fb0user0 c 29 24; fi
		if [ ! -c /dev/fb0user1 ]; then mknod /dev/fb0user1 c 29 25; fi
		if [ ! -c /dev/fb0user2 ]; then mknod /dev/fb0user2 c 29 26; fi
		if [ ! -c /dev/fb0user3 ]; then mknod /dev/fb0user3 c 29 27; fi
		if [ ! -c /dev/fb0user4 ]; then mknod /dev/fb0user4 c 29 28; fi
		if [ ! -c /dev/fb0user5 ]; then mknod /dev/fb0user5 c 29 29; fi
		if [ ! -c /dev/fb0user6 ]; then mknod /dev/fb0user6 c 29 30; fi
		if [ ! -c /dev/fb0user7 ]; then mknod /dev/fb0user7 c 29 31; fi

install-amiga:
		if [ ! -c /dev/fb0ntsc ]; then mknod /dev/fb0ntsc c 29 2; fi
		if [ ! -c /dev/fb0ntsc-lace ]; then mknod /dev/fb0ntsc-lace c 29 3; fi
		if [ ! -c /dev/fb0pal ]; then mknod /dev/fb0pal c 29 4; fi
		if [ ! -c /dev/fb0pal-lace ]; then mknod /dev/fb0pal-lace c 29 5; fi
		if [ ! -c /dev/fb0multiscan ]; then mknod /dev/fb0multiscan c 29 6; fi
		if [ ! -c /dev/fb0multiscan-lace ]; then mknod /dev/fb0multiscan-lace c 29 7; fi
		if [ ! -c /dev/fb0a2024-10 ]; then mknod /dev/fb0a2024-10 c 29 8; fi
		if [ ! -c /dev/fb0a2024-15 ]; then mknod /dev/fb0a2024-15 c 29 9; fi
		if [ ! -c /dev/fb0euro36 ]; then mknod /dev/fb0euro36 c 29 10; fi
		if [ ! -c /dev/fb0euro36-lace ]; then mknod /dev/fb0euro36-lace c 29 11; fi
		if [ ! -c /dev/fb0euro72 ]; then mknod /dev/fb0euro72 c 29 12; fi
		if [ ! -c /dev/fb0euro72-lace ]; then mknod /dev/fb0euro72-lace c 29 13; fi
		if [ ! -c /dev/fb0super72 ]; then mknod /dev/fb0super72 c 29 14; fi
		if [ ! -c /dev/fb0super72-lace ]; then mknod /dev/fb0super72-lace c 29 15; fi
		if [ ! -c /dev/fb0dblntsc ]; then mknod /dev/fb0dblntsc c 29 16; fi
		if [ ! -c /dev/fb0dblntsc-ff ]; then mknod /dev/fb0dblntsc-ff c 29 17; fi
		if [ ! -c /dev/fb0dblntsc-lace ]; then mknod /dev/fb0dblntsc-lace c 29 18; fi
		if [ ! -c /dev/fb0dblpal ]; then mknod /dev/fb0dblpal c 29 19; fi
		if [ ! -c /dev/fb0dblpal-ff ]; then mknod /dev/fb0dblpal-ff c 29 20; fi
		if [ ! -c /dev/fb0dblpal-lace ]; then mknod /dev/fb0dblpal-lace c 29 21; fi
		if [ ! -c /dev/fb0vga ]; then mknod /dev/fb0vga c 29 22; fi
		if [ ! -c /dev/fb0vga70 ]; then mknod /dev/fb0vga70 c 29 23; fi

install-atari:
		if [ ! -c /dev/fb0stlow ]; then mknod /dev/fb0stlow c 29 2; fi
		if [ ! -c /dev/fb0stmid ]; then mknod /dev/fb0stmid c 29 3; fi
		if [ ! -c /dev/fb0sthigh ]; then mknod /dev/fb0sthigh c 29 4; fi
		if [ ! -c /dev/fb0ttlow ]; then mknod /dev/fb0ttlow c 29 5; fi
		if [ ! -c /dev/fb0ttmid ]; then mknod /dev/fb0ttmid c 29 6; fi
		if [ ! -c /dev/fb0tthigh ]; then mknod /dev/fb0tthigh c 29 7; fi
		if [ ! -c /dev/fb0vga2 ]; then mknod /dev/fb0vga2 c 29 8; fi
		if [ ! -c /dev/fb0vga4 ]; then mknod /dev/fb0vga4 c 29 9; fi
		if [ ! -c /dev/fb0vga16 ]; then mknod /dev/fb0vga16 c 29 10; fi
		if [ ! -c /dev/fb0vga256 ]; then mknod /dev/fb0vga256 c 29 11; fi
		if [ ! -c /dev/fb0falh2 ]; then mknod /dev/fb0falh2 c 29 12; fi
		if [ ! -c /dev/fb0falh16 ]; then mknod /dev/fb0falh16 c 29 13; fi

clean:
		$(RM) *.o fbset lex.yy.c modes.tab.c modes.tab.h

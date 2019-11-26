/*
 *  Linux/m68k Frame Buffer Device Configuration
 *
 *  © Copyright 1995 by Geert Uytterhoeven
 *			(Geert.Uytterhoeven@cs.kuleuven.ac.be)
 *
 *  --------------------------------------------------------------------------
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License. See the file COPYING in the main directory of the Linux
 *  distribution for more details.
 */


#include <stdio.h>
#include <sys/types.h>


#define VERSION         "Linux/m68k Frame Buffer Device Configuration " \
			"Version 1.0 (12/8/96)\n"  \
			"(C) Copyright 1995 by Geert Uytterhoeven\n"

#define LOW		(0)
#define HIGH		(1)

#define FALSE		(0)
#define TRUE		(1)


struct VideoMode {
    struct VideoMode *next;
    char *name;
    /* geometry */
    u_long xres;
    u_long yres;
    u_long vxres;
    u_long vyres;
    u_long depth;
    /* timings */
    u_long pixclock;
    u_long left;
    u_long right;
    u_long upper;
    u_long lower;
    u_long hslen;
    u_long vslen;
    /* flags */
    u_long hsync : 1;
    u_long vsync : 1;
    u_long csync : 1;
    u_long extsync : 1;
    u_long bcast : 1;
    u_long laced : 1;
    u_long dblscan : 1;
    /* scanrates */
    double hrate;
    double vrate;
};

extern FILE *yyin;
extern int line;
extern const char *Opt_modedb;

extern int yyparse(void);
extern void Die(const char *fmt, ...) __attribute__ ((noreturn));
extern void AddVideoMode(const struct VideoMode *vmode);


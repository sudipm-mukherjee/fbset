/*
 *  Linux/m68k Frame Buffer Device Configuration
 *
 *  © Copyright 1995 by Geert Uytterhoeven
 *		       (Geert.Uytterhoeven@cs.kuleuven.ac.be)
 *
 *  --------------------------------------------------------------------------
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License. See the file COPYING in the main directory of the Linux
 *  distribution for more details.
 */


#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

struct file;
struct inode;

#include <linux/fb.h>

#include "fbset.h"


    /*
     *  Default Frame Buffer Special Device Node
     */

#define DEFAULT_FRAMEBUFFER	"/dev/fb0current"


    /*
     *  Default Video Mode Database File
     */

#define DEFAULT_MODEDBFILE	"/etc/fb.modes"


    /*
     *  Command Line Options
     */

static const char *ProgramName;

static int Opt_now = 0;
static int Opt_show = 0;
static int Opt_showonly = 1;
static int Opt_info = 0;
static int Opt_version = 0;
static int Opt_verbose = 0;
static int Opt_xfree86 = 0;

static const char *Opt_ifb = NULL;
static const char *Opt_ofb = NULL;
const char *Opt_modedb = DEFAULT_MODEDBFILE;
static const char *Opt_xres = NULL;
static const char *Opt_yres = NULL;
static const char *Opt_vxres = NULL;
static const char *Opt_vyres = NULL;
static const char *Opt_depth = NULL;
static const char *Opt_pixclock = NULL;
static const char *Opt_left = NULL;
static const char *Opt_right = NULL;
static const char *Opt_upper = NULL;
static const char *Opt_lower = NULL;
static const char *Opt_hslen = NULL;
static const char *Opt_vslen = NULL;
static const char *Opt_hsync = NULL;
static const char *Opt_vsync = NULL;
static const char *Opt_csync = NULL;
static const char *Opt_extsync = NULL;
static const char *Opt_bcast = NULL;
static const char *Opt_laced = NULL;
static const char *Opt_double = NULL;
static const char *Opt_move = NULL;
static const char *Opt_step = NULL;
static const char *Opt_modename = NULL;

static struct {
    const char *name;
    const char **value;
} Options[] = {
    { "-ifb", &Opt_ifb },
    { "-ofb", &Opt_ofb },
    { "-db", &Opt_modedb },
    { "-xres", &Opt_xres },
    { "-yres", &Opt_yres },
    { "-vxres", &Opt_vxres },
    { "-vyres", &Opt_vyres },
    { "-depth", &Opt_depth },
    { "-pixclock", &Opt_pixclock },
    { "-left", &Opt_left },
    { "-right", &Opt_right },
    { "-upper", &Opt_upper },
    { "-lower", &Opt_lower },
    { "-hslen", &Opt_hslen },
    { "-vslen", &Opt_vslen },
    { "-hsync", &Opt_hsync },
    { "-vsync", &Opt_vsync },
    { "-csync", &Opt_csync },
    { "-extsync", &Opt_extsync },
    { "-bcast", &Opt_bcast },
    { "-laced", &Opt_laced },
    { "-double", &Opt_double },
    { "-move", &Opt_move },
    { "-step", &Opt_step },
    { NULL, NULL }
};


    /*
     *  Video Mode Database
     */

struct VideoMode *VideoModes = NULL;


    /*
     *  Current Video Mode
     */

struct VideoMode Current;


    /*
     *  Function Prototypes
     */

int OpenFrameBuffer(const char *name);
void CloseFrameBuffer(int fh);
void GetVarScreenInfo(int fh, struct fb_var_screeninfo *var);
void SetVarScreenInfo(int fh, struct fb_var_screeninfo *var);
void GetFixScreenInfo(int fh, struct fb_fix_screeninfo *fix);
static void ConvertFromVideoMode(const struct VideoMode *vmode,
				 struct fb_var_screeninfo *var);
static void ConvertToVideoMode(const struct fb_var_screeninfo *var,
			       struct VideoMode *vmode);
static u_long atoboolean(const char *var);
static void ReadModeDB(void);
static struct VideoMode *FindVideoMode(const char *name);
static void ModifyVideoMode(struct VideoMode *vmode);
static void DisplayVModeInfo(struct VideoMode *vmode);
static void DisplayFBInfo(struct fb_fix_screeninfo *fix);
static int FillScanRates(struct VideoMode *vmode);
static void Usage(void) __attribute__ ((noreturn));
int main(int argc, char *argv[]);


    /*
     *  Print an Error Message and Exit
     */

void Die(const char *fmt, ...)
{
    va_list ap;

    fflush(stdout);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(1);
}


    /*
     *  Open the Frame Buffer Device
     */

int OpenFrameBuffer(const char *name)
{
    int fh;

    if (Opt_verbose)
	printf("Opening frame buffer device `%s'\n", name);

    if ((fh = open(name, O_RDONLY)) == -1)
	Die("open %s: %s\n", name, strerror(errno));
    return(fh);
}


    /*
     *  Close the Frame Buffer Device
     */

void CloseFrameBuffer(int fh)
{
    close(fh);
}


    /*
     *  Get the Variable Part of the Screen Info
     */

void GetVarScreenInfo(int fh, struct fb_var_screeninfo *var)
{
    if (ioctl(fh, FBIOGET_VSCREENINFO, var))
	Die("ioctl FBIOGET_VSCREENINFO: %s\n", strerror(errno));
}


    /*
     *  Set (and Get) the Variable Part of the Screen Info
     */

void SetVarScreenInfo(int fh, struct fb_var_screeninfo *var)
{
    if (ioctl(fh, FBIOPUT_VSCREENINFO, var))
	Die("ioctl FBIOPUT_VSCREENINFO: %s\n", strerror(errno));
}


    /*
     *  Get the Fixed Part of the Screen Info
     */

void GetFixScreenInfo(int fh, struct fb_fix_screeninfo *fix)
{
    if (ioctl(fh, FBIOGET_FSCREENINFO, fix))
	Die("ioctl FBIOGET_FSCREENINFO: %s\n", strerror(errno));
}


    /*
     *  Conversion Routines
     */

static void ConvertFromVideoMode(const struct VideoMode *vmode,
				 struct fb_var_screeninfo *var)
{
    memset(var, 0, sizeof(struct fb_var_screeninfo));
    var->xres = vmode->xres;
    var->yres = vmode->yres;
    var->xres_virtual = vmode->vxres;
    var->yres_virtual = vmode->vyres;
    var->bits_per_pixel = vmode->depth;
    if (Opt_now)
	var->activate = FB_ACTIVATE_NOW;
    else
	var->activate = FB_ACTIVATE_TEST;
    var->pixclock = vmode->pixclock;
    var->left_margin = vmode->left;
    var->right_margin = vmode->right;
    var->upper_margin = vmode->upper;
    var->lower_margin = vmode->lower;
    var->hsync_len = vmode->hslen;
    var->vsync_len = vmode->vslen;
    if (vmode->hsync == HIGH)
	var->sync |= FB_SYNC_HOR_HIGH_ACT;
    if (vmode->vsync == HIGH)
	var->sync |= FB_SYNC_VERT_HIGH_ACT;
    if (vmode->csync == HIGH)
	var->sync |= FB_SYNC_COMP_HIGH_ACT;
    if (vmode->extsync == TRUE)
	var->sync |= FB_SYNC_EXT;
    if (vmode->bcast == TRUE)
	var->sync |= FB_SYNC_BROADCAST;
    if (vmode->laced == TRUE)
	var->vmode = FB_VMODE_INTERLACED;
    else if (vmode->dblscan == TRUE)
	var->vmode = FB_VMODE_DOUBLE;
    else
	var->vmode = FB_VMODE_NONINTERLACED;
    var->vmode |= FB_VMODE_CONUPDATE;
}


static void ConvertToVideoMode(const struct fb_var_screeninfo *var,
			       struct VideoMode *vmode)
{
    vmode->name = NULL;
    vmode->xres = var->xres;
    vmode->yres = var->yres;
    vmode->vxres = var->xres_virtual;
    vmode->vyres = var->yres_virtual;
    vmode->depth = var->bits_per_pixel;
    vmode->pixclock = var->pixclock;
    vmode->left = var->left_margin;
    vmode->right = var->right_margin;
    vmode->upper = var->upper_margin;
    vmode->lower = var->lower_margin;
    vmode->hslen = var->hsync_len;
    vmode->vslen = var->vsync_len;
    vmode->hsync = var->sync & FB_SYNC_HOR_HIGH_ACT ? HIGH : LOW;
    vmode->vsync = var->sync & FB_SYNC_VERT_HIGH_ACT ? HIGH : LOW;
    vmode->csync = var->sync & FB_SYNC_COMP_HIGH_ACT ? HIGH : LOW;
    vmode->extsync = var->sync & FB_SYNC_EXT ? TRUE : FALSE;
    vmode->bcast = var->sync & FB_SYNC_BROADCAST ? TRUE : FALSE;
    vmode->laced = FALSE;
    vmode->dblscan = FALSE;
    switch (var->vmode & FB_VMODE_MASK) {
	case FB_VMODE_INTERLACED:
	    vmode->laced = TRUE;
	    break;
	case FB_VMODE_DOUBLE:
	    vmode->dblscan = TRUE;
	    break;
    }
    FillScanRates(vmode);
}


static u_long atoboolean(const char *var)
{
    u_long value = 0;

    if (!strcasecmp(var, "false") || !strcasecmp(var, "low") ||
	!strcmp(var, "0"))
	value = 0;
    else if (!strcasecmp(var, "true") || !strcasecmp(var, "high") ||
	     !strcmp(var, "1"))
	value = 1;
    else
	Die("Invalid value `%s'\n", var);

    return(value);
}


void AddVideoMode(const struct VideoMode *vmode)
{
    struct VideoMode *vmode2;

    if (FindVideoMode(vmode->name))
	Die("%s:%d: Duplicate mode name `%s'\n", Opt_modedb, line,
	    vmode->name);
    vmode2 = malloc(sizeof(struct VideoMode));
    *vmode2 = *vmode;
    if (!FillScanRates(vmode2))
	Die("%s:%d: Bad video mode `%s'\n", Opt_modedb, line, vmode2->name);
    vmode2->next = VideoModes;
    VideoModes = vmode2;
}


    /*
     *  Read the Video Mode Database
     */

static void ReadModeDB(void)
{
    if (Opt_verbose)
	printf("Reading mode database from file `%s'\n", Opt_modedb);

    if (!(yyin = fopen(Opt_modedb, "r")))
	Die("fopen %s: %s\n", Opt_modedb, strerror(errno));
    yyparse();
    fclose(yyin);
}


    /*
     *  Find a Video Mode
     */

static struct VideoMode *FindVideoMode(const char *name)
{
    struct VideoMode *vmode;

    for (vmode = VideoModes; vmode; vmode = vmode->next)
	if (!strcmp(name, vmode->name))
	    break;

    return(vmode);
}


    /*
     *  Modify a Video Mode
     */

static void ModifyVideoMode(struct VideoMode *vmode)
{
    u_long hstep = 8, vstep = 2;

    if (Opt_xres)
	vmode->xres = strtoul(Opt_xres, NULL, 0);
    if (Opt_yres)
	vmode->yres = strtoul(Opt_yres, NULL, 0);
    if (Opt_vxres)
	vmode->vxres = strtoul(Opt_vxres, NULL, 0);
    if (Opt_vyres)
	vmode->vyres = strtoul(Opt_vyres, NULL, 0);
    if (Opt_depth)
	vmode->depth = strtoul(Opt_depth, NULL, 0);
    if (Opt_pixclock)
	vmode->pixclock = strtoul(Opt_pixclock, NULL, 0);
    if (Opt_left)
	vmode->left = strtoul(Opt_left, NULL, 0);
    if (Opt_right)
	vmode->right = strtoul(Opt_right, NULL, 0);
    if (Opt_upper)
	vmode->upper = strtoul(Opt_upper, NULL, 0);
    if (Opt_lower)
	vmode->lower = strtoul(Opt_lower, NULL, 0);
    if (Opt_hslen)
	vmode->hslen = strtoul(Opt_hslen, NULL, 0);
    if (Opt_vslen)
	vmode->vslen = strtoul(Opt_vslen, NULL, 0);
    if (Opt_hsync)
	vmode->hsync = atoboolean(Opt_hsync);
    if (Opt_vsync)
	vmode->vsync = atoboolean(Opt_vsync);
    if (Opt_csync)
	vmode->csync = atoboolean(Opt_csync);
    if (Opt_extsync)
	vmode->extsync = atoboolean(Opt_extsync);
    if (Opt_bcast)
	vmode->bcast = atoboolean(Opt_bcast);
    if (Opt_laced)
	vmode->laced = atoboolean(Opt_laced);
    if (Opt_double)
	vmode->dblscan = atoboolean(Opt_double);
    if (Opt_step)
	hstep = vstep = strtoul(Opt_step, NULL, 0);
    if (Opt_move)
	if (!strcasecmp(Opt_move, "left")) {
	    if (hstep > vmode->left)
		Die("The left margin cannot be negative\n");
	    vmode->left -= hstep;
	    vmode->right += hstep;
	} else if (!strcasecmp(Opt_move, "right")) {
	    if (hstep > vmode->right)
		Die("The right margin cannot be negative\n");
	    vmode->left += hstep;
	    vmode->right -= hstep;
	} else if (!strcasecmp(Opt_move, "up")) {
	    if (vstep > vmode->upper)
		Die("The upper margin cannot be negative\n");
	    vmode->upper -= vstep;
	    vmode->lower += vstep;
	} else if (!strcasecmp(Opt_move, "down")) {
	    if (vstep > vmode->lower)
		Die("The lower margin cannot be negative\n");
	    vmode->upper += vstep;
	    vmode->lower -= vstep;
	} else
	    Die("Invalid direction `%s'\n", Opt_move);
    if (!FillScanRates(vmode))
	Die("Bad video mode\n");
}


    /*
     *  Display the Video Mode Information
     */

static void DisplayVModeInfo(struct VideoMode *vmode)
{
    u_long res, sstart, send, total;

    puts("");
    if (!Opt_xfree86) {
	puts("mode \"name\"");
	printf("    # H: %5.3f kHz, V: %5.3f Hz\n", vmode->hrate/1000,
	       vmode->vrate);
	printf("    geometry %ld %ld %ld %ld %ld\n", vmode->xres, vmode->yres,
	       vmode->vxres, vmode->vyres, vmode->depth);
	printf("    timings %ld %ld %ld %ld %ld %ld %ld\n", vmode->pixclock,
	       vmode->left, vmode->right, vmode->upper, vmode->lower,
	       vmode->hslen, vmode->vslen);
	if (vmode->hsync)
	    puts("    hsync high");
	if (vmode->vsync)
	    puts("    vsync high");
	if (vmode->csync)
	    puts("    csync high");
	if (vmode->extsync)
	    puts("    extsync true");
	if (vmode->bcast)
	    puts("    bcast true");
	if (vmode->laced)
	    puts("    laced true");
	if (vmode->dblscan)
	    puts("    double true");
	puts("endmode\n");
    } else {
	puts("Mode \"name\"");
	printf("    # H: %5.3f kHz, V: %5.3f Hz\n", vmode->hrate/1000,
	       vmode->vrate);
	printf("    DotClock %5.3f\n", 1E6/vmode->pixclock+0.001);
	res = vmode->xres;
	sstart = res+vmode->right;
	send = sstart+vmode->hslen;
	total = send+vmode->left;
	printf("    HTimings %ld %ld %ld %ld\n", res, sstart, send, total);
	res = vmode->yres;
	sstart = res+vmode->lower;
	send = sstart+vmode->vslen;
	total = send+vmode->upper;
	printf("    VTimings %ld %ld %ld %ld\n", res, sstart, send, total);
	printf("    Flags   ");
	if (vmode->laced)
	    printf(" Interlace");
	if (vmode->dblscan)
	    printf(" DoubleScan");
	if (vmode->hsync)
	    printf(" +HSync");
	else
	    printf(" -HSync");
	if (vmode->vsync)
	    printf(" +VSync");
	else
	    printf(" -VSync");
	if (vmode->csync)
	    puts(" +CSync");
	else
	    puts(" -CSync");
	if (vmode->extsync)
	    puts("    # Warning: XFree86 doesn't support extsync\n");
	if (vmode->bcast)
	    puts("    # Warning: XFree86 doesn't support bcast\n");
	puts("EndMode\n");
    }
}


    /*
     *  Display the Frame Buffer Device Information
     */

static void DisplayFBInfo(struct fb_fix_screeninfo *fix)
{
    puts("Frame buffer device information:");
    printf("    Name      : `%s'\n", fix->id);
    printf("    Address   : 0x%08lx\n", fix->smem_start);
    printf("    Size      : %ld\n", fix->smem_len);
    printf("    Type      : ");
    switch (fix->type) {
	case FB_TYPE_PACKED_PIXELS:
	    puts("PACKED PIXELS");
	    break;
	case FB_TYPE_PLANES:
	    puts("PLANES");
	    break;
	case FB_TYPE_INTERLEAVED_PLANES:
	    printf("INTERLEAVED PLANES (%d bytes interleave)\n",
		   fix->type_aux);
	    break;
	default:
	    printf("%d (UNKNOWN)\n", fix->type);
	    printf("    Type_aux  : %d\n", fix->type_aux);
	    break;
    }
    printf("    Visual    : ");
    switch (fix->visual) {
	case FB_VISUAL_MONO01:
	    puts("MONO01");
	    break;
	case FB_VISUAL_MONO10:
	    puts("MONO10");
	    break;
	case FB_VISUAL_TRUECOLOR:
	    puts("TRUECOLOR");
	    break;
	case FB_VISUAL_PSEUDOCOLOR:
	    puts("PSEUDOCOLOR");
	    break;
	case FB_VISUAL_DIRECTCOLOR:
	    puts("DIRECTCOLOR");
	    break;
	case FB_VISUAL_STATIC_PSEUDOCOLOR:
	    puts("STATIC PSEUDOCOLOR");
	    break;
	case FB_VISUAL_STATIC_DIRECTCOLOR:
	    puts("STATIC DIRECTCOLOR");
	    break;
	default:
	    printf("%d (UNKNOWN)\n", fix->visual);
	    break;
    }
    printf("    XPanStep  : %d\n", fix->xpanstep);
    printf("    YPanStep  : %d\n", fix->ypanstep);
    printf("    YWrapStep : %d\n", fix->ywrapstep);
    printf("    LineLength: %ld\n", fix->line_length);
    puts("");
}


    /*
     *  Calculate the Scan Rates for a Video Mode
     */

static int FillScanRates(struct VideoMode *vmode)
{
    u_long htotal = vmode->left+vmode->xres+vmode->right+vmode->hslen;
    u_long vtotal = vmode->upper+vmode->yres+vmode->lower+vmode->vslen;

    if (vmode->dblscan)
	vtotal <<= 2;
    else if (!vmode->laced)
	vtotal <<= 1;

    if (!htotal || !vtotal || !vmode->pixclock)
	return(0);

    vmode->hrate = 1E12/(htotal*vmode->pixclock);
    vmode->vrate = vmode->hrate/vtotal*2;

    return(1);
}


    /*
     *  Print the Usage Template and Exit
     */

static void Usage(void)
{
    puts(VERSION);
    Die("\nUsage: %s [options] [mode]\n\n"
	"Valid options:\n"
	"  General options:\n"
	"    -h, --help         : display this usage information\n"
	"    -n, --now          : change the video mode immediately\n"
	"                         (default if -fb is not used)\n"
	"    -s, --show         : display video mode settings\n"
	"    -i, --info         : display all frame buffer information\n"
	"    -v, --verbose      : verbose mode\n"
	"    -V, --version      : print version information\n"
	"    -x, --xfree86      : XFree86 compatibility mode\n"
	"  Frame buffer special device nodes:\n"
	"    -ofb <device>      : processed frame buffer device\n"
	"                         (default is " DEFAULT_FRAMEBUFFER ")\n"
	"    -ifb <device>      : input frame buffer device\n"
	"                         (default is same as processed device)\n"
	"  Video mode database:\n"
	"    -db <file>         : video mode database file\n"
	"                         (default is " DEFAULT_MODEDBFILE ")\n"
	"  Display geometry:\n"
	"    -xres <value>      : horizontal resolution (in pixels)\n"
	"    -yres <value>      : vertical resolution (in pixels)\n"
	"    -vxres <value>     : virtual horizontal resolution (in pixels)\n"
	"    -vyres <value>     : virtual vertical resolution (in pixels)\n"
	"    -depth <value>     : display depth (in bits per pixel)\n"
	"    -g, --geometry ... : set all geometry parameters at once\n"
	"  Display timings:\n"
	"    -pixclock <value>  : pixel clock (in picoseconds)\n"
	"    -left <value>      : left margin (in pixels)\n"
	"    -right <value>     : right margin (in pixels)\n"
	"    -upper <value>     : upper margin (in pixel lines)\n"
	"    -lower <value>     : lower margin (in pixel lines)\n"
	"    -hslen <value>     : horizontal sync length (in pixels)\n"
	"    -vslen <value>     : vertical sync length (in pixel lines)\n"
	"    -t, --timings ...  : set all timing parameters at once\n"
	"  Display flags:\n"
	"    -hsync <value>     : horizontal sync polarity (low or high)\n"
	"    -vsync <value>     : vertical sync polarity (low or high)\n"
	"    -csync <value>     : composite sync polarity (low or high)\n"
	"    -extsync <value>   : external sync enable (false or true)\n"
	"    -bcast <value>     : broadcast enable (false or true)\n"
	"    -laced <value>     : interlace enable (false or true)\n"
	"    -double <value>    : doublescan enable (false or true)\n"
	"  Display positioning:\n"
	"    -move <direction>  : move the visible part (left, right, up or down)\n"
	"    -step <value>      : step increment (in pixels or pixel lines)\n"
	"                         (default is 8 horizontal, 2 vertical)\n",
	ProgramName);
}


    /*
     *  Main Routine
     */

int main(int argc, char *argv[])
{
    struct VideoMode *vmode;
    struct fb_var_screeninfo var;
    struct fb_fix_screeninfo fix;
    int fh = -1, i;

    ProgramName = argv[0];

    /*
     *  Parse the Options
     */

    while (--argc > 0) {
	argv++;
	if (!strcmp(argv[0], "-h") || !strcmp(argv[0], "--help"))
	    Usage();
	else if (!strcmp(argv[0], "-n") || !strcmp(argv[0], "--now")) {
	    Opt_now = 1;
	    Opt_showonly = 0;
	} else if (!strcmp(argv[0], "-s") || !strcmp(argv[0], "--show"))
	    Opt_show = 1;
	else if (!strcmp(argv[0], "-i") || !strcmp(argv[0], "--info")) {
	    Opt_show = 1;
	    Opt_info = 1;
	} else if (!strcmp(argv[0], "-v") || !strcmp(argv[0], "--verbose"))
	    Opt_verbose = 1;
	else if (!strcmp(argv[0], "-V") || !strcmp(argv[0], "--version")) {
	    Opt_version = 1;
	    Opt_showonly = 0;
	} else if (!strcmp(argv[0], "-x") || !strcmp(argv[0], "--xfree86"))
	    Opt_xfree86 = 1;
	else if (!strcmp(argv[0], "-g") || !strcmp(argv[0], "--geometry"))
	    if (argc > 5) {
		Opt_xres = argv[1];
		Opt_yres = argv[2];
		Opt_vxres = argv[3];
		Opt_vyres = argv[4];
		Opt_depth = argv[5];
		argc -= 5;
		argv += 5;
		Opt_showonly = 0;
	    } else
		Usage();
	else if (!strcmp(argv[0], "-t") || !strcmp(argv[0], "--timings"))
	    if (argc > 7) {
		Opt_pixclock = argv[1];
		Opt_left = argv[2];
		Opt_right = argv[3];
		Opt_upper = argv[4];
		Opt_lower = argv[5];
		Opt_hslen = argv[6];
		Opt_vslen = argv[7];
		argc -= 7;
		argv += 7;
		Opt_showonly = 0;
	    } else
		Usage();
	else {
	    if (strcmp(&argv[0][2], "fb"))
		Opt_showonly = 0;
	    for (i = 0; Options[i].name; i++)
		if (!strcmp(argv[0], Options[i].name))
		    break;
	    if (Options[i].name)
		if (argc-- > 1) {
		    *Options[i].value = argv[1];
		    argv++;
		} else
		    Usage();
	    else if (!Opt_modename)
		Opt_modename = argv[0];
	    else
		Usage();
	}
    }

    if (Opt_version || Opt_verbose)
	puts(VERSION);

    if (!Opt_ofb) {
	Opt_ofb = DEFAULT_FRAMEBUFFER;
	Opt_now = 1;
    }

    if (!Opt_ifb)
	Opt_ifb = Opt_ofb;

    /*
     *  Get the Video Mode
     */

    if (Opt_modename) {
	/*
	 *  Read the Video Mode Database
	 */

	ReadModeDB();


	if (!(vmode = FindVideoMode(Opt_modename)))
	    Die("Unknown video mode `%s'\n", Opt_modename);      

	Current = *vmode;
	if (Opt_verbose)
	    printf("Using video mode `%s'\n", Opt_modename);
    } else {
	fh = OpenFrameBuffer(Opt_ifb);
	GetVarScreenInfo(fh, &var);
	ConvertToVideoMode(&var, &Current);
	if (Opt_verbose)
	    printf("Using current video mode from `%s'\n", Opt_ifb);
    }

    if (!Opt_showonly) {

	/*
	 *  Optionally Modify the Video Mode
	 */

	ModifyVideoMode(&Current);

	/*
	 *  Set the Video Mode
	 */

	CloseFrameBuffer(fh);
	fh = OpenFrameBuffer(Opt_ofb);
	ConvertFromVideoMode(&Current, &var);
	if (Opt_verbose)
	    printf("Setting video mode to `%s'\n", Opt_ofb);
	SetVarScreenInfo(fh, &var);
	ConvertToVideoMode(&var, &Current);

    }

    /*
     *  Display some Video Mode Information
     */

    if (Opt_show || Opt_showonly)
	DisplayVModeInfo(&Current);

    if (Opt_info) {
	if (Opt_verbose)
	    puts("Getting further frame buffer information");
	GetFixScreenInfo(fh, &fix);
	DisplayFBInfo(&fix);
    }

    /*
     *  Close the Frame Buffer Device
     */

    CloseFrameBuffer(fh);

    exit(0);
}

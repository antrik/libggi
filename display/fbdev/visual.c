/* $Id: visual.c,v 1.7 2001/08/24 01:39:40 skids Exp $
******************************************************************************

   Display-FBDEV: visual handling

   Copyright (C) 1998 Andrew Apted		[andrew@ggi-project.org]
   Copyright (C) 1999-2000 Marcus Sundberg	[marcus@ggi-project.org]

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************************
*/

#include <ggi/internal/ggi-dl.h>
#include <ggi/display/fbdev.h>
#include <ggi/display/linvtsw.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef HAVE_SYS_VT_H
#include <sys/vt.h>
#else
#include <linux/vt.h>
#endif
#ifdef HAVE_LINUX_KDEV_T_H
#include <linux/kdev_t.h>
#endif
#ifdef HAVE_LINUX_MAJOR_H
#include <linux/major.h>
#endif
#include <linux/tty.h>


#define DEFAULT_MODEDB	"/etc/fb.modes"

/* Linux 2.0 compability */
#ifndef FB_MAJOR
# ifdef GRAPHDEV_MAJOR
#  define FB_MAJOR	GRAPHDEV_MAJOR
# else
#  warning FB_MAJOR or GRAPHDEV_MAJOR not defined - defining to 29
#  define FB_MAJOR	29
# endif
#endif

/* KGIcon compability */
#ifndef KGICON_PROBE
# define KGICON_PROBE		_IOR('F', 0xe8, int)
#endif
#ifndef KGICON_PROBE_MAGIC
# define KGICON_PROBE_MAGIC	0x0049474b
#endif
#ifndef DRIVER_GETINFO
struct kgi_driver {
	char manufact[64], model[64];
	int version;
};
# define DRIVER_GETINFO		_IOR(0x00, 0x00, struct kgi_driver)
#endif


static int refcount = 0;
static int vtnum;
static void *_ggi_fbdev_lock = NULL;
#ifdef FBIOGET_CON2FBMAP
static struct fb_con2fbmap origconmap;
#endif

static const gg_option optlist[] =
{
	{ "nokbd",   "no" },
	{ "nomouse", "no" },
	{ "noinput", "no" },  /* shorthand for nokbd + nomouse */
	{ "novt",    "no" },
	{ "physz",   "0,0" },
	{ ":dev",    ""}
};

#define OPT_NOKBD	0
#define OPT_NOMOUSE	1
#define OPT_NOINPUT	2
#define OPT_NOVT	3
#define OPT_PHYSZ	4
#define OPT_DEV		5

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))

#define MAX_DEV_LEN	63
#define DEFAULT_FBNUM	0

#ifdef HAVE_NEW_FBDEV
static char accel_prefix[] = "tgt-fbdev-";
#define PREFIX_LEN	(sizeof(accel_prefix))

typedef struct {
	char *str;
	int   async;
	int   flags;
} accel_info;


static accel_info accel_strings[] = {
/* 0  */{ "kgicon-generic", 0, 0 },	/* no accel - check for KGIcon	*/
	{ NULL, 0, 0 },			/* Atari Blitter		*/
	{ NULL, 0, 0 },			/* Amiga Blitter                */
	{ NULL, 0, 0 },			/* Cybervision64 (S3 Trio64)    */
	{ NULL, 0, 0 },			/* RetinaZ3 (NCR 77C32BLT)      */
	{ NULL, 0, 0 },			/* Cybervision64/3D (S3 ViRGE)	*/
	{ NULL, 0, 0 },			/* ATI Mach 64GX family		*/
	{ NULL, 0, 0 },			/* DEC 21030 TGA		*/
	{ NULL, 0, 0 },			/* ATI Mach 64CT family		*/
	{ NULL, 0, 0 },			/* ATI Mach 64CT family VT class */
/* 10 */{ NULL, 0, 0 },			/* ATI Mach 64CT family GT class */
	{ NULL, 0, 0 },			/* Sun Creator/Creator3D	*/
	{ NULL, 0, 0 },			/* Sun cg6			*/
	{ NULL, 0, 0 },			/* Sun leo/zx			*/
	{ NULL, 0, 0 },			/* IMS Twin Turbo		*/
	{ NULL, 0, 0 },			/* 3Dlabs Permedia 2		*/
	{ "mga-2164w", 1, GGI_FBDEV_4BPP_REV 
	},				/* Matrox MGA2064W (Millenium)	*/
	{ "directfb", 1, 0
	},				/* Matrox MGA1064SG (Mystique)	*/
	{ "mga-2164w", 1, GGI_FBDEV_4BPP_REV 
	},	   			/* Matrox MGA2164W (Millenium II) */
	{ "mga-2164w", 1, GGI_FBDEV_4BPP_REV 
	},	   			/* Matrox MGA2164W (Millenium II AGP)*/
/* 20 */ 
	{ "mga-2164w", 1, GGI_FBDEV_4BPP_REV 
	},	        		/* Matrox G100 (Productiva G100) */
	{ "mga-2164w", 1, GGI_FBDEV_4BPP_REV 
	},	        		/* Matrox G200 (Myst, Mill, ...) */
	{ NULL, 0, 0 },			/* Sun cgfourteen		*/
	{ NULL, 0, 0 },			/* Sun bwtwo			*/
	{ NULL, 0, 0 },			/* Sun cgthree			*/
	{ NULL, 0, 0 },			/* Sun tcx			*/
	{ "mga-g400", 1, GGI_FBDEV_4BPP_REV
	},				/* Matrox G400			*/
	{ "directfb", 1, 0 },		/* nVidia RIVA 128              */
	{ "directfb", 1, 0 },		/* nVidia RIVA TNT              */
	{ "directfb", 1, 0 },		/* nVidia RIVA TNT2             */
/* 30 */{ NULL, 0, 0 },			/* C&T 6555x			*/
	{ "directfb", 1, 0 },		/* 3Dfx Banshee			*/
	{ "directfb", 1, 0 }		/* ATI Rage128 family		*/
};

#define NUM_ACCELS	(sizeof(accel_strings)/sizeof(accel_info))
#endif /* HAVE_NEW_FBDEV */


extern int GGI_fbdev_resetmode(ggi_visual *vis);

static void
switchreq(void *arg)
{
	ggi_visual *vis = arg;
	ggi_fbdev_priv *priv = LIBGGI_PRIVATE(vis);
	gii_event ev;
	ggi_cmddata_switchrequest *data;

	GGIDPRINT_MISC("display-fbdev: switchreq(%p) called\n", vis);

	_giiEventBlank(&ev, sizeof(gii_cmd_event));

	data = (void *)ev.cmd.data;

	ev.size   = sizeof(gii_cmd_event);
	ev.cmd.type = evCommand;
	ev.cmd.code = GGICMD_REQUEST_SWITCH;
	data->request = GGI_REQSW_UNMAP;

	_giiSafeAdd(vis->input, &ev);

	priv->switchpending = 1;
}


static void
switching(void *arg)
{
	ggi_visual *vis = arg;
	ggi_fbdev_priv *priv = LIBGGI_PRIVATE(vis);

	GGIDPRINT_MISC("display-fbdev: switching(%p) called\n", vis);

	priv->ismapped = 0;
	priv->switchpending = 0;
#if 0
#ifdef FBIOGET_CON2FBMAP
	if (refcount > 1) {
		fbdev_doioctl(vis, FBIOPUT_CON2FBMAP, &origconmap);
	}
#endif
#endif
}


static void
do_setpalette(ggi_visual *vis)
{
	ggi_graphtype gt = LIBGGI_MODE(vis)->graphtype;
	ggi_fbdev_priv *priv = LIBGGI_PRIVATE(vis);
	ggi_color *colormap;
	struct fb_cmap cmap;
	int len, i;

	if (GT_SCHEME(gt) != GT_PALETTE) return;

	len = 1 << GT_DEPTH(gt);
	cmap.start  = 0;
	cmap.len    = len;
	cmap.red    = priv->reds;
	cmap.green  = priv->greens;
	cmap.blue   = priv->blues;
	cmap.transp = NULL;
	colormap = vis->palette;

	for (i = 0; i < len; i++) {
		priv->reds[i]   = colormap[i].r;
		priv->greens[i] = colormap[i].g;
		priv->blues[i]  = colormap[i].b;
	}

	if (fbdev_doioctl(vis, FBIOPUTCMAP, &cmap) < 0) {
		GGIDPRINT_COLOR("display-fbdev: PUTCMAP failed.");
	} else {
		GGIDPRINT_COLOR("display-fbdev: restored palette for %p\n",
				vis);
	}
}


static void
switchback(void *arg)
{
	ggi_visual *vis = arg;
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);
	gii_event ev;

	GGIDPRINT_MISC("display-fbdev: switched_back(%p) called\n", vis);

	_giiEventBlank(&ev, sizeof(gii_expose_event));

	ev.any.size   = sizeof(gii_expose_event);
	ev.any.type   = evExpose;

	ev.expose.x = ev.expose.y = 0;
	ev.expose.w = LIBGGI_VIRTX(vis);
	ev.expose.h = LIBGGI_VIRTY(vis);

	_giiSafeAdd(vis->input, &ev);
	GGIDPRINT_MISC("fbdev: EXPOSE sent.\n");

#if 0
	if (refcount > 1) {
		fbdev_doioctl(vis, FBIOPUT_VSCREENINFO, &priv->var);
	}
#endif
	do_setpalette(vis);
	if (priv->fix.xpanstep != 0 || priv->fix.ypanstep != 0) {
		fbdev_doioctl(vis, FBIOPAN_DISPLAY, &priv->var);
	}
	priv->ismapped = 1;
}


static int 
GGI_fbdev_sendevent(ggi_visual *vis, gii_event *ev)
{
	ggi_fbdev_priv *priv = LIBGGI_PRIVATE(vis);

	GGIDPRINT_MISC("GGI_fbdev_sendevent() called\n");

	if (ev->any.type != evCommand) {
		return GGI_EEVUNKNOWN;
	}
	switch (ev->cmd.code) {
	case GGICMD_ACKNOWLEDGE_SWITCH:
		GGIDPRINT_MISC("display-fbdev: switch acknowledge\n");
		if (priv->switchpending) {
			priv->doswitch(vis);
			return 0;
		} else {
			/* No switch pending */
			return GGI_EEVNOTARGET;
		}
		break;
	case GGICMD_NOHALT_ON_UNMAP:
		GGIDPRINT_MISC("display-fbdev: nohalt on\n");
		priv->dohalt = 0;
		priv->autoswitch = 0;
		break;
	case GGICMD_HALT_ON_UNMAP:
		GGIDPRINT_MISC("display-fbdev: halt on\n");
		priv->dohalt = 1;
		priv->autoswitch = 1;
		if (priv->switchpending) {
			/* Do switch and halt */
			priv->doswitch(vis);
			pause();
		}
		break;
	}
	
	return GGI_EEVUNKNOWN;
}


static int
GGI_fbdev_flush(ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);
	
	if (priv->flush) return priv->flush(vis, x, y, w, h, tryflag);

	return 0;
}

static int
GGI_fbdev_idleaccel(ggi_visual *vis)
{
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);

	GGIDPRINT_DRAW("GGI_fbdev_idleaccel(%p) called \n", vis);
	
	if (priv->idleaccel) return priv->idleaccel(vis);

	return 0;
}


static void 
save_palette(ggi_visual *vis)
{
	ggi_fbdev_priv *priv = LIBGGI_PRIVATE(vis);
	struct fb_cmap cmap;
	int len;

	if ((priv->orig_var.bits_per_pixel < 1) || 
	    (priv->orig_var.bits_per_pixel > 8)) {
		priv->orig_reds = NULL;
		return;
	}
	len = 1 << priv->orig_var.bits_per_pixel;
	
	priv->orig_reds = malloc(sizeof(uint16)*len*3);
	if (priv->orig_reds == NULL) {
		return;
	}

	cmap.start = 0;
	cmap.len   = len;
	cmap.red   = priv->orig_reds;
	cmap.green = priv->orig_greens = priv->orig_reds + len;
	cmap.blue  = priv->orig_blues  = priv->orig_greens + len;
	cmap.transp = NULL;

	if (ioctl(LIBGGI_FD(vis), FBIOGETCMAP, &cmap) < 0) {
		GGIDPRINT_COLOR("display-fbdev: GETCMAP failed.\n");
	} else {
		GGIDPRINT_COLOR("display-fbdev: Saved palette (len=%d).\n",
				cmap.len);
	}
}


static void 
restore_palette(ggi_visual *vis)
{
	ggi_fbdev_priv *priv = LIBGGI_PRIVATE(vis);
	struct fb_cmap cmap;

	if (priv->orig_reds == NULL) return;
	
	cmap.start  = 0;
	cmap.len    = 1 << priv->orig_var.bits_per_pixel;
	cmap.red    = priv->orig_reds;
	cmap.green  = priv->orig_greens;
	cmap.blue   = priv->orig_blues;
	cmap.transp = NULL;

	if (ioctl(LIBGGI_FD(vis), FBIOPUTCMAP, &cmap) < 0) {
		GGIDPRINT_COLOR("display-fbdev: PUTCMAP failed.\n");
	} else {
		GGIDPRINT_COLOR("display-fbdev: Restored palette (len=%d).\n", 
				cmap.len);
	}

	free(priv->orig_reds);
	priv->orig_reds = NULL;
}


static int do_cleanup(ggi_visual *vis)
{
	ggi_fbdev_priv *priv = LIBGGI_PRIVATE(vis);
	ggi_fbdev_timing *curtim;

	/* We may be called more than once due to the LibGG cleanup stuff */
	if (priv == NULL) return 0;

	GGIDPRINT("display-fbdev: GGIdlcleanup start.\n");

	if (LIBGGI_FD(vis) >= 0) {
#if 0
#ifdef FBIOGET_CON2FBMAP
		ioctl(LIBGGI_FD(vis), FBIOPUT_CON2FBMAP, &origconmap);
#endif
#endif
		GGI_fbdev_resetmode(vis);
		restore_palette(vis);
		close(LIBGGI_FD(vis));
	}

	if (vis->input != NULL) {
		giiClose(vis->input);
		vis->input = NULL;
	}

	if (priv->normalgc) {
		free(priv->normalgc);
	}
	if (priv->accel) {
		free(priv->accel);
	}
	curtim = priv->timings;
	while (curtim) {
		ggi_fbdev_timing *prevtim = curtim;

		curtim = curtim->next;
		free(prevtim);
	}
	free(priv);
	LIBGGI_PRIVATE(vis) = NULL;

	ggUnregisterCleanup((ggcleanup_func *)do_cleanup, vis);

	ggLock(_ggi_global_lock);
	refcount--;
	if (refcount == 0) {
		ggLockDestroy(_ggi_fbdev_lock);
		_ggi_fbdev_lock = NULL;
	}
	ggUnlock(_ggi_global_lock);

	GGIDPRINT("display-fbdev: GGIdlcleanup done.\n");

	return 0;
}


#ifdef FBIOGET_CON2FBMAP
static int
get_fbdev(void)
{
	struct vt_stat qry_stat;  
	struct fb_con2fbmap map;
	char devname[MAX_DEV_LEN+1];
	int fb, fd;

	/* determine VT number */
	fd = open("/dev/tty", O_RDONLY);

	if (fd < 0) {
		perror("display-fbdev: failed to open tty");
		return DEFAULT_FBNUM;
	}

	if (ioctl(fd, VT_GETSTATE, &qry_stat) == 0) {
		map.console = qry_stat.v_active;
		GGIDPRINT_MISC("display-fbdev: Using VT %d.\n", map.console);
	} else {
		perror("display-fbdev: ioctl(VT_GETSTATE) failed");
		close(fd);
		return DEFAULT_FBNUM;
	}
	close(fd);

	/* Find a framebuffer to open */
	for (fb=0; fb < 32; fb++) {
		sprintf(devname, "/dev/fb%d", fb);
		fd = open(devname, O_RDONLY);
		if (fd >= 0) break;
		sprintf(devname, "/dev/fb/%d", fb);
		fd = open(devname, O_RDONLY);
		if (fd >= 0) break;
	}

	if (fb >= 32) {
		/* Not found */
		GGIDPRINT_MISC("display-fbdev: Could not find a framebuffer "
			       "device with read permission.\n");
		return DEFAULT_FBNUM;
	}

	if (ioctl(fd, FBIOGET_CON2FBMAP, &map) != 0) {
		perror("display-fbdev: ioctl(FBIOGET_CON2FBMAP) failed");
		close(fd);
		return DEFAULT_FBNUM;
	}

	close(fd);

	GGIDPRINT_MISC("display-fbdev: Determined VT %d is on FB %d\n",
		       map.console, map.framebuffer);

	return map.framebuffer;
}
#else
#define get_fbdev() DEFAULT_FBNUM
#endif


#define SKIPWHITE(ptr) while(*(ptr)==' '||*(ptr)=='\t'||*(ptr)=='\n'){(ptr)++;}

static int
get_timings(ggi_fbdev_priv *priv, const char *name)
{
	FILE *infile;
	char buffer[1024],*bufp;
	ggi_fbdev_timing timing, *curtim;

	/* First add the current mode */
	priv->timings = malloc(sizeof(*(priv->timings)));
	if (!priv->timings) return GGI_ENOMEM;

	curtim = priv->timings;

	curtim->xres = priv->orig_var.xres;
	curtim->yres = priv->orig_var.yres;
	curtim->xres_virtual = priv->orig_var.xres_virtual;
	curtim->yres_virtual = priv->orig_var.yres_virtual;
	curtim->bits_per_pixel = priv->orig_var.bits_per_pixel;
	curtim->pixclock = priv->orig_var.pixclock;
	curtim->left_margin = priv->orig_var.left_margin;
	curtim->right_margin = priv->orig_var.right_margin;
	curtim->upper_margin = priv->orig_var.upper_margin;
	curtim->lower_margin = priv->orig_var.lower_margin;
	curtim->hsync_len = priv->orig_var.hsync_len;
	curtim->vsync_len = priv->orig_var.vsync_len;
	curtim->sync = priv->orig_var.sync;
	curtim->vmode = priv->orig_var.vmode;
	curtim->next = NULL;

	/* Open the fb.modes file 
	 */
	infile = fopen(name, "r");
	if (infile == NULL) {
		GGIDPRINT_MODE("display-fbdev: Unable to open: %s\n", name);
		return GGI_ENOFILE;
	}

	/* Scan for all modes.
	 */
	GGIDPRINT_MODE("display-fbdev: Parsing modedb: %s\n", name);
	while (!feof(infile)) {
		/* EOF ? */
		if (NULL == fgets(buffer, sizeof(buffer), infile)) break;

		/* Kill comments */
		bufp = strchr(buffer, '#');
		if (bufp) *bufp = '\0';

		/* Skip whitespace */
		bufp = buffer;
		SKIPWHITE(bufp);

		/* Parse keywords
		 */
		if (0 == strncmp(bufp, "mode", 4)) {
			memset(&timing, 0, sizeof(timing));
			GGIDPRINT_MODE("display-fbdev:   begin: %s",
				       bufp+4);
			continue;
		} else if (0 == strncmp(bufp, "geometry", 8)) {
			sscanf(bufp+8, " %d %d %d %d %d",
			       &timing.xres, &timing.yres,
			       &timing.xres_virtual, &timing.yres_virtual,
			       &timing.bits_per_pixel);
			continue;
		} else if (0 == strncmp(bufp, "timings", 7)) {
			sscanf(bufp+7, " %d %d %d %d %d %d %d",
			       &timing.pixclock,
			       &timing.left_margin, &timing.right_margin,
			       &timing.upper_margin, &timing.lower_margin,
			       &timing.hsync_len, &timing.vsync_len);
			continue;
		} else if (0 == strncmp(bufp, "hsync", 5)) {
			bufp += 5;
			SKIPWHITE(bufp);
			if (0 == strncmp(bufp, "high", 4)) {
				timing.sync |= FB_SYNC_HOR_HIGH_ACT;
			}
			continue;
		} else if (0 == strncmp(bufp, "vsync", 5)) {
			bufp += 5;
			SKIPWHITE(bufp);
			if (0 == strncmp(bufp, "high", 4)) {
				timing.sync |= FB_SYNC_VERT_HIGH_ACT;
			}
			continue;
		} else if (0 == strncmp(bufp, "csync", 5)) {
			bufp += 5;
			SKIPWHITE(bufp);
			if (0 == strncmp(bufp, "high", 4)) {
				timing.sync |= FB_SYNC_COMP_HIGH_ACT;
			}
			continue;
		} else if (0 == strncmp(bufp, "gsync", 5)) {
			bufp += 5;
			SKIPWHITE(bufp);
			if (0 == strncmp(bufp, "high", 4)) {
				timing.sync |= FB_SYNC_ON_GREEN;
			}
			continue;
		} else if (0 == strncmp(bufp, "extsync", 7)) {
			bufp += 7;
			SKIPWHITE(bufp);
			if (0 == strncmp(bufp, "true", 4)) {
				timing.sync |= FB_SYNC_EXT;
			}
			continue;
		} else if (0 == strncmp(bufp, "bcast", 5)) {
			bufp += 5;
			SKIPWHITE(bufp);
			if (0 == strncmp(bufp, "true", 4)) {
				timing.sync |= FB_SYNC_BROADCAST;
			}
			continue;
		} else if (0 == strncmp(bufp, "laced", 5)) {
			bufp += 5;
			SKIPWHITE(bufp);
			if (0 == strncmp(bufp, "true", 4)) {
				timing.vmode |= FB_VMODE_INTERLACED;
			}
			continue;
		} else if (0 == strncmp(bufp, "double", 6)) {
			bufp += 6;
			SKIPWHITE(bufp);
			if (0 == strncmp(bufp, "true", 4)) {
				timing.vmode |= FB_VMODE_DOUBLE;
			}
			continue;
		} else if (0 == strncmp(bufp, "endmode", 7) &&
			   timing.pixclock /* Valid timing entry ? */ &&
			   timing.xres && timing.yres) {
			GGIDPRINT_MODE("display-fbdev:   got %dx%d mode\n",
				       timing.xres, timing.yres);
			curtim->next = malloc(sizeof(timing));
			if (!curtim->next) {
				fclose(infile);
				return GGI_ENOMEM;
			}
			memcpy(curtim->next, &timing, sizeof(timing));
			curtim = curtim->next;
			curtim->next = NULL;
		}
	}

	fclose(infile);
	return 0;
}


#define MAX_DEV_LEN	63

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
	ggi_linvtsw_arg vtswarg;
	const char *devfile = NULL;
	char  devicename[MAX_DEV_LEN+1];
	int   fbnum = -1;
	int   novt = 0;
	int   classicname_failed = 0;
	int   classicname_errno = 0;	/* Keep compiler happy. */
	char *modedb = DEFAULT_MODEDB;
	gg_option options[NUM_OPTS];
	ggi_fbdev_priv *priv;

	GGIDPRINT("display-fbdev: GGIdlinit start.\n");

	memcpy(options, optlist, sizeof(options));

	if (getenv("GGI_FBDEV_OPTIONS") != 0) {
		if (ggParseOptions(getenv("GGI_FBDEV_OPTIONS"),
				   options, NUM_OPTS) == NULL) {
			fprintf(stderr, "display-fbdev: error in "
				"$GGI_FBDEV_OPTIONS\n");
			return GGI_EARGINVAL;
		}
	}
	
	if (args) {
		args = ggParseOptions(args, options, NUM_OPTS);
		if (args == NULL) {
			fprintf(stderr, "display-fbdev: error in "
				"arguments.\n");
			return GGI_EARGINVAL;
		}
	}

	LIBGGI_PRIVATE(vis) = priv = malloc(sizeof(ggi_fbdev_priv));
	if (priv == NULL) {
		return GGI_ENOMEM;
	}
	GGIDPRINT("display-fbdev: Got private mem.\n");
	
	priv->fb_ptr = NULL;
	priv->need_timings = 1;
	priv->flags = 0;

	priv->orig_reds = NULL;

	priv->dohalt = 1;
	priv->autoswitch = 1;
	priv->switchpending = 0;
	priv->ismapped = 1;
	priv->doswitch = NULL;

	priv->iskgi = 0;
	priv->accel = NULL;
	priv->have_accel = 0;
	priv->accelpriv = NULL;
	priv->mmioaddr = NULL;
	priv->flush = NULL;
	priv->idleaccel = NULL;

	if (strlen(options[OPT_DEV].result)) {
		devfile = options[OPT_DEV].result;
	} else if (getenv("FRAMEBUFFER") != NULL) {
		strncpy(devicename, getenv("FRAMEBUFFER"), MAX_DEV_LEN);
		/* Make sure string is terminated */
		devicename[MAX_DEV_LEN] = '\0';
		devfile = devicename;
	} else {
		fbnum = get_fbdev();
	}

	GGIDPRINT("display-fbdev: Parsing input options.\n");
	
	priv->inputs = FBDEV_INP_KBD | FBDEV_INP_MOUSE;

	if (toupper(options[OPT_NOKBD].result[0]) != 'N') {
		priv->inputs &= ~FBDEV_INP_KBD;
	}
	if (toupper(options[OPT_NOMOUSE].result[0]) != 'N') {
		priv->inputs &= ~FBDEV_INP_MOUSE;
	}
	if (toupper(options[OPT_NOINPUT].result[0]) != 'N') {
		priv->inputs &= ~(FBDEV_INP_KBD | FBDEV_INP_MOUSE);
	}
	if (toupper(options[OPT_NOVT].result[0]) != 'N') {
		priv->inputs = 0;
		novt = 1;
	}

	GGIDPRINT("display-fbdev: Parsing physz options.\n");
	do {
		int err;
		err = _ggi_parse_physz(options[OPT_PHYSZ].result, 
				       &(priv->physzflags), &(priv->physz)); 
		if (err != GGI_OK) {
			do_cleanup(vis);
			return err;
		}
	} while (0);

	GGIDPRINT("display-fbdev: Setting up locks.\n");
	ggLock(_ggi_global_lock);
	if (refcount == 0) {
		_ggi_fbdev_lock = ggLockCreate();
		if (_ggi_fbdev_lock == NULL) {
			ggUnlock(_ggi_global_lock);
			free(priv);
			return GGI_ENOMEM;
		}
	}
	priv->lock = _ggi_fbdev_lock;
	priv->refcount = &refcount;
	refcount++;
	ggUnlock(_ggi_global_lock);

	do {
		vtswarg.switchreq = switchreq;
		vtswarg.switching = switching;
		vtswarg.switchback = switchback;
		vtswarg.funcarg = vis;

		vtswarg.dohalt     = &priv->dohalt;
		vtswarg.autoswitch = &priv->autoswitch;
		vtswarg.onconsole = 1;
		if (getenv("GGI_NEWVT")) {
			vtswarg.forcenew = 1;
		} else {
			vtswarg.forcenew = 0;
		}
		vtswarg.novt = novt;

		if (_ggiAddDL(vis, "helper-linux-vtswitch", NULL, &vtswarg, 0)
		    == 0) {
			vtnum = vtswarg.vtnum;
			priv->doswitch = vtswarg.doswitch;
		} else {
			vtnum = -1;
			priv->doswitch = NULL;
		}
	} while (0);

	if (vtswarg.refcount > 1) {
		/* No inputs unless we're first */
		priv->inputs = 0;
	}

	/* Open keyboard and mouse input */
	if (priv->inputs & FBDEV_INP_KBD) {
		char strbuf[64];
		char *inputstr = "input-linux-kbd";

		if (vtnum != -1) {
			sprintf(strbuf, "linux-kbd:/dev/tty%d", vtnum);
			inputstr = strbuf;
		}

		vis->input = giiOpen(inputstr, NULL);
		if (vis->input == NULL) {
			if (vtnum != -1) {
				sprintf(strbuf, "linux-kbd:/dev/vc/%d", vtnum);
				vis->input = giiOpen(inputstr, NULL);
			}
			if (vis->input == NULL) {
				fprintf(stderr,
"display-fbdev: Unable to open linux-kbd, trying stdin input.\n");
				/* We're on the Linux console so we want
				   ansikey. */
				vis->input = giiOpen("stdin:ansikey", NULL);
				if (vis->input == NULL) {
					fprintf(stderr,
"display-fbdev: Unable to open stdin input, try running with '-nokbd'.\n");
					do_cleanup(vis);
					return GGI_ENODEVICE;
				}
			}
		}
	}
	if (priv->inputs & FBDEV_INP_MOUSE) {
		gii_input *inp;
		if ((inp = giiOpen("linux-mouse:auto", &args, NULL)) != NULL) {
			vis->input = giiJoinInputs(vis->input, inp);
		}
	}

	/* Now open the framebuffer device */
	if (devfile) {
		LIBGGI_FD(vis) = open(devfile, O_RDWR);
#ifndef HAVE_LINUX_KDEV_T_H
		/* Extract device number */
		do {
			char *strptr = devicename;
			char *chkptr;

			while (!isdigit(*strptr)) {
				if (*strptr == '\0') break;
				strptr++;
			}
			fbnum = strtol(strptr, &chkptr, 10);
			if (chkptr == strptr) fbnum = -1;
		} while (0);
#endif
	} else {
		devfile = devicename;
		sprintf(devicename, "/dev/fb%d", fbnum);
		LIBGGI_FD(vis) = open(devicename, O_RDWR);
		if (LIBGGI_FD(vis) < 0) {
			classicname_failed=1;
			classicname_errno=errno;
			sprintf(devicename, "/dev/fb/%d", fbnum);
			LIBGGI_FD(vis) = open(devicename, O_RDWR);
		}
	}

	if (LIBGGI_FD(vis) < 0) {
		fprintf(stderr, "display-fbdev: Couldn't open "
			"framebuffer device %s: %s\n", devfile,
			strerror(errno));
		if (classicname_failed) 
			fprintf(stderr, "display-fbdev: Couldn't open "
				"framebuffer device /dev/fb%d: %s\n", fbnum,
				strerror(classicname_errno));
		do_cleanup(vis);
		return GGI_ENODEVICE;
	}
#ifdef HAVE_LINUX_KDEV_T_H
	if (fbnum < 0) {
		struct stat statbuf;

		if (fstat(LIBGGI_FD(vis), &statbuf) == 0 &&
		    S_ISCHR(statbuf.st_mode) &&
		    MAJOR(statbuf.st_rdev) == FB_MAJOR) {
			fbnum = MINOR(statbuf.st_rdev) / 32;
		}
	}
#endif
	priv->fbnum = fbnum;
	priv->vtnum = vtnum;

#ifdef FBIOGET_CON2FBMAP
	if (refcount == 1 && !novt) {
		origconmap.console = priv->vtnum;
		ioctl(LIBGGI_FD(vis), FBIOGET_CON2FBMAP, &origconmap);
	}
#endif

	{
		int rc, iskgi = 0;

		rc = ioctl(LIBGGI_FD(vis), KGICON_PROBE, &iskgi);
		if (rc == 0 && iskgi== KGICON_PROBE_MAGIC) {
			struct kgi_driver info;

			GGIDPRINT_MISC("display-fbdev: Detected KGI driver.\n");
			priv->iskgi = 1;
			priv->need_timings = 0;

			rc = ioctl(LIBGGI_FD(vis), DRIVER_GETINFO, &info);
			if (!rc) {
				GGIDPRINT_MISC("display-fbdev: KGI driver by %s, type %s, version %08x.\n",
					       info.manufact, info.model,
					       info.version);
			}
		}
	}

	/* Read original mode on framebuffer */
	if ((ioctl(LIBGGI_FD(vis),FBIOGET_FSCREENINFO,&priv->orig_fix) < 0) ||
	    (ioctl(LIBGGI_FD(vis),FBIOGET_VSCREENINFO,&priv->orig_var) < 0)) {
		perror("display-fbdev: GET_SCREENINFO");
		close(LIBGGI_FD(vis));
		LIBGGI_FD(vis) = -1;
		do_cleanup(vis);
		return GGI_ENODEVICE;
	}
	save_palette(vis);

	LIBGGI_GC(vis) = priv->normalgc = malloc(sizeof(ggi_gc));
	if (priv->normalgc == NULL) {
		do_cleanup(vis);
		return GGI_ENOMEM;
	}

#ifdef HAVE_NEW_FBDEV
	GGIDPRINT_MISC("display-fbdev: accel: %d, supported: %d\n",
		       priv->orig_fix.accel, NUM_ACCELS);
	if (priv->orig_fix.accel >= 0 && priv->orig_fix.accel < NUM_ACCELS) {
		vis->needidleaccel = accel_strings[priv->orig_fix.accel].async;
		priv->flags |= accel_strings[priv->orig_fix.accel].flags;
		if (accel_strings[priv->orig_fix.accel].str == NULL) {
			goto no_accel_lib;
		}
		priv->accel
			= malloc(strlen(accel_strings[priv->orig_fix.accel].str)
				 + PREFIX_LEN);
		if (priv->accel == NULL) {
			do_cleanup(vis);
			return GGI_ENOMEM;
		}
		sprintf(priv->accel, "%s%s", accel_prefix,
			accel_strings[priv->orig_fix.accel].str);
		GGIDPRINT_MISC("display-fbdev: Have accel: \"%s\"\n",
			       accel_strings[priv->orig_fix.accel].str);
	}
  no_accel_lib:
#endif /* HAVE_NEW_FBDEV */

	if (get_timings(priv, modedb) == GGI_ENOMEM) {
		/* If we can't do a simple malloc() it's better to fail
		   right now. */
		do_cleanup(vis);
		return GGI_ENOMEM;
	}

	/* Mode management */
	vis->opdisplay->getmode   = GGI_fbdev_getmode;
	vis->opdisplay->setmode   = GGI_fbdev_setmode;
	vis->opdisplay->checkmode = GGI_fbdev_checkmode;
	vis->opdisplay->getapi    = GGI_fbdev_getapi;
	vis->opdisplay->flush     = GGI_fbdev_flush;
	vis->opdisplay->idleaccel = GGI_fbdev_idleaccel;
	vis->opdisplay->setflags  = GGI_fbdev_setflags;
	vis->opdisplay->sendevent = GGI_fbdev_sendevent;
	vis->opdisplay->kgicommand= GGI_fbdev_kgicommand;

	/* Register cleanup handler */
	ggRegisterCleanup((ggcleanup_func *)do_cleanup, vis);

	GGIDPRINT("display-fbdev: GGIdlinit success.\n");

	*dlret = GGI_DL_OPDISPLAY;
	return 0;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	return do_cleanup(vis);
}


int GGIdl_fbdev(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		*funcptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>

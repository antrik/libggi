/* $Id: visual.c,v 1.17 2006/01/30 21:51:18 cegger Exp $
******************************************************************************

   Display-VCSA: visual management

   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
   Copyright (C) 2000 Marcus Sundberg	[marcus@ggi-project.org]

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#ifdef HAVE_SYS_VT_H
#include <sys/vt.h>
#else
#include <linux/vt.h>
#endif

#include "config.h"
#include <ggi/display/vcsa.h>
#include <ggi/internal/ggi_debug.h>


static const gg_option optlist[] =
{
	{ "noinput", "no" },
	{ "nokbd",   "no" },
	{ "nomouse", "no" },
	{ "ascii",   "no" },
	{ "shade",   "no" },
	{ "physz",   "0,0" },
	{ ":file",   "" }
};

#define OPT_NOINPUT	0
#define OPT_NOKBD	1
#define OPT_NOMOUSE	2
#define OPT_ASCII	3
#define OPT_SHADE	4
#define OPT_PHYSZ	5
#define OPT_FILE	6

#define NUM_OPTS	(sizeof(optlist)/sizeof(gg_option))



static int which_console(void)
{
	int fd;
	struct vt_stat vtinfo;

	fd = open("/dev/tty", O_RDWR);

	if (fd < 0) {
		perror("display-vcsa: cannot open /dev/tty");
		return GGI_ENODEVICE;
	}

	if (ioctl(fd, VT_GETSTATE, &vtinfo) != 0) {
		perror("display-vcsa: VT_GETSTATE failed");
		fprintf(stderr, "display-vcsa: (You need to be running "
			"directly on a virtual console).\n");
		close(fd);
		return GGI_ENODEVICE;
	}

	close(fd);

	return vtinfo.v_active;
}

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32_t *dlret)
{
	ggi_vcsa_priv *priv;
	gg_option options[NUM_OPTS];
	gii_input *inp;
	char filename[80];
	unsigned char buf[4];
	int vt = -1;
	int err = GGI_ENOMEM;

	DPRINT_MISC("display-vcsa: GGIdlinit start.\n");

	memcpy(options, optlist, sizeof(options));

	if (getenv("GGI_VCSA_OPTIONS") != 0) {
		if (ggParseOptions(getenv("GGI_VCSA_OPTIONS"),
				   options, NUM_OPTS) == NULL) {
			fprintf(stderr,
				"display-vcsa: error in $GGI_VCSA_OPTIONS.\n");
			return GGI_EARGINVAL;
		}
	}

	if (args) {
		args = ggParseOptions(args, options, NUM_OPTS);
		if (args == NULL) {
			fprintf(stderr, "display-vcsa: error in arguments.\n");
			return GGI_EARGINVAL;
		}
	}

 	priv = malloc(sizeof(ggi_vcsa_priv));
	if (priv == NULL) return GGI_ENOMEM;

	LIBGGI_PRIVATE(vis) = vis;

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL){
		goto out_freepriv;
	}

	ggstrlcpy(filename, options[OPT_FILE].result, sizeof(filename));

	err = GGI_ENODEVICE;
	/* work out which console we're on */
	if (filename[0] == '\0') {
		vt = which_console();
		if (vt < 0) {
			goto out_freegc;
		}
		sprintf(filename, "/dev/vcsa%d", vt);
	}

	/* now open the vcsa device */
	DPRINT_MISC("display-vcsa: Using file `%s'.\n", filename);

	LIBGGI_FD(vis) = open(filename, O_RDWR);

	if (LIBGGI_FD(vis) < 0) {
		if (vt > -1) {
			/* Try devfs style device */
			sprintf(filename, "/dev/vcc/a%d", vt);
			LIBGGI_FD(vis) = open(filename, O_RDWR);
		}
		if (LIBGGI_FD(vis) < 0) {
			perror("display-vcsa: Couldn't open vcsa device");
			goto out_freegc;
		}
	}

	/* read vcsa dimensions */
	if (read(LIBGGI_FD(vis), buf, 4) < 4) {
		perror("display-vcsa: Couldn't read vcsa device");
		goto out_closefd;
	}

	priv->width  = buf[1];
	priv->height = buf[0];
	priv->inputs = VCSA_INP_KBD | VCSA_INP_MOUSE;
	priv->flags = 0;

	if (toupper((uint8_t)options[OPT_NOINPUT].result[0]) != 'N') {
		priv->inputs = 0;
	}
	if (toupper((uint8_t)options[OPT_NOKBD].result[0]) != 'N') {
		priv->inputs &= ~VCSA_INP_KBD;
	}
	if (toupper((uint8_t)options[OPT_NOMOUSE].result[0]) != 'N') {
		priv->inputs &= ~VCSA_INP_MOUSE;
	}

	if (toupper((uint8_t)options[OPT_ASCII].result[0]) != 'N') {
		priv->flags |= VCSA_FLAG_ASCII;
	}
	if (toupper((uint8_t)options[OPT_SHADE].result[0]) != 'N') {
		priv->flags |= VCSA_FLAG_SHADE;
	}
	err = _ggi_physz_parse_option(options[OPT_PHYSZ].result, 
                               &(priv->physzflags), &(priv->physz)); 
        if (err != GGI_OK) goto out_closefd;

	/* move cursor somewhere relatively out of the way */
        buf[2] = buf[1];  /* cursor_x */
        buf[3] = buf[0];  /* cursor_y */

	lseek(LIBGGI_FD(vis), 0, SEEK_SET);
	write(LIBGGI_FD(vis), buf, 4);

	/* Open keyboard and mouse input */
	if (priv->inputs & VCSA_INP_KBD) {
		vis->input = giiOpen("input-linux-kbd", NULL);

		if (vis->input == NULL) {
			fprintf(stderr, "display-vcsa: Couldn't open kbd.\n");
			goto out_closefd;
		}
	}
	if (priv->inputs & VCSA_INP_MOUSE) {
		inp = giiOpen("linux-mouse:auto", &args, NULL);
		if (inp != NULL) {
			vis->input = giiJoinInputs(vis->input, inp);
		}
	}

	/* mode management */
	vis->opdisplay->getmode   = GGI_vcsa_getmode;
	vis->opdisplay->setmode   = GGI_vcsa_setmode;
	vis->opdisplay->checkmode = GGI_vcsa_checkmode;
	vis->opdisplay->getapi    = GGI_vcsa_getapi;
	vis->opdisplay->setflags  = GGI_vcsa_setflags;

	DPRINT_MISC("display-vcsa: GGIdlinit success.\n");

	*dlret = GGI_DL_OPDISPLAY;
	return 0;

  out_closefd:
	close(LIBGGI_FD(vis));
  out_freegc:
	free(LIBGGI_GC(vis));
  out_freepriv:
	free(priv);

	return err;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_vcsa_priv *priv = VCSA_PRIV(vis);

	DPRINT_MISC("display-vcsa: GGIdlcleanup start.\n");

	if (LIBGGI_FD(vis) >= 0) {
		GGI_vcsa_resetmode(vis);

		if (vis->input != NULL) {
			giiClose(vis->input);
			vis->input = NULL;
		}

		close(LIBGGI_FD(vis));
		LIBGGI_FD(vis) = -1;
	}

	free(LIBGGI_GC(vis));
	free(priv);

	DPRINT_MISC("display-vcsa: GGIdlcleanup done.\n");

	return 0;
}


EXPORTFUNC
int GGIdl_vcsa(int func, void **funcptr);

int GGIdl_vcsa(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_close **closeptr;

	switch (func) {
	case GGIFUNC_open:
		openptr = (ggifunc_open **)funcptr;
		*openptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		closeptr = (ggifunc_close **)funcptr;
		*closeptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>

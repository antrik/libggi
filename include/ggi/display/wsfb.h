/* $Id: wsfb.h,v 1.2 2003/02/14 16:37:34 fries Exp $
******************************************************************************

   LibGGI wsconsole(4) wsfb target

   Copyright (C) 2003 Todd T. Fries <todd@openbsd.org>

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

#ifndef _GGI_DISPLAY_WSFB_H
#define _GGI_DISPLAY_WSFB_H


#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <dev/wscons/wsconsio.h>

#include <ggi/internal/ggi-dl.h>
#include <ggi/display/modelist.h>

ggifunc_getmode		GGI_wsfb_getmode;
ggifunc_setmode		GGI_wsfb_setmode;
ggifunc_getapi		GGI_wsfb_getapi;
ggifunc_checkmode	GGI_wsfb_checkmode;
ggifunc_setpalvec	GGI_wsfb_setpalvec;

typedef struct wsfb_priv {
	ggi_modelistmode *availmodes;

	int fd;
	char *devname;
	unsigned long *base;
	unsigned long Base, size, mapsize;

	struct wsdisplay_fbinfo info;

	int linebytes, wstype, pagemask, origmode, mode;
} wsfb_priv;

#define WSFB_PRIV(vis) ((wsfb_priv*)LIBGGI_PRIVATE(vis))

#endif /* _GGI_DISPLAY_WSFB_H */

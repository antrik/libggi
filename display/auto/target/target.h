/* $Id: target.h,v 1.1 2004/01/29 13:49:34 cegger Exp $
******************************************************************************

   Auto target for GGI - target option definitions

   Copyright (C) 2004 Christoph Egger

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

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/auto.h>



/* AA-target */
extern struct ggi_auto_TargetOption probe_AA[];

/* DirectX-target */
extern struct ggi_auto_TargetOption probe_DIRECTX[];

/* fbdev-target */
extern struct ggi_auto_TargetOption probe_FBDEV[];

/* KGI-target */
extern struct ggi_auto_TargetOption probe_KGI[];

/* SVGALIB-target */
extern struct ggi_auto_TargetOption probe_SVGALIB[];

/* VGL-target */
extern struct ggi_auto_TargetOption probe_VGL[];

/* X-target */
extern struct ggi_auto_TargetOption probe_X[];


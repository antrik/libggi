/* $Id: misc.c,v 1.44 2007/03/15 15:14:58 pekberg Exp $
******************************************************************************

   X target for GGI, utility functions.

   Copyright (C) 1997      Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 2007      Andreas Beck		[becka@ggi-project.org]
   Copyright (C) 1998      Steve Cheng		[steve@ggi-project.org]
   Copyright (C) 1998-2000 Marcus Sundberg	[marcus@ggi-project.org]
   Copyright (C) 2002      Brian S. Julin	[bri@tull.umassp.edu]

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

#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/display/x.h>

/* return 1 if an X11 Visual (cthis) has a "better" pixelformat (than than)
 * "this" is a C++ keyword, so we use "cthis" instead. 
 */
int _ggi_x_is_better_fmt(XVisualInfo *than, XVisualInfo *cthis)
{
	/* DPRINT_MODE( "_ggi_x_is_better_fmt() entered.\n"); */

	/* prefer color to grayscale */
	if (((than->class==StaticGray) || (than->class==GrayScale))
	    && 
	    ((cthis->class!=StaticGray) && (cthis->class!=GrayScale))) return 1;
	if (((cthis->class==StaticGray) || (cthis->class==GrayScale))
	    && 
	    ((than->class!=StaticGray) && (than->class!=GrayScale))) return -1;

	/* prefer more colors or levels */
	if (than->depth < cthis->depth) return 1;
	if (than->depth > cthis->depth) return 0;

	/* Prefer read-write colormaps if depth is the same. */
	if ((than->class==StaticGray) && (cthis->class==GrayScale))   return 1;
	if ((cthis->class==StaticGray) && (than->class==GrayScale))   return -1;
	if ((than->class==StaticColor)&& (cthis->class==PseudoColor)) return 1;
	if ((cthis->class==StaticColor)&& (than->class==PseudoColor)) return -1;
	if ((than->class==TrueColor)  && (cthis->class==PseudoColor)) return 1;
	if ((cthis->class==TrueColor)  && (than->class==PseudoColor)) return -1;

	/* prefer gammamap to palette if the depth is the same */
	if ((than->class==PseudoColor)&& (cthis->class==DirectColor)) return 1;
	if ((cthis->class==PseudoColor)&& (than->class==DirectColor)) return -1;

	/* prefer truecolor to static color */
	if ((than->class==StaticColor) && (cthis->class==TrueColor))  return 1;
	if ((cthis->class==StaticColor) && (than->class==TrueColor))  return -1;

	/* prefer gamma map to truecolor */
	if ((than->class==StaticColor)&& (cthis->class==DirectColor)) return 1;
	if ((cthis->class==StaticColor)&& (than->class==DirectColor)) return -1;
	if ((than->class==TrueColor)  && (cthis->class==DirectColor)) return 1;
	if ((cthis->class==TrueColor)  && (than->class==DirectColor)) return -1;

	/* don't swap equal visuals */
	if (cthis->class == than->class) return -1;

	/* More? */
	return 0;
}

int _ggi_x_is_better_screen(Screen *than, Screen *cthis)
{
	if (!DoesBackingStore(than) && DoesBackingStore(cthis)) return 1;
	if (DoesBackingStore(than) && !DoesBackingStore(cthis)) return -1;

	if (than->width * than->height < cthis->width * cthis->height)
		return 1;
	if (than->width * than->height > cthis->width * cthis->height)
		return -1;
	if (than->mwidth * than->mheight < cthis->mwidth * cthis->mheight)
		return 1;
	if (than->mwidth * than->mheight > cthis->mwidth * cthis->mheight)
		return -1;

	if (than->ndepths < cthis->ndepths) return 1;
	if (than->ndepths > cthis->ndepths) return 0;

	/* don't swap equal visuals */
	if (cthis->ndepths == than->ndepths) return -1;

	/* More? */
	return 0;
}

/* We cross index and pre-sort the various informational items;
 * This reduces code complexity in checkmode.
 */
void _ggi_x_build_vilist(struct ggi_visual *vis)
{
	int viidx, more;
	int nvisuals;
	ggi_x_priv *priv = GGIX_PRIV(vis);

	nvisuals = priv->nvisuals;
	for (viidx = 0; viidx < priv->nvisuals; viidx++) {
		ggi_x_vi *vi;
		int bufidx;

		vi = priv->vilist + viidx;
		vi->vi = priv->visual + viidx;

		for (bufidx = 0; bufidx < priv->nbufs; bufidx++) {
			if (priv->buflist[bufidx].depth == vi->vi->depth)
				vi->buf = priv->buflist + bufidx;
		}
	}

	do {
		ggi_x_vi *tmp;
		if (priv->nvisuals == nvisuals) break;

		DPRINT_MISC("downsize the visual list to %i visuals\n",
				nvisuals);

		/* We free the unused visuals by downsizing
		 * the allocated memory.
		 */
		tmp = realloc(priv->vilist, sizeof(ggi_x_vi) * nvisuals);
		if (tmp == NULL) {
			/* reallocation failed, although we downsize?
			 * There must be something wrong in the kernel VM.
			 */
			DPRINT("downsizing using realloc() failed!\n");
		}	/* if */
		priv->nvisuals = nvisuals;
		priv->vilist = tmp;

		APP_ASSERT(priv->nvisuals > 0, "nvisuals shouldn't be 0");
	} while(0);

	/* Bubblesort priv->vilist from least-to-most desirable visuals */
 more:
	more = 0;
	viidx = 0;
	while (viidx < (priv->nvisuals - 1)) {
		ggi_x_vi    tmp;
		int restmp;
		XVisualInfo *via, *vib;

		via = (priv->vilist + viidx)->vi;
		vib = (priv->vilist + viidx + 1)->vi;

		restmp = _ggi_x_is_better_fmt(vib, via);
		if (restmp > 0) {
			goto swap;
		} else if (restmp == 0) {
			restmp = _ggi_x_is_better_screen(ScreenOfDisplay(priv->disp,
						vib->screen), ScreenOfDisplay(priv->disp,
							    via->screen));
			if (restmp > 0) {
				goto swap;
			} else if (restmp == 0) {
				if (via->visualid > vib->visualid) {
					goto swap;
				}
			}
		}
		viidx++;
		continue;
	swap:
		memcpy (&tmp, priv->vilist + viidx + 1, sizeof(ggi_x_vi));
		memcpy (priv->vilist + viidx + 1, priv->vilist + viidx, 
			sizeof(ggi_x_vi));
		memcpy (priv->vilist + viidx, &tmp, sizeof(ggi_x_vi));
		more = 1;
		viidx++;
	}
	if (more) goto more;
}

/* This will return GT_INVALID if vi doesn't match gt.
 * It will fill out any GT_AUTO subfields if vi does match gt.
 */
ggi_graphtype _ggi_x_scheme_vs_class(ggi_graphtype gt, ggi_x_vi *vi) {
	ggi_graphtype size, depth;

	if (!vi) {
		fprintf(stderr, "vi == %p\n", (void *)vi);
		return GT_INVALID;
	}	/* if */
	if (!vi->vi) {
		fprintf(stderr, "vi->vi == %p\n", (void *)vi->vi);
		return GT_INVALID;
	}	/* if */
	if (!vi->vi->depth) {
		fprintf(stderr, "vi->vi->depth == %i\n", vi->vi->depth);
		return GT_INVALID;
	}	/* if */

	if (!vi || !vi->vi || !vi->vi->depth) return GT_INVALID;
	depth = vi->vi->depth;
	if (GT_DEPTH(gt) && GT_DEPTH(gt) != depth) return GT_INVALID;

	if (!vi->buf->bits_per_pixel) return GT_INVALID;
	size = vi->buf->bits_per_pixel;
	if (GT_SIZE(gt) && GT_SIZE(gt) != size) return GT_INVALID;

#define GT_CLASS_CONSTRUCT(cls, scheme) \
case cls: return(GT_CONSTRUCT(depth, scheme, size))

	if (GT_SCHEME(gt) == GT_AUTO) {
		switch (vi->vi->class) {
			GT_CLASS_CONSTRUCT(StaticGray,  GT_STATIC_PALETTE);
			GT_CLASS_CONSTRUCT(GrayScale,   GT_GREYSCALE);
			GT_CLASS_CONSTRUCT(StaticColor, GT_STATIC_PALETTE);
			GT_CLASS_CONSTRUCT(PseudoColor, GT_PALETTE);
			GT_CLASS_CONSTRUCT(TrueColor,   GT_TRUECOLOR);
			GT_CLASS_CONSTRUCT(DirectColor, GT_TRUECOLOR);
		default:
			return (GT_INVALID);
		}
	}

	switch (GT_SCHEME(gt)) {
	case GT_GREYSCALE:
		if (vi->vi->class != GrayScale && vi->vi->class != StaticGray) 
			return GT_INVALID;
		break;
	case GT_STATIC_PALETTE:
		if (vi->vi->class != StaticColor) return GT_INVALID;
		break;
	case GT_PALETTE:
		if (vi->vi->class != PseudoColor) return GT_INVALID;
		break;
	case GT_TRUECOLOR:
	  	if (vi->vi->class != TrueColor && vi->vi->class != DirectColor)
			return GT_INVALID;
		break;
	default:
		return GT_INVALID;
	}

	return GT_CONSTRUCT(vi->vi->depth, GT_SCHEME(gt), size);
}

/* Returns GGI_OK if a ggi_mode can be fit to the screen or window.
 * In either case, suggest is filled with a suggested mode.
 * Note this function may be called with tm == suggest.
 */
int _ggi_x_fit_geometry(struct ggi_visual *vis, ggi_mode *tm, 
			ggi_x_vi *vi, ggi_mode *suggest)
{
	ggi_x_priv     *priv;
	int 		res = GGI_OK;
	unsigned int 	w, h, screenw, screenh, screenwmm, screenhmm;

	APP_ASSERT(vis != NULL, "GGIcheckmode: vis == NULL");

	priv = GGIX_PRIV(vis);

	if (suggest != tm) memcpy(suggest, tm, sizeof(ggi_mode));

	screenwmm = DisplayWidthMM(priv->disp, vi->vi->screen);
	screenw   = DisplayWidth(priv->disp, vi->vi->screen);
	screenhmm = DisplayHeightMM(priv->disp, vi->vi->screen);
	screenh   = DisplayHeight(priv->disp, vi->vi->screen);

	if (tm->frames == GGI_AUTO) suggest->frames = 1;

	if ((tm->dpp.x != 1 && tm->dpp.x != GGI_AUTO) ||
	    (tm->dpp.y != 1 && tm->dpp.y != GGI_AUTO)) {
		res = GGI_ENOMATCH;
	}
	suggest->dpp.x = suggest->dpp.y = 1;

	w = screenw;
	h = screenh;
	if ((priv->parentwin != None) && (priv->parentwin == priv->win)) { 
		/* Don't resize an explicitly requested window */ 
		Window dummywin;
		unsigned int dummy;
		APP_ASSERT(priv->parentwin, "no parent here... bad.");
		XGetGeometry(priv->disp, priv->parentwin, &dummywin,
			     (int *)&dummy, (int *)&dummy,
			     &w, &h, &dummy, &dummy);
	}
	else if (priv->win == None) { /* Not a root window */
		/* Don't create a window who's handles/borders are offscreen */
		w = screenw * 9 / 10; 
		w = (signed)((unsigned)(w)+3) & ~3;	/* make likely virt=visible */
		h = screenh * 9 / 10;
	}

	APP_ASSERT(w && h, "Bad max w/h.");

	if (tm->visible.x == GGI_AUTO) {
		if (tm->virt.x == GGI_AUTO) {
			suggest->visible.x = w;
		} else {
			suggest->visible.x = tm->virt.x;
		}
		if ((unsigned)suggest->visible.x > w) suggest->visible.x = w;
	}
	if (tm->visible.y == GGI_AUTO) {
		if (tm->virt.y == GGI_AUTO) {
			suggest->visible.y = h;
		} else {
			suggest->visible.y = tm->virt.y;
		}
		if ((unsigned)suggest->visible.x > w) suggest->visible.x = w;
	}
	if (tm->virt.x == GGI_AUTO) 
		suggest->virt.x = (suggest->visible.x+3) & ~3;
	if (tm->virt.y == GGI_AUTO) 
		suggest->virt.y = suggest->visible.y;

	if (suggest->virt.x < suggest->visible.x) {
		suggest->virt.x = (suggest->visible.x+3) & ~3;
		res = GGI_ENOMATCH;
	}

	/* Not necessary in all cases, but it doesn't hurt */
	if ((suggest->virt.x & ~3) != suggest->virt.x) {
		suggest->virt.x = ((suggest->virt.x+3) & ~3);
		res = GGI_ENOMATCH;
	}

	if (suggest->virt.y < suggest->visible.y) {
		suggest->virt.y = suggest->visible.y;
		res = GGI_ENOMATCH;
	}

        res = _ggi_physz_figure_size(suggest, GGI_PHYSZ_MM, &(priv->physz),
                                (signed)screenwmm, (signed)screenhmm,
				(signed)screenw, (signed)screenh);

	return(res);
}




/* return 1 if one ggi_graphtype is "better" than another. */
int _ggi_x_is_better_gt(ggi_graphtype than, ggi_graphtype cthis) {

	/* prefer color to grayscale */
	if ((GT_SCHEME(than)==GT_GREYSCALE)&&(GT_SCHEME(cthis)!=GT_GREYSCALE))
		return 1;

	/* prefer more colors or levels */
	if (GT_DEPTH(than) < GT_DEPTH(cthis)) return 1;

	/* prefer palette to true/fixed color (of same depth.) */
	if ((GT_SCHEME(than)==GT_STATIC_PALETTE) &&
	    (GT_SCHEME(cthis)==GT_PALETTE))
		return 1;
	if ((GT_SCHEME(than)==GT_TRUECOLOR)&&(GT_SCHEME(cthis)==GT_PALETTE))
		return 1;

	return 0;
}

void _ggi_x_build_pixfmt(struct ggi_visual *vis, ggi_mode *tm, XVisualInfo *vi)
{
	ggi_pixelformat *fmt;

	fmt = LIBGGI_PIXFMT(vis);

	memset(fmt, 0, sizeof(ggi_pixelformat));

	fmt->red_mask   = vi->red_mask;
	fmt->green_mask = vi->green_mask;
	fmt->blue_mask  = vi->blue_mask;

	fmt->depth = GT_DEPTH(tm->graphtype);
	fmt->size  = GT_SIZE(tm->graphtype);

	if (vi->class == StaticColor || vi->class == PseudoColor ||
	    vi->class == StaticGray || vi->class == GrayScale)
	{
		fmt->clut_mask = (1 << vi->depth) - 1;
	} else {
		fmt->clut_mask = 0;
	}

	_ggi_build_pixfmt(fmt);
}

/* Make a newly created parent window presentable */
int _ggi_x_dress_parentwin(struct ggi_visual *vis, ggi_mode *tm)
{
	int rc;
	XSizeHints      hint;
	ggi_x_priv      *priv;
	const char *winname = "GGI-on-X";
	const char *iconname = winname;

	priv = GGIX_PRIV(vis);

	if (priv->windowtitle) winname =priv->windowtitle;
	if (priv->icontitle  ) iconname=priv->icontitle;

	/* Fill in hint structure. */
	hint.x = hint.y         = 0;
	hint.width              = tm->visible.x;
	hint.height             = tm->visible.y;
	hint.flags              = PSize | PMinSize | PMaxSize;
	hint.min_width          = tm->visible.x;
	hint.min_height         = tm->visible.y;
	hint.max_width          = tm->visible.x;
	hint.max_height         = tm->visible.y;
	
	/* Set WM hints and titles */
	rc = XSetStandardProperties(priv->disp, priv->parentwin, winname, iconname,
			       None, NULL, 0, &hint);
	return rc;
}

void _ggi_x_set_xclip (struct ggi_visual *vis, Display *disp, GC gc, 
		       int x, int y, int w, int h)
{
	XRectangle *xrect;
	int i, frames, virty;


	if (vis != NULL) {
		frames = LIBGGI_MODE(vis)->frames;
		virty = LIBGGI_VIRTY(vis);
	} else {
		frames = 1;
		virty = 0;
	}

	xrect = calloc(frames, sizeof(XRectangle));
	if (xrect == NULL) return;

	for (i = 0; i < frames; i++) {
		xrect[i].x = x; xrect[i].width = w;
		xrect[i].y = y + i * virty; xrect[i].height = h;
	}
	XSetClipRectangles(disp, gc, 0, 0, xrect, frames, Unsorted);
	free(xrect);
}

void _ggi_x_create_dot_cursor (struct ggi_visual *vis)
{
	ggi_x_priv *priv;
	Pixmap crsrpix, crsrmask;
	unsigned char crspdat[] = { 0xf8, 0xfa, 0xf8 };
	unsigned char crsmdat[] = { 0xfa, 0xff, 0xfa };
	unsigned int dummy;
	Window root;
	XSetWindowAttributes wa;

	XColor black = {
		0, 0x0, 0x0, 0x0,
		DoRed | DoGreen | DoBlue,
		0
	};
	XColor white = {
		0, 0xffff, 0xffff, 0xffff,
		DoRed | DoGreen | DoBlue,
		0
	};

	priv = GGIX_PRIV(vis);

	if (priv->cursor != None) {
		if (priv->oldcursor != None) {
			XFreeCursor(priv->disp, priv->cursor);
		}
		priv->oldcursor = priv->cursor;
	}

	XGetGeometry(priv->disp, priv->parentwin, &root,
		     (int *)&dummy, (int *)&dummy,
		     &dummy, &dummy, &dummy, &dummy);
            
	crsrpix = XCreateBitmapFromData(priv->disp, root, (char *)crspdat, 3, 3);
	crsrmask = XCreateBitmapFromData(priv->disp, root, (char *)crsmdat, 3, 3);
	priv->cursor = XCreatePixmapCursor(priv->disp, crsrpix, crsrmask,
					   &black, &white, 1, 1);
	wa.cursor = priv->cursor;
	XChangeWindowAttributes(priv->disp, priv->parentwin, CWCursor, &wa); 
	XFreePixmap(priv->disp, crsrpix);
	XFreePixmap(priv->disp, crsrmask);
}


void _ggi_x_create_invisible_cursor (struct ggi_visual *vis)
{
	ggi_x_priv *priv;
	Pixmap crsrpix, crsrmask;
	char crspdat[] = { 0 };
	char crsmdat[] = { 0 };
	unsigned int dummy;
	Window root;
	XSetWindowAttributes wa;

	XColor black = {
		0, 0x0, 0x0, 0x0,
		DoRed | DoGreen | DoBlue,
		0
	};
	XColor white = {
		0, 0xffff, 0xffff, 0xffff,
		DoRed | DoGreen | DoBlue,
		0
	};


	priv = GGIX_PRIV(vis);

	if (priv->cursor != None) {
		if (priv->oldcursor != None) {
			XFreeCursor(priv->disp, priv->cursor);
		}
		priv->oldcursor = priv->cursor;
	}

	XGetGeometry(priv->disp, priv->parentwin, &root,
		     (int *)&dummy, (int *)&dummy,
		     &dummy, &dummy, &dummy, &dummy);
            
	crsrpix = XCreateBitmapFromData(priv->disp, root, crspdat, 1, 1);
	crsrmask = XCreateBitmapFromData(priv->disp, root, crsmdat, 1, 1);
	priv->cursor = XCreatePixmapCursor(priv->disp, crsrpix, crsrmask,
					   &black, &white, 1, 1);
	wa.cursor = priv->cursor;
	XChangeWindowAttributes(priv->disp, priv->parentwin, CWCursor, &wa); 
        XFreePixmap(priv->disp, crsrpix);
        XFreePixmap(priv->disp, crsrmask);
}

void _ggi_x_readback_fontdata (struct ggi_visual *vis)
{
	ggi_x_priv *priv;
	Pixmap fontpix;
	char str[256];
	int i, w, h;
	GC pixgc;

	priv = GGIX_PRIV(vis);

	w = priv->textfont->max_bounds.width;
	h = priv->textfont->max_bounds.ascent
	  + priv->textfont->max_bounds.descent;

	if (priv->fontimg) XDestroyImage(priv->fontimg);

	fontpix = XCreatePixmap(priv->disp, priv->drawable, 
				(unsigned)w * 256, (unsigned)h, 
				(unsigned)priv->vilist[priv->viidx].vi->depth);

	DPRINT_MISC("_ggi_x_readback_fontdata: calling XCreateGC(%p,%p,0,0)\n",
			priv->disp, priv->win);
	pixgc = XCreateGC(priv->disp, priv->win, 0, 0);
	XSetFont(priv->disp, pixgc, priv->textfont->fid);
	_ggi_x_set_xclip(NULL, priv->disp, pixgc, 0, 0, w * 256, h);
	XSetForeground(priv->disp, pixgc, 0);

	XFillRectangle(priv->disp, fontpix, pixgc, 
			0, 0, (unsigned)w * 256, (unsigned)h);
	XSetForeground(priv->disp, pixgc, ~0U);
	for (i = 0; i < 256; i++) str[i] = i;
	XDrawString(priv->disp, fontpix, pixgc, 
		    0, priv->textfont->max_bounds.ascent, 
		    str, 256);
	XSync(priv->disp, 0);
	priv->fontimg = XGetImage(priv->disp, fontpix, 0, 0, 
				  (unsigned)w * 256, (unsigned)h, 
				  AllPlanes, ZPixmap);
	XFreeGC(priv->disp, pixgc);

	/* Reverse endianness if needed. */
	if (priv->fontimg->byte_order == 
#ifdef GGI_LITTLE_ENDIAN
	    LSBFirst
#else
	    MSBFirst
#endif
	    ) goto noswab;
	
	if (priv->fontimg->bits_per_pixel == 16) {
		uint8_t *ximgptr;
		ximgptr = (uint8_t *)(priv->fontimg->data) + 
		  (priv->fontimg->xoffset * priv->fontimg->bits_per_pixel)/8;
		while (h--) {
			int j;
			uint8_t tmp;
			for (j = 0; j < w * 512; j += 2) {
				tmp = *(ximgptr + j);
				*(ximgptr + j) = *(ximgptr + j + 1);
				*(ximgptr + j + 1) = tmp;
			}
			ximgptr += priv->fontimg->bytes_per_line;
		}
	}
	else if (priv->fontimg->bits_per_pixel == 32) {
		uint8_t *ximgptr;
		ximgptr = (uint8_t *)(priv->fontimg->data) + 
		  (priv->fontimg->xoffset * priv->fontimg->bits_per_pixel)/8;
		while (h--) {
			int j;
			uint8_t tmp;
			for (j = 0; j < w * 1024; j += 4) {
				tmp = *(ximgptr + j);
				*(ximgptr + j) = *(ximgptr + j + 3);
				*(ximgptr + j + 3) = tmp;
				tmp = *(ximgptr + j + 1);
				*(ximgptr + j + 1) = *(ximgptr + j + 2);
				*(ximgptr + j + 2) = tmp;
			}
			ximgptr += priv->fontimg->bytes_per_line;
		}
	}

noswab:
	XFreePixmap(priv->disp, fontpix);
}


#ifndef HAVE_XINITIMAGE
XImage *_ggi_x_create_ximage( struct ggi_visual *vis, char *data, int w, int h )
{
	ggi_x_priv *priv;
	ggi_pixelformat *fmt;
	XImage *img0;

	priv = GGIX_PRIV(vis);

	img0 = XCreateImage(priv->disp, priv->vilist[priv->viidx].vi->visual,
			    (unsigned)LIBGGI_PIXFMT(vis)->depth,
			    ZPixmap, 0,
			    data, (unsigned)w, (unsigned)h, 8, 0);

	if (img0 == NULL) {
		DPRINT_MISC("XCreateImage failed\n");
		return NULL;
	}

	fmt = LIBGGI_PIXFMT(vis);

	/* Don't take Byte order information from the Xserver */
#ifdef GGI_LITTLE_ENDIAN
	if (fmt->flags & GGI_PF_REVERSE_ENDIAN)
		img0->byte_order = MSBFirst;
	else
		img0->byte_order = LSBFirst;
#else
	if (fmt->flags & GGI_PF_REVERSE_ENDIAN)
		img0->byte_order = LSBFirst;
	else
		img0->byte_order = MSBFirst;
#endif
	/* Take Bit order information from the Xserver. Why??? */
	img0->bitmap_bit_order = BitmapBitOrder(priv->disp);
	DPRINT_MISC("byte order = %i\n", img0->byte_order);
	DPRINT_MISC("bit order = %i\n", img0->bitmap_bit_order);

	return img0;
}
#else /* HAVE_XINITIMAGE */
/* Allocates and initializes an XImage struct.
 * This function was written because XCreateImage() is not
 * DGA compatible as it requires a visual.
 * The XImage should be freed with free() instead of XFree(). */
XImage *_ggi_x_create_ximage( struct ggi_visual *vis, char *data, int w, int h )
{
	ggi_pixelformat *fmt;
	ggi_x_priv *priv;
	XImage *img0;

	priv = GGIX_PRIV(vis);

	img0 = malloc( sizeof(XImage) );
	if( img0 == NULL )
		return NULL;
	
	img0->width = w;
	img0->height = h;          
	img0->xoffset = 0;         /* number of pixels offset in X direction */
	img0->format = ZPixmap;    /* XYBitmap, XYPixmap, ZPixmap */
	img0->data = data;         /* pointer to image data */

	fmt = LIBGGI_PIXFMT(vis);

	/* Don't take Byte order information from the Xserver */
#ifdef GGI_LITTLE_ENDIAN
	if (fmt->flags & GGI_PF_REVERSE_ENDIAN)
		img0->byte_order = MSBFirst;
	else
		img0->byte_order = LSBFirst;
#else
	if (fmt->flags & GGI_PF_REVERSE_ENDIAN)
		img0->byte_order = LSBFirst;
	else
		img0->byte_order = MSBFirst;
#endif
	/* Take Bit order information from the Xserver. Why??? */
	img0->bitmap_bit_order = BitmapBitOrder(priv->disp);
	DPRINT_MISC("byte order = %i\n", img0->byte_order);
	DPRINT_MISC("bit order = %i\n", img0->bitmap_bit_order);

#if 0
	img0->bitmap_unit = BitmapUnit(priv->disp);	
		/* quant. of scanline 8, 16, 32 */
	img0->bitmap_pad = BitmapPad(priv->disp);	
		/* 8, 16, 32 either XY or ZPixmap */
#endif
	/* Empirical results suggest it's best to put 0 in the 
	 * bitmap_unit field. */
	img0->bitmap_unit = img0->bitmap_pad = 0;
	DPRINT_MISC("bitmap_unit = %i\n", img0->bitmap_unit);
	DPRINT_MISC("bitmap_pad = %i\n", img0->bitmap_pad);


	img0->depth = fmt->depth;	/* depth of image */
	img0->bits_per_pixel = fmt->size;  /* bits per pixel (ZPixmap) */
	img0->red_mask = fmt->red_mask;    /* bits in z arrangment */
	img0->green_mask = fmt->green_mask;
	img0->blue_mask = fmt->blue_mask;
	img0->obdata = NULL;

	/* XInitImage() will calculate this one, if we set it to 0.
	 * But we do not trust XInitImage() to calculate it right.
	 */
	img0->bytes_per_line = ((w * fmt->size + 7) / 8);  

	if( XInitImage(img0) ) {
		free(img0);
		DPRINT("XInitImage failed!\n");
		return NULL;
	}

	return img0;
}
#endif /* HAVE_XINITIMAGE */

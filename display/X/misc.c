/* $Id: misc.c,v 1.5 2002/07/09 13:05:32 cegger Exp $
******************************************************************************

   X target for GGI, utility functions.

   Copyright (C) 1997      Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1998      Andreas Beck		[becka@ggi-project.org]
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

#include <ggi/internal/ggi-dl.h>
#include <ggi/display/x.h>

/* return 1 if an X11 Visual (this) has a "better" pixelformat (than than) */
static int _ggi_x_is_better_fmt(XVisualInfo *than, XVisualInfo *this) {

	/* prefer color to grayscale */
	if (((than->class==StaticGray) || (than->class==GrayScale))
	    && 
	    ((this->class!=StaticGray) && (this->class!=GrayScale)))  return 1;
	if (((this->class==StaticGray) || (this->class==GrayScale))
	    && 
	    ((than->class!=StaticGray) && (than->class!=GrayScale)))  return 0;

	/* prefer more colors or levels */
	if (than->depth < this->depth) return 1;
	if (than->depth > this->depth) return 0;

	/* Prefer read-write colormaps if depth is the same. */
	if ((than->class==StaticGray)  && (this->class==GrayScale))   return 1;
	if ((this->class==StaticGray)  && (than->class==GrayScale))   return 0;
	if ((than->class==StaticColor) && (this->class==PseudoColor)) return 1;
	if ((this->class==StaticColor) && (than->class==PseudoColor)) return 0;
	if ((than->class==TrueColor)   && (this->class==PseudoColor)) return 1;
	if ((this->class==TrueColor)   && (than->class==PseudoColor)) return 0;

	/* prefer palette to gammamap if the depth is the same */
	if ((than->class==DirectColor) && (this->class==PseudoColor)) return 1;
	if ((this->class==DirectColor) && (than->class==PseudoColor)) return 0;

	/* prefer gamma map to truecolor */
	if ((than->class==StaticColor) && (this->class==DirectColor)) return 1;
	if ((this->class==StaticColor) && (than->class==DirectColor)) return 0;
	if ((than->class==TrueColor)   && (this->class==DirectColor)) return 1;
	if ((this->class==TrueColor)   && (than->class==DirectColor)) return 0;

	/* More? */

	return 0;
}

static int _ggi_x_is_better_screen(Screen *than, Screen *this) {

	if (!DoesBackingStore(than) && DoesBackingStore(this)) return 1;
	if (DoesBackingStore(than) && !DoesBackingStore(this)) return 0;
	if (than->width * than->height < this->width * this->height)
		return 1;
	if (than->width * than->height > this->width * this->height)
		return 0;
	if (than->mwidth * than->mheight < this->mwidth * this->mheight)
		return 1;
	if (than->mwidth * than->mheight > this->mwidth * this->mheight)
		return 0;

	if (than->ndepths < this->ndepths) return 1;
	if (than->ndepths > this->ndepths) return 0;

	/* More? */

	return 0;
}


/* return 1, if vi is unique */
static int _ggi_x_visual_is_unique(ggi_visual *vis, int viidx_cmp)
{
	int viidx;
	ggi_x_priv *priv;
	XVisualInfo *via, *vib;
	priv = LIBGGI_PRIVATE(vis);

	via = priv->visual + viidx_cmp;

	for (viidx = 0; viidx < priv->nvisuals; viidx++) {
		vib = priv->visual + viidx;

		if (vib == NULL) continue;
		if (vib->class != via->class) continue;
		if (vib->depth != via->depth) continue;
		if (vib->visualid >= via->visualid) continue;

		GGIDPRINT_MISC("visual %X duplicates %X\n",
			via->visualid, vib->visualid);
		return 0;
	}	/* for */

	return 1;
}	/* _ggi_x_visual_is_unique */


/* We cross index and pre-sort the various informational items;
 * This reduces code complexity in checkmode.
 */
void _ggi_x_build_vilist(ggi_visual *vis)
{
	int viidx, more;
	int nvisuals;
	ggi_x_priv *priv;
	priv = LIBGGI_PRIVATE(vis);

	nvisuals = priv->nvisuals;
	for (viidx = 0; viidx < priv->nvisuals; viidx++) {
		ggi_x_vi *vi;
		int bufidx;

		/* There are duplicate visuals in multihead configurations.
		 * So we have to make sure that the visual list is unique to
		 * avoid an endless loop in _ggi_x_build_vilist().
		 */
		if (!_ggi_x_visual_is_unique(vis, viidx)) {
			nvisuals--;
			continue;
		}	/* if */

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

		GGIDPRINT_MISC("downsize the visual list to %i visuals\n",
				nvisuals);

		/* We free the unused visuals by downsizing
		 * the allocated memory.
		 */
		tmp = realloc(priv->vilist, sizeof(ggi_x_vi) * nvisuals);
		if (tmp == NULL) {
			/* reallocation failed, although we downsize?
			 * There must be something wrong in the kernel VM.
			 */
			GGIDPRINT("downsizing using realloc() failed!\n");
		}	/* if */
		priv->nvisuals = nvisuals;
		priv->vilist = tmp;

		LIBGGI_APPASSERT(priv->nvisuals > 0, "nvisuals shouldn't be 0");
	} while(0);

	/* Bubblesort priv->vilist from least-to-most desirable visuals */
 more:
	more = 0;
	viidx = 0;
	while (viidx < (priv->nvisuals - 1)) {
		ggi_x_vi    tmp;
		XVisualInfo *via, *vib;

		via = (priv->vilist + viidx)->vi;
		vib = (priv->vilist + viidx + 1)->vi;

		if (_ggi_x_is_better_fmt(vib, via)) goto swap;
		if (_ggi_x_is_better_screen(ScreenOfDisplay(priv->disp,
							    vib->screen),
					    ScreenOfDisplay(priv->disp,
							    via->screen)))
		    goto swap;

		/* Don't swap visuals by visualid, because when they are
		 * reswapped by above again and causing another endless loop.
		 */
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
		fprintf(stderr, "vi == %p\n", vi);
		return GT_INVALID;
	}	/* if */
	if (!vi->vi) {
		fprintf(stderr, "vi->vi == %p\n", vi->vi);
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
int _ggi_x_fit_geometry(ggi_visual *vis, ggi_mode *tm, 
			ggi_x_vi *vi, ggi_mode *suggest) {
	ggi_x_priv     *priv;
	int 		res = GGI_OK;
	unsigned int 	w, h, screenw, screenh, screenwmm, screenhmm;

	LIBGGI_APPASSERT(vis != NULL, "GGIcheckmode: vis == NULL");

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
                int dummy;
                unsigned udummy;
		LIBGGI_APPASSERT(priv->parentwin, "no parent here... bad.");
                XGetGeometry(priv->disp, priv->parentwin, &dummywin,
			     &dummy, &dummy, &w, &h, &udummy, &udummy);
	}
	else if (priv->win == None) { /* Not a root window */
		/* Don't create a window who's handles/borders are offscreen */
		w = screenw * 9 / 10; 
		w = (w+3) & ~3;		/* make likely virt=visible */
		h = screenh * 9 / 10;
	}

	LIBGGI_APPASSERT(w && h, "Bad max w/h.");

	if (tm->visible.x == GGI_AUTO) {
		suggest->visible.x = (tm->virt.x == GGI_AUTO) ? w : tm->virt.x;
		if (suggest->visible.x > w) suggest->visible.x = w;
	}
	if (tm->visible.y == GGI_AUTO) {
		suggest->visible.y = (tm->virt.y == GGI_AUTO) ? h : tm->virt.y;
		if (suggest->visible.x > w) suggest->visible.x = w;
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

#define SCREENDPIX \
((screenwmm <= 0) ?  0 : (screenw * 254 / screenwmm / 10))
#define SCREENDPIY \
((screenhmm <= 0) ?  0 : (screenh * 254 / screenhmm / 10))

        res = _ggi_figure_physz(suggest, priv->physzflags, &(priv->physz),
                                SCREENDPIX, SCREENDPIY, screenw, screenh);
	return(res);
}




/* return 1 if one ggi_graphtype is "better" than another. */
int _ggi_x_is_better_gt(ggi_graphtype than, ggi_graphtype this) {

	/* prefer color to grayscale */
	if ((GT_SCHEME(than)==GT_GREYSCALE)&&(GT_SCHEME(this)!=GT_GREYSCALE))
		return 1;

	/* prefer more colors or levels */
	if (GT_DEPTH(than) < GT_DEPTH(than)) return 1;

	/* prefer palette to true/fixed color (of same depth.) */
	if ((GT_SCHEME(than)==GT_STATIC_PALETTE) &&
	    (GT_SCHEME(this)==GT_PALETTE))
		return 1;
	if ((GT_SCHEME(than)==GT_TRUECOLOR)&&(GT_SCHEME(this)==GT_PALETTE))
		return 1;

	return 0;
}

void _ggi_x_build_pixfmt(ggi_visual *vis, ggi_mode *tm, XVisualInfo *vi) {

	ggi_pixelformat *fmt;

	fmt = LIBGGI_PIXFMT(vis);

	memset(fmt, 0, sizeof(ggi_pixelformat));

	fmt->red_mask   = vi->red_mask;
	fmt->green_mask = vi->green_mask;
	fmt->blue_mask  = vi->blue_mask;

	fmt->depth = GT_DEPTH(tm->graphtype);
	fmt->size  = GT_SIZE(tm->graphtype);

	if (vi->class == StaticColor || vi->class == PseudoColor ||
	    vi->class == StaticGray || vi->class == GrayScale) {
		fmt->clut_mask = (1 << vi->depth) - 1;
	} else {
		fmt->clut_mask = 0;
	}

	_ggi_build_pixfmt(fmt);
}

/* Make a newly created parent window presentable */
void _ggi_x_dress_parentwin(ggi_visual *vis, ggi_mode *tm) {
	XSizeHints      hint;
	ggi_x_priv      *priv;
	char *name = "GGI-on-X";
	
	priv = GGIX_PRIV(vis);
	
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
	XSetStandardProperties(priv->disp, priv->parentwin, name, name,
			       None, NULL, 0, &hint);
}

void _ggi_x_set_xclip (Display *disp, GC gc, int x, int y, int w, int h) {
	XRectangle xrect;

	xrect.x = x; xrect.width = w;
	xrect.y = y; xrect.height = h;
	XSetClipRectangles(disp, gc, 0, 0, &xrect, 1, Unsorted);
}

void _ggi_x_create_dot_cursor (ggi_visual *vis) {
	ggi_x_priv *priv;
	Pixmap crsrpix, crsrmask;
	XColor black = { 0, 0x0, 0x0, 0x0 };
	XColor white = { 0, 0xffff, 0xffff, 0xffff };
	char crspdat[] = { 0xf8, 0xfa, 0xf8 };
	char crsmdat[] = { 0xfa, 0xff, 0xfa };
	unsigned int dummy;
	Window root;
	XSetWindowAttributes wa;

	priv = GGIX_PRIV(vis);

	if (priv->cursor) XFreeCursor(priv->disp, priv->cursor);

	XGetGeometry(priv->disp, priv->parentwin, &root, &dummy, &dummy,
                     (int *)&dummy, (int *)&dummy, &dummy, &dummy);
            
	crsrpix = XCreateBitmapFromData(priv->disp, root, crspdat, 3, 3);
	crsrmask = XCreateBitmapFromData(priv->disp, root, crsmdat, 3, 3);
	priv->cursor = XCreatePixmapCursor(priv->disp, crsrpix, crsrmask,
					   &black, &white, 1, 1);
	wa.cursor = priv->cursor;
	XChangeWindowAttributes(priv->disp, priv->parentwin, CWCursor, &wa); 
        XFreePixmap(priv->disp, crsrpix);
        XFreePixmap(priv->disp, crsrmask);
}


void _ggi_x_create_invisible_cursor (ggi_visual *vis) {
	ggi_x_priv *priv;
	Pixmap crsrpix, crsrmask;
	XColor black = { 0, 0x0, 0x0, 0x0 };
	XColor white = { 0, 0xffff, 0xffff, 0xffff };
	char crspdat[] = { 0 };
	char crsmdat[] = { 0 };
	unsigned int dummy;
	Window root;
	XSetWindowAttributes wa;

	priv = GGIX_PRIV(vis);

	if (priv->cursor) XFreeCursor(priv->disp, priv->cursor);

	XGetGeometry(priv->disp, priv->parentwin, &root, &dummy, &dummy,
                     (int *)&dummy, (int *)&dummy, &dummy, &dummy);
            
	crsrpix = XCreateBitmapFromData(priv->disp, root, crspdat, 1, 1);
	crsrmask = XCreateBitmapFromData(priv->disp, root, crsmdat, 1, 1);
	priv->cursor = XCreatePixmapCursor(priv->disp, crsrpix, crsrmask,
					   &black, &white, 1, 1);
	wa.cursor = priv->cursor;
	XChangeWindowAttributes(priv->disp, priv->parentwin, CWCursor, &wa); 
        XFreePixmap(priv->disp, crsrpix);
        XFreePixmap(priv->disp, crsrmask);
}

void _ggi_x_readback_fontdata (ggi_visual *vis) {
	ggi_x_priv *priv;
	Pixmap fontpix;
	char str[256];
	int i, w, h;

	priv = GGIX_PRIV(vis);

	w = priv->textfont->max_bounds.width;
        h = priv->textfont->max_bounds.ascent
	  + priv->textfont->max_bounds.descent;

	if (priv->fontimg) XDestroyImage(priv->fontimg);

	fontpix = XCreatePixmap(priv->disp, priv->drawable, 
				w * 256, h, 
				priv->vilist[priv->viidx].vi->depth);
	XSetForeground(priv->disp, priv->tempgc, 0);
	XFillRectangle(priv->disp, fontpix, priv->tempgc, 
		       0, 0, w * 256, h);
	XSetForeground(priv->disp, priv->tempgc, ~0);
	for (i = 0; i < 256; i++) str[i] = i;
	XDrawString(priv->disp, fontpix, priv->tempgc, 
		    0, priv->textfont->max_bounds.ascent, 
		    str, 256);
	XSync(priv->disp, 0);
	priv->fontimg = XGetImage(priv->disp, fontpix, 0, 0, 
				  w * 256, h, AllPlanes, ZPixmap);

	/* Reverse endianness if needed. */
	if (priv->fontimg->byte_order == 
#ifdef GGI_LITTLE_ENDIAN
	    LSBFirst
#else
	    MSBFirst
#endif
	    ) goto noswab;
	
	if (priv->fontimg->bits_per_pixel == 16) {
		uint8 *ximgptr;
		ximgptr = priv->fontimg->data + 
		  (priv->fontimg->xoffset * priv->fontimg->bits_per_pixel)/8;
		while (h--) {
			int j;
			uint8 tmp;
			for (j = 0; j < w * 512; j += 2) {
				tmp = *(ximgptr + j);
				*(ximgptr + j) = *(ximgptr + j + 1);
				*(ximgptr + j + 1) = tmp;
			}
			ximgptr += priv->fontimg->bytes_per_line;
		}
	}
	else if (priv->fontimg->bits_per_pixel == 32) {
		uint8 *ximgptr;
		ximgptr = priv->fontimg->data + 
		  (priv->fontimg->xoffset * priv->fontimg->bits_per_pixel)/8;
		while (h--) {
			int j;
			uint8 tmp;
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

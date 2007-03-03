/* $Id: stubs.c,v 1.20 2007/03/03 18:19:11 soyt Exp $
******************************************************************************

   Function call stubs.

   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
  
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

#include "config.h"
#include <ggi/internal/internal.h>
#include <ggi/internal/ggi_debug.h>

#undef putc

/************** Mode info ****************/

int ggiSetFlags(ggi_visual_t v,ggi_flags flags)
{  
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiSetFlags(vis, flags);
}
ggi_flags ggiGetFlags(ggi_visual_t v)
{  
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return LIBGGI_FLAGS(vis);
}

const ggi_pixelformat *ggiGetPixelFormat(ggi_visual_t v)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return LIBGGI_PIXFMT(vis);
}

/*************** Lib management ***********************/

ggi_lib_id ggiExtensionLoadDL(ggi_visual_t v, const void *conffilehandle,
			      const char *api,
			      const char *args, void *argptr,
			      const char *symprefix)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return (ggi_lib_id)_ggiAddExtDL(vis, conffilehandle,
					api, args, argptr,
					symprefix);
}

int ggiGetAPI(ggi_visual_t v, int num, char *apiname, char *arguments)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return vis->opdisplay->getapi(vis, num, apiname, arguments);
}

/*************** Display management *******************/

/* Flush pending operations to the display device.
 */

int ggiFlush(ggi_visual_t v)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiFlush(vis);
}

int ggiFlushRegion(ggi_visual_t v, int x, int y, int w, int h)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	/* Do sanity check here so targets can assume correct values */
	if (x < 0) x = 0;
	else if (x > LIBGGI_VIRTX(vis)) return GGI_EARGINVAL;
	if (y < 0) y = 0;
	else if (y > LIBGGI_VIRTY(vis)) return GGI_EARGINVAL;
	if (w < 0 || h < 0) return GGI_EARGINVAL;
	if (x + w > LIBGGI_VIRTX(vis)) w = LIBGGI_VIRTX(vis) - x;
	if (y + h > LIBGGI_VIRTY(vis)) h = LIBGGI_VIRTY(vis) - y;

	return _ggiFlushRegion(vis, x, y, w, h);
}

/* Internal flush function */
int _ggiInternFlush(struct ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	return vis->opdisplay->flush(vis, x, y, w, h, tryflag);
}

/* Idle the accelerator */
int _ggiIdleAccel(struct ggi_visual *vis)
{
	return vis->opdisplay->idleaccel(vis);
}

/* Directly call a KGI command. DONT DO THAT IN NORMAL PROGRAMS !
 */
int _ggiSendKGICommand(struct ggi_visual *vis,int cmd,void *arg)
{ return vis->opdisplay->kgicommand(vis,cmd,arg); }

/* Palette, colors, etc...
 */
int ggiSetPalette(ggi_visual_t v,int s,int len,const ggi_color *cmap)
{ 
	struct ggi_visual *vis = GGI_VISUAL(v);
	APP_ASSERT(cmap != NULL, "ggiSetPalette() called with NULL colormap.");
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	if (cmap == NULL) return GGI_EARGINVAL;
	return _ggiSetPalette(vis, s, len, cmap);
}

int ggiGetPalette(ggi_visual_t v,int s,int len,ggi_color *cmap)
{ 
	struct ggi_visual *vis = GGI_VISUAL(v);
	APP_ASSERT(!( (cmap == NULL) && (len > 0)),
		"ggiGetPalette() called with NULL colormap when len>0.");
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	if ( (cmap == NULL) && (len > 0)) return GGI_EARGREQ;
	return _ggiGetPalette(vis, s, len, cmap);
}

ggi_pixel ggiMapColor(ggi_visual_t v, const ggi_color *col)
{ 
	struct ggi_visual *vis = GGI_VISUAL(v);
	APP_ASSERT(col != NULL, "ggiMapColor() called with NULL color.");
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	if (col == NULL) return GGI_EARGINVAL;
	return _ggiMapColor(vis, col);
}

int ggiUnmapPixel(ggi_visual_t v,ggi_pixel pixel,ggi_color *col)
{ 
	struct ggi_visual *vis = GGI_VISUAL(v);
	APP_ASSERT(col != NULL, "ggiUnmapPixel() called with NULL color.");
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	if (col == NULL) return GGI_EARGINVAL;
	return _ggiUnmapPixel(vis, pixel, col);
}

int ggiPackColors(ggi_visual_t v,void *buf,const ggi_color *cols,int len)
{ 
	struct ggi_visual *vis = GGI_VISUAL(v);
	APP_ASSERT(!( ( (cols == NULL) || (buf == NULL) ) && (len > 0) ),
		"ggiUnpackPixels() called with NULL pixel-buffer or color-buffer when len>0.");
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	if ( ( (cols == NULL) || (buf == NULL) ) && (len > 0) ) return GGI_EARGREQ;
	return _ggiPackColors(vis, buf, cols, len);
}

int ggiUnpackPixels(ggi_visual_t v,const void *buf,ggi_color *cols,int len)
{ 
	struct ggi_visual *vis = GGI_VISUAL(v);
	APP_ASSERT(!( ( (cols == NULL) || (buf == NULL) ) && (len > 0) ),
		"ggiUnpackPixels() called with NULL pixel-buffer or color-buffer when len>0.");
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	if ( ( (cols == NULL) || (buf == NULL) ) && (len > 0) ) return GGI_EARGREQ;
	return _ggiUnpackPixels(vis, buf, cols, len);
}


int ggiSetColorfulPalette(ggi_visual_t v)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	int err;
	int numcols = 1 << GT_DEPTH(LIBGGI_GT(vis));
	ggi_color *pal;

	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	if (GT_SCHEME(LIBGGI_GT(vis)) != GT_PALETTE) return GGI_EARGINVAL;

	pal = malloc(sizeof(ggi_color) * numcols);
	if (pal == NULL) {
		return GGI_ENOMEM;
	}

	_ggi_build_palette(pal, numcols);

	err = _ggiSetPalette(vis, GGI_PALETTE_DONTCARE, numcols, pal);

	free(pal);

	return err;
}


/* Gamma mapping ...
 */

int  ggiGetGamma(ggi_visual_t v,double *r,double *g,double *b)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiGetGamma(vis, r,g,b);
}

int  ggiSetGamma(ggi_visual_t v,double  r,double  g,double  b)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiSetGamma(vis, r,g,b);
}

int  ggiGetGammaMap(ggi_visual_t v,int s,int len,ggi_color *gammamap)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiGetGammaMap(vis, s, len, gammamap);
}

int  ggiSetGammaMap(ggi_visual_t v,int s,int len,const ggi_color *gammamap)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiSetGammaMap(vis, s, len, gammamap);
}

int ggiGammaMax(ggi_visual_t v, uint32_t bitmeaning, int *max_r, int *max_w)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	if (!vis->gamma) return GGI_EARGINVAL;
	switch(bitmeaning) {
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_RED:
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_Y0:
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_Y1:
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_Y2:
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_Y3:
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_Y4:
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_Y5:
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_Y6:
		if (max_w) *max_w = vis->gamma->maxwrite_r;
		if (max_r) *max_r = vis->gamma->maxread_r;
		break;
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_GREEN:
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_U0:
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_U1:
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_U2:
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_U3:
		if (max_w) *max_w = vis->gamma->maxwrite_g;
		if (max_r) *max_r = vis->gamma->maxread_g;
		break;
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_BLUE:
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_V0:
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_V1:
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_V2:
	case GGI_BM_TYPE_COLOR | GGI_BM_SUB_V3:
		if (max_w) *max_w = vis->gamma->maxwrite_b;
		if (max_r) *max_r = vis->gamma->maxread_b;
		break;
	}
	if (vis->gamma->maxwrite_r | vis->gamma->maxwrite_g | 
	    vis->gamma->maxwrite_b) return GGI_OK;
	else return GGI_ENOMATCH; /* Read-only gammamap */
}

/* Origin
 */
int ggiSetOrigin(ggi_visual_t v,int x,int y)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiSetOrigin(vis, x,y);
}

int ggiGetOrigin(ggi_visual_t v,int *x, int *y)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	*x = vis->origin_x; *y = vis->origin_y;
	return 0;
}


/* Generic drawing routines 
 */
int ggiPutc(ggi_visual_t v,int x,int y,char c)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiPutc(vis, x,y, c);
}

int ggiPuts(ggi_visual_t v,int x,int y,const char *str)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiPuts(vis, x,y, str);
}

int ggiGetCharSize(ggi_visual_t v, int *width, int *height)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiGetCharSize(vis, width, height);
}

int ggiFillscreen(ggi_visual_t v)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiFillscreen(vis);
}


int ggiDrawPixel(ggi_visual_t v,int x,int y)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiDrawPixel(vis, x,y);
}

int ggiPutPixel(ggi_visual_t v,int x,int y,ggi_pixel col)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiPutPixel(vis, x,y, col);
}

int ggiGetPixel(ggi_visual_t v,int x,int y,ggi_pixel *col)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiGetPixel(vis, x,y, col);
}



int ggiDrawLine(ggi_visual_t v,int x,int y,int xe,int ye)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiDrawLine(vis, x,y, xe,ye);
}

int ggiDrawHLine(ggi_visual_t v,int x,int y,int w)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiDrawHLine(vis, x,y, w);
}

int ggiPutHLine(ggi_visual_t v,int x,int y,int w,const void *buf)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiPutHLine(vis, x,y, w, buf);
}

int ggiGetHLine(ggi_visual_t v,int x,int y,int w,void *buf)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiGetHLine(vis, x,y, w, buf);
}

int ggiDrawVLine(ggi_visual_t v,int x,int y,int h)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiDrawVLine(vis, x,y, h);
}

int ggiPutVLine(ggi_visual_t v,int x,int y,int h,const void *buf)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiPutVLine(vis, x,y, h, buf);
}

int ggiGetVLine(ggi_visual_t v,int x,int y,int h,void *buf)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiGetVLine(vis, x,y, h, buf);
}

int ggiDrawBox(ggi_visual_t v,int x,int y,int w,int h)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiDrawBox(vis, x,y, w,h);
}

int ggiPutBox(ggi_visual_t v,int x,int y,int w,int h,const void *buf)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiPutBox(vis, x,y, w,h, buf);
}

int ggiGetBox(ggi_visual_t v,int x,int y,int w,int h,void *buf)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiGetBox(vis, x,y, w,h, buf);
}


int ggiCopyBox(ggi_visual_t v,int x,int y,int w,int h,int nx,int ny)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiCopyBox(vis, x,y, w,h, nx,ny);
}

int ggiCrossBlit(ggi_visual_t s,int sx,int sy,int w,int h,
		 ggi_visual_t d,int dx,int dy)
{
	struct ggi_visual *src = GGI_VISUAL(s);
	struct ggi_visual *dst = GGI_VISUAL(d);

	LIB_ASSERT(src != NULL, "broken/invalid visual\n");
	LIB_ASSERT(dst != NULL, "broken/invalid visual\n");

	if (src == dst) {
		return _ggiCopyBox(dst, sx, sy, w, h, dx, dy);
	}
	/* Note: We use dst to map the request to, as it will normally be
	   the "smarter" device. */
	return _ggiCrossBlit(src, sx, sy, w, h, dst, dx, dy);
}


/* Frames
 */

int ggiSetDisplayFrame(ggi_visual_t v, int frameno)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiSetDisplayFrame(vis, frameno);
}

int ggiSetReadFrame(ggi_visual_t v, int frameno)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiSetReadFrame(vis, frameno);
}

int ggiSetWriteFrame(ggi_visual_t v, int frameno)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiSetWriteFrame(vis, frameno);
}

int ggiGetDisplayFrame(ggi_visual_t v)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiGetDisplayFrame(vis);
}

int ggiGetReadFrame(ggi_visual_t v)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiGetReadFrame(vis);
}

int ggiGetWriteFrame(ggi_visual_t v)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIB_ASSERT(vis != NULL, "broken/invalid visual\n");
	return _ggiGetWriteFrame(vis);
}


/* Resource management
 */

int ggiResourceFastAcquire(ggi_resource_t res, uint32_t actype)
{
	return res->acquire(res, actype);
}

int ggiResourceFastRelease(ggi_resource_t res)
{
	return res->release(res);
}

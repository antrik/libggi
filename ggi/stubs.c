/* $Id: stubs.c,v 1.16 2005/09/19 07:45:15 cegger Exp $
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

int ggiSetFlags(ggi_visual *vis,ggi_flags flags)
{  
	return vis->opdisplay->setflags(vis,flags);
}
ggi_flags ggiGetFlags(ggi_visual *vis)
{  
	return LIBGGI_FLAGS(vis);
}

const ggi_pixelformat *ggiGetPixelFormat(ggi_visual *vis)
{
	return LIBGGI_PIXFMT(vis);
}

/*************** Lib management ***********************/

ggi_lib_id ggiExtensionLoadDL(ggi_visual_t vis, const void *conffilehandle,
			      const char *api,
			      const char *args, void *argptr,
			      const char *symprefix)
{
	return (ggi_lib_id)_ggiAddExtDL(vis, conffilehandle,
					api, args, argptr,
					symprefix);
}

int ggiGetAPI(ggi_visual_t vis, int num, char *apiname, char *arguments)
{
	return vis->opdisplay->getapi(vis, num, apiname, arguments);
}

/*************** Display management *******************/

/* Flush pending operations to the display device.
 */

int ggiFlush(ggi_visual *vis)
{
        return vis->opdisplay->flush(vis, 0, 0,
				     LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis), 1);
}

int ggiFlushRegion(ggi_visual *vis, int x, int y, int w, int h)
{
	/* Do sanity check here so targets can assume correct values */
	if (x < 0) x = 0;
	else if (x > LIBGGI_VIRTX(vis)) return GGI_EARGINVAL;
	if (y < 0) y = 0;
	else if (y > LIBGGI_VIRTY(vis)) return GGI_EARGINVAL;
	if (w < 0 || h < 0) return GGI_EARGINVAL;
	if (x + w > LIBGGI_VIRTX(vis)) w = LIBGGI_VIRTX(vis) - x;
	if (y + h > LIBGGI_VIRTY(vis)) h = LIBGGI_VIRTY(vis) - y;

	return vis->opdisplay->flush(vis, x, y, w, h, 1);
}

/* Internal flush function */
int _ggiInternFlush(ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	return vis->opdisplay->flush(vis, x, y, w, h, tryflag);
}

/* Idle the accelerator */
int _ggiIdleAccel(ggi_visual *vis)
{
	return vis->opdisplay->idleaccel(vis);
}

/* Directly call a KGI command. DONT DO THAT IN NORMAL PROGRAMS !
 */
int _ggiSendKGICommand(ggi_visual *vis,int cmd,void *arg)
{ return vis->opdisplay->kgicommand(vis,cmd,arg); }

/* Palette, colors, etc...
 */
int ggiSetPalette(ggi_visual *vis,int s,int len,const ggi_color *cmap)
{ 
	APP_ASSERT(cmap != NULL, "ggiSetPalette() called with NULL colormap.");
	if (cmap == NULL) return GGI_EARGINVAL;
	return vis->opcolor->setpalvec(vis,s,len,cmap); 
}

int ggiGetPalette(ggi_visual *vis,int s,int len,ggi_color *cmap)
{ 
	APP_ASSERT(!( (cmap == NULL) && (len > 0)),
		"ggiGetPalette() called with NULL colormap when len>0.");
	if ( (cmap == NULL) && (len > 0)) return GGI_EARGREQ;
	return vis->opcolor->getpalvec(vis,s,len,cmap); 
}

ggi_pixel ggiMapColor(ggi_visual *vis, const ggi_color *col)
{ 
	APP_ASSERT(col != NULL, "ggiMapColor() called with NULL color.");
	if (col == NULL) return GGI_EARGINVAL;
	return vis->opcolor->mapcolor(vis,col);
}

int ggiUnmapPixel(ggi_visual *vis,ggi_pixel pixel,ggi_color *col)
{ 
	APP_ASSERT(col != NULL, "ggiUnmapPixel() called with NULL color.");
	if (col == NULL) return GGI_EARGINVAL;
	return vis->opcolor->unmappixel(vis,pixel,col);
}

int ggiPackColors(ggi_visual *vis,void *buf,const ggi_color *cols,int len)
{ 
	APP_ASSERT(!( ( (cols == NULL) || (buf == NULL) ) && (len > 0) ),
		"ggiUnpackPixels() called with NULL pixel-buffer or color-buffer when len>0.");
	if ( ( (cols == NULL) || (buf == NULL) ) && (len > 0) ) return GGI_EARGREQ;
	return vis->opcolor->packcolors(vis,buf,cols,len);
}

int ggiUnpackPixels(ggi_visual *vis,const void *buf,ggi_color *cols,int len)
{ 
	APP_ASSERT(!( ( (cols == NULL) || (buf == NULL) ) && (len > 0) ),
		"ggiUnpackPixels() called with NULL pixel-buffer or color-buffer when len>0.");
	if ( ( (cols == NULL) || (buf == NULL) ) && (len > 0) ) return GGI_EARGREQ;
	return vis->opcolor->unpackpixels(vis,buf,cols,len);
}


int ggiSetColorfulPalette(ggi_visual *vis)
{
	int err;
	int numcols = 1 << GT_DEPTH(LIBGGI_GT(vis));
	ggi_color *pal;

	if (GT_SCHEME(LIBGGI_GT(vis)) != GT_PALETTE) return GGI_EARGINVAL;

	pal = malloc(sizeof(ggi_color) * numcols);
	if (pal == NULL) {
		return GGI_ENOMEM;
	}

	_ggi_build_palette(pal, numcols);

	err = ggiSetPalette(vis, GGI_PALETTE_DONTCARE, numcols, pal);

	free(pal);

	return err;
}


/* Gamma mapping ...
 */

int  ggiGetGamma(ggi_visual_t vis,ggi_float *r,ggi_float *g,ggi_float *b)
{ return vis->opcolor->getgamma(vis,r,g,b); }

int  ggiSetGamma(ggi_visual_t vis,ggi_float  r,ggi_float  g,ggi_float  b)
{ return vis->opcolor->setgamma(vis,r,g,b); }

int  ggiGetGammaMap(ggi_visual_t vis,int s,int len,ggi_color *gammamap)
{ return vis->opcolor->getgammamap(vis,s,len,gammamap); }

int  ggiSetGammaMap(ggi_visual_t vis,int s,int len,const ggi_color *gammamap)
{ return vis->opcolor->setgammamap(vis,s,len,gammamap); }

int ggiGammaMax(ggi_visual_t vis, uint32_t bitmeaning, int *max_r, int *max_w)
{
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
int ggiSetOrigin(ggi_visual *vis,int x,int y)
{ return vis->opdraw->setorigin(vis,x,y); }

int ggiGetOrigin(ggi_visual *vis,int *x, int *y)
{ *x = vis->origin_x; *y = vis->origin_y; return 0; }


/* Generic drawing routines 
 */
int ggiPutc(ggi_visual *vis,int x,int y,char c)
{ return vis->opdraw->putc(vis,x,y,c); }

int ggiPuts(ggi_visual *vis,int x,int y,const char *str)
{ return vis->opdraw->puts(vis,x,y,str); }

int ggiGetCharSize(ggi_visual *vis, int *width, int *height)
{ return vis->opdraw->getcharsize(vis, width, height); }


int ggiFillscreen(ggi_visual *vis)
{ return vis->opdraw->fillscreen(vis); }


int _ggiDrawPixelNC(ggi_visual *vis,int x,int y)
{ return vis->opdraw->drawpixel_nc(vis,x,y); }

int ggiDrawPixel(ggi_visual *vis,int x,int y)
{ return vis->opdraw->drawpixel(vis,x,y); }

int _ggiPutPixelNC(ggi_visual *vis,int x,int y,ggi_pixel col)
{ return vis->opdraw->putpixel_nc(vis,x,y,col); }

int ggiPutPixel(ggi_visual *vis,int x,int y,ggi_pixel col)
{ return vis->opdraw->putpixel(vis,x,y,col); }

int ggiGetPixel(ggi_visual *vis,int x,int y,ggi_pixel *col)
{ return vis->opdraw->getpixel(vis,x,y,col); }



int ggiDrawLine(ggi_visual *vis,int x,int y,int xe,int ye)
{ return vis->opdraw->drawline(vis,x,y,xe,ye); }

int _ggiDrawHLineNC(ggi_visual *vis,int x,int y,int w)
{ return vis->opdraw->drawhline_nc(vis,x,y,w); }

int ggiDrawHLine(ggi_visual *vis,int x,int y,int w)
{ return vis->opdraw->drawhline(vis,x,y,w); }

int ggiPutHLine(ggi_visual *vis,int x,int y,int w,const void *buf)
{ return vis->opdraw->puthline(vis,x,y,w,buf); }

int ggiGetHLine(ggi_visual *vis,int x,int y,int w,void *buf)
{ return vis->opdraw->gethline(vis,x,y,w,buf); }


int _ggiDrawVLineNC(ggi_visual *vis,int x,int y,int h)
{ return vis->opdraw->drawvline_nc(vis,x,y,h); }

int ggiDrawVLine(ggi_visual *vis,int x,int y,int h)
{ return vis->opdraw->drawvline(vis,x,y,h); }

int ggiPutVLine(ggi_visual *vis,int x,int y,int h,const void *buf)
{ return vis->opdraw->putvline(vis,x,y,h,buf); }

int ggiGetVLine(ggi_visual *vis,int x,int y,int h,void *buf)
{ return vis->opdraw->getvline(vis,x,y,h,buf); }


int ggiDrawBox(ggi_visual *vis,int x,int y,int w,int h)
{ return vis->opdraw->drawbox(vis,x,y,w,h); }

int ggiPutBox(ggi_visual *vis,int x,int y,int w,int h,const void *buf)
{ return vis->opdraw->putbox(vis,x,y,w,h,buf); }

int ggiGetBox(ggi_visual *vis,int x,int y,int w,int h,void *buf)
{ return vis->opdraw->getbox(vis,x,y,w,h,buf); }


int ggiCopyBox(ggi_visual *vis,int x,int y,int w,int h,int nx,int ny)
{ return vis->opdraw->copybox(vis,x,y,w,h,nx,ny); }

int ggiCrossBlit(ggi_visual *src,int sx,int sy,int w,int h,
		 ggi_visual *dst,int dx,int dy)
{
	if (src == dst) {
		return dst->opdraw->copybox(dst, sx, sy, w, h, dx, dy);
	}
	/* Note: We use dst to map the request to, as it will normally be
	   the "smarter" device. */
	return dst->opdraw->crossblit(src, sx, sy, w, h, dst, dx, dy);
}


/* Frames
 */

int ggiSetDisplayFrame(ggi_visual_t vis, int frameno)
{ return vis->opdraw->setdisplayframe(vis, frameno); }

int ggiSetReadFrame(ggi_visual_t vis, int frameno)
{ return vis->opdraw->setreadframe(vis, frameno); }

int ggiSetWriteFrame(ggi_visual_t vis, int frameno)
{ return vis->opdraw->setwriteframe(vis, frameno); }

int ggiGetDisplayFrame(ggi_visual_t vis)
{ return vis->d_frame_num; }

int ggiGetReadFrame(ggi_visual_t vis)
{ return vis->r_frame_num; }

int ggiGetWriteFrame(ggi_visual_t vis)
{ return vis->w_frame_num; }


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

/* $Id: dga.c,v 1.12 2005/02/10 18:06:18 cegger Exp $
******************************************************************************

   XFree86-DGA extension support for display-x

   Copyright (C) 1997-1998 Steve Cheng		[steve@ggi-project.org]
   Copyright (C) 1999-2000 Marcus Sundberg	[marcus@ggi-project.org]
   Copyright (C) 2002      Brian S. Julin	[bri@tull.umassp.edu]
   Copyright (C) 2005      Joseph Crayne	[oh.hello.joe@gmail.com]

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
#include <ggi/display/x.h>
#include <ggi/internal/ggi_debug.h>
#include <X11/extensions/xf86dga.h>
#include <string.h>

static int ggi_xdga_getmodelist(ggi_visual *vis) {
	ggi_x_priv *priv;
	XDGAMode *modes;
	int screen;

	priv = GGIX_PRIV(vis);

	screen = priv->vilist[priv->viidx].vi->screen;
	priv->modes_num = 0;
	priv->modes_priv = modes = 
		XDGAQueryModes(priv->disp, screen, &(priv->modes_num));
	if (priv->modes_priv == NULL) return GGI_ENODEVICE;
	if (priv->modes_num <= 0) return GGI_ENODEVICE;

	return GGI_OK;
}

#if 0
	
        priv->modes = calloc(sizeof(ggi_modelistmode), priv->modes_num);
        if (priv->modes == NULL) {
		XFree(priv->modes_priv);
		return GGI_ENOMEM;
        }
        
        for (i = 0; i < priv->modes_num; i++) {

		priv->modes[i].x = modes[i].viewportWidth;
		priv->modes[i].y = modes[i].viewportHeight;
		priv->modes[i].bpp = modes[i].depth;

#define GGI_XDGA_GTCONSTRUCT(ggigt) \
priv->modes[i].gt = GT_CONSTRUCT(modes[i].depth, ggigt, modes[i].bitsPerPixel);

		switch(modes[i].visualClass) {
		case TrueColor:
		case DirectColor: 
			GGI_XDGA_GTCONSTRUCT(GT_TRUECOLOR);   
			break;
		case PseudoColor: 
			GGI_XDGA_GTCONSTRUCT(GT_PALETTE);     
			break;
		case StaticGray:
		case GrayScale:   
			GGI_XDGA_GTCONSTRUCT(GT_GREYSCALE);   
			break;
		case StaticColor: 
			GGI_XDGA_GTCONSTRUCT(GT_STATIC_PALETTE);
			break;
		default:
#warning handle this case
			break;
		}
		DPRINT_MISC("Found mode: %dx%d\n",
			       priv->modes[i].x, priv->modes[i].y);
        }
	return GGI_OK;
}

#endif

static int ggi_xdga_restore_mode(ggi_visual *vis)
{
	ggi_x_priv *priv;
	int screen;

	priv = GGIX_PRIV(vis);
	screen = DefaultScreen(priv->disp);

	if (priv->priv != NULL) XFree(priv->priv);
	priv->priv = XDGASetMode(priv->disp, screen, 0);
	if (priv->priv != NULL) XFree(priv->priv); /* Docs not explicit */

	return GGI_OK;
}

static int ggi_xdga_enter_mode(ggi_visual *vis, int num) {
	XDGADevice *dev;
	XDGAMode *modes;
	ggi_x_priv *priv = GGIX_PRIV(vis);
	int screen = priv->vilist[priv->viidx].vi->screen;
	/* Window child_return; */
	/* Window root; */
	/* int x, y; */


	dev = priv->priv;

	if (dev != NULL) XFree(dev);

	if ((num+1) > priv->modes_num) {
		DPRINT("helper-x-dga: Bug somewhere -- bad mode index.\n");
		return GGI_ENODEVICE;
	}
    
	XMoveWindow(priv->disp, priv->parentwin, 0, 0);

	modes = (XDGAMode *)(priv->modes_priv);
	num = modes[num].num;
	DPRINT_MODE("\tXDGASetMode(%x, %d, %x) %d called.\n", 
		     priv->disp, priv->vilist[priv->viidx].vi->screen, 
		     num, DefaultScreen(priv->disp));

	priv->priv = dev = XDGASetMode(priv->disp, screen, num);
	if (dev == NULL) return GGI_ENODEVICE;
	/*priv->fb = dev->data;*/
	/* For now, only pixmap modes are allowed.. */
	LIB_ASSERT(modes[num].flags & XDGAPixmap, "bad pixmap!!");
	LIB_ASSERT(dev->pixmap, "null pixmap!!");
	priv->drawable = dev->pixmap;
	//priv->win = priv->drawable; /* This is BAD? Yes. */
	DPRINT_MODE("disp,drawable = (%x,%x)...\n",
			priv->disp, priv->drawable);
#if 0
	XClearWindow(priv->disp, priv->drawable);
	ggUSleep(3000000);
	DPRINT_MODE("tested ok.\n");
#endif

#if 0
	/* 
	 * Here we translate window origin to Root window origin 
	 * in order to get the viewport centered on the window.
	 * So no need to use override-redirect nor iCCM.
	 */
	root = DefaultRootWindow(priv->disp);
	
	XTranslateCoordinates(priv->disp, priv->parentwin, root, 
			      0, 0, &x, &y, &child_return);

	XDGASetViewport(priv->disp,
			       priv->vilist[priv->viidx].vi->screen, 
			       x,
			       y,
                   0);

#endif

	DPRINT_MODE("leaving helper-x-dga setmode.\n");
	return GGI_OK;
}

static void ggi_xdga_checkmode_adapt(ggi_mode *m, XDGAMode *dgamode, 
				     ggi_x_priv *priv )
{
	int screen = priv->vilist[priv->viidx].vi->screen;

	m->visible.x = dgamode->viewportWidth;
	m->visible.y = dgamode->viewportHeight;
	m->virt.x = dgamode->imageWidth;
	m->virt.y = dgamode->imageHeight;
	m->dpp.x = 1;
	m->dpp.y = 1;

	/* frames not supported (yet?) */
	m->frames = 1;

	m->graphtype = GT_CONSTRUCT(dgamode->depth,
					 (dgamode->depth <=
					  9) ? GT_PALETTE :
					 GT_TRUECOLOR, dgamode->bitsPerPixel);


	
#define SCREENWMM DisplayWidthMM(priv->disp, screen)
#define SCREENW   DisplayWidth(priv->disp, screen)
#define SCREENHMM DisplayHeightMM(priv->disp, screen)
#define SCREENH   DisplayHeight(priv->disp, screen)
#define SCREENDPIX \
((SCREENWMM <= 0) ?  0 : (SCREENW * m->dpp.x * 254 / SCREENWMM / 10))
#define SCREENDPIY \
((SCREENHMM <= 0) ?  0 : (SCREENH * m->dpp.x * 254 / SCREENHMM / 10))
		m->size.x = GGI_AUTO;
		m->size.y = GGI_AUTO;
		_ggi_physz_figure_size(m, priv->physzflags,
				       &(priv->physz), SCREENDPIX,
				       SCREENDPIY, SCREENW, SCREENH);
#undef SCREENWMM
#undef SCREENW
#undef SCREENHMM
#undef SCREENH
#undef SCREENDPIX
#undef SCREENDPIY

}

#include <ggi/display/modelist.h>
#define WANT_GENERIC_CHECKMODE
#include "../../../common/modelist.inc"


/* This function performs the CheckMode operation and returns
 * the number of the best mode.  */
static int ggi_xdga_validate_mode(ggi_visual *vis, int num, ggi_mode *mode)
{
	ggi_x_priv *priv;
	XDGAMode *dgamodes;
	ggi_checkmode_t *cm;
	int i;
	int err;
	int no_modes = 1; /* true we haven't found a mode */

	priv = GGIX_PRIV(vis);

	dgamodes = (XDGAMode *)(priv->modes_priv);


	if ( num>=0 ) {
		return (dgamodes[num+1].flags & XDGAPixmap) ? 
			dgamodes[num+1].num : GGI_ENOMATCH;
	}        

    
	/* Find max values for maxed->virt and such. */

	cm = _GGI_generic_checkmode_create();

	/* Initialize best mode search */
	_GGI_generic_checkmode_init( cm, mode );

	for(i=0; i<priv->modes_num; i++ ) {
		/* For now, only support modes that allow xlib rendering...
		 * Things to come: this check is only neccessary if we
		 * have no framebuffer or /dev/mem access. */
		if ( dgamodes[i].flags & XDGAPixmap ) {

			DPRINT("found valid mode number: %i\n", i );
			/* Turn dgamode structure into a ggimode suggestion */
			ggi_xdga_checkmode_adapt( mode, &dgamodes[i], priv );

			/* At this point, we may in the future want to adjust
			 * mode to better match the requested mode (cm->req)
			 * in order to, for example, support a range of 
			 * virtual resolutions... */

			/* Let the checkmode API decide if its the best so 
			 * far. */
			_GGI_generic_checkmode_update( cm, mode, i );
			no_modes = 0; /* false, we have a mode now */
		}
	}

	err = _GGI_generic_checkmode_finish( cm, mode, &i );
	_GGI_generic_checkmode_destroy(cm);

	if( no_modes ) return GGI_ENOMATCH;

	return i;  
}


#if 0
static int ggi_xdga_mmap (ggi_visual *vis) {
	ggi_x_priv *priv;

	priv = GGIX_PRIV(vis);

	/* This should have been taken care of by enter_mode */

#warning deal with banking here when backporting to older DGA
	return GGI_OK;
}

static int ggi_xdga_makerenderer (ggi_visual *vis) {
	ggi_x_priv *priv;
	XDGADevice *dev;

	priv = GGIX_PRIV(vis);
	dev = priv->priv;

	if (priv->slave != NULL) ggiClose(priv->slave);

#warning is dev->pixmap guaranteed None when pixmap flag not set in DGAmode?
	if (dev->pixmap) {
		priv->drawable = dev->pixmap;
#warning load dga accels here
		return GGI_OK;
	}

#warning create slave vis here.
	return GGI_OK;
}

#endif

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	ggi_x_priv *priv;
	int dgafeat, i, j;

	priv = GGIX_PRIV(vis);

	XF86DGAQueryVersion(priv->disp, &i, &j);
	DPRINT("display-DGA version %d.%d\n", i, j);
	if (i < 1) {
		fprintf(stderr, "Your XF86DGA is too old (%d.%d).\n", i, j);
		return GGI_ENODEVICE;
	}
	
	XF86DGAQueryDirectVideo(priv->disp,DefaultScreen(priv->disp),&dgafeat);
	if (!(dgafeat & XF86DGADirectPresent)) {
		fprintf(stderr, "helper-x-dga: No direct video capability!\n");
		return GGI_ENODEVICE;
	}

#if 0
	priv->createfb = ggi_xdga_mmap;
	priv->createdrawable = ggi_xdga_makerenderer;
#endif

	ggi_xdga_getmodelist(vis);

	/* provide mode handling */
	priv->mlfuncs.validate = ggi_xdga_validate_mode;
	priv->mlfuncs.enter = ggi_xdga_enter_mode;
	priv->mlfuncs.getlist = ggi_xdga_getmodelist;
	priv->mlfuncs.restore = ggi_xdga_restore_mode;

	/* TODO: if we can open the frame buffer or /dev/mem,
	 * then we should override x drawing primitives.. */

	*dlret = 0;
	return GGI_OK;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	ggi_x_priv *priv = GGIX_PRIV(vis);
    
	if( priv->modes_num > 0 )
		XFree( priv->modes_priv );

	return GGI_OK;
}


EXPORTFUNC
int GGIdl_helper_x_dga(int func, void **funcptr);

int GGIdl_helper_x_dga(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = (void *)GGIopen;
		return GGI_OK;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return GGI_OK;
	case GGIFUNC_close:
		*funcptr = (void *)GGIclose;
		return GGI_OK;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>

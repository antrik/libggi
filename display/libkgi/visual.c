/* $Id: visual.c,v 1.13 2006/01/30 21:44:20 cegger Exp $
******************************************************************************

   Display-libkgi: visual handling

   Copyright(C) 2001 by Brian S. Julin [bri@tull.umassp.edu]

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

#include <ggi/display/libkgi.h>

static int refcount = 0;
static void *_ggi_libkgi_lock = NULL;
static const gg_option optlist[] =
{
        { "physz",   "0,0" }
};
#define OPT_PHYSZ       0
#define NUM_OPTS        (sizeof(optlist)/sizeof(gg_option))

int GGI_libkgi_flush(ggi_visual *vis, int x, int y, int w, int h, 
			    int tryflag)
{
        ggi_libkgi_priv *priv = LIBKGI_PRIV(vis);
 
  fprintf(stderr, "GGI_libkgi_flush\n");
       
        if (priv->flush) return priv->flush(vis, x, y, w, h, tryflag);

        return 0;
}


/* kgicommand obselete */


int GGI_libkgi_getapi(ggi_visual *vis, int num, char *apiname, char *arguments)
{
	*arguments = '\0';

        switch(num) {
        case 0:
                sprintf(apiname, "display-libkgi");
  fprintf(stderr, "libkgi getapi returned %s\n", apiname);
                return 0;
        case 1:
                sprintf(apiname, "display-libkgi-%s", 
			LIBKGI_PRIV(vis)->suggest);
  fprintf(stderr, "libkgi getapi returned %s\n", apiname);
                return 0;
        case 2:
                strcpy(apiname, "generic-stubs");
  fprintf(stderr, "libkgi getapi returned %s\n", apiname);
                return 0;
        case 3:
                sprintf(apiname, "generic-linear-%d",GT_DEPTH(LIBGGI_GT(vis)));
  fprintf(stderr, "libkgi getapi returned %s\n", apiname);
                return 0;
        case 4: strcpy(apiname, "generic-color");
  fprintf(stderr, "libkgi getapi returned %s\n", apiname);
                return 0;
        }

        return GGI_ENOMATCH;
}

int GGI_libkgi_setflags(ggi_visual *vis, ggi_flags flags)
{
  fprintf(stderr, "GGI_libkgi_setflags\n");
        LIBGGI_FLAGS(vis) = flags;
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */
        return 0;
}

int GGI_libkgi_idleaccel(ggi_visual *vis)
{
        ggi_libkgi_priv *priv = LIBKGI_PRIV(vis);

  fprintf(stderr, "GGI_libkgi_idleaccel\n");

        DPRINT_DRAW("GGI_libkgi_idleaccel(%p) called \n", vis);
        
        if (priv->idleaccel) return priv->idleaccel(vis);

        return 0;
}

int GGI_libkgi_getmode(ggi_visual *vis, ggi_mode *mode)
{
        DPRINT_MODE("display-libkgi: getmode\n");
        
        memcpy(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

        return 0;
}

int GGI_libkgi_checkmode(ggi_visual *vis, ggi_mode *mode)
{
        int err;
	ggiGA_resource_list reqlist;
	struct ggiGA_resource_props rend;
	ggiGA_resource_handle motor_h, carb_h;

        DPRINT_MODE("display-libkgi: checkmode\n");

	/* Make sure LibGAlloc is attached. */
	if (!LIBKGI_PRIV(vis)->galloc_loaded) {
		DPRINT("Attaching LibGAlloc\n");
		err = ggiGAAttach(vis);
		if (err) {
			fprintf(stderr, "Eek, couldn't attach LibGAlloc.\n");
			return err;
		}
		LIBKGI_PRIV(vis)->galloc_loaded = 1;
	}

	/* Since we cannot guarantee otherwise on other targets,
	 * calling ggiSetMode wipes out all ancilary resources.
	 */
	reqlist = NULL;
	err = ggiGAAddMode(vis, &reqlist, mode, NULL, NULL);
	if (err) {
		fprintf(stderr, "ggiGAAddMode failed.");
		return (err);
	}
	ggiGAClearMotorProperties(&rend);
	rend.sub.motor.div_max.x = 1;
	rend.sub.motor.div_max.y = 1;
	rend.sub.motor.mul_max.x = 1;
	rend.sub.motor.mul_max.y = 1;
	rend.sub.motor.div_min.x = 1;
	rend.sub.motor.div_min.y = 1;
	rend.sub.motor.mul_min.x = 1;
	rend.sub.motor.mul_min.y = 1;

	err = ggiGAAdd(&reqlist, &rend, 
		       GA_RT_RENDERER_DRAWOPS | GA_RT_MOTOR, 
		       &motor_h);
	if (err) {
		fprintf(stderr, "ggiGAAdd failed.");
		ggiGAEmptyList(&reqlist);
		return (err);
	}
	ggiGAClearCarbProperties(&rend);
	/* TODO: fill out carb for solid drawops (no Z/Alpha) */

	err = ggiGAAdd(&reqlist, &rend, 
		       GA_RT_RENDERER_DRAWOPS | GA_RT_CARB, 
		       &carb_h);
	if (err) {
		fprintf(stderr, "ggiGAAdd failed.");
		ggiGAEmptyList(&reqlist);
		return (err);
	}

	ggiGATagOnto(reqlist, motor_h, carb_h);

	err = ggiGACheck(vis, reqlist, &reqlist);
	memcpy(mode, ggiGAGetGGIMode(reqlist /* mode is first resource */),
	       sizeof(ggi_mode));
        if (err != 0) return err;

        DPRINT_MODE("display-libkgi: setmode success.\n");

        return 0;
}

int GGI_libkgi_setmode(ggi_visual *vis, ggi_mode *mode)
{
        int err;
	ggiGA_resource_list reqlist;
	struct ggiGA_resource_props rend;
	ggiGA_resource_handle motor_h, carb_h;

        DPRINT_MODE("display-libkgi: setmode\n");

	/* Make sure LibGAlloc is attached. */
	if (!(LIBKGI_PRIV(vis)->galloc_loaded)) {
		DPRINT("Attaching LibGAlloc\n");
		err = ggiGAAttach(vis);
		if (err) {
			fprintf(stderr, "Eek, couldn't attach LibGAlloc.\n");
			return err;
		}
		LIBKGI_PRIV(vis)->galloc_loaded = 1;
	}

	/* Since we cannot guarantee otherwise on other targets,
	 * calling ggiSetMode wipes out all ancilary resources.
	 */
	reqlist = NULL;
	err = ggiGAAddMode(vis, &reqlist, mode, NULL, NULL);
	if (err) {
		fprintf(stderr, "ggiGAAddMode failed.");
		return (err);
	}
	ggiGAClearMotorProperties(&rend);
	rend.sub.motor.div_max.x = 1;
	rend.sub.motor.div_max.y = 1;
	rend.sub.motor.mul_max.x = 1;
	rend.sub.motor.mul_max.y = 1;
	rend.sub.motor.div_min.x = 1;
	rend.sub.motor.div_min.y = 1;
	rend.sub.motor.mul_min.x = 1;
	rend.sub.motor.mul_min.y = 1;

	err = ggiGAAdd(&reqlist, &rend,
		       GA_RT_RENDERER_DRAWOPS | GA_RT_MOTOR, 
		       &motor_h);
	if (err) {
		fprintf(stderr, "ggiGAAdd failed.");
		ggiGAEmptyList(&reqlist);
		return (err);
	}
	ggiGAClearCarbProperties(&rend);
	/* TODO: fill out carb for solid drawops (no Z/Alpha) */

	err = ggiGAAdd(&reqlist, &rend,
		       GA_RT_RENDERER_DRAWOPS | GA_RT_CARB, 
		       &carb_h);
	if (err) {
		fprintf(stderr, "ggiGAAdd failed.");
		ggiGAEmptyList(&reqlist);
		return (err);
	}

	ggiGATagOnto(reqlist, motor_h, carb_h);

	/* 
	 * This will load the batchop to serve the rendering functions.
         * (The batchop sublib will, in turn, load generic renderers 
	 *  if needed.)
	 */
	err = ggiGASet(vis, reqlist, &reqlist);
	memcpy(mode, ggiGAGetGGIMode(reqlist /* mode is first resource */),
	       sizeof(ggi_mode));
        if (err != 0) return err;

        DPRINT_MODE("display-libkgi: setmode success.\n");

        return 0;
}

int GGI_libkgi_sendevent (struct ggi_visual *vis, ggi_event *event)
{
  fprintf(stderr, "GGI_libkgi_sendevent\n");
	return 0;
}

static int do_cleanup(ggi_visual *vis)
{
        ggi_libkgi_priv *priv = LIBKGI_PRIV(vis);

        /* We may be called more than once due to the LibGG cleanup stuff */
        if (priv == NULL) return 0;

        DPRINT("display-libkgi: GGIdlcleanup start.\n");

        if (LIBGGI_FD(vis) >= 0) close(LIBGGI_FD(vis));

        if (vis->input != NULL) {
                giiClose(vis->input);
                vis->input = NULL;
        }

	free(priv);
        LIBKGI_PRIV(vis) = NULL;

        ggUnregisterCleanup((ggcleanup_func *)do_cleanup, vis);

        ggLock(_ggi_global_lock);
        refcount--;
        refcount--;
        if (refcount == 0) {
                ggLockDestroy(_ggi_libkgi_lock);
                _ggi_libkgi_lock = NULL;
        }
        ggUnlock(_ggi_global_lock);

        DPRINT("display-libkgi: GGIdlcleanup done.\n");

        return 0;
}

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
                        const char *args, void *argptr, uint32_t *dlret)
{
        gg_option options[NUM_OPTS];
        ggi_libkgi_priv *priv;
	int err;

	/* We need LibGAlloc to be initialized.  Seems OK to do so
	 * from inside here. It would be nice to Attach it here too,
	 * but I'm less confident that that would work :-)
	 */
	ggiGAInit();

        DPRINT("display-libkgi: GGIdlinit start.\n");

        memcpy(options, optlist, sizeof(options));
        if (args) {
                args = ggParseOptions(args, options, NUM_OPTS);
                if (args == NULL) {
                        fprintf(stderr, "display-libkgi: error in "
                                "arguments.\n");
                        return GGI_EARGINVAL;
                }
        }

        LIBKGI_PRIV(vis) = priv = malloc(sizeof(ggi_libkgi_priv));
        if (priv == NULL) {
                return GGI_ENOMEM;
        }

        priv->have_accel = 0;
        priv->accelpriv = NULL;
        priv->flush = NULL;
        priv->idleaccel = NULL;

	sprintf(priv->suggest, "foodrv");

        DPRINT("display-libkgi: Parsing physz options.\n");
	err = _ggi_physz_parse_option(options[OPT_PHYSZ].result, 
			       &(priv->physzflags), &(priv->physz)); 
	if (err != GGI_OK) {
		do_cleanup(vis);
		return err;
	}

#if 0
	/* Don't know how this will pan out */
	err = kgiInit(&priv->ctx, &priv->client_name, &priv->client_version);
	if (err != KGI_EOK) {
		do_cleanup(vis);
		return err;
	}
	LIBGGI_FD(vis) = priv->ctx.mapper.fd;
#endif

        DPRINT("display-libkgi: Setting up locks.\n");
        ggLock(_ggi_global_lock);
        if (refcount == 0) {
                _ggi_libkgi_lock = ggLockCreate();
                if (_ggi_libkgi_lock == NULL) {
                        ggUnlock(_ggi_global_lock);
                        free(priv);
                        return GGI_ENOMEM;
                }
        }
        priv->lock = _ggi_libkgi_lock;
        priv->refcount = &refcount;
        refcount++;
        ggUnlock(_ggi_global_lock);

	priv->galloc_loaded = 0;

	LIBKGI_PRIV(vis) = priv;

        /* Mode management */
        vis->opdisplay->flush     = GGI_libkgi_flush;
	/* kgicommand obselete */
        vis->opdisplay->getapi    = GGI_libkgi_getapi;
        vis->opdisplay->setflags  = GGI_libkgi_setflags;
        vis->opdisplay->idleaccel = GGI_libkgi_idleaccel;
        vis->opdisplay->getmode   = GGI_libkgi_getmode;
        vis->opdisplay->checkmode = GGI_libkgi_checkmode;
        vis->opdisplay->setmode   = GGI_libkgi_setmode;
        vis->opdisplay->sendevent = GGI_libkgi_sendevent;

	/* GC management */
        vis->opgc->gcchanged = GGI_libkgi_gcchanged;

	/* Drawops. We don't supply _nc variants as we only do fully 
	 * implemented renderers.
	 */
	vis->opdraw->setorigin		= GGI_libkgi_setorigin;
	vis->opdraw->setdisplayframe	= GGI_libkgi_setdisplayframe;
        vis->opdraw->setreadframe	= GGI_libkgi_setreadframe;
        vis->opdraw->setwriteframe	= GGI_libkgi_setwriteframe;
        vis->opdraw->fillscreen		= GGI_libkgi_fillscreen;
        vis->opdraw->putc		= GGI_libkgi_putc;
        vis->opdraw->puts		= GGI_libkgi_puts;
        vis->opdraw->getcharsize	= GGI_libkgi_getcharsize;
        vis->opdraw->drawpixel		= GGI_libkgi_drawpixel;
        vis->opdraw->putpixel		= GGI_libkgi_putpixel;
        vis->opdraw->getpixel		= GGI_libkgi_getpixel;
        vis->opdraw->drawline		= GGI_libkgi_drawline;
        vis->opdraw->drawhline		= GGI_libkgi_drawhline;
        vis->opdraw->puthline		= GGI_libkgi_puthline;
        vis->opdraw->gethline		= GGI_libkgi_gethline;
        vis->opdraw->drawvline		= GGI_libkgi_drawvline;
        vis->opdraw->putvline		= GGI_libkgi_putvline;
        vis->opdraw->getvline		= GGI_libkgi_getvline;
        vis->opdraw->drawbox		= GGI_libkgi_drawbox;
        vis->opdraw->putbox		= GGI_libkgi_putbox;
        vis->opdraw->getbox		= GGI_libkgi_getbox;
        vis->opdraw->copybox		= GGI_libkgi_copybox;
        vis->opdraw->crossblit		= GGI_libkgi_crossblit;

	/* Color ops will use generic color libs. */

        /* Register cleanup handler */
        ggRegisterCleanup((ggcleanup_func *)do_cleanup, vis);

        DPRINT("display-libkgi: GGIdlinit success.\n");

        *dlret = GGI_DL_OPDISPLAY;
        return 0;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
        return do_cleanup(vis);
}

EXPORTFUNC
int GGIdl_kgilib(int func, void **funcptr);

int GGIdl_kgilib(int func, void **funcptr)
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

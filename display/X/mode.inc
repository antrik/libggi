/* $Id: mode.inc,v 1.3 2001/07/31 08:15:25 cegger Exp $
******************************************************************************

   Mode management for display-X and display-Xlib.

   Copyright (C) 1997      Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1998      Andreas Beck		[becka@ggi-project.org]
   Copyright (C) 1998      Steve Cheng		[steve@ggi-project.org]
   Copyright (C) 1998-2000 Marcus Sundberg	[marcus@ggi-project.org]

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

/*
 * This file is included by Xlib/mode.c and X/mode.c
 * It contains the following common functions:
 *
 *   int _ggi_x_resize(ggi_visual_t vis, int w, int h, ggi_event *ev);
 *   static int _GGIgetbpp(ggi_visual *vis);
 *   static int _GGIbasiccheck(ggi_visual *vis,ggi_mode *tm,
                               XVisualInfo *vinfo);
 *   int GGI_X_checkmode(ggi_visual *vis,ggi_mode *tm);
 *   int GGI_X_setmode(ggi_visual *vis,ggi_mode *tm);
 *   int GGI_X_getmode(ggi_visual *vis,ggi_mode *tm);
 */


int _ggi_x_resize(ggi_visual_t vis, int w, int h, ggi_event *ev)
{
	ggi_cmddata_switchrequest *swreq;

	GGIDPRINT_DRAW("_ggi_x_resize(%p, %dx%d, %p) called\n",
		       vis, w, h, ev);

	if (LIBGGI_MODE(vis)->visible.x == w &&
	    LIBGGI_MODE(vis)->visible.y == h) {
		return 1;
	}

	ev->any.size=	sizeof(gii_cmd_nodata_event)+
			sizeof(ggi_cmddata_switchrequest);
	ev->any.type=	evCommand;
	ev->cmd.code=	GGICMD_REQUEST_SWITCH;

	swreq = (ggi_cmddata_switchrequest *) ev->cmd.data;
	swreq->request = GGI_REQSW_MODE;
	ggiGetMode(vis, &swreq->mode);
	swreq->mode.visible.x = w;
	swreq->mode.visible.y = h;

	return 0;
}


static int
_get_visual(Display *disp, VisualID id, XVisualInfo *vinfo)
{
	int nret;
	XVisualInfo *retvis;
	XVisualInfo argvis;

	argvis.visualid = id;

	retvis = XGetVisualInfo(disp, VisualIDMask, &argvis, &nret);
	if (retvis == NULL) return -1;

	memcpy(vinfo, retvis, sizeof(XVisualInfo));
	XFree(retvis);

	return 0;
}

static int
_scheme2class(uint32 scheme)
{
	int ret;

	switch (scheme) {
	case GT_TRUECOLOR: ret = TrueColor;	break;
	case GT_GREYSCALE: ret = GrayScale;	break;
	case GT_PALETTE:   ret = PseudoColor;	break;
	case GT_STATIC_PALETTE: ret = StaticColor; break;
	/* Negative classes are non-valid as of X11R6.4, might break if the
	   X protocol ever changes... */
	case GT_AUTO:
		ret = -1;
		break;
	default:
		ret = -2;
		break;
	}

	return ret;
}

static uint32
_class2scheme(int cls)
{
	uint32 ret;

	switch (cls) {
	case StaticGray:
	case GrayScale:	  ret = GT_GREYSCALE;	break;
	case StaticColor: ret = GT_STATIC_PALETTE; break;
	case PseudoColor: ret = GT_PALETTE;	break;
	case TrueColor:   ret = GT_TRUECOLOR;	break;
	case DirectColor: /* What is this ? */
	default:
		ret = GT_INVALID;
		break;
	}

	return ret;
}

static int _GGIgetbpp(ggi_x_priv *priv)
{
	XImage *bppcheckimage;
	unsigned int bits_per_pixel = 0;

	bppcheckimage = XGetImage(priv->xwin.x.display,
				  RootWindow(priv->xwin.x.display, priv->xwin.x.screen),
				  0, 0, 1, 1, AllPlanes, ZPixmap);
	if (bppcheckimage != NULL) {
		bits_per_pixel = bppcheckimage->bits_per_pixel;
		XDestroyImage(bppcheckimage);
	}
	return bits_per_pixel;
}


/* X was designed before C++ ... */
#if defined(__cplusplus) || defined(c_plusplus)
#define class  c_class
#endif

static int _GGIbasiccheck(ggi_visual *vis, ggi_mode *tm, XVisualInfo *vinfo)
{
	ggi_x_priv *priv = LIBGGI_PRIVATE(vis);
	int err = 0;
	int bpp, nret;
	XVisualInfo *retvis;
	XVisualInfo argvis;
	long vismask = 0;
	ggi_graphtype defgt;

	bpp = _GGIgetbpp(priv);
	_get_visual(priv->xwin.x.display,
		    DefaultVisual(priv->xwin.x.display, priv->xwin.x.screen)->visualid,
		    vinfo);
	defgt = GT_CONSTRUCT(vinfo->depth, _class2scheme(vinfo->class), bpp);

	GGIDPRINT_MODE("_GGIbasiccheck: defgt = 0x%x, tmgt = 0x%x\n",
		    defgt, tm->graphtype);
	GGIDPRINT_MODE("Default visual -  ID: 0x%x, depth: %d, class: %d\n",
		    vinfo->visualid,
		    vinfo->depth,
		    vinfo->class);

	if (tm->graphtype == GT_AUTO) {
		tm->graphtype = defgt;
		return 0;
	}

	if (GT_DEPTH(tm->graphtype) != GT_AUTO) vismask |= VisualDepthMask;
	if (GT_SCHEME(tm->graphtype) != GT_AUTO) vismask |= VisualClassMask;
	argvis.depth = GT_DEPTH(tm->graphtype);
	argvis.class = _scheme2class(GT_SCHEME(tm->graphtype));

	retvis = XGetVisualInfo(priv->xwin.x.display, vismask, &argvis, &nret);
	if (retvis == NULL) {
		tm->graphtype = defgt;
		err = -1;
	} else {
		int size;

		memcpy(vinfo, retvis, sizeof(XVisualInfo));
		XFree(retvis);

		/* Hackomania, but I don't know any better way :( */
		size = (vinfo->depth + 7) & ~7;
		if (size == 24 && bpp == 32) size = 32;

		/* FIXME - support subschemes */
		if (GT_SUBSCHEME(tm->graphtype) != GT_AUTO ||
		    (GT_SIZE(tm->graphtype) != size &&
		     GT_SIZE(tm->graphtype) != GT_AUTO)) {
			err = -1;
		}

		tm->graphtype = GT_CONSTRUCT(vinfo->depth,
					     _class2scheme(vinfo->class),
					     size);
		GGIDPRINT_MODE("_GGIbasiccheck: gt = 0x%x\n", tm->graphtype);
		GGIDPRINT_MODE("Visual -  ID: 0x%x, depth: %d, class: %d\n",
			    vinfo->visualid,
			    vinfo->depth,
			    vinfo->class);
	}

	return err;
}

/*
 * Check mode
 */
int GGI_X_checkmode(ggi_visual *vis, ggi_mode *tm)
{
	XVisualInfo vinfo;      /* Dummy here */
	ggi_x_priv *priv;
	int err = 0;

	LIBGGI_APPASSERT(vis != NULL, "GGIcheckmode: Visual == NULL");

	priv = LIBGGI_PRIVATE(vis);

	if (tm->visible.x == GGI_AUTO) {
		tm->visible.x = tm->virt.x == GGI_AUTO
			? priv->xwin.defsize.x : tm->virt.x;
	}
	if (tm->visible.y == GGI_AUTO) {
		tm->visible.y = tm->virt.y == GGI_AUTO
			? priv->xwin.defsize.y : tm->virt.y;
	}
	if (tm->virt.x == GGI_AUTO) tm->virt.x = (tm->visible.x+3) & ~3;
	if (tm->virt.y == GGI_AUTO) tm->virt.y = tm->visible.y;

	if (tm->virt.x < tm->visible.x) {
		tm->virt.x = (tm->visible.x+3) & ~3;
		err = -1;
	}

	/* Not necessary in all cases, but it doesn't hurt */
	if ((tm->virt.x & ~3) != tm->virt.x) {
		tm->virt.x = ((tm->virt.x+3) & ~3);
		err = -1;
	}

	if (tm->virt.y < tm->visible.y) {
		tm->virt.y = tm->visible.y;
		err = -1;
	}

	if (tm->frames == GGI_AUTO) tm->frames = 1;

#ifdef GGI_X_TARGET
	if (tm->frames > 8) {
		tm->frames = 8;
#endif
#ifdef GGI_XLIB_TARGET
	if (tm->frames > 1) {
		tm->frames = 1;
#endif
		err = -1;
	}

	if ((tm->dpp.x != 1 && tm->dpp.x != GGI_AUTO) ||
	    (tm->dpp.y != 1 && tm->dpp.y != GGI_AUTO)) {
		err = -1;
	}
	tm->dpp.x = tm->dpp.y = 1;

#define SCREENWMM DisplayWidthMM(priv->xwin.x.display, priv->xwin.x.screen)
#define SCREENW   DisplayWidth(priv->xwin.x.display, priv->xwin.x.screen)
#define SCREENHMM DisplayHeightMM(priv->xwin.x.display, priv->xwin.x.screen)
#define SCREENH   DisplayHeight(priv->xwin.x.display, priv->xwin.x.screen)
#define SCREENDPIX \
((SCREENWMM <= 0) ?  0 : (SCREENW * tm->dpp.x * 254 / SCREENWMM / 10))
#define SCREENDPIY \
((SCREENHMM <= 0) ?  0 : (SCREENH * tm->dpp.x * 254 / SCREENHMM / 10))

        err = _ggi_figure_physz(tm, 
				priv->xwin.x.physzflags, &(priv->xwin.x.physz),
                                SCREENDPIX, SCREENDPIY, SCREENW, SCREENH);

#undef SCREENWMM 
#undef SCREENW   
#undef SCREENHMM 
#undef SCREENH   
#undef SCREENDPIX
#undef SCREENDPIY

	err = _GGIbasiccheck(vis, tm, &vinfo);

	return err;
}

/************************/
/* get the current mode */
/************************/
int GGI_X_getmode(ggi_visual *vis,ggi_mode *tm)
{
	LIBGGI_APPASSERT(vis != NULL, "GGIgetmode: Visual == NULL");

	/* We assume the mode in the visual to be o.k. */
	memcpy(tm,LIBGGI_MODE(vis),sizeof(ggi_mode));

	return 0;
}

int GGI_X_getapi(ggi_visual *vis,int num, char *apiname ,char *arguments)
{
	*arguments = '\0';
	switch (num) {
		case 0:
#ifdef GGI_X_TARGET
			strcpy(apiname  ,"display-x");
#endif
#ifdef GGI_XLIB_TARGET
			strcpy(apiname  ,"display-xlib");
#endif
			return 0;
		case 1:
			strcpy(apiname ,"generic-stubs");
			return 0;
		case 2:
			strcpy(apiname ,"generic-color");
			return 0;
#ifdef GGI_X_TARGET
                case 3:
			sprintf(apiname, "generic-linear-%d",
				GT_SIZE(LIBGGI_MODE(vis)->graphtype));
			return 0;
#endif /* GGI_X_TARGET */
	}

	return -1;
}

static int _GGIdomode(ggi_visual *vis)
{
	int err,id;
	char sugname[256],args[256];

	_ggiZapMode(vis, 0);

	for (id=1; 0 == GGI_X_getapi(vis,id,sugname,args); id++) {
		err = _ggiOpenDL(vis, sugname, args, NULL);
		if (err) {
#ifdef GGI_X_TARGET
			fprintf(stderr,
				"display-x: Can't open the %s (%s) library.\n",
				sugname, args);
#endif
#ifdef GGI_XLIB_TARGET
			fprintf(stderr, "display-xlib: Can't open the %s (%s) library.\n",
				sugname, args);
#endif
			/* In our special case, fail is always fatal. */
			return err;
		} else {
			GGIDPRINT_LIBS("X: GGIsetmode: success in loading %s (%s)\n",
				       sugname, args);
		}
	}

	/* color */
	/*vis->opcolor->mapcolor=GGImapcolor;*/
	/*vis->opcolor->unmappixel=GGIunmappixel;*/
	if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE) {
		vis->opcolor->setpalvec=GGI_X_setpalvec;
	}

#ifdef GGI_X_TARGET
	/* X can pan on a larger bitmap ;-) */
	vis->opdraw->setorigin=GGI_X_setorigin;

	/* X can also page flip ;-) */
	vis->opdraw->setdisplayframe=GGI_X_setdisplayframe;
#endif
#ifdef GGI_XLIB_TARGET
	vis->opgc->gcchanged=GGI_Xlib_gcchanged;

		/* pixels */
	vis->opdraw->drawpixel_nc=GGI_Xlib_drawpixel;
	vis->opdraw->drawpixel=GGI_Xlib_drawpixel;
	vis->opdraw->putpixel_nc=GGI_Xlib_putpixel;
	vis->opdraw->putpixel=GGI_Xlib_putpixel;
	vis->opdraw->getpixel=GGI_Xlib_getpixel;

		/* lines */
	vis->opdraw->drawline=GGI_Xlib_drawline;
	vis->opdraw->drawhline_nc=GGI_Xlib_drawhline;
	vis->opdraw->drawhline=GGI_Xlib_drawhline;
	vis->opdraw->puthline=GGI_Xlib_puthline;
	vis->opdraw->drawvline_nc=GGI_Xlib_drawvline;
	vis->opdraw->drawvline=GGI_Xlib_drawvline;
	vis->opdraw->putvline=GGI_Xlib_putvline;

		/* boxes */
	vis->opdraw->drawbox=GGI_Xlib_drawbox;
	vis->opdraw->putbox=GGI_Xlib_putbox;
	vis->opdraw->copybox=GGI_Xlib_copybox;
	vis->opdraw->getbox=GGI_Xlib_getbox;
	vis->opdraw->fillscreen=GGI_Xlib_fillscreen;
		/* text */
#if 0
	/* Not operational yet */
	if (GGIXLIB_PRIV(vis)->textfont) {
		GGIDPRINT_MODE("Xlib: using XDrawString() for text\n");
		vis->opdraw->getcharsize=GGI_Xlib_getcharsize;
		vis->opdraw->putc=GGI_Xlib_putc;
	}
#endif
#endif /* GGI_XLIB_TARGET */

	ggiIndicateChange(vis, GGI_CHG_APILIST);

	return 0;
}

int GGI_X_setmode(ggi_visual *vis, ggi_mode *tm)
{
	ggi_x_priv *priv=LIBGGI_PRIVATE(vis);
	XVisualInfo vinfo;
	int newwin = 1;
	int rc, i;

	if ((rc=GGI_X_checkmode(vis,tm))) return rc;

	ggLock(priv->xwin.x.xliblock);

#ifdef GGI_X_TARGET
	MANSYNC_ignore(vis);
#endif /* GGI_X_TARGET */

	rc = _GGIbasiccheck(vis, tm, &vinfo);
	if (rc != 0) {
		GGIDPRINT_MODE("_GGIbasiccheck() failed!\n");
#ifdef GGI_X_TARGET
		MANSYNC_cont(vis);
#endif
		ggUnlock(priv->xwin.x.xliblock);
		return rc;
	}

	/* Fill in ggi_pixelformat */
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	LIBGGI_PIXFMT(vis)->red_mask   = vinfo.red_mask;
	LIBGGI_PIXFMT(vis)->green_mask = vinfo.green_mask;
	LIBGGI_PIXFMT(vis)->blue_mask  = vinfo.blue_mask;

	LIBGGI_PIXFMT(vis)->depth = GT_DEPTH(tm->graphtype);
	LIBGGI_PIXFMT(vis)->size  = GT_SIZE(tm->graphtype);
	if (GT_SCHEME(tm->graphtype) == GT_PALETTE ||
	    GT_SCHEME(tm->graphtype) == GT_STATIC_PALETTE) {
		LIBGGI_PIXFMT(vis)->clut_mask
			= (1 << GT_DEPTH(tm->graphtype)) - 1;
	} else {
		LIBGGI_PIXFMT(vis)->clut_mask = 0;
	}

	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

#ifdef GGI_X_TARGET
	priv->xoff   = priv->yoff = 0;
	priv->ysplit = tm->visible.y;
	priv->viswidth  = tm->visible.x;
	priv->visheight = tm->visible.y;

	_GGI_X_freedbs(vis, priv);
#endif /* GGI_X_TARGET */

	if (priv->xwin.wintype == GGIX_NORMAL) {
		if (priv->xwin.window != RootWindow(priv->xwin.x.display, priv->xwin.x.screen)) {
			if (vinfo.visualid == priv->xwin.visual.visualid) {
				newwin = 0;
			} else {
				XDestroyWindow(priv->xwin.x.display, priv->xwin.window);
			}
		}
		priv->xwin.visual = vinfo;
		if (newwin) {
			priv->xwin.window = XCreateWindow(priv->xwin.x.display,
				RootWindow(priv->xwin.x.display,priv->xwin.x.screen),
				0, 0, tm->visible.x, tm->visible.y, 0,
				priv->xwin.visual.depth, InputOutput,
				priv->xwin.visual.visual, 0, NULL);
			XDefineCursor(priv->xwin.x.display, priv->xwin.window,
				      priv->xwin.cursor);
		}
	}
	GGIDPRINT_MODE("X: has window 0x%x\n", priv->xwin.window);

	if (priv->xwin.wintype == GGIX_NORMAL) {
		XSizeHints hint;
		char *name = "GGI-on-X";

		/* Fill in hint structure. */
		hint.x      = hint.y      = 0;
		hint.width  = tm->visible.x;
		hint.height = tm->visible.y;
		hint.flags  = PSize | PMinSize | PMaxSize;
		hint.min_width = tm->visible.x;
		hint.min_height = tm->visible.y;
		hint.max_width = tm->visible.x;
		hint.max_height = tm->visible.y;

		/* Set WM hints and titles */
		XSetStandardProperties(priv->xwin.x.display, priv->xwin.window, name, name,
				       None, NULL, 0, &hint);

		GGIDPRINT_MODE("X: Prepare to resize.\n");
		XResizeWindow(priv->xwin.x.display, priv->xwin.window,
			      tm->visible.x, tm->visible.y);
		GGIDPRINT_MODE("X: Resize\n");
	}

#ifdef GGI_X_TARGET
#ifdef HAVE_SHM
	if (priv->have_shm) {
		for(i = 0; i<tm->frames; i++) {
			void *fbaddr;
			GGIDPRINT_MODE("X: Creating shm image #%d\n", i);

			priv->ximage = XShmCreateImage(
				priv->xwin.x.display, vinfo.visual,
				vinfo.depth,
				ZPixmap, NULL,
				&(priv->shminfo[i]),
				tm->virt.x,tm->virt.y);

			priv->shminfo[i].shmid =
				shmget(IPC_PRIVATE, (unsigned int)
				       (tm->virt.x*tm->virt.y
					* GT_ByPP(tm->graphtype)),
				       IPC_CREAT | 0777);

			fbaddr = priv->shminfo[i].shmaddr
				= priv->ximage->data =
				shmat(priv->shminfo[i].shmid,0,0);

			priv->shminfo[i].readOnly = False;

			GGIDPRINT_MODE("X: shmat success at %p.\n", fbaddr);

			/* Check if XShm is available */
			ggLock(_ggi_global_lock); /* Entering protected
						     section */
			shmerror = 0;
			oldshmerrorhandler = XSetErrorHandler(shmerrorhandler);
			XShmAttach(priv->xwin.x.display, &(priv->shminfo[i]));

			XSync(priv->xwin.x.display, 0);
			XSetErrorHandler(oldshmerrorhandler);

			if (shmerror) {
				if (fbaddr != NULL) {
					shmdt(fbaddr);
				}
				if (priv->ximage) {
					XDestroyImage(priv->ximage);
					priv->ximage = NULL;
				}
				priv->shminfo[i].shmid = -1;
				priv->have_shm = 0;
				GGIDPRINT_MODE("X: XShmAttach failed - trying without shm\n");
				break;
			} else {
				shmctl(priv->shminfo[i].shmid, IPC_RMID, 0);
				GGIDPRINT_MODE("X: ShmImage #%d allocated\n", i);
				priv->ximage_list[i] = priv->ximage;
			}
			ggUnlock(_ggi_global_lock); /* Exiting protected
						       section */
		}
	}

	if (! priv->have_shm)
#endif	/* HAVE_SHM */
	{
		for (i = 0; i < tm->frames; i++) {
			void *fbaddr = _ggi_malloc(tm->virt.x * tm->virt.y
						    * GT_ByPP(tm->graphtype));
			GGIDPRINT_MODE("X: Buffer allocated %p\n", fbaddr);

			priv->ximage_list[i] = XCreateImage(
				priv->xwin.x.display, vinfo.visual, vinfo.depth,
				ZPixmap, /* format */
				0, 	 /* offset */
				fbaddr,	 /* data */
				tm->virt.x, tm->virt.y, /* size */
				8,	 /* bitmap_pad*/
				0);
			GGIDPRINT_MODE("X: Image #%d allocated\n", i);
#ifdef GGI_LITTLE_ENDIAN
			priv->ximage_list[i]->byte_order = LSBFirst;
			priv->ximage_list[i]->bitmap_bit_order = LSBFirst;
#else
			priv->ximage_list[i]->byte_order = MSBFirst;
			priv->ximage_list[i]->bitmap_bit_order = MSBFirst;
#endif
		}
	}
	/* Set up directbuffers */
	for (i = 0; i < tm->frames; i++) {
		LIBGGI_APPLIST(vis)->last_targetbuf
			= _ggi_db_add_buffer(LIBGGI_APPLIST(vis),
					     _ggi_db_get_new());
		LIBGGI_APPBUFS(vis)[i]->frame = i;
		LIBGGI_APPBUFS(vis)[i]->type
			= GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		LIBGGI_APPBUFS(vis)[i]->read = LIBGGI_APPBUFS(vis)[i]->write
			= priv->ximage_list[i]->data;
		LIBGGI_APPBUFS(vis)[i]->layout = blPixelLinearBuffer;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride
			= priv->ximage_list[i]->bytes_per_line;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.pixelformat
			= LIBGGI_PIXFMT(vis);
	}
	LIBGGI_APPLIST(vis)->first_targetbuf
		= LIBGGI_APPLIST(vis)->last_targetbuf - (tm->frames-1);
	priv->ximage = priv->ximage_list[0];

#endif /* GGI_X_TARGET */

	if (newwin) {
		XEvent event;
#ifdef GGI_XLIB_TARGET
		XSetWindowAttributes attrib;
#endif

		if (priv->xwin.x.cmap) XFreeColormap(priv->xwin.x.display,priv->xwin.x.cmap);
		if (vis->palette) {
			free(vis->palette);
			vis->palette = NULL;
		}

		if (priv->xwin.x.gc) XFreeGC(priv->xwin.x.display, priv->xwin.x.gc);
		priv->xwin.x.gc = XCreateGC(priv->xwin.x.display, priv->xwin.window, 0, 0);
#ifdef GGI_XLIB_TARGET
		if (priv->textfont) {
			XSetFont(priv->xwin.x.display, priv->xwin.x.gc, priv->textfont->fid);
		}
#endif
		GGIDPRINT_MODE("X GC allocated.\n");

		XSync(priv->xwin.x.display, 0);

		if (priv->xwin.wintype == GGIX_NORMAL) {
			GGIDPRINT_MODE("X: About to map\n");
			/* Map window. */
			XSelectInput(priv->xwin.x.display, priv->xwin.window,
				     ExposureMask);
			XMapRaised(priv->xwin.x.display, priv->xwin.window);

			/* Wait for window to become mapped */
			XNextEvent (priv->xwin.x.display, &event);
			GGIDPRINT_MODE("X: Window Mapped\n");

			/* Select input events to listen for */
			XSelectInput(priv->xwin.x.display, priv->xwin.window,
				     KeyPressMask | KeyReleaseMask |
				     ButtonPressMask | ButtonReleaseMask |
				     EnterWindowMask | LeaveWindowMask |
				     ExposureMask | PointerMotionMask);
		}

		if (GT_SCHEME(tm->graphtype) == GT_PALETTE) {
			/* Create X colormap */
			priv->xwin.x.cmap
				= XCreateColormap(priv->xwin.x.display, priv->xwin.window,
						  vinfo.visual, AllocAll);
		} else {
			priv->xwin.x.cmap
				= XCreateColormap(priv->xwin.x.display, priv->xwin.window,
						  vinfo.visual, AllocNone);
		}
		if (GT_SCHEME(tm->graphtype) == GT_PALETTE ||
		    GT_SCHEME(tm->graphtype) == GT_STATIC_PALETTE) {
			priv->xwin.x.nocols = 1 << GT_DEPTH(tm->graphtype);

			vis->palette = _ggi_malloc(sizeof(ggi_color)
						   * priv->xwin.x.nocols);
		}


#ifdef GGI_XLIB_TARGET
		if (priv->tempgc) XFreeGC(priv->xwin.x.display, priv->tempgc);
		priv->tempgc = XCreateGC(priv->xwin.x.display, priv->xwin.window, 0, 0);
		GGIDPRINT_MODE("X-lib temp GC allocated.\n");

		/* Turn on backing store */
		attrib.backing_store = Always;
		XChangeWindowAttributes(priv->xwin.x.display, priv->xwin.window,
					CWBackingStore, &attrib);
#endif /* GGI_XLIB_TARGET */
	}

	if (GT_SCHEME(tm->graphtype) == GT_PALETTE ||
	    GT_SCHEME(tm->graphtype) == GT_STATIC_PALETTE) {
		Colormap defcmap;
		XColor color_cell;

		/* get the deafult color map (i.e. the shared one)
		   (minimize color flashing between windows)
		   (Thanks to Wouter for this code.) */
		defcmap = DefaultColormap(priv->xwin.x.display, priv->xwin.x.screen);

		/* Fill the colormap with the original colors */
		for (i=0; i<priv->xwin.x.nocols; i++) {
			color_cell.pixel = i;
			color_cell.flags = DoRed | DoGreen | DoBlue;
			XQueryColor(priv->xwin.x.display, defcmap, &color_cell);
			if (GT_SCHEME(tm->graphtype) == GT_PALETTE) {
				XStoreColor(priv->xwin.x.display, priv->xwin.x.cmap,
					    &color_cell);
			}

			vis->palette[i].r = color_cell.red;
			vis->palette[i].g = color_cell.green;
			vis->palette[i].b = color_cell.blue;
		}

		priv->xwin.cmap_first=256;
		priv->xwin.cmap_last=0;

		GGIDPRINT_MODE("X: copied default colormap into private colormap (%x)\n",
			    priv->xwin.x.cmap);
	}

	ggUnlock(priv->xwin.x.xliblock);

	GGIDPRINT_MODE("X: Sync\n");
	XSync(priv->xwin.x.display, 0);
	GGIDPRINT_MODE("X: Sync done\n");

	memcpy(LIBGGI_MODE(vis), tm, sizeof(ggi_mode));

	rc = _GGIdomode(vis);

	/* Tell inputlib about the new window */
	if (priv->xwin.x.inp) {
		gii_event ev;
		gii_xwin_cmddata_setparam *data
			= (gii_xwin_cmddata_setparam *) ev.cmd.data;
		ev.cmd.size = sizeof(gii_cmd_event);
		ev.cmd.type = evCommand;
		ev.cmd.target = priv->xwin.x.inp->origin;
		ev.cmd.code = GII_CMDCODE_XWINSETPARAM;
		data->win = priv->xwin.window;
		data->ptralwaysrel = 0;

		giiEventSend(priv->xwin.x.inp, &ev);
	}

#ifdef GGI_X_TARGET
	MANSYNC_cont(vis);
#endif /* GGI_X_TARGET */

	return rc;
}

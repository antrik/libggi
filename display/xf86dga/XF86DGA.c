/* $Id: XF86DGA.c,v 1.8 2005/01/29 08:54:16 cegger Exp $

Copyright (c) 1995  Jon Tombs
Copyright (c) 1995,1996  The XFree86 Project, Inc
Copyright (c) 1999  Marcus Sundberg [marcus@ggi-project.org]

*/

#include <unistd.h>

/* THIS IS NOT AN X CONSORTIUM STANDARD */

/* Defines */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE	199309L
#endif
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#define _XOPEN_SOURCE	500L
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#ifndef _SVID_SOURCE
#define _SVID_SOURCE
#endif
#define FUNCPROTO	15
#define NARROWPROTO
#define XTHREADS
#define XUSE_MTSAFE_API

#define NEED_EVENTS
#define NEED_REPLIES
#include <X11/Xlibint.h>
#include "xf86dga.h"
#include "xf86dgastr.h"
#include <X11/extensions/Xext.h>
#include "extutil.h"

static XExtensionInfo _xf86dga_info_data;
static XExtensionInfo *xf86dga_info = &_xf86dga_info_data;
static char *xf86dga_extension_name = XF86DGANAME;

#define XF86DGACheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, xf86dga_extension_name, val)

/*****************************************************************************
 *                                                                           *
 *			   private utility routines                          *
 *                                                                           *
 *****************************************************************************/

static XEXT_GENERATE_CLOSE_DISPLAY(close_display, xf86dga_info)

static /* const */ XExtensionHooks xf86dga_extension_hooks = {
	NULL,			/* create_gc */
	NULL,			/* copy_gc */
	NULL,			/* flush_gc */
	NULL,			/* free_gc */
	NULL,			/* create_font */
	NULL,			/* free_font */
	close_display,		/* close_display */
	NULL,			/* wire_to_event */
	NULL,			/* event_to_wire */
	NULL,			/* error */
	NULL,			/* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY(find_display, xf86dga_info,
				  xf86dga_extension_name,
				  &xf86dga_extension_hooks, 0, NULL)


/*****************************************************************************
 *                                                                           *
 *		    public XFree86-DGA Extension routines                *
 *                                                                           *
 *****************************************************************************/
/* prototypes to suppress compiler warnings */
Bool _ggi_XF86DGAQueryExtension(Display * dpy, int *event_basep,
				int *error_basep);
Bool _ggi_XF86DGAQueryVersion(Display * dpy, int *majorVersion,
			      int *minorVersion);
Bool _ggi_XF86DGAGetVideoLL(Display * dpy, int screen, int *offset,
			    int *width, int *bank_size, int *ram_size);
Bool _ggi_XF86DGADirectVideoLL(Display * dpy, int screen, int enable);
Bool _ggi_XF86DGASetViewPort(Display * dpy, int screen, int x, int y);
Bool _ggi_XF86DGAInstallColormap(Display * dpy, int screen, Colormap cmap);
Bool _ggi_XF86DGAQueryDirectVideo(Display * dpy, int screen, int *flags);
Bool _ggi_XF86DGACopyArea(Display * dpy, int screen, Drawable d,
			  GC gc,
			  int src_x, int src_y,
			  unsigned int width, unsigned int height,
			  int dst_x, int dst_y);
Bool _ggi_XF86DGAFillRectangle(Display * dpy, int screen,
			       Drawable d, GC gc,
			       int x, int y,
			       unsigned int width, unsigned int height);
void _ggi_XF86DGAUnmap(void);
int _ggi_XF86DGADirectVideo(Display * dis, int screen, int enable);
int _ggi_XF86DGAGetVideo(Display * dis, int screen, char **addr,
			 int *width, int *bank, int *ram);


Bool _ggi_XF86DGAQueryExtension(Display * dpy, int *event_basep,
				int *error_basep)
{
	XExtDisplayInfo *info = find_display(dpy);

	if (XextHasExtension(info)) {
		*event_basep = info->codes->first_event;
		*error_basep = info->codes->first_error;
		return True;
	} else {
		return False;
	}
}


Bool _ggi_XF86DGAQueryVersion(Display * dpy, int *majorVersion,
			      int *minorVersion)
{
	XExtDisplayInfo *info = find_display(dpy);
	xXF86DGAQueryVersionReply rep;
	xXF86DGAQueryVersionReq *req;

	XF86DGACheckExtension(dpy, info, False);

	LockDisplay(dpy);
	GetReq(XF86DGAQueryVersion, req);
	req->reqType = info->codes->major_opcode;
	req->dgaReqType = X_XF86DGAQueryVersion;
	if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
		UnlockDisplay(dpy);
		SyncHandle();
		return False;
	}
	*majorVersion = rep.majorVersion;
	*minorVersion = rep.minorVersion;
	UnlockDisplay(dpy);
	SyncHandle();
	return True;
}

Bool _ggi_XF86DGAGetVideoLL(Display * dpy, int screen, int *offset,
			    int *width, int *bank_size, int *ram_size)
{
	XExtDisplayInfo *info = find_display(dpy);
	xXF86DGAGetVideoLLReply rep;
	xXF86DGAGetVideoLLReq *req;

	XF86DGACheckExtension(dpy, info, False);

	LockDisplay(dpy);
	GetReq(XF86DGAGetVideoLL, req);
	req->reqType = info->codes->major_opcode;
	req->dgaReqType = X_XF86DGAGetVideoLL;
	req->screen = screen;
	if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
		UnlockDisplay(dpy);
		SyncHandle();
		return False;
	}

	*offset = /*(char *) */ rep.offset;
	*width = rep.width;
	*bank_size = rep.bank_size;
	*ram_size = rep.ram_size;

	UnlockDisplay(dpy);
	SyncHandle();
	return True;
}


Bool _ggi_XF86DGADirectVideoLL(Display * dpy, int screen, int enable)
{
	XExtDisplayInfo *info = find_display(dpy);
	xXF86DGADirectVideoReq *req;

	XF86DGACheckExtension(dpy, info, False);

	LockDisplay(dpy);
	GetReq(XF86DGADirectVideo, req);
	req->reqType = info->codes->major_opcode;
	req->dgaReqType = X_XF86DGADirectVideo;
	req->screen = screen;
	req->enable = enable;
	UnlockDisplay(dpy);
	SyncHandle();
	XSync(dpy, False);
	return True;
}

Bool _ggi_XF86DGASetViewPort(Display * dpy, int screen, int x, int y)
{
	XExtDisplayInfo *info = find_display(dpy);
	xXF86DGASetViewPortReq *req;

	XF86DGACheckExtension(dpy, info, False);

	LockDisplay(dpy);
	GetReq(XF86DGASetViewPort, req);
	req->reqType = info->codes->major_opcode;
	req->dgaReqType = X_XF86DGASetViewPort;
	req->screen = screen;
	req->x = x;
	req->y = y;
	UnlockDisplay(dpy);
	SyncHandle();
	XSync(dpy, False);
	return True;
}


Bool _ggi_XF86DGAInstallColormap(Display * dpy, int screen, Colormap cmap)
{
	XExtDisplayInfo *info = find_display(dpy);
	xXF86DGAInstallColormapReq *req;

	XF86DGACheckExtension(dpy, info, False);

	LockDisplay(dpy);
	GetReq(XF86DGAInstallColormap, req);
	req->reqType = info->codes->major_opcode;
	req->dgaReqType = X_XF86DGAInstallColormap;
	req->screen = screen;
	req->id = cmap;
	UnlockDisplay(dpy);
	SyncHandle();
	XSync(dpy, False);
	return True;
}


Bool _ggi_XF86DGAQueryDirectVideo(Display * dpy, int screen, int *flags)
{
	XExtDisplayInfo *info = find_display(dpy);
	xXF86DGAQueryDirectVideoReply rep;
	xXF86DGAQueryDirectVideoReq *req;

	XF86DGACheckExtension(dpy, info, False);

	LockDisplay(dpy);
	GetReq(XF86DGAQueryDirectVideo, req);
	req->reqType = info->codes->major_opcode;
	req->dgaReqType = X_XF86DGAQueryDirectVideo;
	req->screen = screen;
	if (!_XReply(dpy, (xReply *) & rep, 0, xFalse)) {
		UnlockDisplay(dpy);
		SyncHandle();
		return False;
	}
	*flags = rep.flags;
	UnlockDisplay(dpy);
	SyncHandle();
	return True;
}


Bool _ggi_XF86DGACopyArea(Display * dpy, int screen, Drawable d,
			  GC gc,
			  int src_x, int src_y,
			  unsigned int width, unsigned int height,
			  int dst_x, int dst_y)
{
	XExtDisplayInfo *info = find_display(dpy);
	xXF86DGACopyAreaReq *req;

	XF86DGACheckExtension(dpy, info, False);

	LockDisplay(dpy);
	FlushGC(dpy, gc);
	GetReq(XF86DGACopyArea, req);
	req->reqType = info->codes->major_opcode;
	req->dgaReqType = X_XF86DGACopyArea;
	req->screen = screen;
	req->drawable = d;
	req->gc = gc->gid;
	req->srcX = src_x;
	req->srcY = src_y;
	req->dstX = dst_x;
	req->dstY = dst_y;
	req->width = width;
	req->height = height;
	UnlockDisplay(dpy);
	SyncHandle();
	return True;
}


Bool _ggi_XF86DGAFillRectangle(Display * dpy, int screen,
			       Drawable d, GC gc,
			       int x, int y,
			       unsigned int width, unsigned int height)
{
	XExtDisplayInfo *info = find_display(dpy);
	xXF86DGAFillRectangleReq *req;

	XF86DGACheckExtension(dpy, info, False);

	LockDisplay(dpy);
	FlushGC(dpy, gc);
	GetReq(XF86DGAFillRectangle, req);
	req->reqType = info->codes->major_opcode;
	req->dgaReqType = X_XF86DGAFillRectangle;
	req->screen = screen;
	req->drawable = d;
	req->gc = gc->gid;
	req->x = x;
	req->y = y;
	req->width = width;
	req->height = height;
	UnlockDisplay(dpy);
	SyncHandle();
	return True;
}


/* Helper functions */

#include <X11/Xmd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#if defined(ISC)
# define HAS_SVR3_MMAP
# include <sys/types.h>
# include <errno.h>

# include <sys/at_ansi.h>
# include <sys/kd.h>

# include <sys/sysmacros.h>
# include <sys/immu.h>
# include <sys/region.h>

# include <sys/mmap.h>
#else
# if !defined(Lynx)
#  include <sys/mman.h>
# else
#  include <sys/types.h>
#  include <errno.h>
#  include <smem.h>
# endif
#endif

#ifndef MAP_FAILED
#define MAP_FAILED ((void*)-1)
#endif

#if defined(SVR4) && !defined(sun) && !defined(SCO325)
#define DEV_MEM "/dev/pmem"
#else
#define DEV_MEM "/dev/mem"
#endif

#if defined(ISC) && defined(HAS_SVR3_MMAP)
struct kd_memloc XFree86mloc;
#endif

static int memory_fd;
static char *_XFree86addr = NULL;
static int _XFree86size = 0;

int _ggi_XF86DGADirectVideo(Display * dis, int screen, int enable)
{
	if (enable & XF86DGADirectGraphics) {
#if !defined(ISC) && !defined(HAS_SVR3_MMAP) && !defined(Lynx)
		if (_XFree86addr && _XFree86size)
			if (mprotect
			    (_XFree86addr, _XFree86size,
			     PROT_READ | PROT_WRITE)) {
				fprintf(stderr,
					"_ggi_XF86DGADirectVideo: mprotect (%s)\n",
					strerror(errno));
				exit(-3);
			}
#endif
	} else {
		if (_XFree86addr && _XFree86size)
#if !defined(ISC) && !defined(HAS_SVR3_MMAP)
#ifndef Lynx
			if (mprotect
			    (_XFree86addr, _XFree86size, PROT_READ)) {
				fprintf(stderr,
					"_ggi_XF86DGADirectVideo: mprotect (%s)\n",
					strerror(errno));
				exit(-4);
			}
#else
			smem_create(NULL, _XFree86addr, _XFree86size,
				    SM_DETACH);
		smem_remove("XF86DGA");
#endif
#endif
	}
	_ggi_XF86DGADirectVideoLL(dis, screen, enable);

	return 1;
}


int _ggi_XF86DGAGetVideo(Display * dis, int screen, char **addr,
			 int *width, int *bank, int *ram)
{
	int offset;
	char *devname;

	_ggi_XF86DGAGetVideoLL(dis, screen, &offset, width, bank, ram);

#ifndef Lynx
#if defined(ISC) && defined(HAS_SVR3_MMAP)
	devname = "/dev/mmap";
#else
	devname = getenv("GGI_DGA_FBDEV");
	if (devname == NULL) {
		devname = DEV_MEM;
	} else {
		/* Framebuffer devices always have offset 0 */
		offset = 0;
	}
#endif
	if ((memory_fd = open(devname, O_RDWR)) < 0) {
		fprintf(stderr,
			"_ggi_XF86DGAGetVideo: failed to open %s (%s)\n",
			devname, strerror(errno));
		return 0;
	}
#endif

#if defined(ISC) && defined(HAS_SVR3_MMAP)
	XFree86mloc.vaddr = (char *) 0;
	XFree86mloc.physaddr = (char *) offset;
	XFree86mloc.length = *bank;
	XFree86mloc.ioflg = 1;

	if ((*addr =
	     (char *) ioctl(memory_fd, MAP,
			    &XFree86mloc)) != (char *) -1) {
		offset = (int) XFree86mloc.physaddr;
		*bank = (int) XFree86mloc.length;
	} else {
		fprintf(stderr,
			"_ggi_XF86DGAGetVideo: failed to mmap /dev/mmap (%s)\n",
			strerror(errno));
#else				/* !ISC */
#ifdef Lynx
	*addr =
	    (void *) smem_create("XF86DGA", (char *) offset, *bank,
				 SM_READ | SM_WRITE);
	if (*addr == NULL) {
		fprintf(stderr,
			"_ggi_XF86DGAGetVideo: smem_create() failed (%s)\n",
			strerror(errno));
#else				/* !Lynx */
#ifndef MAP_FILE
#define MAP_FILE 0
#endif
	/* This requires linux-0.99.pl10 or above */
	*addr = (void *) mmap(NULL, *bank, PROT_READ,
			      MAP_FILE | MAP_SHARED, memory_fd,
			      (off_t) offset);
	if (*addr == MAP_FAILED) {
		fprintf(stderr,
			"_ggi_XF86DGAGetVideo: failed to mmap %s (%s)\n",
			devname, strerror(errno));
#endif				/* !Lynx */
#endif				/* !ISC && !HAS_SVR3_MMAP */
		return 0;
	}
	_XFree86size = *bank;
	_XFree86addr = *addr;

	return 1;
}


void _ggi_XF86DGAUnmap(void)
{
	munmap(_XFree86addr, _XFree86size);
	close(memory_fd);
}

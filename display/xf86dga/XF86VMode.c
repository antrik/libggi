/*

Copyright (c) 1995  Kaleb S. KEITHLEY

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL Kaleb S. KEITHLEY BE LIABLE FOR ANY CLAIM, DAMAGES 
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Kaleb S. KEITHLEY 
shall not be used in advertising or otherwise to promote the sale, use 
or other dealings in this Software without prior written authorization
from Kaleb S. KEITHLEY.

*/

/* THIS IS NOT AN X CONSORTIUM STANDARD */

#define _POSIX_C_SOURCE	199309L
#define _POSIX_SOURCE
#define _XOPEN_SOURCE	500L
#define _BSD_SOURCE
#define _SVID_SOURCE  
#define FUNCPROTO	15
#define NARROWPROTO
#define XTHREADS 
#define XUSE_MTSAFE_API

#define NEED_EVENTS
#define NEED_REPLIES
#include <X11/Xlibint.h>
#include <X11/extensions/xf86vmstr.h>
#include <X11/extensions/Xext.h>
#include "extutil.h"

#ifndef MODE_BAD
#define MODE_BAD 255
#endif

static XExtensionInfo _xf86vidmode_info_data;
static XExtensionInfo *xf86vidmode_info = &_xf86vidmode_info_data;
static char *xf86vidmode_extension_name = XF86VIDMODENAME;

#define XF86VidModeCheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, xf86vidmode_extension_name, val)

/*****************************************************************************
 *                                                                           *
 *			   private utility routines                          *
 *                                                                           *
 *****************************************************************************/

static int close_display();
static /* const */ XExtensionHooks xf86vidmode_extension_hooks = {
    NULL,				/* create_gc */
    NULL,				/* copy_gc */
    NULL,				/* flush_gc */
    NULL,				/* free_gc */
    NULL,				/* create_font */
    NULL,				/* free_font */
    close_display,			/* close_display */
    NULL,				/* wire_to_event */
    NULL,				/* event_to_wire */
    NULL,				/* error */
    NULL,				/* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY (find_display, xf86vidmode_info, 
				   xf86vidmode_extension_name, 
				   &xf86vidmode_extension_hooks, 
				   0, NULL)

static XEXT_GENERATE_CLOSE_DISPLAY (close_display, xf86vidmode_info)


/*****************************************************************************
 *                                                                           *
 *		    public XFree86-VidMode Extension routines                *
 *                                                                           *
 *****************************************************************************/

Bool _ggi_XF86VidModeQueryExtension(Display *dpy, int *event_basep,
				    int *error_basep)
{
    XExtDisplayInfo *info = find_display (dpy);

    if (XextHasExtension(info)) {
	*event_basep = info->codes->first_event;
	*error_basep = info->codes->first_error;
	return True;
    } else {
	return False;
    }
}

Bool _ggi_XF86VidModeQueryVersion(Display* dpy, int* majorVersion,
				  int* minorVersion)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeQueryVersionReply rep;
    xXF86VidModeQueryVersionReq *req;

    XF86VidModeCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86VidModeQueryVersion, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeQueryVersion;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
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

Bool _ggi_XF86VidModeGetAllModeLines(Display* dpy, int screen, int* modecount,
				     XF86VidModeModeInfo ***modelinesPtr)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeGetAllModeLinesReply rep;
    xXF86VidModeGetAllModeLinesReq *req;
    XF86VidModeModeInfo *mdinfptr, **modelines;
    xXF86VidModeModeInfo xmdline;
    int i;
    int majorVersion, minorVersion;
    Bool protocolBug = False;

    XF86VidModeCheckExtension (dpy, info, False);

    /*
     * Note: There was a bug in the protocol implementation in versions
     * 0.x with x < 8 (the .private field wasn't being passed over the wire).
     * Check the server's version, and accept the old format if appropriate.
     */

    _ggi_XF86VidModeQueryVersion(dpy, &majorVersion, &minorVersion);
    if (majorVersion == 0 && minorVersion < 8) {
	protocolBug = True;
    }
    
    LockDisplay(dpy);
    GetReq(XF86VidModeGetAllModeLines, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeGetAllModeLines;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 
        (SIZEOF(xXF86VidModeGetAllModeLinesReply) - SIZEOF(xReply)) >> 2, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        return False;
    }
    *modecount = rep.modecount;

    if (!(modelines = (XF86VidModeModeInfo **) Xcalloc(rep.modecount,
                                          sizeof(XF86VidModeModeInfo *)
                                          +sizeof(XF86VidModeModeInfo)))) {
        _XEatData(dpy, (rep.modecount) * sizeof(xXF86VidModeModeInfo));
        Xfree(modelines);
        return False;
    }
    mdinfptr = (XF86VidModeModeInfo *) (
			    (char *) modelines
			    + rep.modecount*sizeof(XF86VidModeModeInfo *)
		    );

    for (i = 0; i < rep.modecount; i++) {
        modelines[i] = mdinfptr++;
        _XRead32(dpy, &xmdline, sizeof(xXF86VidModeModeInfo));
        modelines[i]->dotclock   = xmdline.dotclock;
        modelines[i]->hdisplay   = xmdline.hdisplay;
        modelines[i]->hsyncstart = xmdline.hsyncstart;
        modelines[i]->hsyncend   = xmdline.hsyncend;
        modelines[i]->htotal     = xmdline.htotal;
        modelines[i]->vdisplay   = xmdline.vdisplay;
        modelines[i]->vsyncstart = xmdline.vsyncstart;
        modelines[i]->vsyncend   = xmdline.vsyncend;
        modelines[i]->vtotal     = xmdline.vtotal;
        modelines[i]->flags      = xmdline.flags;
	if (protocolBug) {
	    modelines[i]->privsize = 0;
	    modelines[i]->private = NULL;
	} else {
            modelines[i]->privsize   = xmdline.privsize;
	    if (xmdline.privsize > 0) {
	        if (!(modelines[i]->private =
			    Xcalloc(xmdline.privsize, sizeof(INT32)))) {
		    _XEatData(dpy, (xmdline.privsize) * sizeof(INT32));
		    Xfree(modelines[i]->private);
	        } else {
		    _XRead32(dpy, modelines[i]->private,
			     xmdline.privsize * sizeof(INT32));
	        }
	    } else {
                modelines[i]->private = NULL;
	    }
	}
    }
    *modelinesPtr = modelines;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool _ggi_XF86VidModeSwitchToMode(Display* dpy, int screen,
			     XF86VidModeModeInfo* modeline)
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86VidModeSwitchToModeReq *req;
    int majorVersion, minorVersion;
    Bool protocolBug = False;

    XF86VidModeCheckExtension (dpy, info, False);

    /*
     * Note: There was a bug in the protocol implementation in versions
     * 0.x with x < 8 (the .private field wasn't expected to be sent over
     * the wire).  Check the server's version, and accept the old format
     * if appropriate.
     */

    _ggi_XF86VidModeQueryVersion(dpy, &majorVersion, &minorVersion);
    if (majorVersion == 0 && minorVersion < 8) {
	protocolBug = True;
    }
    
    LockDisplay(dpy);
    GetReq(XF86VidModeSwitchToMode, req);
    req->reqType = info->codes->major_opcode;
    req->xf86vidmodeReqType = X_XF86VidModeSwitchToMode;
    req->screen = screen;
    req->dotclock =	modeline->dotclock;
    req->hdisplay =	modeline->hdisplay;
    req->hsyncstart =	modeline->hsyncstart;
    req->hsyncend =	modeline->hsyncend;
    req->htotal =	modeline->htotal;
    req->vdisplay =	modeline->vdisplay;
    req->vsyncstart =	modeline->vsyncstart;
    req->vsyncend =	modeline->vsyncend;
    req->vtotal =	modeline->vtotal;
    req->flags =	modeline->flags;
    if (protocolBug) {
	req->privsize = 0;
    } else {
	req->privsize =	modeline->privsize;
	if (modeline->privsize) {
	    req->length += modeline->privsize;
	    Data32(dpy, (long *) modeline->private,
	           modeline->privsize * sizeof(INT32));
	}
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

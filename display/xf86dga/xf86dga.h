/* $XFree86: xc/include/extensions/xf86dga.h,v 3.5 1996/10/18 14:57:24 dawes Exp $ */
/*

Copyright (c) 1995  Jon Tombs
Copyright (c) 1995  XFree86 Inc

*/

#ifndef _XF86DGA_H_
#define _XF86DGA_H_

#include <X11/Xfuncproto.h>

#define X_XF86DGAQueryVersion		0
#define X_XF86DGAGetVideoLL		1
#define X_XF86DGADirectVideo		2
#define X_XF86DGAGetViewPortSize	3
#define X_XF86DGASetViewPort		4
#define X_XF86DGAGetVidPage		5
#define X_XF86DGASetVidPage		6
#define X_XF86DGAInstallColormap	7
#define X_XF86DGAQueryDirectVideo	8
#define X_XF86DGAViewPortChanged	9
#define X_XF86DGACopyArea		10
#define X_XF86DGAFillRectangle		11

#define XF86DGADirectPresent		0x0001
#define XF86DGADirectGraphics		0x0002
#define XF86DGADirectMouse		0x0004
#define XF86DGADirectKeyb		0x0008
#define XF86DGAHasColormap		0x0100
#define XF86DGADirectColormap		0x0200
#define XF86DGAAccelPresent		0x0400
#define XF86DGADoAccel			0x0800

#define XF86DGANumberEvents		0

#define XF86DGAClientNotLocal		0
#define XF86DGANoDirectVideoMode	1
#define XF86DGAScreenNotActive		2
#define XF86DGADirectNotActivated	3
#define XF86DGANumberErrors		(XF86DGADirectNotActivated + 1)

#endif /* _XF86DGA_H_ */

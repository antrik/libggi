/* $Id: types.h,v 1.1 2001/05/12 23:03:18 cegger Exp $
******************************************************************************

   LibGGI general definitions, data structures, etc.

   Copyright (C) 1995-1996	Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 1995-1996	Steffen Seeger	[seeger@ggi-project.org]
   Copyright (C) 1998		Marcus Sundberg	[marcus@ggi-project.org]
  
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

#ifndef	_GGI_TYPES_H
#define	_GGI_TYPES_H

#include <ggi/system.h>

/*
******************************************************************************
 LibGGI datatypes and structures
******************************************************************************
*/

#define        GGI_AUTO     (0)

typedef struct { sint16   x, y; }				ggi_coord;
typedef	uint32							ggi_pixel;

typedef	uint32							ggi_attr;
#define	ATTR_FGCOLOR	0x0000FF00	/* fgcolor clut index	*/
#define	ATTR_BGCOLOR	0x000000FF	/* bgcolor clut index	*/
#define	ATTR_NORMAL	0x00000000	/* normal style		*/
#define	ATTR_HALF	0x00010000	/* half intensity	*/
#define	ATTR_BRIGHT	0x00020000	/* high intensity	*/
#define	ATTR_INTENSITY	0x00030000	/* mask to get intensity*/
#define	ATTR_UNDERLINE	0x00040000	/* underline attribute	*/
#define	ATTR_BOLD	0x00080000	/* bold style		*/
#define	ATTR_ITALIC	0x00100000	/* italic style		*/
#define	ATTR_REVERSE	0x00200000	/* reverse fg/bg	*/
#define	ATTR_BLINK	0x00800000	/* enable blinking	*/
#define	ATTR_FONT	0xFF000000	/* font table		*/
#define	ATTR_COLOR(fg, bg)	((bg & 0xFF) | ((fg & 0xFF) << 8))

typedef struct { uint16 r,g,b,a; }				ggi_color;
typedef struct { uint16	size; ggi_color	*data; }		ggi_clut;

#define	GGI_COLOR_PRECISION 16	/* 16 bit per R,G, B value	*/

/*
 * Graphtypes
 */
typedef uint32 ggi_graphtype;

#define GT_DEPTH_SHIFT		(0)
#define GT_SIZE_SHIFT		(8)
#define GT_SUBSCHEME_SHIFT	(16)
#define GT_SCHEME_SHIFT		(24)

#define GT_DEPTH_MASK		(0xff << GT_DEPTH_SHIFT)
#define GT_SIZE_MASK		(0xff << GT_SIZE_SHIFT)
#define GT_SUBSCHEME_MASK	(0xff << GT_SUBSCHEME_SHIFT)
#define GT_SCHEME_MASK		(0xff << GT_SCHEME_SHIFT)

/* Macros to extract info from a ggi_graphtype. */
#define GT_DEPTH(x)		(((x) & GT_DEPTH_MASK) >> GT_DEPTH_SHIFT)
#define GT_SIZE(x)		(((x) & GT_SIZE_MASK) >> GT_SIZE_SHIFT)
#define GT_SUBSCHEME(x)		((x) & GT_SUBSCHEME_MASK)
#define GT_SCHEME(x)		((x) & GT_SCHEME_MASK)

/* Macros to set info in a ggi_graphtype. */
#define GT_SETDEPTH(gt,x) \
    do { (gt) = ((gt) & ~GT_DEPTH_MASK) | ((x)<<GT_DEPTH_SHIFT); } while (0)
#define GT_SETSIZE(gt,x) \
    do { (gt) = ((gt) & ~GT_SIZE_MASK) | ((x)<<GT_SIZE_SHIFT); } while (0)
#define GT_SETSUBSCHEME(gt,x) \
    do { (gt) = ((gt) & ~GT_SUBSCHEME_MASK) | (x); } while (0)
#define GT_SETSCHEME(gt,x) \
    do { (gt) = ((gt) & ~GT_SCHEME_MASK) | (x); } while (0)

/* Enumerated schemes */
#define GT_TEXT			((0x01) << GT_SCHEME_SHIFT)
#define GT_TRUECOLOR		((0x02) << GT_SCHEME_SHIFT)
#define GT_GREYSCALE		((0x03) << GT_SCHEME_SHIFT)
#define GT_PALETTE		((0x04) << GT_SCHEME_SHIFT)
#define GT_STATIC_PALETTE	((0x05) << GT_SCHEME_SHIFT)

/* Subschemes */
#define GT_SUB_REVERSE_ENDIAN   ((0x01) << GT_SUBSCHEME_SHIFT)
#define GT_SUB_HIGHBIT_RIGHT    ((0x02) << GT_SUBSCHEME_SHIFT)
#define GT_SUB_PACKED_GETPUT    ((0x04) << GT_SUBSCHEME_SHIFT)

/* Macro that constructs a graphtype */
#define GT_CONSTRUCT(depth,scheme,size) \
	((depth) | (scheme) | ((size) << GT_SIZE_SHIFT))

/* Common graphtypes */
#define GT_TEXT16	GT_CONSTRUCT(4, GT_TEXT, 16)
#define GT_TEXT32	GT_CONSTRUCT(8, GT_TEXT, 32)
#define GT_1BIT		GT_CONSTRUCT(1, GT_PALETTE, 1)
#define GT_2BIT		GT_CONSTRUCT(2, GT_PALETTE, 2)
#define GT_4BIT		GT_CONSTRUCT(4, GT_PALETTE, 4)
#define GT_8BIT		GT_CONSTRUCT(8, GT_PALETTE, 8)
#define GT_15BIT	GT_CONSTRUCT(15, GT_TRUECOLOR, 16)
#define GT_16BIT	GT_CONSTRUCT(16, GT_TRUECOLOR, 16)
#define GT_24BIT	GT_CONSTRUCT(24, GT_TRUECOLOR, 24)
#define GT_32BIT	GT_CONSTRUCT(24, GT_TRUECOLOR, 32)
#define GT_AUTO		(0)
#define GT_INVALID	(0xffffffff)

/*
 * ggi_mode structure
 */
typedef struct		/* requested by user and changed by driver    */
{
	sint32		frames;		/* frames needed		    */
	ggi_coord	visible;	/* vis. pixels, may change slightly */
	ggi_coord	virt;		/* virtual pixels, may change	    */
	ggi_coord	size;		/* size of visible in mm	    */
	ggi_graphtype	graphtype;	/* which mode ?			    */
	ggi_coord	dpp;		/* dots per pixel		    */
} ggi_mode;


/*
******************************************************************************
 LibGGI specific events
******************************************************************************
*/

#define GGI_CMDFLAG_LIBGGI (GII_CMDFLAG_EXTERNAL>>1)

/* Tell target that the application should not/should be halted when the
   display is unmapped.	The default is to halt the application.
*/
#define GGICMD_NOHALT_ON_UNMAP (GII_CMDFLAG_EXTERNAL | GGI_CMDFLAG_LIBGGI \
				| GII_CMDFLAG_NODATA | 0x01)
#define GGICMD_HALT_ON_UNMAP (GII_CMDFLAG_EXTERNAL | GGI_CMDFLAG_LIBGGI \
			      | GII_CMDFLAG_NODATA | 0x02)

/* Requests the application to switch target/mode, or to stop drawing on
   the visual.
   The latter is only sent if the application has explicitly requested
   GGICMD_NOHALT_ON_UNMAP. When a GGI_REQSW_UNMAP request is sent the
   application should respond by sending a GGICMD_ACKNOWLEDGE_SWITCH event
   as quickly as possible. After the acknowledge event is sent the
   application must not draw onto the visual until it recieves an evExpose
   event, which tells the application that the visual is mapped back again.
*/

#define GGICMD_REQUEST_SWITCH (GII_CMDFLAG_EXTERNAL \
			       | GGI_CMDFLAG_LIBGGI | 0x01)

/* Used for 'request' field in ggi_cmddata_switchrequest */
#define GGI_REQSW_UNMAP		0x01
#define GGI_REQSW_MODE		0x02
#define GGI_REQSW_TARGET	0x04

typedef struct {
	uint32		request;
	ggi_mode	mode;
	char		target[64];
} ggi_cmddata_switchrequest;


#define GGICMD_ACKNOWLEDGE_SWITCH (GII_CMDFLAG_EXTERNAL | GGI_CMDFLAG_LIBGGI \
				   | GII_CMDFLAG_NODATA | 0x03)

#endif /* _GGI_TYPES_H */

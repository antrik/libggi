/* $Id: mode.c,v 1.21 2007/01/18 12:42:37 pekberg Exp $
******************************************************************************

   LibGGI Mode management.

   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1998 Hartmut Niemann

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
#include <ggi/internal/gg_replace.h>
#include <ggi/internal/internal.h>
#include <ggi/internal/ggi_debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Static variables */
static ggi_mode _ggiDefaultMode =
{
	GGI_AUTO,               /* frames */
	{GGI_AUTO,GGI_AUTO},    /* visible size */
	{GGI_AUTO,GGI_AUTO},    /* virtual size */
	{GGI_AUTO,GGI_AUTO},    /* size in mm (don't care) */
	GT_AUTO,                /* graphtype */
	{GGI_AUTO,GGI_AUTO},    /* dots per pixel */
	/* 0 */
};


void _ggiSetDefaultMode(const char *str)
{
	ggiParseMode(str, &_ggiDefaultMode);
}


static void _ggiCheck4Defaults(ggi_mode *tm)
{
#define DOCHECK(what)  \
	if (tm->what == GGI_AUTO) tm->what=_ggiDefaultMode.what
	
	DOCHECK(frames);
	DOCHECK(visible.x);
	DOCHECK(visible.y);
	DOCHECK(virt.x);
	DOCHECK(virt.y);
	DOCHECK(dpp.x);
	DOCHECK(dpp.y);
	DOCHECK(graphtype);

#undef DOCHECK
}

/********************************/
/* set any mode (text/graphics) */
/********************************/

int ggiSetMode(ggi_visual_t v,ggi_mode *tm)
{ 
	struct ggi_visual *vis = GGI_VISUAL(v);
	int retval;
	ggi_mode oldmode;

	APP_ASSERT(vis != NULL, "ggiSetMode: vis == NULL");
	APP_ASSERT(tm != NULL,  "ggiSetMode: tm == NULL");

#ifdef DEBUG
	if ((_ggiDebug & DEBUG_CORE)
	    || (_ggiDebug & DEBUG_MODE)) {
		fprintf(stderr, "LibGGI: ggiSetMode(%p, ", (void *)vis);
		ggiFPrintMode(stderr, tm);
		fprintf(stderr, ") called\n");
	}
#endif
	ggLock(vis->mutex);

	DPRINT_MODE("ggiSetMode: trying (vis %dx%d virt %dx%d)\n",
		    tm->visible.x,tm->visible.y,tm->virt.x,tm->virt.y);
	_ggiCheck4Defaults(tm);
               
	memcpy(&oldmode, tm, sizeof(ggi_mode));
	DPRINT_MODE("ggiSetMode: trying2 (vis %dx%d virt %dx%d)\n",
		tm->visible.x,tm->visible.y,tm->virt.x,tm->virt.y);
	DPRINT_MODE("ggiSetMode: calling %p\n",vis->opdisplay->setmode);

	retval=vis->opdisplay->setmode(vis,tm);

	if (retval < 0) {
		fprintf(stderr, "LibGGI: Failed to set mode: ");
		ggiFPrintMode(stderr, &oldmode);
		fprintf(stderr, "\n");
	} else {
		int i;
		ggi_color col;

		DPRINT_CORE("ggiSetMode: set to frame 0, origin = {0,0}\n");
		ggiSetDisplayFrame(v, 0);
		ggiSetReadFrame(v, 0);
		ggiSetOrigin(v,0,0);
		DPRINT_CORE("ggiSetMode: set GC\n");
		/* Set clipping rectangle to the full (virtual) screen */
		ggiSetGCClipping(v,0,0,tm->virt.x,tm->virt.y);
		DPRINT_CORE("ggiSetMode: success (vis %dx%d virt %dx%d)\n",
			    tm->visible.x,tm->visible.y,tm->virt.x,tm->virt.y);
		/* Set foreground and background to black */
		col.r = 0;
		col.g = 0;
		col.b = 0;
		ggiSetGCForeground(v, ggiMapColor(v, &col));
		ggiSetGCBackground(v, ggiMapColor(v, &col));
		/* Clear frames to black */
		for (i = 0; i < tm->frames; i++) {
			DPRINT_CORE("ggiSetMode: SetWriteFrame %d\n", i);
			ggiSetWriteFrame(v, i);
#ifdef DEBUG
			if (vis->w_frame) {
				DPRINT_CORE("ggiSetMode: frame address: "
					       "%p\n", vis->w_frame->write);
			}
#endif
			DPRINT_CORE("ggiSetMode: FillScreen %d\n", i);
			ggiFillscreen(v);
		}
		ggiSetWriteFrame(v, 0);
		ggiFlush(v);
	}

	DPRINT_CORE("ggiSetMode: done!\n");
	ggUnlock(vis->mutex);

	return retval;
}

/**********************************/
/* check any mode (text/graphics) */
/**********************************/

int ggiCheckMode(ggi_visual_t v, ggi_mode *tm)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	APP_ASSERT(vis != NULL, "ggiCheckMode: vis == NULL");
	APP_ASSERT(tm != NULL,  "ggiCheckMode: tm == NULL");

	DPRINT_CORE("ggiCheckMode(%p, %p) called\n", vis, tm);

	_ggiCheck4Defaults(tm);
	return vis->opdisplay->checkmode(vis,tm);
}

/************************/
/* get the current mode */
/************************/

int ggiGetMode(ggi_visual_t v,ggi_mode *tm)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	APP_ASSERT(vis != NULL, "ggiGetMode: vis != NULL");
	APP_ASSERT(tm != NULL,  "ggiGetMode: tm != NULL");

	DPRINT_CORE("ggiGetMode(%p, %p) called\n", vis, tm);

	return vis->opdisplay->getmode(vis,tm);
}

/******************/
/* set a textmode */
/******************/
int ggiSetTextMode(ggi_visual_t v, int cols,int rows,
				   int vcols,int vrows,
				   int fontsizex,int fontsizey,
				   ggi_graphtype type)
{
	ggi_mode mode;
	
	DPRINT_CORE("ggiSetTextMode(%p, %d, %d, %d, %d, %d, %d, 0x%x) called\n",
		    v, cols, rows, vcols, vrows, fontsizex, fontsizey, type);
	
	mode.frames    = GGI_AUTO;
	mode.visible.x = cols;
	mode.visible.y = rows;
	mode.virt.x    = vcols;
	mode.virt.y    = vrows;
	mode.size.x    = mode.size.y = GGI_AUTO;
	mode.graphtype = type;
	mode.dpp.x     = fontsizex;
	mode.dpp.y     = fontsizey;
	
	return ggiSetMode(v,&mode);
}

/*************************/
/* check a text mode */
/*************************/
int ggiCheckTextMode(ggi_visual_t v, int cols,int rows,
				     int vcols,int vrows,
				     int fontsizex,int fontsizey,
				     ggi_graphtype type,
				     ggi_mode *md)
{
	int rc;
	ggi_mode mode;
        
	DPRINT_CORE("ggiCheckTextMode(%p, %d, %d, %d, %d, %d, %d, 0x%x, %p) called\n",
		    v, cols, rows, vcols, vrows, fontsizex, fontsizey,
		    type, md);
	
	mode.frames    = GGI_AUTO;
	mode.visible.x = cols;
	mode.visible.y = rows;
	mode.virt.x    = vcols;
	mode.virt.y    = vrows;
	mode.size.x    = mode.size.y = GGI_AUTO;
	mode.graphtype = type;
	mode.dpp.x     = fontsizex;
	mode.dpp.y     = fontsizey;

	rc = ggiCheckMode(v,&mode);
	if (md) *md = mode;	/* give back the mode if asked for. */
	return rc;
}

/***********************/
/* set a graphics mode */
/***********************/
int ggiSetGraphMode(ggi_visual_t v,int xsize,int ysize,
		    int xvirtual,int yvirtual,ggi_graphtype type)
{
	ggi_mode mode;
	DPRINT_CORE("ggiSetGraphMode(%p, %d, %d, %d, %d, 0x%x) called\n",
		    v, xsize, ysize, xvirtual, yvirtual, type);
	
	mode.frames    = GGI_AUTO;
	mode.visible.x = xsize;
	mode.visible.y = ysize;
	mode.virt.x    = xvirtual;
	mode.virt.y    = yvirtual;
	mode.size.x    = mode.size.y = GGI_AUTO;
	mode.graphtype = type;
	mode.dpp.x     = mode.dpp.y = GGI_AUTO;
	
	return ggiSetMode(v,&mode);
}

/*************************/
/* check a graphics mode */
/*************************/
int ggiCheckGraphMode(ggi_visual_t v,int xsize,int ysize,
		      int xvirtual,int yvirtual,ggi_graphtype type,
		      ggi_mode *md)
{
	int rc;
	ggi_mode mode;
	
	DPRINT_CORE("ggiCheckGraphMode(%p, %d, %d, %d, %d, 0x%x, %p) called\n",
		    v, xsize, ysize, xvirtual, yvirtual, type, md);
	
	mode.frames    = GGI_AUTO;
	mode.visible.x = xsize;
	mode.visible.y = ysize;
	mode.virt.x    = xvirtual;
	mode.virt.y    = yvirtual;
	mode.size.x    = mode.size.y = GGI_AUTO;
	mode.graphtype = type;
	mode.dpp.x     = mode.dpp.y = GGI_AUTO;

	rc = ggiCheckMode(v,&mode);
	if (md) *md = mode;	/* give back the mode if asked for. */
	return rc;
}


/*
  Set a graphics mode with frames
*/
int ggiSetSimpleMode(ggi_visual_t v, int xsize, int ysize, int frames,
		     ggi_graphtype type)
{
	ggi_mode mode;
	DPRINT_CORE("ggiSetSimpleMode(%p, %d, %d, %d, 0x%x) called\n",
		    v, xsize, ysize, frames, type);
	
	mode.frames    = frames;
	mode.visible.x = xsize;
	mode.visible.y = ysize;
	mode.virt.x    = mode.virt.y = GGI_AUTO;
	mode.size.x    = mode.size.y = GGI_AUTO;
	mode.graphtype = type;
	mode.dpp.x     = mode.dpp.y = GGI_AUTO;
	
	return ggiSetMode(v, &mode);
}

/*
  Check a graphics mode with frames
*/
int ggiCheckSimpleMode(ggi_visual_t visual, int xsize, int ysize, int frames,
		       ggi_graphtype type, ggi_mode *md)
{
	int rc;
	ggi_mode mode;
	
	DPRINT_CORE("ggiCheckSimpleMode(%p, %d, %d, %d, 0x%x, %p) called\n",
		    visual, xsize, ysize, frames, type, md);
	
	mode.frames    = frames;
	mode.visible.x = xsize;
	mode.visible.y = ysize;
	mode.virt.x    = mode.virt.y = GGI_AUTO;
	mode.size.x    = mode.size.y = GGI_AUTO;
	mode.graphtype = type;
	mode.dpp.x     = mode.dpp.y = GGI_AUTO;

	rc = ggiCheckMode(visual, &mode);
	if (md) *md = mode;	/* give back the mode if asked for. */
	return rc;
}

/*******************/
/* print mode      */
/*******************/

int ggiSPrintMode(char *s, const ggi_mode *m)
{
	int n;
	
	if (m->visible.x != GGI_AUTO || m->visible.y != GGI_AUTO) {
		sprintf(s, "%dx%d.%n", m->visible.x, m->visible.y, &n);
		s += n;
	}
	if (m->virt.x != GGI_AUTO || m->virt.y != GGI_AUTO) {
		sprintf(s, "V%dx%d.%n", m->virt.x, m->virt.y, &n);
		s += n;
	}
	if (m->frames != GGI_AUTO) {
		sprintf(s, "F%d.%n", m->frames, &n);
		s += n;
	}
	if (m->dpp.x != GGI_AUTO || m->dpp.y != GGI_AUTO) {
		sprintf(s, "D%dx%d.%n", m->dpp.x, m->dpp.y, &n);
		s += n;
	}
	
	*s++ = '[';

	switch (GT_SCHEME(m->graphtype)) {
		case GT_AUTO: break;
		case GT_TEXT:      *s++ = 'T'; break;
		case GT_TRUECOLOR: *s++ = 'C'; break;
		case GT_GREYSCALE: *s++ = 'K'; break;
		case GT_PALETTE:   *s++ = 'P'; break;
		default:           *s++ = '?'; break;
	}

	if (GT_DEPTH(m->graphtype) != GT_AUTO) {
		sprintf(s, "%u%n", GT_DEPTH(m->graphtype), &n);
		s += n;
	}
	if (GT_SIZE(m->graphtype) != GT_AUTO) {
		sprintf(s, "/%u%n", GT_SIZE(m->graphtype), &n);
		s += n;
	}

	if (GT_SUBSCHEME(m->graphtype) & GT_SUB_REVERSE_ENDIAN)
		*s++ = 'R';
	if (GT_SUBSCHEME(m->graphtype) & GT_SUB_HIGHBIT_RIGHT)
		*s++ = 'H';
	if (GT_SUBSCHEME(m->graphtype) & GT_SUB_PACKED_GETPUT)
		*s++ = 'G';

	*s++ = ']';  *s = 0;

	return 0;
}

int ggiFPrintMode(FILE *s, const ggi_mode *m)
{
	char buf[256];

	ggiSPrintMode(buf, m);

	return fprintf(s, buf);
}


/*******************/
/* parse mode      */
/*******************/

/*
 * format = size virt dpp frames graphtype.   (in any order)
 *
 * size = ['S'] X 'x' Y [ 'x' depth ]    
 * virt = 'V' X 'x' Y
 * dpp  = 'D' X 'x' Y
 * frames = 'F' frames
 * graphtype = '[' scheme [depth] ['/' size] [subscheme] ']'  |  scheme depth
 * scheme = 'C' | 'P' | 'K' | 'T'
 * subscheme = ['R'] ['H'] ['G']     (in any order)
 *
 * Anything and Everything (!) can be omitted, all ommitted values
 * default to GGI_AUTO (and GT_AUTO for the graphtype).  
 * Whitespace and '.' symbols ignored.  Case insensitive.
 *
 * Examples include:
 * 640x480           just a visible size
 * 640x480#640x960   same size, but double-height virtual screen
 * #1024x768         only virtual size defined
 *
 * 80x40[T]          (default-bitsized) text mode with 80x40 characters
 * #x100[T]          text mode with 100 virtual lines
 * 640x400[8]        640x400 at 8 bits per pixel
 * 640x400[GT_8BIT]  same as above, but palettized
 *
 * 320x200x15	     320x200 with 32768 colors
 * 320x200[C15]      320x200 with 32768 colors (hicolor)
 * 320x200[C/16]     320x200 with 16 bit pixels (also hicolor)
 * 320x200[C24/32]   320x200, 32 bit pixels, 16777216 colors (truecolor)
 * 320x200[GT_32BIT] same as above
 * 320x200[K2H]      320x200, 2 bit pixels, greyscale, high bit right
 *
 * The only way of specifying GGI_AUTO is omitting the parameter;
 *
 * Returncode:
 *  0 on success, i.e. the string was correct.
 *    ignored characters, like GT_ and a position information
 *    do not make it fail, and a missing ] from the bitdepth field
 *    is ignored, no failure
 * -1 if there is text that can not be parsed. 
 *    This text is printed to stderr.
 *    All parameters parsed so far are written into m!
 *
 * So m contains all parameters that have been successfully parsed.
 * For most applications there will be no need for testing parsemode
 * for failure.
 */

#define SKIPSPACE   while ((*s) && (isspace((uint8_t)*s) || (*s == '.'))) s++
 
/* scan the integer from the string pointer s */

#define SCANINT(x,def)                             \
	SKIPSPACE;  x = def;                       \
	if (isdigit((uint8_t) *s)) {                 \
		x = *s++ - '0';                    \
		while (isdigit((uint8_t) *s)) {      \
			x = x*10 + (*s++ - '0');   \
		}                                  \
	}                                          \
	SKIPSPACE

#define CHECKGTMODE(str,len,val)  \
	if (strncasecmp(s, str, len) == 0)  \
		{ m->graphtype = val; s += len; continue; }

int ggiParseMode(const char * s, ggi_mode * m)
{
	int depth;
	int subscheme;

	if (s == NULL) s = "";

	DPRINT_CORE("ggiParseMode(\"%s\", %p) called\n", s, m);

	*m = _ggiDefaultMode;

	while (*s) {

		SKIPSPACE;

		/* visible */
		if ((tolower((uint8_t)*s)=='s') || isdigit((uint8_t)*s)) {
			if (! isdigit((uint8_t)*s)) s++;
			SCANINT(m->visible.x, GGI_AUTO);
			if (tolower((uint8_t) *s) == 'x') {
				s++; SCANINT(m->visible.y, GGI_AUTO);
			}
			if (tolower((uint8_t) *s) == 'x') {
				s++; SCANINT(depth, GT_AUTO);
				GT_SETDEPTH(m->graphtype, depth);
			}
			continue;
		}
		
		/* virtual */
		if ((*s=='#') || (tolower((uint8_t) *s)=='v')) {
			s++; SCANINT(m->virt.x, GGI_AUTO);
			if (tolower((uint8_t) *s) == 'x') {
				s++; SCANINT(m->virt.y, GGI_AUTO);
			}
			continue;
		}

		/* frames */
		if (tolower((uint8_t) *s)=='f') {
			s++; SCANINT(m->frames, GGI_AUTO);
			continue;
		}

		if (tolower((uint8_t) *s)=='d') {  /* dpp */
			s++; SCANINT(m->dpp.x, GGI_AUTO);
			if (tolower((uint8_t) *s) == 'x') {
				s++; SCANINT(m->dpp.y, GGI_AUTO);
			}
			continue;
		}

		if (tolower((uint8_t) *s)=='p') {  /* palette */
			s++; SCANINT(depth, GT_AUTO);
			GT_SETSCHEME(m->graphtype, GT_PALETTE);
			GT_SETDEPTH(m->graphtype, depth);
			continue;
		}

		if (tolower((uint8_t) *s)=='c') {  /* truecolor */
			s++; SCANINT(depth, GT_AUTO);
			GT_SETSCHEME(m->graphtype, GT_TRUECOLOR);
			GT_SETDEPTH(m->graphtype, depth);
			continue;
		}

		if (tolower((uint8_t) *s)=='k') {  /* greyscale */
			s++; SCANINT(depth, GT_AUTO);
			GT_SETSCHEME(m->graphtype, GT_GREYSCALE);
			GT_SETDEPTH(m->graphtype, depth);
			continue;
		}

		if (tolower((uint8_t) *s)=='t') {  /* text */
			s++; SCANINT(depth, GT_AUTO);
			GT_SETSCHEME(m->graphtype, GT_TEXT);
			GT_SETDEPTH(m->graphtype, depth);
			continue;
		}

		if (*s != '[') {
			break;
		}

		s++;

		CHECKGTMODE("GT_1BIT]",   8,  GT_1BIT);
		CHECKGTMODE("GT_2BIT]",   8,  GT_2BIT);
		CHECKGTMODE("GT_4BIT]",   8,  GT_4BIT);
		CHECKGTMODE("GT_8BIT]",   8,  GT_8BIT);
		CHECKGTMODE("GT_15BIT]",  9,  GT_15BIT);
		CHECKGTMODE("GT_16BIT]",  9,  GT_16BIT);
		CHECKGTMODE("GT_24BIT]",  9,  GT_24BIT);
		CHECKGTMODE("GT_32BIT]",  9,  GT_32BIT);
		CHECKGTMODE("GT_TEXT16]", 10, GT_TEXT16);
		CHECKGTMODE("GT_TEXT32]", 10, GT_TEXT32);
	
		if (tolower((uint8_t) *s) == 't') {  /* text */
			GT_SETSCHEME(m->graphtype, GT_TEXT);
			s++;
		} else
		if (tolower((uint8_t) *s) == 'p') {  /* palette */
			GT_SETSCHEME(m->graphtype, GT_PALETTE);
			s++;
		} else
		if (tolower((uint8_t) *s) == 'c') {  /* truecolor */
			GT_SETSCHEME(m->graphtype, GT_TRUECOLOR);
			s++;
		} else
		if (tolower((uint8_t) *s) == 'k') {  /* greyscale */
			GT_SETSCHEME(m->graphtype, GT_GREYSCALE);
			s++;
		}
			
		SCANINT(depth, GT_AUTO);
		GT_SETDEPTH(m->graphtype, depth);

		if (*s == '/') {
			s++; SCANINT(depth, GT_AUTO);
			GT_SETSIZE(m->graphtype, depth);
		}

		subscheme = GT_AUTO;
next_subscheme:
		if (tolower((uint8_t) *s) == 'r') {  /* reverse endian */
			s++; SKIPSPACE;
			subscheme |= GT_SUB_REVERSE_ENDIAN;
			goto next_subscheme;
		}
		if (tolower((uint8_t) *s) == 'h') {  /* high bit right */
			s++; SKIPSPACE;
			subscheme |= GT_SUB_HIGHBIT_RIGHT;
			goto next_subscheme;
		}
		if (tolower((uint8_t) *s) == 'g') {  /* packed get/put */
			s++; SKIPSPACE;
			subscheme |= GT_SUB_PACKED_GETPUT;
			goto next_subscheme;
		}
		GT_SETSUBSCHEME(m->graphtype, subscheme);

		if (*s == ']') {
			s++;
		} else {
			fprintf(stderr,"ggiParseMode: missing `]' "
				"or bad graphtype\n.");
			break;
		}
	}

	if (*s) {
		fprintf(stderr, "ggiParseMode: trailing text `%s' "
			"ignored. Parsed mode is ", s);
		ggiFPrintMode(stderr, m);
		fprintf(stderr, "\n");
		return -1;
	}
	
	return 0;
}

#undef SKIPSPACE
#undef SCANINT
#undef CHECKGTMODE


#if 0   /* the ORIGINAL version */
int ggiParseMode(const char * s, ggi_mode * m)
{
	int bitdepth=0;
	int negative=0;  /* negative flag for positions */
	int xposition=0;
	int yposition=0;
	DPRINT_CORE("ggiParseMode(%p, %p) called\n", s, m);
	*m = _ggiDefaultMode;
#define SKIPSPACE   while ( (*s!='\000') && (isspace((uint8_t)*s)) ) s++;
/* scan the integer from the string pointer s */
#define SCANINT(x)  SKIPSPACE;                 \
		    if (isdigit((uint8_t)*s)){        \
		       x=*s-'0';               \
	               s++;                    \
		       while (isdigit((uint8_t)*s)){  \
			  x = x*10+ ((int)*s) -'0'; \
			  s++;                 \
		       }                       \
		    }                          \
		    SKIPSPACE
	/* first number is visible-x: */
	SCANINT(m->visible.x);
	if (tolower((uint8_t)*s) == 'x') { /* now for the y */
		s++;
		SCANINT(m->visible.y);
	}
	if (*s == '#'){ /* virtual starts here */
		s++;
		SCANINT(m->virt.x);
		if (tolower((uint8_t)*s) == 'x') { /* now for the y */
			s++;
			SCANINT(m->virt.y);
		}
	}
	if (tolower((uint8_t)*s) == 'd') { /* dpp starts here */
		s++;
		SCANINT(m->dpp.x);
		if (tolower((uint8_t)*s) == 'x') { /* now for the y */
			s++;
			SCANINT(m->dpp.y);
		}
	}
	if (tolower((uint8_t)*s) == 'f') { /* frames starts here */
		s++;
		SCANINT(m->frames);
	}
	if (*s == '[') { /* graphtype starts here */
		s++;
#define CHECKGTMODE(str,len,val)  \
	if (strncasecmp(s,(str),(len)) == 0)  \
		{ m->graphtype = (val); s += len; }
		CHECKGTMODE("GT_1BIT]",   8,  GT_1BIT)   else
		CHECKGTMODE("GT_2BIT]",   8,  GT_2BIT)   else
		CHECKGTMODE("GT_4BIT]",   8,  GT_4BIT)   else
		CHECKGTMODE("GT_8BIT]",   8,  GT_8BIT)   else
		CHECKGTMODE("GT_15BIT]",  9,  GT_15BIT)  else
		CHECKGTMODE("GT_16BIT]",  9,  GT_16BIT)  else
		CHECKGTMODE("GT_24BIT]",  9,  GT_24BIT)  else
		CHECKGTMODE("GT_32BIT]",  9,  GT_32BIT)  else
		CHECKGTMODE("GT_TEXT16]", 10, GT_TEXT16) else
		CHECKGTMODE("GT_TEXT32]", 10, GT_TEXT32) else
		{
			/* scheme */
			if (tolower((uint8_t)*s) == 't') {  /* text mode */
				GT_SETSCHEME(m->graphtype, GT_TEXT);
				s++;
			} else
			if (tolower((uint8_t)*s) == 'p') {  /* palette mode */
				GT_SETSCHEME(m->graphtype, GT_PALETTE);
				s++;
			} else
			if (tolower((uint8_t)*s) == 'c') {  /* truecolor mode */
				GT_SETSCHEME(m->graphtype, GT_TRUECOLOR);
				s++;
			} else
			if (tolower((uint8_t)*s) == 'k') {  /* greyscale mode */
				GT_SETSCHEME(m->graphtype, GT_GREYSCALE);
				s++;
			}
			bitdepth = GT_AUTO;
			SCANINT(bitdepth);
			GT_SETDEPTH(m->graphtype, bitdepth);
			if (*s == '/') {
				s++;
				bitdepth = GT_AUTO;
				SCANINT(bitdepth);
				GT_SETSIZE(m->graphtype, bitdepth);
			}
			if (*s == ']') {
				s++;
			} else {
				fprintf(stderr,"ggiParseMode: warning: ] "
					"missing or bad graphtype\n.");
			}
		}
#undef CHECKGTMODE
	}
	if ((*s=='-') || (*s=='+')){ /* x position starts */
		if (*s=='-'){
			negative=1;
		}
		s++;
		SCANINT(xposition);
		if (negative){
			negative=0;
			xposition = - xposition;
		}
		fprintf(stderr,"X position %d ignored.\n",xposition);
	}
       	if ((*s=='-') || (*s=='+')){ /* y position starts */
		if (*s=='-'){
			negative=1;
		}
		s++;
		SCANINT(yposition);
		if (negative){
			negative=0;
			yposition = - yposition;
		}
		fprintf(stderr,"Y position %d ignored.\n",yposition);
	}
	if (*s !='\000'){
		fprintf(stderr,"trailing text %s ignored.\n"
			"parsed mode is ",s);
		ggiFPrintMode(stderr,m);
		fprintf(stderr,"\n");
		return -1;
	}
#undef SCANINT
	return 0;
}
#endif


/***************************************/
/* PHYsical SiZe (physz) handling      */
/***************************************/

int _ggi_physz_parse_option(const char *optstr, int *physzflag, ggi_coord *physz)
{
	/* This function parses a string gotten through the -physz= option,
	 * contained in optstr, and fills out the values physzflag and physz 
	 * based on what is in that string.  The latter two are stored in
	 * the visual's target private area (not all targets use the -physz
	 * option.)
	 *
	 * physz gets the integer values in the option string. physzflag can 
	 * contain two flags, one (GGI_PHYSZ_OVERRIDE) designating that the 
	 * values are not defaults, rather overrides for a misconfigured 
	 * display.  The second, GGI_PHYSZ_DPI designates that the sizes
	 * in the string are in dots-per-inch, otherwise the sizes are
	 * assumed to be the full size of the display  in millimeters. 
	 * (which may not be the same as the size of the visual, on targets 
	 * where the visual is a subregion of a display system such as X).
	 */

	char *endptr;
	const char *nptr = optstr;

	*physzflag = 0;
	physz->x =physz->y = GGI_AUTO;

	/* The 'N' is there by default, if the option was not filled in. */
	if (*nptr == 'N' || *nptr == 'n') return GGI_OK;

	/* Check if we should *always* override the display system values */
	if (*nptr == '=') {
		nptr++;
		*physzflag |= GGI_PHYSZ_OVERRIDE;
	}

	physz->x = strtoul(nptr, &endptr, 0);

	if (*nptr == '\0' || *endptr != ',') {
		*physzflag = 0;
		physz->x = physz->y = GGI_AUTO;
		return GGI_EARGINVAL;
	}

	nptr = endptr + 1;

	physz->y = strtoul(nptr, &endptr, 0);

	if (*nptr != '\0' && 
	    (*endptr == 'd' || *endptr == 'D') && 
	    (*(endptr + 1) == 'p' || *(endptr + 1) == 'P') && 
	    (*(endptr + 2) == 'i' || *(endptr + 2) == 'I'))
	{
		endptr += 3;
		*physzflag |= GGI_PHYSZ_DPI;
	}

	if (*nptr == '\0' || *endptr != '\0') {
		*physzflag = 0;
		physz->x =physz->y = GGI_AUTO;
		return GGI_EARGINVAL;
	}

	return GGI_OK;
}


int _ggi_physz_figure_visible(ggi_mode *mode, int def_x, int def_y,
				int physzflag, const ggi_coord *screen_size,
				const ggi_coord *screen_res)
{
	/* This function validates/suggests values in mode->size to
	 * designate the physical screen size in millimeters.
	 *
	 * mode->dpp is assumed to already contain valid values.
	 * def_x and def_y are the default visible sizes from the target
	 * private area
	 *
	 * screen_size is the screen size in dpi, if physzflag is
	 * GGI_PHYSZ_DPI, otherwise screen_size is in mm.
	 *
	 * screen_res is the screen size in pixels or 0 on fullscreen
	 * targets.
	 */

	ggi_coord size, res;
	ggi_mode tmp;

	DPRINT_MODE("_ggi_physz_figure_visible(%p) called\n",
			mode);

	LIB_ASSERT(mode != NULL, "Invalid mode");
	LIB_ASSERT(screen_size != NULL, "Invalid screen size");
	LIB_ASSERT(screen_res != NULL, "Invalid screen resolution");

	memset(&tmp, GGI_AUTO, sizeof(tmp));

	size = *screen_size;
	res = *screen_res;

	if ((mode->visible.x == GGI_AUTO)
	   && (mode->virt.x == GGI_AUTO)
	   && (mode->size.x == GGI_AUTO))
	{
		tmp.visible.x = tmp.virt.x = def_x;

	} else if ((mode->visible.x == GGI_AUTO) && (mode->virt.x == GGI_AUTO)) {
		if (size.x == GGI_AUTO) size.x = mode->size.x;
		if (res.x == GGI_AUTO) res.x = def_x;

		if (physzflag & GGI_PHYSZ_DPI) {
			tmp.visible.x = (mode->size.x * 254 / 10) * size.x / mode->dpp.x;
		} else {
			tmp.visible.x = mode->size.x * res.x / size.x;
		}
	} else if (mode->visible.x == GGI_AUTO) {
		tmp.visible.x = mode->virt.x;

	} else if (mode->virt.x == GGI_AUTO) {
		tmp.virt.x = mode->visible.x;
	}


	if ((mode->visible.y == GGI_AUTO)
	   && (mode->virt.y == GGI_AUTO)
	   && (mode->size.y == GGI_AUTO))
	{
		tmp.visible.y = tmp.virt.y = def_y;

	} else if ((mode->visible.y == GGI_AUTO) && (mode->virt.y == GGI_AUTO)) {
		if (size.y == GGI_AUTO) size.y = mode->size.y;
		if (res.y == GGI_AUTO) res.y = def_y;

		if (physzflag & GGI_PHYSZ_DPI) {
			tmp.visible.y = (mode->size.y * 254 / 10) * size.y / mode->dpp.y;
		} else {
			tmp.visible.y = mode->size.y * res.y / size.y;
		}
	} else if (mode->visible.y == GGI_AUTO) {
		tmp.visible.y = mode->virt.y;

	} else if (mode->virt.y == GGI_AUTO) {
		tmp.virt.y = mode->visible.y;
	}


	DPRINT_MODE("_ggi_physz_figure_visible: mode dpp (%i,%i), size (%i,%i)\n",
			mode->dpp.x, mode->dpp.y,
			mode->size.x, mode->size.y);

	DPRINT_MODE("_ggi_physz_figure_visible: visible (%i,%i), virt (%i,%i)\n",
			tmp.visible.x, tmp.visible.y, mode->virt.x, mode->virt.y);

	if ((mode->virt.x != GGI_AUTO)
	  && (tmp.visible.x > mode->virt.x))
	{
		tmp.visible.x = mode->virt.x;
	}
	if ((mode->virt.y != GGI_AUTO)
	  && (tmp.visible.y > mode->virt.y))
	{
		tmp.visible.y = mode->virt.y;
	}

	if (tmp.visible.x <= 0) tmp.visible.x = 0;
	if (tmp.visible.y <= 0) tmp.visible.y = 0;


	if ((mode->visible.x != GGI_AUTO && mode->visible.x != tmp.visible.x)
	  || (mode->visible.y != GGI_AUTO && mode->visible.y != tmp.visible.y))
	{
		DPRINT_MODE("_ggi_physz_figure_visible: "
			"physical size (%i,%i) doesn't match (%i,%i)\n",
			mode->size.x, mode->size.y, mode->visible.x, mode->visible.y);

		return GGI_ENOMATCH;
	}


	mode->visible = tmp.visible;
	mode->virt = tmp.virt;

	DPRINT_MODE("_ggi_physz_figure_visible: visible (%i,%i), virt (%i,%i)\n",
			mode->visible.x, mode->visible.y, mode->virt.x, mode->virt.y);
	DPRINT_MODE("_ggi_physz_figure_visible: leaving\n");

	return GGI_OK;
}


int _ggi_physz_figure_size(ggi_mode *mode, int physzflag, const ggi_coord *op_sz, 
			int dpix, int dpiy, int dsx, int dsy)
{

	/* This function validates/suggests values in mode->size to
	 * designate the physical screen size in millimeters.
	 *
	 * mode->visible and mode->dpp are assumed to already contain 
	 * valid values.
	 *
	 * The physzflag and op_sz parameters are from the visual's 
	 * target private area, as set by the above _ggi_physz_parse_option
	 * function.
	 *
	 * The dpix, dpiy parameters contain the dpi of the display.
	 *
	 * The dsx, dsy parameters contain the size in pixels of the
	 * entire display, which on visuals using a subregion of 
	 * a display system, such as X, is the size of the entire screen.
	 */

	long xsize, ysize;
	int err = GGI_OK;

	xsize = ysize = 0;

	if (physzflag & GGI_PHYSZ_DPI) {
		xsize = (physzflag & GGI_PHYSZ_OVERRIDE) ? op_sz->x : dpix;
		ysize = (physzflag & GGI_PHYSZ_OVERRIDE) ? op_sz->y : dpiy;
		if (xsize <= 0 || ysize <= 0) {
			xsize = op_sz->x;
			ysize = op_sz->y;
		}
		if (xsize <= 0 || ysize <= 0) goto nosize;
		/* find absolute size in mm */
		xsize = mode->visible.x * mode->dpp.x * 254 / xsize / 10;
		ysize = mode->visible.y * mode->dpp.y * 254 / ysize / 10;

	} else if (physzflag & GGI_PHYSZ_MM) {
		xsize = (physzflag & GGI_PHYSZ_OVERRIDE) ? op_sz->x : dpix;
		ysize = (physzflag & GGI_PHYSZ_OVERRIDE) ? op_sz->y : dpiy;
		if (xsize <= 0 || ysize <= 0) {
			xsize = op_sz->x;
			ysize = op_sz->y;
		}
		if (xsize <= 0 || ysize <= 0) goto nosize;
		/* Now xsize and ysize are in mm, but scale them
		 * to mode->visible */
		xsize = xsize * mode->visible.x / dsx;
		ysize = ysize * mode->visible.y / dsy;

	} else {
		if (physzflag & GGI_PHYSZ_OVERRIDE) {
			xsize = op_sz->x;
			ysize = op_sz->y;
		} else if (dpix > 0 && dpiy > 0) {
			xsize = (dsx * mode->dpp.x * 254 / dpix / 10);
			ysize = (dsy * mode->dpp.y * 254 / dpiy / 10);
		}
		if (xsize <= 0 || ysize <= 0) {
			xsize = op_sz->x;
			ysize = op_sz->y;
		}
		if (xsize <= 0 || ysize <= 0) goto nosize;
		if (dsx <= 0 || dsy <= 0) goto nosize;
		xsize = xsize * mode->visible.x / dsx;
		ysize = ysize * mode->visible.y / dsy;
	}

	if ((mode->size.x != xsize && mode->size.x != GGI_AUTO) ||
	    (mode->size.y != ysize && mode->size.y != GGI_AUTO))
	{
		DPRINT_MODE("_ggi_physz_figure_size: "
			"physical size (%i,%i) doesn't match (%i,%i)\n",
			xsize, ysize, mode->size.x, mode->size.y);
		err = GGI_ENOMATCH;
	}

	mode->size.x = (int)xsize;
	mode->size.y = (int)ysize;

	return err;

 nosize:
	if ((mode->size.x != GGI_AUTO) || (mode->size.y != GGI_AUTO))
		err = GGI_ENOMATCH;
	return err;
}

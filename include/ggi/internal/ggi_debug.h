/* $Id: ggi_debug.h,v 1.3 2004/10/31 13:11:05 cegger Exp $
******************************************************************************

   LibGGI debugging macros
   
   Copyright (C) 2004      Eric Faurot	        [eric.faurot@info.unicaen.fr]
   Copyright (C) 1998-1999 Marcus Sundberg	[marcus@ggi-project.org]
   
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

#ifndef _GGI_INTERNAL_GGI_DEBUG_H
#define _GGI_INTERNAL_GGI_DEBUG_H

#include <stdarg.h>
#include <stdio.h>
#include <ggi/types.h>		/* needed for uint32 */
#include <ggi/ggi-defs.h>	/* needed for GGIAPIVAR */

__BEGIN_DECLS

/* Exported variables */
GGIAPIVAR uint32     _ggiDebug;

__END_DECLS


/* Debugging types
 * bit 0 is reserved! */

#define GGIDEBUG_CORE		(1<<1)	/*   2 */
#define GGIDEBUG_MODE		(1<<2)	/*   4 */
#define GGIDEBUG_COLOR		(1<<3)	/*   8 */
#define GGIDEBUG_DRAW		(1<<4)	/*  16 */
#define GGIDEBUG_MISC		(1<<5)	/*  32 */
#define GGIDEBUG_LIBS		(1<<6)	/*  64 */
#define GGIDEBUG_EVENTS		(1<<7)	/* 128 */

#define GGIDEBUG_ALL	 0x7fffffff
#define GGIDEBUG_SYNC	 0x80000000

#define GGIDEBUG_ISSYNC  (_ggiDebug&GGIDEBUG_SYNC)

#ifdef __GNUC__

#ifdef DEBUG
#define GGIDPRINT(args...)        if (_ggiDebug & GGIDEBUG_ALL)    ggDPrintf(GGIDEBUG_ISSYNC,"LibGGI",args)
#define GGIDPRINT_CORE(args...)   if (_ggiDebug & GGIDEBUG_CORE)   ggDPrintf(GGIDEBUG_ISSYNC,"LibGGI",args)
#define GGIDPRINT_MODE(args...)   if (_ggiDebug & GGIDEBUG_MODE)   ggDPrintf(GGIDEBUG_ISSYNC,"LibGGI",args)
#define GGIDPRINT_COLOR(args...)  if (_ggiDebug & GGIDEBUG_COLOR)  ggDPrintf(GGIDEBUG_ISSYNC,"LibGGI",args)
#define GGIDPRINT_DRAW(args...)   if (_ggiDebug & GGIDEBUG_DRAW)   ggDPrintf(GGIDEBUG_ISSYNC,"LibGGI",args)
#define GGIDPRINT_MISC(args...)   if (_ggiDebug & GGIDEBUG_MISC)   ggDPrintf(GGIDEBUG_ISSYNC,"LibGGI",args)
#define GGIDPRINT_LIBS(args...)   if (_ggiDebug & GGIDEBUG_LIBS)   ggDPrintf(GGIDEBUG_ISSYNC,"LibGGI",args)
#define GGIDPRINT_EVENTS(args...) if (_ggiDebug & GGIDEBUG_EVENTS) ggDPrintf(GGIDEBUG_ISSYNC,"LibGGI",args)
#else /* DEBUG */
#define GGIDPRINT(args...)		do{}while(0)
#define GGIDPRINT_CORE(args...)		do{}while(0)
#define GGIDPRINT_MODE(args...)		do{}while(0)
#define GGIDPRINT_COLOR(args...)	do{}while(0)
#define GGIDPRINT_DRAW(args...)		do{}while(0)
#define GGIDPRINT_MISC(args...)		do{}while(0)
#define GGIDPRINT_LIBS(args...)		do{}while(0)
#define GGIDPRINT_EVENTS(args...)	do{}while(0)
#endif /* DEBUG */

#else /* __GNUC__ */

__BEGIN_DECLS

#ifdef DEBUG
#define GGIDPRINTIF(mask) do {                   \
    if (_ggiDebug & mask) {                      \
	    va_list args;                        \
	    fprintf(stderr, "LibGGI: ");         \
	    va_start(args, form);                \
	    vfprintf(stderr, form, args);        \
	    va_end(args);                        \
	    if (GGIDEBUG_ISSYNC) fflush(stderr); \
    }                                            \
} while(0)
#else /* DEBUG */
#define GGIDPRINTIF(mask)  do{}while(0)
#endif  /* DEBUG */

static inline void GGIDPRINT(const char *form,...)        { GGIDPRINTIF(GGIDEBUG_ALL);    }
static inline void GGIDPRINT_CORE(const char *form,...)   { GGIDPRINTIF(GGIDEBUG_CORE);   }
static inline void GGIDPRINT_MODE(const char *form,...)   { GGIDPRINTIF(GGIDEBUG_MODE);   }
static inline void GGIDPRINT_COLOR(const char *form,...)  { GGIDPRINTIF(GGIDEBUG_COLOR);  }
static inline void GGIDPRINT_DRAW(const char *form,...)   { GGIDPRINTIF(GGIDEBUG_DRAW);   }
static inline void GGIDPRINT_MISC(const char *form,...)   { GGIDPRINTIF(GGIDEBUG_MISC);   }
static inline void GGIDPRINT_LIBS(const char *form,...)   { GGIDPRINTIF(GGIDEBUG_LIBS);   }
static inline void GGIDPRINT_EVENTS(const char *form,...) { GGIDPRINTIF(GGIDEBUG_EVENTS); }

__END_DECLS

#endif /* __GNUC__ */

#ifdef DEBUG
#define LIBGGI_ASSERT(x,str) \
{ if (!(x)) { \
	fprintf(stderr,"LIBGGI:%s:%d: INTERNAL ERROR: %s\n",__FILE__,__LINE__,str); \
	exit(1); \
} }
#define LIBGGI_APPASSERT(x,str) \
{ if (!(x)) { \
	fprintf(stderr,"LIBGGI:%s:%d: APPLICATION ERROR: %s\n",__FILE__,__LINE__,str); \
	exit(1); \
} }
#else /* DEBUG */
#define LIBGGI_ASSERT(x,str)	do{}while(0)
#define LIBGGI_APPASSERT(x,str)	do{}while(0)
#endif /* DEBUG */

#ifdef DEBUG
# define GGID0(x)	x
#else
# define GGID0(x)	/* empty */
#endif

#ifdef GGIDLEV
# if GGIDLEV == 1
#  define GGID1(x)	x
#  define GGID2(x)	/* empty */
#  define GGID3(x)	/* empty */
# elif GGIDLEV == 2
#  define GGID1(x)	x
#  define GGID2(x)	x
#  define GGID3(x)	/* empty */
# elif GGIDLEV > 2
#  define GGID1(x)	x
#  define GGID2(x)	x
#  define GGID3(x)	x
# endif
#else
# define GGID1(x)	/* empty */
# define GGID2(x)	/* empty */
# define GGID3(x)	/* empty */
#endif

#endif /* _GGI_INTERNAL_GGI_DEBUG_H */

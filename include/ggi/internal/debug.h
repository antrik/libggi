/* $Id: debug.h,v 1.2 2002/10/09 21:53:55 cegger Exp $
******************************************************************************

   LibGGI debugging macros

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

#ifndef _GGI_INTERNAL_DEBUG_H
#define _GGI_INTERNAL_DEBUG_H

#include <stdio.h>
#include <stdarg.h>
#include <ggi/types.h>
#include <ggi/gg.h>


__BEGIN_DECLS

/* Exported variables */
#ifdef BUILDING_LIBGGI
extern uint32     _ggiDebugState;
extern int        _ggiDebugSync;
#else
IMPORTVAR uint32  _ggiDebugState;
IMPORTVAR int     _ggiDebugSync;
#endif

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

#define GGIDEBUG_ALL	0xffffffff

#ifdef __GNUC__

#ifdef DEBUG
#define GGIDPRINT(args...)        if (_ggiDebugState) { ggDPrintf(_ggiDebugSync,"LibGGI",args); }
#define GGIDPRINT_CORE(args...)   if (_ggiDebugState & GGIDEBUG_CORE) { ggDPrintf(_ggiDebugSync,"LibGGI",args); }
#define GGIDPRINT_MODE(args...)   if (_ggiDebugState & GGIDEBUG_MODE) { ggDPrintf(_ggiDebugSync,"LibGGI",args); }
#define GGIDPRINT_COLOR(args...)  if (_ggiDebugState & GGIDEBUG_COLOR) { ggDPrintf(_ggiDebugSync,"LibGGI",args); }
#define GGIDPRINT_DRAW(args...)   if (_ggiDebugState & GGIDEBUG_DRAW) { ggDPrintf(_ggiDebugSync,"LibGGI",args); }
#define GGIDPRINT_MISC(args...)   if (_ggiDebugState & GGIDEBUG_MISC) { ggDPrintf(_ggiDebugSync,"LibGGI",args); }
#define GGIDPRINT_LIBS(args...)   if (_ggiDebugState & GGIDEBUG_LIBS) { ggDPrintf(_ggiDebugSync,"LibGGI",args); }
#define GGIDPRINT_EVENTS(args...) if (_ggiDebugState & GGIDEBUG_EVENTS) { ggDPrintf(_ggiDebugSync,"LibGGI",args); }
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

static inline void GGIDPRINT(const char *form,...)
{
#ifdef DEBUG
	if (_ggiDebugState) {
		va_list args;

		fprintf(stderr, "LibGGI: ");
		va_start(args, form);
		vfprintf(stderr, form, args);
		va_end(args);
		if (_ggiDebugSync) fflush(stderr);
	}
#endif
}

static inline void GGIDPRINT_CORE(const char *form,...)
{
#ifdef DEBUG
	if (_ggiDebugState & GGIDEBUG_CORE) {
		va_list args;

		fprintf(stderr, "LibGGI: ");
		va_start(args, form);
		vfprintf(stderr, form, args);
		va_end(args);
		if (_ggiDebugSync) fflush(stderr);
	}
#endif
}

static inline void GGIDPRINT_MODE(const char *form,...)
{
#ifdef DEBUG
	if (_ggiDebugState & GGIDEBUG_MODE) {
		va_list args;

		fprintf(stderr, "LibGGI: ");
		va_start(args, form);
		vfprintf(stderr, form, args);
		va_end(args);
		if (_ggiDebugSync) fflush(stderr);
	}
#endif
}

static inline void GGIDPRINT_COLOR(const char *form,...)
{
#ifdef DEBUG
	if (_ggiDebugState & GGIDEBUG_COLOR) {
		va_list args;

		fprintf(stderr, "LibGGI: ");
		va_start(args, form);
		vfprintf(stderr, form, args);
		va_end(args);
		if (_ggiDebugSync) fflush(stderr);
	}
#endif
}

static inline void GGIDPRINT_DRAW(const char *form,...)
{
#ifdef DEBUG
	if (_ggiDebugState & GGIDEBUG_DRAW) {
		va_list args;

		fprintf(stderr, "LibGGI: ");
		va_start(args, form);
		vfprintf(stderr, form, args);
		va_end(args);
		if (_ggiDebugSync) fflush(stderr);
	}
#endif
}

static inline void GGIDPRINT_MISC(const char *form,...)
{
#ifdef DEBUG
	if (_ggiDebugState & GGIDEBUG_MISC) {
		va_list args;

		fprintf(stderr, "LibGGI: ");
		va_start(args, form);
		vfprintf(stderr, form, args);
		va_end(args);
		if (_ggiDebugSync) fflush(stderr);
	}
#endif
}

static inline void GGIDPRINT_LIBS(const char *form,...)
{
#ifdef DEBUG
	if (_ggiDebugState & GGIDEBUG_LIBS) {
		va_list args;

		fprintf(stderr, "LibGGI: ");
		va_start(args, form);
		vfprintf(stderr, form, args);
		va_end(args);
		if (_ggiDebugSync) fflush(stderr);
	}
#endif
}

static inline void GGIDPRINT_EVENTS(const char *form,...)
{
#ifdef DEBUG
	if (_ggiDebugState & GGIDEBUG_EVENTS) {
		va_list args;

		fprintf(stderr, "LibGGI: ");
		va_start(args, form);
		vfprintf(stderr, form, args);
		va_end(args);
		if (_ggiDebugSync) fflush(stderr);
	}
#endif
}

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

#endif /* _GGI_INTERNAL_DEBUG_H */

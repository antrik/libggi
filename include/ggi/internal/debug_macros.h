/*
******************************************************************************

   Debug macros definition

   Copyright (C) 2004      Eric Faurot	        [eric.faurot@gmail.com]
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

/**
 * Important: Do _not_ modify this file directly. It is imported from
 * "tools/buildframework/include/ggi/internal/debug_macros.h".
 * Change that file instead.
 */

#include <stdio.h>
#include <stdarg.h>

#define DEBUG_ALL	0x0fffffff /* Allowed fields mask  */
#define DEBUG_ENABLED	0x80000000 /* debug enabled        */
#define DEBUG_SYNC	0x40000000 /* debug sync           */
#define DEBUG_LEVEL     0x30000000 /* debug level mask     */

#define DEBUG_0         0x00000000
#define DEBUG_1         0x10000000
#define DEBUG_2         0x20000000
#define DEBUG_3         0x30000000

#define DEBUG_NOTICE   " "
#define DEBUG_WARNING  "!! Warning: "
#define DEBUG_ERROR    "** Error: "
#define DEBUG_CRITICAL "*** Critical: "

#define DEBUG_INFO	 __FILE__,__PRETTY_FUNCTION__,__LINE__

#ifdef  DEBUG

#ifndef DLEVEL
# define DLEVEL 3
#endif

#define DMESSAGE(state, severity, level, field) do {                         \
    if ((state&field) && ((state&DEBUG_LEVEL) >= level)) {                   \
	    va_list args;                                                    \
	    fprintf(stderr, "[" DEBUG_NAMESPACE "] " severity);              \
	    va_start(args, form);                                            \
	    vfprintf(stderr, form, args);                                    \
	    va_end(args);                                                    \
	    if (state & DEBUG_SYNC) fflush(stderr);                          \
    }                                                                        \
} while(0)

/*
static inline void _dmessage(unsigned int state,
			     const char * severity,
			     unsigned int level,
			     unsigned int fields,
			     const char * form, ...) {
	if ((state&field) && ((state&DEBUG_LEVEL) >= level)) {
		va_list args;
		fprintf(stderr, "[" DEBUG_NAMESPACE "] %s", severity);
		va_start(args, form);
		vfprintf(stderr, form, args);
		va_end(args);
		if (state & DEBUG_SYNC) fflush(stderr);
	}
}
*/

#if DLEVEL >= 0
# define DNOTICE0(state, field)  DMESSAGE(state, DEBUG_NOTICE,  DEBUG_0, field)
# define DWARNING0(state, field) DMESSAGE(state, DEBUG_WARNING, DEBUG_0, field)
# define DERROR0(state, field)   DMESSAGE(state, DEBUG_ERROR,   DEBUG_0, field)
#else
# define DNOTICE0(state, field)  do{}while(0)
# define DWARNING0(state, field) do{}while(0)
# define DERROR0(state, field)   do{}while(0)
#endif /* DLEVEL 0 */
#if DLEVEL >= 1
# define DNOTICE1(state, field)  DMESSAGE(state, DEBUG_NOTICE,  DEBUG_1, field)
# define DWARNING1(state, field) DMESSAGE(state, DEBUG_WARNING, DEBUG_1, field)
# define DERROR1(state, field)   DMESSAGE(state, DEBUG_ERROR,   DEBUG_1, field)
#else
# define DNOTICE1(state, field)  do{}while(0)
# define DWARNING1(state, field) do{}while(0)
# define DERROR1(state, field)   do{}while(0)
#endif /* DLEVEL 1 */
#if DLEVEL >= 2
# define DNOTICE2(state, field)  DMESSAGE(state, DEBUG_NOTICE,  DEBUG_2, field)
# define DWARNING2(state, field) DMESSAGE(state, DEBUG_WARNING, DEBUG_2, field)
# define DERROR2(state, field)   DMESSAGE(state, DEBUG_ERROR,   DEBUG_2, field)
#else
# define DNOTICE2(state, field)  do{}while(0)
# define DWARNING2(state, field) do{}while(0)
# define DERROR2(state, field)   do{}while(0)
#endif /* DLEVEL 2 */
#if DLEVEL >= 3
# define DNOTICE3(state, field)  DMESSAGE(state, DEBUG_NOTICE,  DEBUG_3, field)
# define DWARNING3(state, field) DMESSAGE(state, DEBUG_WARNING, DEBUG_3, field)
# define DERROR3(state, field)   DMESSAGE(state, DEBUG_ERROR,   DEBUG_3, field)
#else
# define DNOTICE3(state, field)  do{}while(0)
# define DWARNING3(state, field) do{}while(0)
# define DERROR3(state, field)   do{}while(0)
#endif /* DLEVEL 3 */

/* This macro will eventually be renamed... */
/* #define DPRINTIF DNOTICE0 */

#define LIB_ASSERT(cond, msg) do {                                           \
    if (!(cond)) {                                                           \
	fprintf(stderr,                                                      \
		"[" DEBUG_NAMESPACE "] %s:%s:%d: INTERNAL ERROR: %s\n",      \
		DEBUG_INFO, msg);                                            \
	exit(1); }                                                           \
} while(0)


#define APP_ASSERT(cond, msg) do {                                           \
    if (!(cond)) {                                                           \
	fprintf(stderr,                                                      \
		"[" DEBUG_NAMESPACE "] %s:%s:%d: APPLICATION ERROR: %s\n",   \
		DEBUG_INFO, msg);                                            \
	exit(1); }                                                           \
} while(0)

/* */

#else   /* DEBUG */

#define DNOTICE0(state, field)  do{}while(0)
#define DWARNING0(state, field) do{}while(0)
#define DERROR0(state, field)   do{}while(0)

#define DNOTICE1(state, field)  do{}while(0)
#define DWARNING1(state, field) do{}while(0)
#define DERROR1(state, field)   do{}while(0)

#define DNOTICE2(state, field)  do{}while(0)
#define DWARNING2(state, field) do{}while(0)
#define DERROR2(state, field)   do{}while(0)

#define DNOTICE3(state, field)  do{}while(0)
#define DWARNING3(state, field) do{}while(0)
#define DERROR3(state, field)   do{}while(0)

#define LIB_ASSERT(x,str)                 do{}while(0)
#define APP_ASSERT(x,str)                 do{}while(0)

#endif  /* DEBUG */

#define MAKE_DEBUG(var) \
static inline void DPRINT   (const char*form,...){ DNOTICE0(var,DEBUG_ENABLED); }\
static inline void DERROR   (const char*form,...){ DNOTICE0(var,DEBUG_ENABLED); }\
static inline void DWARNING (const char*form,...){ DNOTICE0(var,DEBUG_ENABLED); }\
static inline void DCRITICAL(const char*form,...){ DNOTICE0(var,DEBUG_ENABLED); }

#define MAKE_DEBUG_FUNCS(sfx,var,val) \
static inline void DPRINT_##sfx     (const char*form,...){DNOTICE0(var,val);}\
static inline void DPRINT_##sfx##1  (const char*form,...){DNOTICE1(var,val);}\
static inline void DPRINT_##sfx##2  (const char*form,...){DNOTICE2(var,val);}\
static inline void DPRINT_##sfx##3  (const char*form,...){DNOTICE3(var,val);}\
static inline void DWARNING_##sfx   (const char*form,...){DWARNING0(var,val);}\
static inline void DWARNING_##sfx##1(const char*form,...){DWARNING1(var,val);}\
static inline void DWARNING_##sfx##2(const char*form,...){DWARNING2(var,val);}\
static inline void DWARNING_##sfx##3(const char*form,...){DWARNING3(var,val);}\
static inline void DERROR_##sfx     (const char*form,...){DERROR0(var,val);}\
static inline void DERROR_##sfx##1  (const char*form,...){DERROR1(var,val);}\
static inline void DERROR_##sfx##2  (const char*form,...){DERROR2(var,val);}\
static inline void DERROR_##sfx##3  (const char*form,...){DERROR3(var,val);}\


/* Compatibility */

#define DPRINTIF(state, field) DMESSAGE(state,DEBUG_NOTICE,DEBUG_0,field)

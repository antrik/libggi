/*
******************************************************************************

   Debug macros definition
   
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

/**
 * Important: Do _not_ modify this file directly. It is imported from
 * "tools/buildframework/include/ggi/internal/debug_macros.h".
 * Change that file instead.
 */

#include <stdio.h>
#include <stdarg.h>

#define DEBUG_ALL	0x7fffffff
#define DEBUG_SYNC	0x80000000
#define DEBUG_INFO	 __FILE__,__PRETTY_FUNCTION__,__LINE__

#ifdef  DEBUG

#define DPRINTIF(debugvar, mask) do {             \
    if (debugvar & mask) {                        \
	    va_list args;                         \
	    fprintf(stderr, DEBUG_NAMESPACE ": ");   \
	    va_start(args, form);                 \
	    vfprintf(stderr, form, args);         \
	    va_end(args);                         \
	    if (debugvar & DEBUG_SYNC) fflush(stderr);     \
    }                                             \
} while(0)

#define LIB_ASSERT(x,str) do { if (!(x)) { \
	fprintf(stderr,DEBUG_NAMESPACE ": %s:%s:%d: INTERNAL ERROR: %s\n",DEBUG_INFO,str); \
	exit(1); } } while(0)

#define APP_ASSERT(x,str) do { if (!(x)) { \
	fprintf(stderr,DEBUG_NAMESPACE ": %s:%s:%d: APPLICATION ERROR: %s\n",DEBUG_INFO,str); \
	exit(1); } } while(0)

#else   /* DEBUG */

#define DPRINTIF(mask)    do{}while(0)
#define LIB_ASSERT(x,str) do{}while(0)
#define APP_ASSERT(x,str) do{}while(0)

#endif  /* DEBUG */

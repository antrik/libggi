/* $Id: ggilibinit.h,v 1.2 2004/09/18 09:51:13 cegger Exp $
******************************************************************************

   LibGGI platform specific definitions

   Copyright (C) 1998   Marcus Sundberg         [marcus@ggi-project.org]

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


#ifndef _GGILIBINIT_H
#define _GGILIBINIT_H

#if 0
#ifdef __CYGWIN32__
#include <windows.h> 

struct _reent *_impure_ptr;
extern struct _reent *__imp_reent_data;

int WINAPI _ggi_dllentry (HANDLE h, 
			  DWORD reason,
			  void *ptr)
{
    _impure_ptr = __imp_reent_data;

    return 1;
}
#endif /* __CYGWIN32__ */
#endif

#endif /* _GGILIBINIT_H */

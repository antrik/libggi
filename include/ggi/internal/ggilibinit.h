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

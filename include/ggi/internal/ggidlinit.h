#ifndef _GGIDLINIT_H
#define _GGIDLINIT_H

#if 0
#ifdef __CYGWIN32__
#include <windows.h> 
#include <cygwin32/cygwin_dll.h>

DECLARE_CYGWIN_DLL(_ggi_dllentry2);

struct _reent *_impure_ptr;
extern struct _reent *__imp_reent_data;

int WINAPI _ggi_dllentry2 (HANDLE h, 
			  DWORD reason,
			  void *ptr)
{
  _impure_ptr = __imp_reent_data;
  
    return 1;
}
#endif /* __CYGWIN32__ */
#endif

#endif /* _GGIDLINIT_H */

/* $Id: visual.c,v 1.3 2002/09/08 21:37:45 soyt Exp $
*****************************************************************************

   LibGGI DirectX target - Initialization

   Copyright (C) 1999 John Fortin       [fortinj@ibm.net]
   Copyright (C) 2000 Marcus Sundberg   [marcus@ggi-project.org]

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

#include <stdlib.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/directx.h>

#include "ddinit.h"


typedef struct {
        HANDLE hWnd;
        HINSTANCE hInstance;
} GGIGII, *lpGGIGII;


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
                        const char *args, void *argptr, uint32 *dlret)
{
        directx_priv *priv;
        GGIGII ggigii;

        GGIDPRINT("DirectX-target starting\n");

        priv = malloc(sizeof(directx_priv));
        if (priv == NULL) {
                return GGI_ENOMEM;
        }
        if ((LIBGGI_GC(vis) = malloc(sizeof(ggi_gc))) == NULL) {
                free(priv);
                return GGI_ENOMEM;
        }
        if (!DDInit(priv)) {
                free(LIBGGI_GC(vis));
                free(priv);
                return GGI_ENODEVICE;
        }
        LIBGGI_PRIVATE(vis) = priv;

        ggigii.hWnd = priv->hWnd;
        ggigii.hInstance = priv->hInstance;

        vis->input = giiOpen("directx", &ggigii, NULL);

        vis->opdisplay->setmode = GGI_directx_setmode;
        vis->opdisplay->getmode = GGI_directx_getmode;
        vis->opdisplay->checkmode = GGI_directx_checkmode;
        vis->opdisplay->flush = GGI_directx_flush;

        *dlret = GGI_DL_OPDISPLAY;
        return 0;
}


int GGIdlcleanup(ggi_visual * vis)
{
        directx_priv *priv = LIBGGI_PRIVATE(vis);

#if 0
        DDShutdown();
#endif
        free(priv);

        return 0;
}


int GGIdl_directx(int func, void **funcptr)
{
        switch (func) {
        case GGIFUNC_open:
                *funcptr = GGIopen;
                return 0;
        case GGIFUNC_exit:
                *funcptr = NULL;
                return 0;
        case GGIFUNC_close:
                *funcptr = NULL;
                return 0;
        default:
                *funcptr = NULL;
        }

        return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>

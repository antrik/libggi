/* $Id: ddinit.h,v 1.16 2004/09/24 11:09:09 pekberg Exp $
*****************************************************************************

   LibGGI DirectX target - Header for internal functions

   Copyright (C) 1999 John Fortin       [fortinj@ibm.net]

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
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/directx.h>
#include <ggi/input/directx.h>


#define NAME			"GGI-on-DX"
#define TITLE			"GGI-on-DX"
#define WM_DDMESSAGEBOX		0x7FFF
#define WM_DDCHANGEMODE		0x7FFE
#define WM_DDEND		0x7FFD
#define WM_DDFULLSCREEN		0x7FFC

typedef struct directx_fullscreen
{
	directx_priv *priv;
	ggi_mode *mode;
	HRESULT hr;
	HANDLE event;
} directx_fullscreen;


__BEGIN_DECLS

int DDInit(ggi_visual *vis);
void DDShutdown(directx_priv *priv);
void DDRedraw(ggi_visual *vis, int x, int y, int w, int h);
void DDRedrawAll(ggi_visual *vis);
int DDChangeMode(ggi_visual *vis, ggi_mode *mode);
int DDMatchMode(ggi_visual *vis, ggi_mode *mode,
		int *depth, int *defwidth, int *defheight);

__END_DECLS

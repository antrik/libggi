/* $Id: ddinit.h,v 1.2 2002/09/08 21:37:45 soyt Exp $
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


#define NAME			"GGI-on-DX"
#define TITLE			"GGI-on-DX"
#define WM_DDMESSAGEBOX		0x7FFF
#define WM_DDPUTPIXEL		0x7FFE
#define WM_DDDRAWLINE		0x7FFD
#define WM_DDCHANGEMODE		0x7FFC
#define MAX_DISPLAYMODES	100

typedef struct DisplayMode
{
	uint32 width;
	uint32 height;
	uint32 bpp;
} DisplayMode, *LPDisplayMode;

typedef struct DDChangeModeStruct
{
	DWORD width;
	DWORD height;
	DWORD BPP;
} DDCMS, *LPDDCMS;

typedef struct DDMessageBoxStruct
{
	HWND hWnd;
	LPCTSTR text;
	LPCTSTR caption;
	UINT type;
} DDMBS, *LPDDMBS;


__BEGIN_DECLS

HANDLE DDInit(directx_priv *);
DWORD WINAPI DDInitThread(LPVOID lpParm);
HRESULT redraw(void);
int DDDrawLine(directx_priv *priv, int x0, int y0, int x1, int y1,
	       ggi_pixel pix);
int DDPutPixel(directx_priv *priv, int x, int y, ggi_pixel pix);
int DXGetPixel(directx_priv *priv, int x, int y, ggi_pixel *color);
int DXChangeMode(directx_priv *priv, DWORD width, DWORD height, DWORD BPP);
HRESULT DDChangeMode(directx_priv *priv, DDCMS * ddcms);
int DDCheckMode(ggi_mode * tm);
char *DDLock(void);
HRESULT DDUnlock(void);
HRESULT DDMessageBox(HWND hWnd, LPCTSTR text, LPCTSTR caption);

__END_DECLS

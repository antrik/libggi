/* $Id: ddcommon.c,v 1.1 2003/10/08 08:51:16 cegger Exp $
*****************************************************************************

   LibGGI DirectX target - Common non-DirectX version specific functions

   Copyright (C) 1999-2000 John Fortin  [fortinj@ibm.net]

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
#include "ddinit.h"

#include "../common/ggi-auto.inc"

extern HWND hWnd;
extern char *lpSurfaceAdd;

HRESULT DDShutdown(void)
{
	return SendMessage(hWnd, WM_DESTROY, 0, (LPARAM) NULL);
}


char *DDLock(void)
{
	return (char *) lpSurfaceAdd;
}


HRESULT DDUnlock(void)
{
	return 0;
}


HRESULT DDChangeMode(directx_priv * priv, DDCMS * ddcms)
{
	return SendMessage(hWnd, WM_DDCHANGEMODE, 0, (LPARAM) ddcms);
}


int DDCheckMode(ggi_visual *vis, ggi_mode * mode)
{
	uint8 i;
	uint8 err = 0;
	int depth, width, height, defwidth, defheight;
	ggi_graphtype deftype;

	GetScreenParams(&depth, &width, &height);
	defwidth = width * 9 / 10;
	defheight = height * 9 / 10;

	/* handle AUTO */
	_GGIhandle_ggiauto(mode, defwidth, defheight);

	if (mode->frames < 1) {
		err = -1;
		mode->frames = 1;
	} else if (mode->frames > 2) {
		err = -1;
		mode->frames = 1;
	}

	switch (depth) {
	case 1:
		deftype = GT_1BIT;
		break;
	case 2:
		deftype = GT_2BIT;
		break;
	case 4:
		deftype = GT_4BIT;
		break;
	case 8:
		deftype = GT_8BIT;
		break;
	case 15:
		deftype = GT_15BIT;
		break;
	case 16:
		deftype = GT_16BIT;
		break;
	case 24:
		deftype = GT_24BIT;
		break;
	case 32:
		deftype = GT_32BIT;
		break;
	default:
		deftype = GT_AUTO;
		err = -1;
		break;
	}

	if (GT_DEPTH(mode->graphtype) == GT_AUTO) {
		mode->graphtype = deftype;
	}

	if (!(mode->visible.x > 0 && mode->visible.y > 0 &&
		mode->visible.x <= width && mode->visible.y <= height &&
		GT_SIZE(mode->graphtype) == depth))
	{
		mode->visible.x = defwidth;
		mode->visible.y = defheight;
		mode->graphtype = deftype;
		err = -1;
	}

	if (mode->virt.x != mode->visible.x) {
		mode->virt.x = mode->visible.x;
		err = -1;
	}
	if (mode->virt.y != mode->visible.y) {
		mode->virt.y = mode->visible.y;
		err = -1;
	}

	if ((mode->dpp.x != 1 && mode->dpp.x != GGI_AUTO) ||
	   (mode->dpp.y != 1 && mode->dpp.y != GGI_AUTO))
	{
		err = -1;
	}
	mode->dpp.x = mode->dpp.y = 1;

	if (err) return err;
	err = _ggi_figure_physz(mode, DIRECTX_PRIV(vis)->physzflags,
				&(DIRECTX_PRIV(vis)->physz),
				0, 0, mode->visible.x, mode->visible.y);

	return err;
}


HRESULT DDMessageBox(HWND hWnd, LPCTSTR text, LPCTSTR caption)
{
	DDMBS MessageData;

	MessageData.hWnd = hWnd;
	MessageData.text = text;
	MessageData.caption = caption;
	MessageData.type = MB_OK;
	SendMessage(hWnd, WM_DDMESSAGEBOX, 0, (LPARAM) & MessageData);

	return 0;
}


void GetScreenParams(int *depth, int *width, int *height)
{
	HWND wnd = GetDesktopWindow();
	HDC dc = GetDC(wnd);
	*depth = GetDeviceCaps(dc, BITSPIXEL);
	*width = GetDeviceCaps(dc, HORZRES);
	*height = GetDeviceCaps(dc, VERTRES);
	ReleaseDC(wnd, dc);
}

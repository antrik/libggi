/* $Id: ddinit.c,v 1.32 2004/09/13 12:57:04 pekberg Exp $
*****************************************************************************

   LibGGI DirectX target - Internal functions

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
#include <process.h>

static void DDCreateClass(directx_priv *priv);
static int DDCreateWindow(ggi_visual *vis);
static int DDCreateThread(ggi_visual *vis);
static int DDCreateSurface(directx_priv *priv, int frames,
			   DWORD vw, DWORD vh);
static void DDDestroySurface(directx_priv *priv);
static void DDChangeWindow(directx_priv *priv, DWORD width, DWORD height);

long FAR PASCAL
WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int
DDInit(ggi_visual *vis)
{
	directx_priv *priv = GGIDIRECTX_PRIV(vis);
	/* get the application instance */
	priv->hInstance = GetModuleHandle(NULL);

	/* create and register the window class */
	DDCreateClass(priv);

	if (priv->hParent) {
		/* create the window */
		if (!DDCreateWindow(vis))
			return 0;
	} else {
		/* start the event loop here */
		if (!DDCreateThread(vis)) {
			DDShutdown(priv);
			return 0;
		}
	}
	return 1;
}

void
DDShutdown(directx_priv *priv)
{
	/* kill the timer callback */
	if (priv->timer_id)
		KillTimer(priv->hWnd, priv->timer_id);

	/* destroy the window and the surface */
	if (priv->hWnd && !priv->hParent) {
		GGIDPRINT("display-directx: End session\n");
		PostThreadMessage(priv->nThreadID, WM_DDEND, 0, 0);
	}
	DDDestroySurface(priv);

	if (priv->lpddext != NULL) {
		IDirectDraw2_Release(priv->lpddext);
		priv->lpddext = NULL;
	}
	if (priv->lpdd != NULL) {
		IDirectDraw_Release(priv->lpdd);
		priv->lpdd = NULL;
	}

	/* stop the event loop */
	if (priv->hThread &&
	    WaitForSingleObject(priv->hThread, 2000) != WAIT_OBJECT_0) {
		/* asta la vista, baby */
		GGIDPRINT("display-directx: "
			  "Terminating helper thread harshly\n");
		TerminateThread(priv->hThread, 0);
	}
	if (priv->hThread)
		CloseHandle(priv->hThread);

	/* Get rid of the window class if we registered it */
	if (priv->wndclass)
		UnregisterClass((LPCTSTR) priv->wndclass, priv->hInstance);

	/* get rid of the cursor if we created one */
	if (priv->hCursor)
		DestroyCursor(priv->hCursor);

	if (priv->hInit)
		CloseHandle(priv->hInit);

	priv->hWnd = NULL;
	priv->wndclass = 0;
	priv->hThread = NULL;
	priv->nThreadID = 0;
	priv->hCursor = NULL;
	priv->hInit = NULL;
	priv->timer_id = 0;
}

void CALLBACK TimerProc(HWND, UINT, UINT, DWORD);

int
DDChangeMode(directx_priv *priv, int frames,
	     DWORD virtw, DWORD virth,
	     DWORD width, DWORD height, DWORD BPP)
{
	/* destroy any existing surface */
	DDDestroySurface(priv);

	/* recreate the primary surface and back storage */
	if (!DDCreateSurface(priv, frames, virtw, virth))
		return 0;

	/* set the new window size */
	DDChangeWindow(priv, width, height);

	/* set a timer to have the window refreshed at regular intervals,
	   and show the window */
	priv->timer_id = SetTimer(priv->hWnd, 1, 33, NULL);
	ShowWindow(priv->hWnd, SW_SHOWNORMAL);
	if (priv->hParent == NULL)
		SetForegroundWindow(priv->hWnd);

	return 1;
}

void
DDRedraw(ggi_visual *vis, int x, int y, int w, int h)
{
	directx_priv *priv = GGIDIRECTX_PRIV(vis);
	RECT SrcWinPos, DestWinPos;
	SrcWinPos.left   = x;
	SrcWinPos.right  = x + w;
	SrcWinPos.top    = y;
	SrcWinPos.bottom = y + h;
	DestWinPos = SrcWinPos;
	DestWinPos.right -= vis->origin_x;
	if (DestWinPos.right <= 0)
		return;
	DestWinPos.bottom -= vis->origin_y;
	if (DestWinPos.bottom <= 0)
		return;
	DestWinPos.left -= vis->origin_x;
	if (DestWinPos.left < 0) {
		SrcWinPos.left -= DestWinPos.left;
		DestWinPos.left = 0;
	}
	DestWinPos.top -= vis->origin_y;
	if (DestWinPos.top < 0) {
		SrcWinPos.top -= DestWinPos.top;
		DestWinPos.top = 0;
	}
	ClientToScreen(priv->hWnd, (POINT *) & DestWinPos.left);
	ClientToScreen(priv->hWnd, (POINT *) & DestWinPos.right);
	/* draw the stored image on the primary surface */
	IDirectDrawSurface_Blt(priv->lppdds, &DestWinPos,
			       priv->lpbdds[vis->d_frame_num], &SrcWinPos,
			       DDBLT_WAIT, NULL);
}

void
DDRedrawAll(ggi_visual *vis)
{
	directx_priv *priv = GGIDIRECTX_PRIV(vis);
	RECT SrcWinPos, DestWinPos;
	GetClientRect(priv->hWnd, &SrcWinPos);
	SrcWinPos.left   += vis->origin_x;
	SrcWinPos.top    += vis->origin_y;
	SrcWinPos.right  += vis->origin_x;
	SrcWinPos.bottom += vis->origin_y;
	GetClientRect(priv->hWnd, &DestWinPos);
	ClientToScreen(priv->hWnd, (POINT *) & DestWinPos.left);
	ClientToScreen(priv->hWnd, (POINT *) & DestWinPos.right);
	/* draw the stored image on the primary surface */
	IDirectDrawSurface_Blt(priv->lppdds, &DestWinPos,
			       priv->lpbdds[vis->d_frame_num], &SrcWinPos,
			       DDBLT_WAIT, NULL);
}

/* internal routines ********************************************************/

/* cursors */

static BYTE ANDmaskInvCursor[] = {
	0xff, 0xff, 0xff, 0xff,	// line 1
	0xff, 0xff, 0xff, 0xff,	// line 2
	0xff, 0xff, 0xff, 0xff,	// line 3
	0xff, 0xff, 0xff, 0xff,	// line 4

	0xff, 0xff, 0xff, 0xff,	// line 5
	0xff, 0xff, 0xff, 0xff,	// line 6
	0xff, 0xff, 0xff, 0xff,	// line 7
	0xff, 0xff, 0xff, 0xff,	// line 8

	0xff, 0xff, 0xff, 0xff,	// line 9
	0xff, 0xff, 0xff, 0xff,	// line 10
	0xff, 0xff, 0xff, 0xff,	// line 11
	0xff, 0xff, 0xff, 0xff,	// line 12

	0xff, 0xff, 0xff, 0xff,	// line 13
	0xff, 0xff, 0xff, 0xff,	// line 14
	0xff, 0xff, 0xff, 0xff,	// line 15
	0xff, 0xff, 0xff, 0xff,	// line 16

	0xff, 0xff, 0xff, 0xff,	// line 17
	0xff, 0xff, 0xff, 0xff,	// line 18
	0xff, 0xff, 0xff, 0xff,	// line 19
	0xff, 0xff, 0xff, 0xff,	// line 20

	0xff, 0xff, 0xff, 0xff,	// line 21
	0xff, 0xff, 0xff, 0xff,	// line 22
	0xff, 0xff, 0xff, 0xff,	// line 23
	0xff, 0xff, 0xff, 0xff,	// line 24

	0xff, 0xff, 0xff, 0xff,	// line 25
	0xff, 0xff, 0xff, 0xff,	// line 26
	0xff, 0xff, 0xff, 0xff,	// line 27
	0xff, 0xff, 0xff, 0xff,	// line 28

	0xff, 0xff, 0xff, 0xff,	// line 29
	0xff, 0xff, 0xff, 0xff,	// line 30
	0xff, 0xff, 0xff, 0xff,	// line 31
	0xff, 0xff, 0xff, 0xff	// line 32
};

static BYTE XORmaskInvCursor[] = {
	0x00, 0x00, 0x00, 0x00,	// line 1
	0x00, 0x00, 0x00, 0x00,	// line 2
	0x00, 0x00, 0x00, 0x00,	// line 3
	0x00, 0x00, 0x00, 0x00,	// line 4

	0x00, 0x00, 0x00, 0x00,	// line 5
	0x00, 0x00, 0x00, 0x00,	// line 6
	0x00, 0x00, 0x00, 0x00,	// line 7
	0x00, 0x00, 0x00, 0x00,	// line 8

	0x00, 0x00, 0x00, 0x00,	// line 9
	0x00, 0x00, 0x00, 0x00,	// line 10
	0x00, 0x00, 0x00, 0x00,	// line 11
	0x00, 0x00, 0x00, 0x00,	// line 12

	0x00, 0x00, 0x00, 0x00,	// line 13
	0x00, 0x00, 0x00, 0x00,	// line 14
	0x00, 0x00, 0x00, 0x00,	// line 15
	0x00, 0x00, 0x00, 0x00,	// line 16

	0x00, 0x00, 0x00, 0x00,	// line 17
	0x00, 0x00, 0x00, 0x00,	// line 18
	0x00, 0x00, 0x00, 0x00,	// line 19
	0x00, 0x00, 0x00, 0x00,	// line 20

	0x00, 0x00, 0x00, 0x00,	// line 21
	0x00, 0x00, 0x00, 0x00,	// line 22
	0x00, 0x00, 0x00, 0x00,	// line 23
	0x00, 0x00, 0x00, 0x00,	// line 24

	0x00, 0x00, 0x00, 0x00,	// line 25
	0x00, 0x00, 0x00, 0x00,	// line 26
	0x00, 0x00, 0x00, 0x00,	// line 27
	0x00, 0x00, 0x00, 0x00,	// line 28

	0x00, 0x00, 0x00, 0x00,	// line 29
	0x00, 0x00, 0x00, 0x00,	// line 30
	0x00, 0x00, 0x00, 0x00,	// line 31
	0x00, 0x00, 0x00, 0x00	// line 32
};

static BYTE ANDmaskDotCursor[] = {
	0xff, 0xff, 0xff, 0xff,	// line 1
	0xff, 0xff, 0xff, 0xff,	// line 2
	0xff, 0xff, 0xff, 0xff,	// line 3
	0xff, 0xff, 0xff, 0xff,	// line 4

	0xff, 0xff, 0xff, 0xff,	// line 5
	0xff, 0xff, 0xff, 0xff,	// line 6
	0xff, 0xff, 0xff, 0xff,	// line 7
	0xff, 0xff, 0xff, 0xff,	// line 8

	0xff, 0xff, 0xff, 0xff,	// line 9
	0xff, 0xff, 0xff, 0xff,	// line 10
	0xff, 0xff, 0xff, 0xff,	// line 11
	0xff, 0xff, 0xff, 0xff,	// line 12

	0xff, 0xff, 0xff, 0xff,	// line 13
	0xff, 0xff, 0xff, 0xff,	// line 14
	0xff, 0xff, 0xff, 0xff,	// line 15
	0xff, 0xfe, 0xff, 0xff,	// line 16

	0xff, 0xfd, 0x7f, 0xff,	// line 17
	0xff, 0xfe, 0xff, 0xff,	// line 18
	0xff, 0xff, 0xff, 0xff,	// line 19
	0xff, 0xff, 0xff, 0xff,	// line 20

	0xff, 0xff, 0xff, 0xff,	// line 21
	0xff, 0xff, 0xff, 0xff,	// line 22
	0xff, 0xff, 0xff, 0xff,	// line 23
	0xff, 0xff, 0xff, 0xff,	// line 24

	0xff, 0xff, 0xff, 0xff,	// line 25
	0xff, 0xff, 0xff, 0xff,	// line 26
	0xff, 0xff, 0xff, 0xff,	// line 27
	0xff, 0xff, 0xff, 0xff,	// line 28

	0xff, 0xff, 0xff, 0xff,	// line 29
	0xff, 0xff, 0xff, 0xff,	// line 30
	0xff, 0xff, 0xff, 0xff,	// line 31
	0xff, 0xff, 0xff, 0xff	// line 32
};

static BYTE XORmaskDotCursor[] = {
	0x00, 0x00, 0x00, 0x00,	// line 1
	0x00, 0x00, 0x00, 0x00,	// line 2
	0x00, 0x00, 0x00, 0x00,	// line 3
	0x00, 0x00, 0x00, 0x00,	// line 4

	0x00, 0x00, 0x00, 0x00,	// line 5
	0x00, 0x00, 0x00, 0x00,	// line 6
	0x00, 0x00, 0x00, 0x00,	// line 7
	0x00, 0x00, 0x00, 0x00,	// line 8

	0x00, 0x00, 0x00, 0x00,	// line 9
	0x00, 0x00, 0x00, 0x00,	// line 10
	0x00, 0x00, 0x00, 0x00,	// line 11
	0x00, 0x00, 0x00, 0x00,	// line 12

	0x00, 0x00, 0x00, 0x00,	// line 13
	0x00, 0x00, 0x00, 0x00,	// line 14
	0x00, 0x00, 0x00, 0x00,	// line 15
	0x00, 0x01, 0x00, 0x00,	// line 16

	0x00, 0x02, 0x80, 0x00,	// line 17
	0x00, 0x01, 0x00, 0x00,	// line 18
	0x00, 0x00, 0x00, 0x00,	// line 19
	0x00, 0x00, 0x00, 0x00,	// line 20

	0x00, 0x00, 0x00, 0x00,	// line 21
	0x00, 0x00, 0x00, 0x00,	// line 22
	0x00, 0x00, 0x00, 0x00,	// line 23
	0x00, 0x00, 0x00, 0x00,	// line 24

	0x00, 0x00, 0x00, 0x00,	// line 25
	0x00, 0x00, 0x00, 0x00,	// line 26
	0x00, 0x00, 0x00, 0x00,	// line 27
	0x00, 0x00, 0x00, 0x00,	// line 28

	0x00, 0x00, 0x00, 0x00,	// line 29
	0x00, 0x00, 0x00, 0x00,	// line 30
	0x00, 0x00, 0x00, 0x00,	// line 31
	0x00, 0x00, 0x00, 0x00	// line 32
};


static void
DDSizing(directx_priv *priv, WPARAM wParam, LPRECT rect)
{
	int xsize, ysize;
	RECT diff, test1, test2;
	DWORD ws_style = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;

	/* Calculate diff between client and window coords */
	test1.top    = 200;
	test1.left   = 200;
	test1.right  = 400;
	test1.bottom = 400;
	test2 = test1;
	AdjustWindowRectEx(&test2, ws_style, FALSE, 0);
	diff.top    = test1.top    - test2.top;
	diff.bottom = test1.bottom - test2.bottom;
	diff.left   = test1.left   - test2.left;
	diff.right  = test1.right  - test2.right;

	/* Adjust to client coords */
	rect->top    += diff.top;
	rect->bottom += diff.bottom;
	rect->left   += diff.left;
	rect->right  += diff.right;

	/* Calculate new size, with regard to max/min/step */
	xsize = rect->right  - rect->left;
	ysize = rect->bottom - rect->top;
	if (xsize < priv->xmin)
		xsize = priv->xmin;
	if (xsize > priv->xmax)
		xsize = priv->xmax;
	if (ysize < priv->ymin)
		ysize = priv->ymin;
	if (ysize > priv->ymax)
		ysize = priv->ymax;
	xsize -= (xsize - priv->xmin) % priv->xstep;
	ysize -= (ysize - priv->ymin) % priv->ystep;

	/* Move the appropriate edge(s) */
	switch (wParam) {
	case WMSZ_LEFT:
	case WMSZ_BOTTOMLEFT:
	case WMSZ_TOPLEFT:
		rect->left = rect->right - xsize;
		break;
	case WMSZ_RIGHT:
	case WMSZ_BOTTOMRIGHT:
	case WMSZ_TOPRIGHT:
		rect->right = rect->left + xsize;
		break;
	}
	switch (wParam) {
	case WMSZ_TOP:
	case WMSZ_TOPRIGHT:
	case WMSZ_TOPLEFT:
		rect->top = rect->bottom - ysize;
		break;
	case WMSZ_BOTTOM:
	case WMSZ_BOTTOMRIGHT:
	case WMSZ_BOTTOMLEFT:
		rect->bottom = rect->top + ysize;
		break;
	}

	/* Adjust back to window coords */
	rect->top    -= diff.top;
	rect->bottom -= diff.bottom;
	rect->left   -= diff.left;
	rect->right  -= diff.right;
}

/* GGI window procedure */

long FAR PASCAL
WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPCREATESTRUCT lpcs;
	PAINTSTRUCT ps;
	HDC hdc;
	ggi_visual *vis = (ggi_visual *) GetWindowLong(hWnd, GWL_USERDATA);
	directx_priv *priv = NULL;

	if (vis)
		priv = GGIDIRECTX_PRIV(vis);

	switch (message) {

	case WM_USER:
		if (!TryEnterCriticalSection(&priv->cs)) {
			/* spin */
			PostMessage(hWnd, message, wParam, lParam);
			return 0;
		}
		DDRedrawAll(vis);
		EnterCriticalSection(&priv->redrawcs);
		priv->redraw = 1;
		LeaveCriticalSection(&priv->redrawcs);
		LeaveCriticalSection(&priv->cs);
		return 0;

	case WM_TIMER:
		if (wParam != 1)
			break;
		/* TODO: return 0 if async mode. */
		/* Fall through */
	case WM_PAINT:
		if (!TryEnterCriticalSection(&priv->cs)) {
			int redraw = 0;
			EnterCriticalSection(&priv->redrawcs);
			redraw = priv->redraw;
			priv->redraw = 0;
			LeaveCriticalSection(&priv->redrawcs);
			if (redraw)
				/* spin */
				PostMessage(hWnd, WM_USER, wParam, lParam);
			return 0;
		}
		if (message == WM_PAINT)
			hdc = BeginPaint(hWnd, &ps);
		DDRedrawAll(vis);
		if (message == WM_PAINT)
			EndPaint(hWnd, &ps);
		LeaveCriticalSection(&priv->cs);
		return 0;

	case WM_SIZING:
		EnterCriticalSection(&priv->sizingcs);
		if (priv->xstep < 0) {
			LeaveCriticalSection(&priv->sizingcs);
			break;
		}
		DDSizing(priv, wParam, (LPRECT) lParam);
		LeaveCriticalSection(&priv->sizingcs);
		return TRUE;

	case WM_CREATE:
		lpcs = (LPCREATESTRUCT) lParam;
		SetWindowLong(hWnd,
			      GWL_USERDATA, (DWORD) lpcs->lpCreateParams);
		return 0;

	case WM_CLOSE:
		if (!priv->hParent)
			exit(1);
		break;

	case WM_SETTINGCHANGE:
		if (priv->inp) {
			gii_event ev;

			GGIDPRINT("display-directx: tell inputlib about "
				  "new system parameters\n");

			ev.cmd.size = sizeof(gii_cmd_event);
			ev.cmd.type = evCommand;
			ev.cmd.target = priv->inp->origin;
			ev.cmd.code = GII_CMDCODE_DXSETTINGCHANGE;

			giiEventSend(priv->inp, &ev);
			return 0;
		}
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/* create and register the GGI window class */

static void
DDCreateClass(directx_priv *priv)
{
	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = priv->hInstance;
	wc.hIcon =
	    LoadIcon(priv->hInstance, MAKEINTRESOURCE("DirectX.ico"));

	switch (priv->cursortype) {
	case 0:
		wc.hCursor = CreateCursor(priv->hInstance,
					  16, 16, /* hot spot x, y */
					  32, 32, /* size x, y */
					  ANDmaskInvCursor, /* masks */
					  XORmaskInvCursor);
		priv->hCursor = wc.hCursor;
		break;
	case 1:
		wc.hCursor = CreateCursor(priv->hInstance,
					  16, 16, /* hot spot x, y */
					  32, 32, /* size x, y */
					  ANDmaskDotCursor, /* masks */
					  XORmaskDotCursor);
		priv->hCursor = wc.hCursor;
		break;
	default:
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		priv->hCursor = NULL;
		break;
	}
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NAME;
	wc.lpszClassName = NAME;
	priv->wndclass = RegisterClass(&wc);
}

/* create the GGI window */

static int
DDCreateWindow(ggi_visual *vis)
{
	directx_priv *priv = GGIDIRECTX_PRIV(vis);
	int w = 640, h = 480;	/* default window size */
	int ws_flags = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;
	/* toplevel flags */

	if (priv->hParent) {
		/* determine the parent window size */
		RECT r;
		GetWindowRect(priv->hParent, &r);
		w = r.right - r.left;
		h = r.bottom - r.top;
		/* flags for a child window */
		ws_flags = WS_CHILD;
	} else {
		/* adjust the window size to accommodate for
		 * the client area
		 */
		RECT r;
		r.left = r.top = 0;
		r.right = w;
		r.bottom = h;
		AdjustWindowRectEx(&r, ws_flags, 0, 0);
		w = r.right - r.left;
		h = r.bottom - r.top;
	}

	/* create the window */
	priv->hWnd = CreateWindowEx(0, NAME, TITLE, ws_flags,
				    CW_USEDEFAULT, 0, w, h,
				    priv->hParent, NULL,
				    priv->hInstance, vis);
	if (!priv->hWnd) {
		if (priv->hCursor)
			DestroyCursor(priv->hCursor);
		priv->hCursor = NULL;
		return 0;
	}

	/* make sure the window is initially hidden */
	ShowWindow(priv->hWnd, SW_HIDE);

	/* initialize the DirectDraw interface */
	DirectDrawCreate(NULL, &priv->lpdd, NULL);
	IDirectDraw_QueryInterface(priv->lpdd, &IID_IDirectDraw2,
				   (LPVOID *) & priv->lpddext);

	return 1;
}

/* create the event loop */

static unsigned __stdcall
DDEventLoop(void *lpParm)
{
	MSG msg;
	ggi_visual *vis = (ggi_visual *) lpParm;
	directx_priv *priv = GGIDIRECTX_PRIV(vis);

	/* create the window */
	if (!DDCreateWindow(vis)) {
		SetEvent(priv->hInit);
		return 0;
	}

	SetEvent(priv->hInit);

	while (GetMessage(&msg, NULL, 0, 0)) {
		if (msg.hwnd == NULL && msg.message == WM_DDEND) {
			/* Use PostThreadMessage to get here */
			GGIDPRINT("display-directx: Ending session, "
				  "destroying window.\n");
			DestroyWindow(priv->hWnd);
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	GGIDPRINT("display-directx: Helper thread terminating\n");

#ifndef __CYGWIN__
	_endthreadex(msg.wParam);
#endif
	return msg.wParam;
}

static int
DDCreateThread(ggi_visual *vis)
{
	directx_priv *priv = GGIDIRECTX_PRIV(vis);
	priv->hInit = CreateEvent(NULL, FALSE, FALSE, NULL);
#ifdef __CYGWIN__
	priv->hThread = CreateThread(NULL, 0,
				     (LPTHREAD_START_ROUTINE) DDEventLoop,
				     vis, 0, &priv->nThreadID);
#else
	priv->hThread =
	    (HANDLE) _beginthreadex(NULL, 0, DDEventLoop, vis, 0,
				    (unsigned) &priv->nThreadID);
#endif
	if (priv->hThread) {
		WaitForSingleObject(priv->hInit, INFINITE);
		return 1;
	} else {
		CloseHandle(priv->hInit);
		priv->hInit = NULL;
		return 0;
	}
}

/* initialize and finalize the primary surface and back storage */

static int
DDCreateSurface(directx_priv *priv, int frames,
		DWORD virtw, DWORD virth)
{
	HRESULT hr;
	LPDIRECTDRAWCLIPPER pClipper;
	DDSURFACEDESC pddsd, bddsd;
	int i;

	IDirectDraw_SetCooperativeLevel(priv->lpddext, priv->hWnd,
					DDSCL_NORMAL);

	/* create the primary surface */
	memset(&pddsd, 0, sizeof(pddsd));
	pddsd.dwSize = sizeof(pddsd);
	pddsd.dwFlags = DDSD_CAPS;
	pddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	hr = IDirectDraw_CreateSurface(priv->lpddext, &pddsd,
				       &priv->lppdds, NULL);
	if (hr != 0) {
		fprintf(stderr,
			"Init Primary Surface Failed RC = %ld. Exiting\n",
			hr & 0xffff);
		exit(-1);
	}
	IDirectDraw_CreateClipper(priv->lpddext, 0, &pClipper, NULL);
	IDirectDrawClipper_SetHWnd(pClipper, 0, priv->hWnd);
	IDirectDrawSurface_SetClipper(priv->lppdds, pClipper);
	IDirectDrawClipper_Release(pClipper);
	pddsd.dwSize = sizeof(pddsd);
	IDirectDrawSurface_GetSurfaceDesc(priv->lppdds, &pddsd);

	/* create the back storages */
	for (i = 0; i < frames; ++i) {
		memset(&bddsd, 0, sizeof(bddsd));
		bddsd.dwSize = sizeof(bddsd);
		bddsd.dwFlags =
		    DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH |
		    DDSD_PIXELFORMAT;
		bddsd.ddsCaps.dwCaps =
		    DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		bddsd.dwWidth = virtw;
		bddsd.dwHeight = virth;
		bddsd.lPitch =
		    virtw * (pddsd.ddpfPixelFormat.dwRGBBitCount + 7) / 8;

		/* set up the pixel format */
		ZeroMemory(&bddsd.ddpfPixelFormat, sizeof(DDPIXELFORMAT));
		bddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		bddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
		bddsd.ddpfPixelFormat.dwRGBBitCount =
		    pddsd.ddpfPixelFormat.dwRGBBitCount;
		bddsd.ddpfPixelFormat.dwRBitMask =
		    pddsd.ddpfPixelFormat.dwRBitMask;
		bddsd.ddpfPixelFormat.dwGBitMask =
		    pddsd.ddpfPixelFormat.dwGBitMask;
		bddsd.ddpfPixelFormat.dwBBitMask =
		    pddsd.ddpfPixelFormat.dwBBitMask;

		hr = IDirectDraw_CreateSurface(priv->lpddext,
					       &bddsd, &priv->lpbdds[i],
					       NULL);
		if (hr) {
			fprintf(stderr,
				"Init Backup Failed RC = %ld. Exiting\n",
				hr & 0xffff);
			exit(-1);
		}
		IDirectDrawSurface2_Lock(priv->lpbdds[i], NULL, &bddsd,
					 DDLOCK_SURFACEMEMORYPTR, NULL);
		priv->lpSurfaceAdd[i] = (char *) bddsd.lpSurface;
		IDirectDrawSurface2_Unlock(priv->lpbdds[i],
					   bddsd.lpSurface);
	}

	/* set private mode parameters */
	priv->maxX = virtw;
	priv->maxY = virth;
	priv->ColorDepth = pddsd.ddpfPixelFormat.dwRGBBitCount;
	priv->BPP = (priv->ColorDepth + 7) / 8;
	priv->pitch = priv->maxX * priv->BPP;

	return 1;
}

static void
DDDestroySurface(directx_priv *priv)
{
	int i;
	for (i = 0; i < GGI_DISPLAY_DIRECTX_FRAMES; ++i) {
		if (priv->lpbdds[i] != NULL) {
			IDirectDrawSurface_Release(priv->lpbdds[i]);
			priv->lpbdds[i] = NULL;
		}
	}
	if (priv->lppdds != NULL) {
		IDirectDrawSurface_Release(priv->lppdds);
		priv->lppdds = NULL;
	}
}

/* set a new window size */

static void
DDChangeWindow(directx_priv *priv, DWORD width, DWORD height)
{
	int ws_style = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;
	RECT r1, r2;
	if (priv->hParent)
		return;
	EnterCriticalSection(&priv->sizingcs);
	priv->xmin = width;
	priv->ymin = height;
	priv->xmax = width;
	priv->ymax = height;
	priv->xstep = 1;
	priv->ystep = 1;
	LeaveCriticalSection(&priv->sizingcs);
	GetWindowRect(priv->hWnd, &r1);
	r2 = r1;
	AdjustWindowRectEx(&r2, ws_style, FALSE, 0);
	MoveWindow(priv->hWnd, r1.left, r1.top,
		   width + (r2.right - r2.left) - (r1.right - r1.left),
		   height + (r2.bottom - r2.top) - (r1.bottom - r1.top),
		   TRUE);
}

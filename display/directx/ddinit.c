/* $Id: ddinit.c,v 1.21 2004/03/25 10:28:32 pekberg Exp $
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
static int DDCreateWindow(directx_priv *priv);
static int DDCreateThread(directx_priv *priv);
static int DDCreateSurface(directx_priv *priv);
static void DDDestroySurface(directx_priv *priv);
static void DDChangeWindow(directx_priv *priv, DWORD width, DWORD height);

long FAR PASCAL
WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int DDInit(directx_priv *priv)
{
  /* get the application instance */
  priv->hInstance = GetModuleHandle(NULL);

  /* create and register the window class */
  DDCreateClass(priv);

  if (priv->hParent) {
    /* create the window */
    if (!DDCreateWindow(priv))
      return 0;
  } else {
    /* start the event loop here */
    if (!DDCreateThread(priv)) {
      DDShutdown(priv);
      return 0;
    }
  }
  return 1;
}

void DDShutdown(directx_priv *priv)
{
  /* kill the timer callback */
  if (priv->timer_id)
    KillTimer(priv->hWnd, priv->timer_id);

  /* destroy the window and the surface */
  if (priv->hWnd && !priv->hParent)
    DestroyWindow(priv->hWnd);
  DDDestroySurface(priv);

  /* stop the event loop */
  if (priv->hThreadID &&
      WaitForSingleObject(priv->hThreadID, 100) != WAIT_OBJECT_0)
    /* asta la vista, baby */
    TerminateThread(priv->hThreadID, 0);

  /* get rid of the cursor if we created one */
  if (priv->hCursor) DestroyCursor(priv->hCursor);

  if (priv->hInit) CloseHandle(priv->hInit);

  priv->hWnd = NULL;
  priv->hThreadID = NULL;
  priv->hCursor = NULL;
  priv->hInit = NULL;
  priv->timer_id = 0;
}

void CALLBACK TimerProc(HWND, UINT, UINT, DWORD);

int DDChangeMode(directx_priv *priv, DWORD width, DWORD height, DWORD BPP)
{
  /* destroy any existing surface */
  DDDestroySurface(priv);

  /* recreate the primary surface and back storage */
  if (!DDCreateSurface(priv))
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

void DDRedraw(directx_priv * priv)
{
  RECT SrcWinPos, DestWinPos;
  GetClientRect(priv->hWnd, &SrcWinPos);
  GetClientRect(priv->hWnd, &DestWinPos);
  ClientToScreen(priv->hWnd, (POINT*)&DestWinPos.left);
  ClientToScreen(priv->hWnd, (POINT*)&DestWinPos.right);
  /* draw the stored image on the primary surface */
  IDirectDrawSurface_Blt(priv->lppdds, &DestWinPos,
			 priv->lpbdds, &SrcWinPos, DDBLT_WAIT, NULL);
}

/* internal routines ********************************************************/

/* cursors */

static BYTE ANDmaskInvCursor[] = 
{ 
    0xff, 0xff, 0xff, 0xff,   // line 1
    0xff, 0xff, 0xff, 0xff,   // line 2
    0xff, 0xff, 0xff, 0xff,   // line 3
    0xff, 0xff, 0xff, 0xff,   // line 4

    0xff, 0xff, 0xff, 0xff,   // line 5
    0xff, 0xff, 0xff, 0xff,   // line 6
    0xff, 0xff, 0xff, 0xff,   // line 7
    0xff, 0xff, 0xff, 0xff,   // line 8

    0xff, 0xff, 0xff, 0xff,   // line 9
    0xff, 0xff, 0xff, 0xff,   // line 10
    0xff, 0xff, 0xff, 0xff,   // line 11
    0xff, 0xff, 0xff, 0xff,   // line 12

    0xff, 0xff, 0xff, 0xff,   // line 13
    0xff, 0xff, 0xff, 0xff,   // line 14
    0xff, 0xff, 0xff, 0xff,   // line 15
    0xff, 0xff, 0xff, 0xff,   // line 16

    0xff, 0xff, 0xff, 0xff,   // line 17
    0xff, 0xff, 0xff, 0xff,   // line 18
    0xff, 0xff, 0xff, 0xff,   // line 19
    0xff, 0xff, 0xff, 0xff,   // line 20

    0xff, 0xff, 0xff, 0xff,   // line 21
    0xff, 0xff, 0xff, 0xff,   // line 22
    0xff, 0xff, 0xff, 0xff,   // line 23
    0xff, 0xff, 0xff, 0xff,   // line 24

    0xff, 0xff, 0xff, 0xff,   // line 25
    0xff, 0xff, 0xff, 0xff,   // line 26
    0xff, 0xff, 0xff, 0xff,   // line 27
    0xff, 0xff, 0xff, 0xff,   // line 28

    0xff, 0xff, 0xff, 0xff,   // line 29
    0xff, 0xff, 0xff, 0xff,   // line 30
    0xff, 0xff, 0xff, 0xff,   // line 31
    0xff, 0xff, 0xff, 0xff    // line 32
};
 
static BYTE XORmaskInvCursor[] = 
{ 
    0x00, 0x00, 0x00, 0x00,   // line 1
    0x00, 0x00, 0x00, 0x00,   // line 2
    0x00, 0x00, 0x00, 0x00,   // line 3
    0x00, 0x00, 0x00, 0x00,   // line 4

    0x00, 0x00, 0x00, 0x00,   // line 5
    0x00, 0x00, 0x00, 0x00,   // line 6
    0x00, 0x00, 0x00, 0x00,   // line 7
    0x00, 0x00, 0x00, 0x00,   // line 8

    0x00, 0x00, 0x00, 0x00,   // line 9
    0x00, 0x00, 0x00, 0x00,   // line 10
    0x00, 0x00, 0x00, 0x00,   // line 11
    0x00, 0x00, 0x00, 0x00,   // line 12

    0x00, 0x00, 0x00, 0x00,   // line 13
    0x00, 0x00, 0x00, 0x00,   // line 14
    0x00, 0x00, 0x00, 0x00,   // line 15
    0x00, 0x00, 0x00, 0x00,   // line 16

    0x00, 0x00, 0x00, 0x00,   // line 17
    0x00, 0x00, 0x00, 0x00,   // line 18
    0x00, 0x00, 0x00, 0x00,   // line 19
    0x00, 0x00, 0x00, 0x00,   // line 20

    0x00, 0x00, 0x00, 0x00,   // line 21
    0x00, 0x00, 0x00, 0x00,   // line 22
    0x00, 0x00, 0x00, 0x00,   // line 23
    0x00, 0x00, 0x00, 0x00,   // line 24

    0x00, 0x00, 0x00, 0x00,   // line 25
    0x00, 0x00, 0x00, 0x00,   // line 26
    0x00, 0x00, 0x00, 0x00,   // line 27
    0x00, 0x00, 0x00, 0x00,   // line 28

    0x00, 0x00, 0x00, 0x00,   // line 29
    0x00, 0x00, 0x00, 0x00,   // line 30
    0x00, 0x00, 0x00, 0x00,   // line 31
    0x00, 0x00, 0x00, 0x00    // line 32
};

static BYTE ANDmaskDotCursor[] = 
{ 
    0xff, 0xff, 0xff, 0xff,   // line 1
    0xff, 0xff, 0xff, 0xff,   // line 2
    0xff, 0xff, 0xff, 0xff,   // line 3
    0xff, 0xff, 0xff, 0xff,   // line 4

    0xff, 0xff, 0xff, 0xff,   // line 5
    0xff, 0xff, 0xff, 0xff,   // line 6
    0xff, 0xff, 0xff, 0xff,   // line 7
    0xff, 0xff, 0xff, 0xff,   // line 8

    0xff, 0xff, 0xff, 0xff,   // line 9
    0xff, 0xff, 0xff, 0xff,   // line 10
    0xff, 0xff, 0xff, 0xff,   // line 11
    0xff, 0xff, 0xff, 0xff,   // line 12

    0xff, 0xff, 0xff, 0xff,   // line 13
    0xff, 0xff, 0xff, 0xff,   // line 14
    0xff, 0xff, 0xff, 0xff,   // line 15
    0xff, 0xfe, 0xff, 0xff,   // line 16

    0xff, 0xfd, 0x7f, 0xff,   // line 17
    0xff, 0xfe, 0xff, 0xff,   // line 18
    0xff, 0xff, 0xff, 0xff,   // line 19
    0xff, 0xff, 0xff, 0xff,   // line 20

    0xff, 0xff, 0xff, 0xff,   // line 21
    0xff, 0xff, 0xff, 0xff,   // line 22
    0xff, 0xff, 0xff, 0xff,   // line 23
    0xff, 0xff, 0xff, 0xff,   // line 24

    0xff, 0xff, 0xff, 0xff,   // line 25
    0xff, 0xff, 0xff, 0xff,   // line 26
    0xff, 0xff, 0xff, 0xff,   // line 27
    0xff, 0xff, 0xff, 0xff,   // line 28

    0xff, 0xff, 0xff, 0xff,   // line 29
    0xff, 0xff, 0xff, 0xff,   // line 30
    0xff, 0xff, 0xff, 0xff,   // line 31
    0xff, 0xff, 0xff, 0xff    // line 32
};
 
static BYTE XORmaskDotCursor[] = 
{ 
    0x00, 0x00, 0x00, 0x00,   // line 1
    0x00, 0x00, 0x00, 0x00,   // line 2
    0x00, 0x00, 0x00, 0x00,   // line 3
    0x00, 0x00, 0x00, 0x00,   // line 4

    0x00, 0x00, 0x00, 0x00,   // line 5
    0x00, 0x00, 0x00, 0x00,   // line 6
    0x00, 0x00, 0x00, 0x00,   // line 7
    0x00, 0x00, 0x00, 0x00,   // line 8

    0x00, 0x00, 0x00, 0x00,   // line 9
    0x00, 0x00, 0x00, 0x00,   // line 10
    0x00, 0x00, 0x00, 0x00,   // line 11
    0x00, 0x00, 0x00, 0x00,   // line 12

    0x00, 0x00, 0x00, 0x00,   // line 13
    0x00, 0x00, 0x00, 0x00,   // line 14
    0x00, 0x00, 0x00, 0x00,   // line 15
    0x00, 0x01, 0x00, 0x00,   // line 16

    0x00, 0x02, 0x80, 0x00,   // line 17
    0x00, 0x01, 0x00, 0x00,   // line 18
    0x00, 0x00, 0x00, 0x00,   // line 19
    0x00, 0x00, 0x00, 0x00,   // line 20

    0x00, 0x00, 0x00, 0x00,   // line 21
    0x00, 0x00, 0x00, 0x00,   // line 22
    0x00, 0x00, 0x00, 0x00,   // line 23
    0x00, 0x00, 0x00, 0x00,   // line 24

    0x00, 0x00, 0x00, 0x00,   // line 25
    0x00, 0x00, 0x00, 0x00,   // line 26
    0x00, 0x00, 0x00, 0x00,   // line 27
    0x00, 0x00, 0x00, 0x00,   // line 28

    0x00, 0x00, 0x00, 0x00,   // line 29
    0x00, 0x00, 0x00, 0x00,   // line 30
    0x00, 0x00, 0x00, 0x00,   // line 31
    0x00, 0x00, 0x00, 0x00    // line 32
};


static void
DDSizing(directx_priv *priv, WPARAM wParam, LPRECT rect)
{
	int xsize, ysize;
	RECT diff, test1, test2;
	DWORD ws_style = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;

	/* Calculate diff between client and window coords */
	test1.top = 200;
	test1.left = 200;
	test1.right = 400;
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
	xsize = rect->right - rect->left;
	ysize = rect->bottom - rect->top;
	if(xsize < priv->xmin)
		xsize = priv->xmin;
	if(xsize > priv->xmax)
		xsize = priv->xmax;
	if(ysize < priv->ymin)
		ysize = priv->ymin;
	if(ysize > priv->ymax)
		ysize = priv->ymax;
	xsize -= (xsize - priv->xmin) % priv->xstep;
	ysize -= (ysize - priv->ymin) % priv->ystep;

	/* Move the appropriate edge(s) */
	switch(wParam) {
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
	switch(wParam) {
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
	directx_priv *priv =
		(directx_priv *)GetWindowLong(hWnd, GWL_USERDATA);

	switch (message) {

	case WM_USER:
		if(!TryEnterCriticalSection(&priv->cs)) {
			/* spin */
			PostMessage(hWnd, message, wParam, lParam);
			return 0;
		}
		DDRedraw(priv);
		EnterCriticalSection(&priv->redrawcs);
		priv->redraw = 1;
		LeaveCriticalSection(&priv->redrawcs);
		LeaveCriticalSection(&priv->cs);
		return 0;

	case WM_TIMER:
		if(wParam != 1)
			break;
		/* Fall through */
	case WM_PAINT:
		if(!TryEnterCriticalSection(&priv->cs)) {
			int redraw = 0;
			EnterCriticalSection(&priv->redrawcs);
			redraw = priv->redraw;
			priv->redraw = 0;
			LeaveCriticalSection(&priv->redrawcs);
			if(redraw)
				/* spin */
				PostMessage(hWnd, WM_USER, wParam, lParam);
			return 0;
		}
		if(message == WM_PAINT)
			hdc = BeginPaint(hWnd, &ps);
		DDRedraw(priv);
		if(message == WM_PAINT)
			EndPaint(hWnd, &ps);
		LeaveCriticalSection(&priv->cs);
		return 0;

	case WM_SIZING:
		EnterCriticalSection(&priv->sizingcs);
		if(priv->xstep < 0) {
			LeaveCriticalSection(&priv->sizingcs);
			break;
		}
		DDSizing(priv, wParam, (LPRECT)lParam);
		LeaveCriticalSection(&priv->sizingcs);
		return TRUE;

	case WM_CREATE:
		lpcs = (LPCREATESTRUCT)lParam;
		SetWindowLong(hWnd,
		              GWL_USERDATA,
		              (DWORD)lpcs->lpCreateParams);
		return 0;

	case WM_CLOSE:
		if (!priv->hParent)
			exit(1);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/* create and register the GGI window class */

static void DDCreateClass(directx_priv *priv)
{
  WNDCLASS wc;

  wc.style = 0;
  wc.lpfnWndProc = WindowProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = priv->hInstance;
  wc.hIcon = LoadIcon(priv->hInstance, MAKEINTRESOURCE("DirectX.ico"));

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
  RegisterClass(&wc);
}

/* create the GGI window */

static int DDCreateWindow(directx_priv *priv)
{
  int w = 640, h = 480; /* default window size */
  int ws_flags = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;
		  /* toplevel flags */

  if (priv->hParent) {
    /* determine the parent window size */
    RECT r;
    GetWindowRect(priv->hParent, &r);
    w = r.right-r.left;
    h = r.bottom-r.top;
    /* flags for a child window */
    ws_flags = WS_CHILD;
  } else {
    /* adjust the window size to accommodate for the client area */
    RECT r;
    r.left = r.top = 0;
    r.right = w; r.bottom = h;
    AdjustWindowRectEx(&r, ws_flags, 0, 0);
    w = r.right-r.left;
    h = r.bottom-r.top;
  }

  /* create the window */
  priv->hWnd = CreateWindowEx(0, NAME, TITLE, ws_flags,
			      CW_USEDEFAULT, 0, w, h,
			      priv->hParent, NULL,
			      priv->hInstance, priv);
  if (!priv->hWnd) {
    if (priv->hCursor) DestroyCursor(priv->hCursor);
    priv->hCursor = NULL;
    return 0;
  }

  /* make sure the window is initially hidden */
  ShowWindow(priv->hWnd, SW_HIDE);

  /* initialize the DirectDraw interface */
  DirectDrawCreate(NULL, &priv->lpdd, NULL);
  IDirectDraw_QueryInterface(priv->lpdd, &IID_IDirectDraw2,
			     (LPVOID*)&priv->lpddext);

  return 1;
}

/* create the event loop */
 
static unsigned __stdcall
DDEventLoop(void *lpParm)
{
  MSG msg;
  directx_priv *priv = (directx_priv*)lpParm;

  /* create the window */
  if (!DDCreateWindow(priv)) {
    SetEvent(priv->hInit);
    return 0;
  }

  SetEvent(priv->hInit);

  while (GetMessage(&msg, priv->hWnd, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return msg.wParam;
}

static int DDCreateThread(directx_priv *priv)
{
  unsigned ThreadID;
  priv->hInit = CreateEvent(NULL, FALSE, FALSE, NULL);
  priv->hThreadID = (HANDLE)_beginthreadex(NULL, 0, DDEventLoop, priv, 0,
					   &ThreadID);
  if (priv->hThreadID) {
    WaitForSingleObject(priv->hInit, INFINITE);
    return 1;
  } else {
    CloseHandle(priv->hInit);
    priv->hInit = NULL;
    return 0;
  }
}

/* initialize and finalize the primary surface and back storage */

static int DDCreateSurface(directx_priv *priv)
{
  HRESULT hr;
  LPDIRECTDRAWCLIPPER pClipper;
  DDSURFACEDESC pddsd, bddsd;

  IDirectDraw_SetCooperativeLevel(priv->lpddext, priv->hWnd, DDSCL_NORMAL);

  /* create the primary surface */
  memset(&pddsd, 0, sizeof(pddsd));
  pddsd.dwSize = sizeof(pddsd);
  pddsd.dwFlags = DDSD_CAPS;
  pddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
  hr = IDirectDraw_CreateSurface(priv->lpddext, &pddsd, &priv->lppdds, NULL);
  if (hr != 0) {
    fprintf(stderr, "Init Primary Surface Failed RC = %ld. Exiting\n",
	    hr & 0xffff);
    exit(-1);
  }
  IDirectDraw_CreateClipper(priv->lpddext, 0, &pClipper, NULL);
  IDirectDrawClipper_SetHWnd(pClipper, 0, priv->hWnd);
  IDirectDrawSurface_SetClipper(priv->lppdds, pClipper);
  IDirectDrawClipper_Release(pClipper);
  pddsd.dwSize = sizeof(pddsd);
  IDirectDrawSurface_GetSurfaceDesc(priv->lppdds, &pddsd);

  /* create the back storage */
  memset(&bddsd, 0, sizeof(bddsd));
  bddsd.dwSize = sizeof(bddsd);
  bddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH
    | DDSD_PIXELFORMAT;
  bddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  bddsd.dwWidth = pddsd.dwWidth;
  bddsd.dwHeight = pddsd.dwHeight;
  bddsd.lPitch = pddsd.lPitch;

  /* set up the pixel format */
  ZeroMemory(&bddsd.ddpfPixelFormat, sizeof(DDPIXELFORMAT));
  bddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
  bddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
  bddsd.ddpfPixelFormat.dwRGBBitCount = pddsd.ddpfPixelFormat.dwRGBBitCount;
  bddsd.ddpfPixelFormat.dwRBitMask = pddsd.ddpfPixelFormat.dwRBitMask;
  bddsd.ddpfPixelFormat.dwGBitMask = pddsd.ddpfPixelFormat.dwGBitMask;
  bddsd.ddpfPixelFormat.dwBBitMask = pddsd.ddpfPixelFormat.dwBBitMask;

  hr = IDirectDraw_CreateSurface(priv->lpddext, &bddsd, &priv->lpbdds, NULL);
  if (hr) {
    fprintf(stderr, "Init Backup Failed RC = %ld. Exiting\n", hr & 0xffff);
    exit(-1);
  }
  IDirectDrawSurface2_Lock(priv->lpbdds, NULL, &bddsd,
			   DDLOCK_SURFACEMEMORYPTR, NULL);
  priv->lpSurfaceAdd = (char*)bddsd.lpSurface;
  IDirectDrawSurface2_Unlock(priv->lpbdds, bddsd.lpSurface);

  /* set private mode parameters */
  priv->maxX = pddsd.dwWidth;
  priv->maxY = pddsd.dwHeight;
  priv->ColorDepth = pddsd.ddpfPixelFormat.dwRGBBitCount;
  priv->BPP = priv->ColorDepth / 8;
  priv->pitch = priv->maxX * priv->BPP;

  return 1;
}

static void DDDestroySurface(directx_priv *priv)
{
  if (priv->lpbdds != NULL) {
    IDirectDrawSurface_Release(priv->lpbdds);
    priv->lpbdds = NULL;
  }
  if (priv->lppdds != NULL) {
    IDirectDrawSurface_Release(priv->lppdds);
    priv->lppdds = NULL;
  }
}

/* set a new window size */

static void DDChangeWindow(directx_priv *priv, DWORD width, DWORD height)
{
  if (!priv->hParent) {
    EnterCriticalSection(&priv->sizingcs);
    priv->xmin = width;
    priv->ymin = height;
    priv->xmax = width;
    priv->ymax = height;
    priv->xstep = 1;
    priv->ystep = 1;
    LeaveCriticalSection(&priv->sizingcs);
    int ws_style = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;
    RECT r;
    GetWindowRect(priv->hWnd, &r);
    r.right = r.left+width;
    r.bottom = r.top+height;
    AdjustWindowRectEx(&r, ws_style, 0, 0);
    width = r.right-r.left;
    height = r.bottom-r.top;
    if (r.left < 0) r.left = 0;
    if (r.top < 0) r.top = 0;
    MoveWindow(priv->hWnd, r.left, r.top, width, height, TRUE);
  }
}


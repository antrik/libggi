/* $Id: ddinit.c,v 1.14 2003/10/10 05:48:20 cegger Exp $
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


static int CreatePrimary(directx_priv * priv);
static int CreateBackup(directx_priv * priv);
static void ReleaseAllObjects(directx_priv * priv);


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


long FAR PASCAL
WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	DDMBS DDMessage;
	DDCMS ddcms;
	gii_event giiev;
	directx_priv *priv =
	  (directx_priv *)GetWindowLong(hWnd, GWL_USERDATA);


	switch (message) {

	case WM_MOVE:
		ggLock(priv->lock);
		GetClientRect(hWnd, &priv->SrcWinPos);
		GetClientRect(hWnd, &priv->DestWinPos);
		ClientToScreen(hWnd, (POINT *) & priv->DestWinPos.left);
		ClientToScreen(hWnd, (POINT *) & priv->DestWinPos.right);
		ggUnlock(priv->lock);
		return 0;


	case WM_TIMER:
		ggLock(priv->lock);
		DDRedraw(priv);
		ggUnlock(priv->lock);
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		ggLock(priv->lock);
		DDRedraw(priv);
		ggUnlock(priv->lock);
		EndPaint(hWnd, &ps);
		return 0;

	case WM_DESTROY:
		/* Clean up and close the app */
		ggLock(priv->lock);
		ReleaseAllObjects(priv);
		ggUnlock(priv->lock);
		PostQuitMessage(0);
		return 0;

	case WM_DDMESSAGEBOX:
		DDMessage = *(LPDDMBS) lParam;
		MessageBox(DDMessage.hWnd, DDMessage.text, DDMessage.caption, DDMessage.type);
		return 0;

	case WM_DDCHANGEMODE:
		ddcms = *(LPDDCMS) lParam;
		/* GGI_directx_setmode already holds the lock here */
		ReleaseAllObjects(priv);
		IDirectDraw_SetCooperativeLevel(priv->lpddext, hWnd, DDSCL_NORMAL);
		CreatePrimary(priv);
		CreateBackup(priv);
		/* temporarily release the lock so that WM_MOVE can be
		   processed */
		ggUnlock(priv->lock);
		MoveWindow(hWnd, 50, 50, ddcms.width + 8, ddcms.height + 28, TRUE);
		SetTimer(hWnd, 1, 33, NULL);
		ShowWindow(hWnd, SW_SHOWNORMAL);
		ggLock(priv->lock);
		return 0;

	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

static HRESULT DDMessageBox(HWND hWnd, LPCTSTR text, LPCTSTR caption)
{
	DDMBS MessageData;

	MessageData.hWnd = hWnd;
	MessageData.text = text;
	MessageData.caption = caption;
	MessageData.type = MB_OK;
	SendMessage(hWnd, WM_DDMESSAGEBOX, 0, (LPARAM) & MessageData);

	return 0;
}

static void ReleaseAllObjects(directx_priv * priv)
{
	if (priv->lpbdds != NULL) {
		IDirectDraw_Release(priv->lpbdds);
		priv->lpbdds = NULL;
	}
	if (priv->lppdds != NULL) {
		IDirectDraw_Release(priv->lppdds);
		priv->lppdds = NULL;
	}
}

static int CreatePrimary(directx_priv * priv)
{
	HRESULT hr;
	LPDIRECTDRAWCLIPPER pClipper;
	char errstr[50];
	DDSURFACEDESC pddsd;

	memset(&pddsd, 0, sizeof(pddsd));
	pddsd.dwSize = sizeof(pddsd);
	pddsd.dwFlags = DDSD_CAPS;
	pddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	hr = IDirectDraw_CreateSurface(priv->lpddext, &pddsd, &priv->lppdds, NULL);
	if (hr != 0) {
#ifdef HAVE_SNPRINTF
		snprintf(errstr, 50, "Init Primary Surface Failed RC = %d.  Exiting", (int)hr);
#else
		sprintf(errstr, "Init Primary Surface Failed RC = %d.  Exiting", (int)hr);
#endif
		DDMessageBox(priv->hWnd, errstr, "Primary Surface");
		exit(-1);
	}	/* if */
	IDirectDraw_CreateClipper(priv->lpddext, 0, &pClipper, NULL);
	IDirectDrawClipper_SetHWnd(pClipper, 0, priv->hWnd);
	IDirectDrawSurface_SetClipper(priv->lppdds, pClipper);
	IDirectDrawClipper_Release(pClipper);
	pddsd.dwSize = sizeof(pddsd);
	return 0;
}

static int CreateBackup(directx_priv * priv)
{
        HRESULT rc;
        char message[100];
	DDSURFACEDESC pddsd, bddsd;

	pddsd.dwSize = sizeof(pddsd);
	IDirectDrawSurface_GetSurfaceDesc(priv->lppdds, &pddsd);

        memset(&bddsd, 0, sizeof(bddsd));
        bddsd.dwSize = sizeof(bddsd);
        bddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH
            | DDSD_PIXELFORMAT;
        bddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
        bddsd.dwWidth = pddsd.dwWidth;
        bddsd.dwHeight = pddsd.dwHeight;
        bddsd.lPitch = pddsd.lPitch;

/* Set up the pixel format */
	ZeroMemory(&bddsd.ddpfPixelFormat, sizeof(DDPIXELFORMAT));
	bddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	bddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	bddsd.ddpfPixelFormat.dwRGBBitCount = pddsd.ddpfPixelFormat.dwRGBBitCount;
	bddsd.ddpfPixelFormat.dwRBitMask = pddsd.ddpfPixelFormat.dwRBitMask;
	bddsd.ddpfPixelFormat.dwGBitMask = pddsd.ddpfPixelFormat.dwGBitMask;
	bddsd.ddpfPixelFormat.dwBBitMask = pddsd.ddpfPixelFormat.dwBBitMask;

	rc = IDirectDraw_CreateSurface(priv->lpddext, &bddsd, &priv->lpbdds, NULL);
	if (rc) {
#ifdef HAVE_SNPRINTF
		sprintf(message, 100, "CreateBackup error : %ld. Exiting", rc & 0xffff);
#else
		sprintf(message, "CreateBackup error : %ld. Exiting", rc & 0xffff);
#endif
		DDMessageBox(priv->hWnd, message, "Backup");
		exit(-1);
	}
	IDirectDrawSurface2_Lock(priv->lpbdds, NULL, &bddsd, DDLOCK_SURFACEMEMORYPTR, NULL);
	priv->lpSurfaceAdd = (char *) bddsd.lpSurface;
	IDirectDrawSurface2_Unlock(priv->lpbdds, bddsd.lpSurface);

	return rc;
}

static DWORD WINAPI DDInitThread(LPVOID lpParm);

HANDLE
DDInit(directx_priv * priv)
{
	DWORD ThreadID;

	priv->hInit = CreateEvent(NULL, FALSE, FALSE, NULL);
	priv->hThreadID = CreateThread(NULL, 0, DDInitThread, (LPVOID) priv, 0,
				       &ThreadID);
	if (priv->hThreadID) {
		WaitForSingleObject(priv->hInit, INFINITE);
	}
	return priv->hThreadID;
}

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
 
static DWORD WINAPI
DDInitThread(LPVOID lpParm)
{
	WNDCLASS wc;
	MSG msg;
	directx_priv *priv = (directx_priv*)lpParm;

	priv->hInstance = GetModuleHandle(NULL);

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

	/* Create a window */
	priv->hWnd = CreateWindowEx(0,
			      NAME,
			      TITLE,
			      WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX,
			      0,
			      0,
			      640 + 8,
			      480 + 28,
			      NULL,
			      NULL,
			      priv->hInstance,
			      NULL);
	if (!priv->hWnd) {
		if (priv->hCursor) DestroyCursor(priv->hCursor);
		return FALSE;
	}
	SetWindowLong(priv->hWnd, GWL_USERDATA, (DWORD)priv);
	ShowWindow(priv->hWnd, SW_HIDE);

	DirectDrawCreate(NULL, &priv->lpdd, NULL);
	IDirectDraw_QueryInterface(priv->lpdd, &IID_IDirectDraw2, (LPVOID *) & priv->lpddext);
	SetEvent(priv->hInit);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

HRESULT DDRedraw(directx_priv * priv)
{
	IDirectDrawSurface_Blt(priv->lppdds, &priv->DestWinPos,
			       priv->lpbdds, &priv->SrcWinPos, DDBLT_WAIT, NULL);
	return 0;
}

int DDChangeMode(directx_priv * priv, DWORD width, DWORD height, DWORD BPP)
{
	DDCMS ddcms;
	char message[100];
	int rc;

	ddcms.width = width;
	ddcms.height = height;
	ddcms.BPP = BPP;


	rc = SendMessage(priv->hWnd, WM_DDCHANGEMODE, 0, (LPARAM) &ddcms);
	if (rc) {
#ifdef HAVE_SNPRINTF
		snprintf(message, 100, "ChangeMode failed with rc= %d\n",
			rc & 0xffff);
#else
		sprintf(message, "ChangeMode failed with rc= %d\n",
			rc & 0xffff);
#endif
		DDMessageBox(priv->hWnd, message, "INFO");
	} else {
		DDSURFACEDESC pddsd;

		pddsd.dwSize = sizeof(pddsd);
		IDirectDrawSurface_GetSurfaceDesc(priv->lppdds, &pddsd);

		priv->maxX = pddsd.dwWidth;
		priv->maxY = pddsd.dwHeight;
		priv->ColorDepth = pddsd.ddpfPixelFormat.dwRGBBitCount;
		priv->BPP = priv->ColorDepth / 8;
		priv->pitch = priv->maxX * priv->BPP;
	}

	return rc;
}

HRESULT DDShutdown(directx_priv * priv)
{
	HRESULT rc = SendMessage(priv->hWnd, WM_DESTROY, 0, (LPARAM) NULL);
	if (priv->hThreadID) {
	  /* wait for the event loop to finish */
	  if (WaitForSingleObject(priv->hThreadID, 100) != WAIT_OBJECT_0) {
	    /* asta la vista, baby */
	    TerminateThread(priv->hThreadID, 0);
	  }
	}
	priv->hThreadID = NULL;
	return rc;
}

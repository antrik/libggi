/* $Id: ddinit.c,v 1.6 2003/10/07 15:27:51 cegger Exp $
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

#include "../common/ggi-auto.inc"


LPDIRECTDRAW lpdd = NULL;
LPDIRECTDRAW4 lpddext = NULL;
LPDIRECTDRAWSURFACE4 lppdds = NULL;
LPDIRECTDRAWSURFACE4 lpbdds = NULL;
LPDIRECTDRAWCLIPPER pClipper;
DDSURFACEDESC2 pddsd;
DDSURFACEDESC2 bddsd;
DDPIXELFORMAT ddpf;
HWND hWnd;
HINSTANCE hInstance;
HANDLE hSem;
HANDLE hThreadID;
char *lpSurfaceAdd;
static int Active = 0;
static int PtrActive = 1;
static int ClipActive = 0;
static MINMAXINFO MaxSize;
RECT DestWinPos, SrcWinPos;

DisplayMode DisplayModes[MAX_DISPLAYMODES];
int nDisplayModes = 0;

int CreatePrimary(void);
int CreateBackup(void);
int GetDesc(directx_priv *priv);
void ReleaseAllObjects(void);

static HRESULT CALLBACK EnumDisplayModesCallback(LPDDSURFACEDESC pddsd,
						 LPVOID Context);


long FAR PASCAL
WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	DDMBS DDMessage;
	DDCMS ddcms;
	gii_event giiev;


	switch (message) {

	case WM_ACTIVATEAPP:
		/* Pause if minimized or not the top window */
		Active = 1;
		return 0;

	case WM_KILLFOCUS:
		PtrActive = 1;
		Active = 0;
		return 0;

	case WM_SETFOCUS:
		Active = 1;
		return 0;

	case WM_GETMINMAXINFO:

		((LPMINMAXINFO) lParam)->ptMaxSize.x = MaxSize.ptMaxSize.x;
		((LPMINMAXINFO) lParam)->ptMaxSize.y = MaxSize.ptMaxSize.y;
		return 0;

	case WM_MOVE:
		GetClientRect(hWnd, &SrcWinPos);
		GetClientRect(hWnd, &DestWinPos);
		ClientToScreen(hWnd, (POINT *) & DestWinPos.left);
		ClientToScreen(hWnd, (POINT *) & DestWinPos.right);
		return 0;


	case WM_TIMER:
		redraw();
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		redraw();
		EndPaint(hWnd, &ps);
		return 0;

	case WM_DESTROY:
		/* Clean up and close the app */
		ReleaseAllObjects();
		PostQuitMessage(0);
		return 0;

	case WM_DDMESSAGEBOX:
		DDMessage = *(LPDDMBS) lParam;
		MessageBox(DDMessage.hWnd, DDMessage.text, DDMessage.caption, DDMessage.type);
		return 0;

	case WM_DDCHANGEMODE:
		ddcms = *(LPDDCMS) lParam;
		ReleaseAllObjects();
		IDirectDraw_SetCooperativeLevel(lpddext, hWnd, DDSCL_NORMAL);
		CreatePrimary();
		CreateBackup();
		MaxSize.ptMaxSize.x = ddcms.width + 8;
		MaxSize.ptMaxSize.y = ddcms.height + 28;
		MoveWindow(hWnd, 50, 50, ddcms.width + 8, ddcms.height + 28, TRUE);
		ShowWindow(hWnd, SW_SHOWNORMAL);
		SetTimer(hWnd, 1, 33, NULL);
		return 0;

	case WM_MOUSEMOVE:
		if (PtrActive && Active) {
			PtrActive = 0;
			while (ShowCursor(FALSE) >= 0);
		}	/* if */
		break;

	case WM_NCMOUSEMOVE:
		if (!PtrActive) {
			PtrActive = 1;
			while (ShowCursor(TRUE) < 0);
		}	/* if */
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void ReleaseAllObjects(void)
{
	if (lpbdds != NULL) {
		IDirectDraw_Release(lpbdds);
		lpbdds = NULL;
	}
	if (lppdds != NULL) {
		IDirectDraw_Release(lppdds);
		lppdds = NULL;
	}
}

int CreatePrimary(void)
{
	HRESULT hr;
	LPDIRECTDRAWCLIPPER pClipper;
	char errstr[50];


	memset(&pddsd, 0, sizeof(pddsd));
	pddsd.dwSize = sizeof(pddsd);
	pddsd.dwFlags = DDSD_CAPS;
	pddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	hr = IDirectDraw_CreateSurface(lpddext, &pddsd, &lppdds, NULL);
	if (hr != 0) {
		sprintf(errstr, "Init Primary Surface Failed RC = %d.  Exiting", (int)hr);
		DDMessageBox(hWnd, errstr, "Primary Surface");
		exit(-1);
	}	/* if */
	IDirectDraw_CreateClipper(lpddext, 0, &pClipper, NULL);
	IDirectDrawClipper_SetHWnd(pClipper, 0, hWnd);
	IDirectDrawSurface_SetClipper(lppdds, pClipper);
	IDirectDrawClipper_Release(pClipper);
	pddsd.dwSize = sizeof(pddsd);
	return IDirectDrawSurface_GetSurfaceDesc(lppdds, &pddsd);
}

int GetDesc(directx_priv * priv)
{
	pddsd.dwSize = sizeof(pddsd);

	IDirectDrawSurface_GetSurfaceDesc(lppdds, &pddsd);

	priv->hWnd = hWnd;

/*	priv->pitch = pddsd.lPitch; */

	priv->maxX = pddsd.dwWidth;
	priv->maxY = pddsd.dwHeight;
	priv->ColorDepth = pddsd.ddpfPixelFormat.dwRGBBitCount;
	priv->BPP = priv->ColorDepth / 8;
	priv->pitch = priv->maxX * priv->BPP;

/*	priv->RedMask = pddsd.ddpfPixelFormat.dwRBitMask;
	priv->GreenMask = pddsd.ddpfPixelFormat.dwGBitMask;
	priv->BlueMask = pddsd.ddpfPixelFormat.dwBBitMask;
*/
	return 0;
}

int CreateBackup(void)
{
        HRESULT rc;
        char message[100];

        memset(&bddsd, 0, sizeof(bddsd));
        bddsd.dwSize = sizeof(bddsd);
        bddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH
            | DDSD_LPSURFACE | DDSD_PIXELFORMAT;
        bddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
        lpSurfaceAdd = (char *) malloc(pddsd.dwWidth * pddsd.dwHeight
                              * pddsd.ddpfPixelFormat.dwRGBBitCount / 8);
        ZeroMemory(lpSurfaceAdd, (DWORD) (pddsd.dwWidth * pddsd.dwHeight
                             * pddsd.ddpfPixelFormat.dwRGBBitCount / 8));
        bddsd.lpSurface = lpSurfaceAdd;
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

        rc = IDirectDraw_CreateSurface(lpddext, &bddsd, &lpbdds, NULL);
        if (rc) {
                sprintf(message, "CreateBackup error : %ld", rc & 0xffff);
                DDMessageBox(hWnd, message, "Redraw");
        }
        return rc;
}

HANDLE
DDInit(directx_priv * priv)
{
        DWORD ThreadID;

        hSem = CreateSemaphore(NULL, 0, 1, NULL);
        hThreadID = CreateThread(NULL, 0, DDInitThread, (LPVOID) priv, 0,
                                 &ThreadID);
        if (hThreadID) {
                WaitForSingleObject(hSem, INFINITE);
        }
        priv->hWnd = hWnd;
        priv->hInstance = hInstance;
        return hThreadID;
}

DWORD WINAPI
DDInitThread(LPVOID lpParm)
{
        WNDCLASS wc;
        MSG msg;

        Active = 0;
        hInstance = GetModuleHandle(NULL);

        wc.style = 0;
        wc.lpfnWndProc = WindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE("DirectX.ico"));
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
        wc.lpszMenuName = NAME;
        wc.lpszClassName = NAME;
        RegisterClass(&wc);

        /* Create a window */
        hWnd = CreateWindowEx(0,
                              NAME,
                              TITLE,
                              WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX,
                              0,
                              0,
                              640 + 8,
                              480 + 28,
                              NULL,
                              NULL,
                              hInstance,
                              NULL);
        if (!hWnd)
                return FALSE;

        DirectDrawCreate(NULL, &lpdd, NULL);
        IDirectDraw_QueryInterface(lpdd, &IID_IDirectDraw4, (LPVOID *) & lpddext);
        IDirectDraw_EnumDisplayModes(lpdd, 0, NULL, NULL, EnumDisplayModesCallback);
        ReleaseSemaphore(hSem, 1, NULL);
        Active = 1;

        while (GetMessage(&msg, NULL, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }

        return msg.wParam;
}

char locked = 0;

char *DDLock(void)
{
        return (char *) lpSurfaceAdd;
}

HRESULT DDUnlock(void)
{
        return 0;
}

HRESULT redraw(void)
{

        IDirectDrawSurface_Blt(lppdds, &DestWinPos,
                               lpbdds, &SrcWinPos, DDBLT_WAIT, NULL);
        return 0;
}

int DXChangeMode(directx_priv * priv, DWORD width, DWORD height, DWORD BPP)
{
        DDCMS ddcms;
        char message[100];
        int rc;

        ddcms.width = width;
        ddcms.height = height;
        ddcms.BPP = BPP;


        rc = DDChangeMode(priv, &ddcms);
        if (rc) {
                sprintf(message, "ChangeMode failed with rc= %d\n",
                        rc & 0xffff);
                DDMessageBox(hWnd, message, "INFO");
        } else {
                GetDesc(priv);
        }

        return rc;
}

HRESULT DDChangeMode(directx_priv * priv, DDCMS * ddcms)
{
        return SendMessage(hWnd, WM_DDCHANGEMODE, 0, (LPARAM) ddcms);
}

int DDCheckMode(ggi_visual *vis, ggi_mode * mode)
{
	uint8 i;
	uint8 err = 0;
	uint8 modefound;

	/* handle AUTO */
	_GGIhandle_ggiauto(mode, 640, 480);

	if (mode->frames < 1) {
		err = -1;
		mode->frames = 1;
	} else if (mode->frames > 2) {
		err = -1;
		mode->frames = 1;
	}

	if (GT_DEPTH(mode->graphtype) == GGI_AUTO) {
		HWND wnd = GetDesktopWindow();
                HDC dc = GetDC(wnd);
                int depth = GetDeviceCaps(dc, BITSPIXEL);
                ReleaseDC(wnd, dc);
                switch (depth) {
                case 1:
                        mode->graphtype = GT_1BIT;
                        break;
                case 2:
                        mode->graphtype = GT_2BIT;
                        break;
                case 4:
                        mode->graphtype = GT_4BIT;
                        break;
                case 8:
                        mode->graphtype = GT_8BIT;
                        break;
                case 15:
                        mode->graphtype = GT_15BIT;
                        break;
                case 16:
                        mode->graphtype = GT_16BIT;
                        break;  
                case 24:
                        mode->graphtype = GT_24BIT;
                        break;
                case 32:
                        mode->graphtype = GT_32BIT;
                        break;
		}
	}

	modefound = 0;
	for (i = 0; i < nDisplayModes; i++) {
		if (DisplayModes[i].width == mode->visible.x
		    && DisplayModes[i].height == mode->visible.y
		    && DisplayModes[i].bpp == GT_SIZE(mode->graphtype))
		{
			modefound = 1;
		}
	}

	if (!modefound) {
		mode->visible.x = 640;
		mode->visible.y = 480;
		mode->graphtype = GT_16BIT;
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


static HRESULT CALLBACK EnumDisplayModesCallback(LPDDSURFACEDESC pddsd, LPVOID Context)
{
        if (nDisplayModes == MAX_DISPLAYMODES - 1)
                return DDENUMRET_CANCEL;

	DisplayModes[nDisplayModes].width = pddsd->dwWidth;
	DisplayModes[nDisplayModes].height = pddsd->dwHeight;
	DisplayModes[nDisplayModes].bpp = pddsd->ddpfPixelFormat.dwRGBBitCount;

	nDisplayModes++;

	return DDENUMRET_OK;
}


DDMBS MessageData;

HRESULT DDMessageBox(HWND hWnd, LPCTSTR text, LPCTSTR caption)
{

	MessageData.hWnd = hWnd;
	MessageData.text = text;
	MessageData.caption = caption;
	MessageData.type = MB_OK;
	SendMessage(hWnd, WM_DDMESSAGEBOX, 0, (LPARAM) & MessageData);
	return 0;
}

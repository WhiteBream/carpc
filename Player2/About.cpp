// About.cpp: implementation of the About class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "About.h"
#include <stdio.h>
#include <shlobj.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

typedef struct _AboutParms
{
	Graphics   * gr;
	HINSTANCE    hInst;
	HWND         hParent;
	RECT         ourRect;
	Brush      * br_blue;
	Font       * fn;
	INT          offset;
	HICON        ico;
	BOOL         clicked;
	HBITMAP		 hBmp;
} AboutParms;

LRESULT CALLBACK AboutWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ATOM RegisterAboutClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize         = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= AboutWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(LONG_PTR);
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= CreateSolidBrush(RGB(200, 255, 200));
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_ABOUT");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

static const LPWSTR pDefault = L"Version 1.2, \xA9 2007\n"
								L"Copyright \xA9 2000-2005, Henk Bliek\n"
								L"Copyright \xA9 2006-2007, White Bream\n\n"
								L"Database powered by SqLite3\n"
								L"Multimedia by Microsoft DirectShow";

LRESULT CALLBACK AboutWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPCREATESTRUCT	cs;
	PAINTSTRUCT		ps;
	AboutParms		*dw;
	RECT			pRect;
	RectF			rf;
	StringFormat	fmt;

	dw = (AboutParms *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if((dw == NULL) && (message != WM_CREATE))
		return(DefWindowProc(hWnd, message, wParam, lParam));

	GetClientRect(hWnd, &pRect);

	switch (message)
	{
		case WM_CREATE:
			dw = (AboutParms *)GlobalAlloc(GPTR, sizeof(AboutParms));
			if(dw == NULL)
				return(1L);	// Error
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)dw);

			cs = (LPCREATESTRUCT)lParam;
			dw->hParent = cs->hwndParent;
			dw->hInst = cs->hInstance;
			dw->offset = (INT)cs->lpCreateParams;

			dw->br_blue = new SolidBrush(Color(255, 0, 0, 192));
			dw->fn = new Font(L"Arial", (float)dw->offset / 3);
			dw->gr = new Graphics(hWnd, FALSE);
			dw->clicked = FALSE;

			CreateWindow(TEXT("STATIC"), NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY, dw->offset / 2, dw->offset / 2,
				122, 24, hWnd, (HMENU)IDC_CARGOLOGO, dw->hInst, NULL);
			dw->hBmp = LoadBitmap(dw->hInst, MAKEINTRESOURCE(IDB_CARGO));
			SendDlgItemMessage(hWnd, IDC_CARGOLOGO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)dw->hBmp);

			/* Window elements */
//			dw->ico = LoadIcon(cs->hInstance, (LPCTSTR)IDI_INFO);
//			if(dw->ico)
//			{
//				hdc = GetDC(hWnd);
//				DrawIconEx(hdc, 10, 10, dw->ico, 48, 48, NULL, NULL, DI_NORMAL);
//				ReleaseDC(hWnd, hdc);
//			}
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
//			if(dw->ico)
//			{
//				hdc = GetDC(hWnd);
//				DrawIconEx(hdc, 10, 10, dw->ico, 48, 48, NULL, NULL, DI_NORMAL);
//				ReleaseDC(hWnd, hdc);
//			}

			/* Name text */
			rf.X = (float)dw->offset + 122;
			rf.Y = (float)dw->offset / 2;
			rf.Width = (float)pRect.right - dw->offset - 122;
			rf.Height = (float)dw->offset / 2;
			fmt.SetAlignment(StringAlignmentNear);
			fmt.SetTrimming(StringTrimmingNone);
			dw->gr->DrawString(L"Media Player", -1, dw->fn, rf, &fmt, dw->br_blue);

			/* Info text */
			rf.X = (float)dw->offset;
			rf.Y = (float)3 * dw->offset / 2;
			rf.Width = (float)pRect.right - 3 * dw->offset / 2;
			rf.Height = (float)pRect.bottom - 3 * dw->offset / 2;
			dw->gr->DrawString(pDefault, -1, dw->fn, rf, &fmt, dw->br_blue);

			EndPaint(hWnd, &ps);
			break;

		case WM_NCDESTROY:
			delete dw->gr;
			delete dw->fn;
			delete dw->br_blue;
			if(dw->ico)
				DestroyIcon(dw->ico);
			if(dw->hBmp)
				DeleteObject(dw->hBmp);
			GlobalFree(dw);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
			break;

		case WM_LBUTTONDOWN:
			dw->clicked = TRUE;
			break;

		case WM_LBUTTONUP:
			if(dw->clicked == TRUE)
				PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}


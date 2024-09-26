// Help.cpp: implementation of the Help class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Help.h"
#include <stdio.h>
#include <shlobj.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

typedef struct _HelpParms
{
	Graphics   * gr;
	HINSTANCE    hInst;
	HWND         hParent;
	RECT         ourRect;
	Brush      * br_blue;
	Font       * fn;
	INT          offset;
	HICON        ico;
	BOOL		clicked;
} HelpParms;

LRESULT CALLBACK HelpWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ATOM RegisterHelpClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize         = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= HelpWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(LONG_PTR);
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= CreateSolidBrush(RGB(200, 255, 200));
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_HELP");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

static const LPWSTR pDefault = L"Tap a button for a short explanation of its function\n\n\n(not supported yet)";

LRESULT CALLBACK HelpWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPCREATESTRUCT	cs;
	PAINTSTRUCT		ps;
	HelpParms		*dw;
	RECT			pRect;
	RectF			rf;
	StringFormat	fmt;
	HDC				hdc;

	dw = (HelpParms *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if((dw == NULL) && (message != WM_CREATE))
		return(DefWindowProc(hWnd, message, wParam, lParam));

	GetClientRect(hWnd, &pRect);

	switch (message)
	{
		case WM_CREATE:
			dw = (HelpParms *)GlobalAlloc(GPTR, sizeof(HelpParms));
			if(dw == NULL)
				return(1L);	// Error
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)dw);

			cs = (LPCREATESTRUCT)lParam;
			dw->hParent = cs->hwndParent;
			dw->offset = (INT)cs->lpCreateParams;

			dw->br_blue = new SolidBrush(Color(255, 0, 0, 192));
			dw->fn = new Font(L"Arial", (float)dw->offset / 3);
			dw->gr = new Graphics(hWnd, FALSE);
			dw->clicked = FALSE;

			/* Window elements */
			dw->ico = LoadIcon(cs->hInstance, (LPCTSTR)IDI_INFO);
			if(dw->ico)
			{
				hdc = GetDC(hWnd);
				DrawIconEx(hdc, 10, 10, dw->ico, 48, 48, NULL, NULL, DI_NORMAL);
				ReleaseDC(hWnd, hdc);
			}
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			if(dw->ico)
			{
				hdc = GetDC(hWnd);
				DrawIconEx(hdc, 10, 10, dw->ico, 48, 48, NULL, NULL, DI_NORMAL);
				ReleaseDC(hWnd, hdc);
			}

			/* Info text */
			rf.X = (float)dw->offset / 2 + 48;
			rf.Y = (float)dw->offset / 2 - 3;
			rf.Width = (float)pRect.right - dw->offset - 48;
			rf.Height = (float)pRect.bottom - dw->offset / 2;
			fmt.SetAlignment(StringAlignmentNear);
			fmt.SetTrimming(StringTrimmingNone);
			dw->gr->DrawString(pDefault, -1, dw->fn, rf, &fmt, dw->br_blue);

			EndPaint(hWnd, &ps);
			break;

		case WM_SETTEXT:
			break;

		case WM_NCDESTROY:
			delete dw->gr;
			delete dw->fn;
			delete dw->br_blue;
			if(dw->ico)
				DestroyIcon(dw->ico);
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


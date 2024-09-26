// Remove.cpp: implementation of the Remove class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Remove.h"
#include <stdio.h>
#include <shlobj.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

typedef struct _RemoveParms
{
	Graphics   * gr;
	Brush      * bkGnd;
	HINSTANCE    hInst;
	HWND         hParent;
	RECT         ourRect;
	Brush      * br_black;
	Brush      * br_white;
	Brush      * br_blue;
	Pen        * pn;
	Font       * fn;
	INT          offset;
	Database   * hDb;
	MediaDescr * pMedia;
} RemoveParms;

LRESULT CALLBACK RemoveWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ATOM RegisterRemoveClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize         = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= RemoveWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(LONG_PTR);
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_REMOVE");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}


LRESULT CALLBACK RemoveWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPCREATESTRUCT  cs;
	PAINTSTRUCT     ps;
	RemoveParms   * dw;
	size_t          len;
	RECT            pRect, pEdit;
	INT             col1, col2, col3;
	LPTSTR			pString;
	RectF      rf;
	StringFormat        fmt;
	HFONT hArial;
	LOGFONT lf;

	dw = (RemoveParms *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if((dw == NULL) && (message != WM_CREATE))
		return(DefWindowProc(hWnd, message, wParam, lParam));

	GetClientRect(hWnd, &pRect);

	switch (message)
	{
		case WM_NCCREATE:

			break;

		case WM_CREATE:
			dw = (RemoveParms *)GlobalAlloc(GPTR, sizeof(RemoveParms));
			if(dw == NULL)
				return(1L);	// Error
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)dw);

			cs = (LPCREATESTRUCT)lParam;
			dw->hParent = cs->hwndParent;
			dw->offset = ((RemoveParm *)cs->lpCreateParams)->offset;
			dw->hDb = ((RemoveParm *)cs->lpCreateParams)->hDb;
			dw->pMedia = dw->hDb->CopyDescr(((RemoveParm *)cs->lpCreateParams)->pMedia);

			dw->bkGnd = new SolidBrush(Color(255, 200, 255, 200));
			dw->br_black = new SolidBrush(Color(255, 0, 0, 0));
			dw->br_white = new SolidBrush(Color(255, 255, 255, 255));
			dw->br_blue = new SolidBrush(Color(255, 0, 0, 192));
			dw->pn = new Pen(Color(255, 0, 0, 0), 1);
			dw->fn = new Font(L"Arial", (float)dw->offset / 3);
			dw->gr = new Graphics(hWnd, FALSE);

			/* Window elements */
			len = 1 + dw->offset / 3;
			col1 = pRect.right / 3;
			col2 = col1 + pRect.right / 3;
			col3 = pRect.right;

			CreateWindow(TEXT("EDIT"), NULL, WS_CHILD | WS_VISIBLE | ES_READONLY | ES_MULTILINE, dw->offset / 2, pRect.bottom / 2,
				pRect.right - dw->offset - 2, dw->offset - 2, hWnd, (HMENU)IDC_DIR, cs->hInstance, NULL);
			pString = dw->hDb->DescriptorToDoubleString(dw->pMedia, (MediaField)(FLD_ARTIST | FLD_TITLE));
			SetWindowText(GetDlgItem(hWnd, IDC_DIR), pString);
			GlobalFree((HGLOBAL)pString);
			/* Set edit font */
			memset(&lf, 0, sizeof(lf));
			lf.lfHeight = 24;
			lf.lfWeight = 500;
			StringCbCopyW(lf.lfFaceName, sizeof(lf.lfFaceName), (LPCWSTR)L"Arial");
			hArial = CreateFontIndirect(&lf);
			SendMessage(GetDlgItem(hWnd, IDC_DIR), WM_SETFONT, (WPARAM)hArial, (LPARAM)TRUE);


			CreateWindow(WBBUTTON, TEXT("REMOVE"), WS_CHILD | WS_VISIBLE, 0, pRect.bottom - dw->offset,
				col1 - 0, dw->offset, hWnd, (HMENU)IDC_GO, cs->hInstance, NULL);

			CreateWindow(WBBUTTON, TEXT("DELETE"), WS_CHILD | WS_VISIBLE, col1, pRect.bottom - dw->offset,
				col2 - col1, dw->offset, hWnd, (HMENU)IDC_OK, cs->hInstance, NULL);

			CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, col2, pRect.bottom - dw->offset,
				col3 - col2, dw->offset, hWnd, (HMENU)IDC_CANCEL, cs->hInstance, NULL);
			AssignIcon(cs->hInstance, hWnd, IDC_CANCEL, IDI_DEL);

			SetFocus(GetDlgItem(hWnd, IDC_CANCEL));
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			dw->gr->FillRectangle(dw->bkGnd, 0, 0, pRect.right, pRect.bottom);
			len = pRect.bottom;
			/* Border around edit control */
			GetClientRect(GetDlgItem(hWnd, IDC_DIR), &pEdit);
			dw->gr->DrawRectangle(dw->pn, dw->offset / 2 - 1, pRect.bottom / 2 - 1, pEdit.right + 1, pEdit.bottom + 1);
			/* Info text */
			rf.X = (float)dw->offset / 2 - 3;
			rf.Y = (float)dw->offset / 2 - 3;
			rf.Width = (float)pRect.right - dw->offset;
			rf.Height = (float)3 * dw->offset / 2;
			fmt.SetAlignment(StringAlignmentNear);
			fmt.SetTrimming(StringTrimmingNone);
			dw->gr->DrawString(L"Remove file from database or delete file from harddisk?", -1, dw->fn, rf, &fmt, dw->br_blue);

			EndPaint(hWnd, &ps);
			break;

		case WM_NCDESTROY:
			delete dw->gr;
			delete dw->fn;
			delete dw->br_black;
			delete dw->br_white;
			delete dw->br_blue;
			delete dw->pn;
			delete dw->bkGnd;
			GlobalFree(dw->pMedia);
			GlobalFree(dw);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
			break;

		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					switch(LOWORD(wParam))
					{
						case IDC_GO:
							SendMessage(dw->hParent, WM_REMOVEDELETE, (WPARAM)1, (LPARAM)dw->pMedia);
							PostMessage(hWnd, WM_CLOSE, 0, 0);
							break;

						case IDC_OK:
							SendMessage(dw->hParent, WM_REMOVEDELETE, (WPARAM)2, (LPARAM)dw->pMedia);
							PostMessage(hWnd, WM_CLOSE, 0, 0);
							break;

						case IDC_CANCEL:
							/* DestroyWindow gives troubles! */
							SendMessage(dw->hParent, WM_REMOVEDELETE, (WPARAM)0, (LPARAM)dw->pMedia);
							PostMessage(hWnd, WM_CLOSE, 0, 0);
							break;
					}
					break;
			}
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}


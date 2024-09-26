// Edit.cpp: implementation of the Edit library.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Edit.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define MAX_EDIT_LENGTH		25

typedef struct _EditWndInfo
{
	int            id;
	POINT          pWnd;
	HWND           hParent;
	HWND           hEdit;
	HDC            hdc;
	Graphics     * editGr;
	Font         * font;
	RectF	     * fontRect;
	StringFormat * fontFormat;
	Brush        * fontColor;
	Brush        * backColor;
	Pen			 * borderColor;
	WCHAR          text[MAX_EDIT_LENGTH];
} EditWndInfo;

LRESULT CALLBACK EditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


#define CLASSNAME		TEXT("WB_EDIT")

//////////////////////////////////////////////////////////////////////
int EditInit(HINSTANCE hInstance, BOOL vLoadUnload)
{
	WNDCLASSEX     wcex;
	int             ret;

	if(vLoadUnload)
	{
		wcex.cbSize			= sizeof(WNDCLASSEX);
		wcex.style			= 0;
		wcex.lpfnWndProc	= EditProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= sizeof(LONG_PTR);
		wcex.hInstance		= hInstance;
		wcex.hIcon			= NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= NULL;
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= CLASSNAME;
		wcex.hIconSm		= NULL;

		ret = RegisterClassEx(&wcex);
	}
	else
	{
		ret = UnregisterClass(CLASSNAME, hInstance);
	}
	return(ret);
}

LRESULT CALLBACK EditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT     ps;
	EditWndInfo   * bi;
	LPCREATESTRUCT  cs;
	INT             fs;
	RECT            wr;

	if(message == WM_CREATE)
	{
		bi = (EditWndInfo *)GlobalAlloc(GPTR, sizeof(EditWndInfo));
		if(bi == NULL)
			return(1L);	// Error
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)bi);

		cs = (LPCREATESTRUCT)lParam;
		bi->hParent = cs->hwndParent;
		bi->id = (int)cs->hMenu;
		fs = 16;
		if((cs->cy / 3) < fs)
			fs = cs->cy / 3;
		bi->font = new Font(L"Arial", (float)fs);
		bi->fontFormat = new StringFormat();
		bi->fontFormat->SetAlignment(StringAlignmentCenter);
		bi->fontFormat->SetTrimming(StringTrimmingNone);
		bi->fontFormat->SetLineAlignment(StringAlignmentCenter);
		//bi->fontFormat->SetFormatFlags(StringFormatFlagsNoWrap);
		bi->fontRect = new RectF(4, 4, (float)cs->cx - 8, (float)cs->cy - 8);
		bi->fontColor = new SolidBrush(Color(255, 0, 0, 0));
		bi->backColor = new SolidBrush(Color(255, 255, 255, 255));
		bi->borderColor = new Pen(Color(255, 0, 0, 0));
		bi->hdc = GetDC(hWnd);
		bi->editGr = new Graphics(hWnd, FALSE);
		bi->pWnd.x = cs->cx;
		bi->pWnd.y = cs->cy;
		bi->hEdit = CreateWindow(TEXT("EDIT"), NULL, WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)111, NULL, NULL);
		memset(bi->text, 0, sizeof(bi->text));
		return(0);
	}

	bi = (EditWndInfo *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if(bi == NULL)
		return(DefWindowProc(hWnd, message, wParam, lParam));

	switch (message)
	{
		case WM_DESTROY:
			DestroyWindow(bi->hEdit);
			if(bi->font) delete bi->font;
			if(bi->fontFormat) delete bi->fontFormat;
			if(bi->fontRect) delete bi->fontRect;
			if(bi->fontColor) delete bi->fontColor;
			if(bi->backColor) delete bi->backColor;
			if(bi->borderColor) delete bi->borderColor;
			if(bi->editGr) delete bi->editGr;
			if(bi->hdc) ReleaseDC(hWnd, bi->hdc);
			GlobalFree((void *)bi);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)0);
			break;

		case WM_COMMAND:
			if(HIWORD(wParam) == EN_CHANGE)
			{
				GetDlgItemTextW(hWnd, 111, bi->text, MAX_EDIT_LENGTH);
				MessageBox(hWnd, bi->text, L"Yo",0);
				PostMessage(hWnd, WM_PAINT, 0, 0);
			}
			if(LOWORD(wParam) == 111)
				SendMessage(bi->hParent, message, wParam, lParam);
			break;

		case WM_SIZE:
			if(bi->editGr) delete bi->editGr;
			if(bi->hdc) ReleaseDC(hWnd, bi->hdc);
			bi->hdc = GetDC(hWnd);
			bi->editGr = new Graphics(hWnd, FALSE);
			break;

		case WM_PAINT:
			BeginPaint(hWnd, &ps);

			GetClientRect(hWnd, &wr);
			bi->fontRect->Width = (REAL)wr.right - 8;
			bi->editGr->DrawRectangle(bi->borderColor, 0, 0, wr.right - 1, wr.bottom - 1);
			bi->editGr->FillRectangle(bi->backColor, 1, 1, wr.right - 2, wr.bottom - 2);
			bi->editGr->DrawString(bi->text, MAX_EDIT_LENGTH, bi->font, *bi->fontRect, bi->fontFormat, bi->fontColor);

			EndPaint(hWnd, &ps);
			break;

		default:
			return(SendMessage(bi->hEdit, message, wParam, lParam));
   }
   return(0);
}




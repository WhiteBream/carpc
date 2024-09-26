// Ticker.cpp: implementation of the Ticker library.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Ticker.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

#define UPDATE_SPEED		75		// ms.
#define UPDATE_INITIAL		1000	// ms.
#define MOVING_SPEED		5		// pixels/step

typedef struct _TickerInfo
{
	INT		   id;
	HWND       hParent;
	HDC        hdc;
	HANDLE	   hp;
	LPWSTR     wtxt;
	Graphics * g;
	Font     * font;
	Color    * cl_white;	/* 2 brushes, for white and blue */
	Brush    * br_blue;
	Pen      * pn;
	FLOAT      endp;
	RectF      rf;
	BOOL       dir;
} TickerInfo, * pTickerInfo;

LRESULT CALLBACK TickerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////////////////////////
INT TickerInit(HINSTANCE hInstance)
{
	WNDCLASSEX     wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= TickerProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(LONG_PTR);
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= L"WB_TICKERBOX";
	wcex.hIconSm		= NULL;

	return(RegisterClassEx(&wcex));
}


LRESULT CALLBACK TickerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT     ps;
	LPCREATESTRUCT  cs;
	RECT            wr;
	pTickerInfo		ti;
	HANDLE			hHeap;
	GraphicsContainer  gc;
	FontFamily * fm;
	StringFormat fmt;
	RectF  ss;
	PointF pf(0,0);

	ti = (pTickerInfo)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	GetClientRect(hWnd, &wr);

	fmt.SetAlignment(StringAlignmentNear);
	fmt.SetTrimming(StringTrimmingNone);
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);

	switch (message)
	{
		case WM_CREATE:
			hHeap = GetProcessHeap();
			ti = (pTickerInfo)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(TickerInfo));
			if(ti == NULL)
				return(1L);	// Error
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)ti);
			ti->hp = hHeap;
			ti->wtxt = NULL;

			cs = (LPCREATESTRUCT)lParam;
			ti->hParent = cs->hwndParent;
			ti->id = (INT)cs->hMenu;
			// GDI+ drawing class
			ti->g = new Graphics(hWnd, FALSE);
			ti->hdc = GetDC(hWnd);
			// Background brushes & pen
			ti->pn = new Pen(Color(255, 0, 0, 0), 1);
			ti->cl_white = new Color(255, 255, 255, 255);
			ti->br_blue = new SolidBrush(Color(255, 0, 0, 192));
			// Text font
			fm = new FontFamily(L"Arial");
			ti->font = new Font(fm, (float)wr.bottom - 4, FontStyleRegular, UnitPixel);
			delete fm;
			// String rectangle
			ti->rf.X = (float)2;
			ti->rf.Y = (float)0;
			ti->rf.Width = (float)wr.right - 4;
			ti->rf.Height = (float)wr.bottom;

			SetTimer(hWnd, 1, 100, NULL);
			break;

		case WM_DESTROY:
			ReleaseDC(hWnd, ti->hdc); 
			KillTimer(hWnd, 1);
			if(ti->font) delete ti->font;
			if(ti->g) delete ti->g;
			if(ti->cl_white) delete ti->cl_white;
			if(ti->br_blue) delete ti->br_blue;
			if(ti->pn) delete ti->pn;
			if(ti->wtxt)
				HeapFree(ti->hp, 0, ti->wtxt);
			HeapFree(ti->hp, 0, (LPVOID)ti);
			ti = NULL;
			break;

		case WM_SIZE:
			// Update GDI+ drawing class
			if(ti->g) delete ti->g;
			ti->g = new Graphics(hWnd, FALSE);
			ti->hdc = GetDC(hWnd);
			// Update the font size
			if(ti->font) delete ti->font;
			fm = new FontFamily(L"Arial");
			ti->font = new Font(fm, (float)wr.bottom - 4, FontStyleRegular, UnitPixel);
			delete fm;
			// String rectangle
			ti->rf.X = (float)0;
			ti->rf.Y = (float)0;
			ti->rf.Width = (float)wr.right;
			ti->rf.Height = (float)wr.bottom;
			break;

		case WM_PAINT:
			// Do the initial displaying part
			ti->hdc = BeginPaint(hWnd, &ps);
			ti->g->ResetClip();
			ti->g->Clear(*ti->cl_white);
			if(ti->wtxt)
				ti->g->DrawString(ti->wtxt, -1, ti->font, ti->rf, &fmt, ti->br_blue);
			ti->g->DrawRectangle(ti->pn, 0, 0, wr.right - 1, wr.bottom - 1);
			ti->g->SetClip(Rect(1, 1, wr.right - 2, wr.bottom - 2));
			EndPaint(hWnd, &ps);
			break;

		case WM_SETTEXT:
			KillTimer(hWnd, 1);
			// Copy the window text
			if(ti->wtxt)
				HeapFree(ti->hp, 0, ti->wtxt);
			ti->wtxt = (LPWSTR)HeapAlloc(ti->hp, HEAP_ZERO_MEMORY, 2 * lstrlen((LPWSTR)lParam) + 2);
			lstrcpy(ti->wtxt, (LPWSTR)lParam);
			// Calculate the string length
			ti->g->MeasureString(ti->wtxt, -1, ti->font, pf, &fmt, &ss);
			ti->rf.X = 0;
			ti->rf.Width = ss.Width;
			ti->endp = (float)wr.right - ss.Width;
			ti->dir = TRUE;
			// Do the displaying part
			gc = ti->g->BeginContainer();
			ti->g->Clear(*ti->cl_white);
			ti->g->DrawString(ti->wtxt, -1, ti->font, ti->rf, &fmt, ti->br_blue);
			ti->g->EndContainer(gc);
			// Set the refresh timer
			if(ss.Width > (wr.right - 2))
				SetTimer(hWnd, 1, UPDATE_INITIAL, NULL);
			break;

		case WM_TIMER:
			// Reset the refresh timer to the end delay
			SetTimer(hWnd, 1, UPDATE_INITIAL, NULL);
			// Update the display direction
			if(ti->rf.X < ti->endp)
				ti->dir = TRUE;
			else if(ti->rf.X >= 0)
			{
				ti->rf.X = 0;
				ti->dir = FALSE;
			}
			else
				// Reset the refresh timer
				SetTimer(hWnd, 1, UPDATE_SPEED, NULL);

			// Do the displaying part
			gc = ti->g->BeginContainer();
			ti->g->Clear(*ti->cl_white);
			ti->g->DrawString(ti->wtxt, -1, ti->font, ti->rf, &fmt, ti->br_blue);
			ti->g->EndContainer(gc);
			// Update the position
			if(ti->dir)
				ti->rf.X += 2 * MOVING_SPEED;
			else
				ti->rf.X -= MOVING_SPEED;
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}




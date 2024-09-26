// Ticker.cpp: implementation of the Ticker library.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Ticker.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define UPDATE_SPEED		14//75		// ms.
#define UPDATE_INITIAL		500//1000	// ms.
#define MOVING_SPEED		1//5		// pixels/step

typedef struct _TickerInfo
{
	INT		   id;
	HWND       hParent;
	HDC        hdc;
	LPWSTR     wtxt;
	Graphics * g;
	Image				*image;
	Graphics			*graph;
	Font     * font;
	Color    * cl_white;	/* 2 brushes, for white and blue */
	Brush    * br_blue;
	Pen      * pn;
	FLOAT      endp;
	RectF      rf;
	BOOL       dir;
	RECT                wr;
	StringFormat        fmt;
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
	wcex.lpszClassName	= TEXT("WB_TICKERBOX");
	wcex.hIconSm		= NULL;

	return(RegisterClassEx(&wcex));
}

static void
Paint(pTickerInfo ti)
{
	if(ti)
	{
		ti->graph->Clear(*ti->cl_white);
		if(ti->wtxt && ti->font)
			ti->graph->DrawString(ti->wtxt, -1, ti->font, ti->rf, &ti->fmt, ti->br_blue);
		ti->graph->DrawRectangle(ti->pn, 0, 0, ti->wr.right - 1, ti->wr.bottom - 1);

		ti->g->DrawImage(ti->image, 0, 0);
	}
}

LRESULT CALLBACK TickerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT         ps;
	LPCREATESTRUCT      cs;
	pTickerInfo		    ti;
	FontFamily        * fm;
	RectF               ss;
	INT                 len;
	PointF              pf(0,0);

	ti = (pTickerInfo)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (message)
	{
		case WM_CREATE:
			ti = (pTickerInfo)GlobalAlloc(GPTR, sizeof(TickerInfo));
			if(ti == NULL)
				return(1L);	// Error
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)ti);
			ti->wtxt = NULL;

			cs = (LPCREATESTRUCT)lParam;
			ti->hParent = cs->hwndParent;
			ti->id = (INT)cs->hMenu;
			// GDI+ drawing class
			GetClientRect(hWnd, &ti->wr);
			ti->g = new Graphics(hWnd, FALSE);
			ti->image = new Bitmap(ti->wr.right, ti->wr.bottom);
			ti->graph = new Graphics(ti->image);
			ti->hdc = GetDC(hWnd);
			// Background brushes & pen
			ti->pn = new Pen(Color(255, 0, 0, 0), 1);
			ti->cl_white = new Color(255, 255, 255, 255);
			ti->br_blue = new SolidBrush(Color(255, 0, 0, 192));
			// Text font
			ti->fmt.SetAlignment(StringAlignmentNear);
			ti->fmt.SetTrimming(StringTrimmingNone);
			ti->fmt.SetFormatFlags(StringFormatFlagsNoWrap);
			fm = new FontFamily(L"Arial");
			if(fm)
			{
				ti->font = new Font(fm, (float)ti->wr.bottom - 4, FontStyleRegular, UnitPixel);
				delete fm;
			}
			// String rectangle
			ti->rf.X = (float)2;
			ti->rf.Y = (float)0;
			ti->rf.Width = (float)ti->wr.right - 4;
			ti->rf.Height = (float)ti->wr.bottom;

			SetTimer(hWnd, 1, 100, NULL);
			break;

		case WM_DESTROY:
			if(ti)
			{
				if(ti->hdc) ReleaseDC(hWnd, ti->hdc); 
				KillTimer(hWnd, 1);
				if(ti->font) delete ti->font;
				if(ti->g) delete ti->g;
				if(ti->graph) delete ti->graph;
				if(ti->image) delete ti->image;
				if(ti->cl_white) delete ti->cl_white;
				if(ti->br_blue) delete ti->br_blue;
				if(ti->pn) delete ti->pn;
				if(ti->wtxt)
					GlobalFree((LPVOID)ti->wtxt);
				GlobalFree((LPVOID)ti);
				ti = NULL;
			}
			break;

		case WM_SIZE:
			GetClientRect(hWnd, &ti->wr);
			// Update GDI+ drawing class
			if(ti->g) delete ti->g;
			ti->g = new Graphics(hWnd, FALSE);
			if(ti->image) delete ti->image;
			ti->image = new Bitmap(ti->wr.right, ti->wr.bottom);
			if(ti->graph) delete ti->graph;
			ti->graph = new Graphics(ti->image);
			// Update the font size
			if(ti->font) delete ti->font;
			fm = new FontFamily(L"Arial");
			if(fm)
			{
				ti->font = new Font(fm, (float)ti->wr.bottom - 4, FontStyleRegular, UnitPixel);
				delete fm;
			}
			// String rectangle
			ti->rf.X = (float)0;
			ti->rf.Y = (float)0;
			ti->rf.Width = (float)ti->wr.right;
			ti->rf.Height = (float)ti->wr.bottom;
			break;

		case WM_PAINT:
			ti->hdc = BeginPaint(hWnd, &ps);
			Paint(ti);
			EndPaint(hWnd, &ps);
			break;

		case WM_SETTEXT:
			KillTimer(hWnd, 1);
			// Copy the window text
			if(ti->wtxt)
				GlobalFree(ti->wtxt);
			len = 2 * lstrlen((LPWSTR)lParam) + 2;
			ti->wtxt = (LPWSTR)GlobalAlloc(GPTR, len);
			if(ti->wtxt)
			{
				StringCbCopyW(ti->wtxt, len, (LPWSTR)lParam);
				// Calculate the string length
				ti->g->MeasureString(ti->wtxt, -1, ti->font, pf, &ti->fmt, &ss);
			}
			ti->rf.X = 0;
			ti->rf.Width = ss.Width;
			ti->endp = (float)ti->wr.right - ss.Width;
			ti->dir = TRUE;
			// Set the refresh timer
			SetTimer(hWnd, 1, UPDATE_INITIAL, NULL);
			Paint(ti);
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

			// Update the position
			if(ti->rf.Width > ti->wr.right)
				ti->rf.X += (ti->dir? 2 : -1);
			Paint(ti);
			break;

		case WM_CHAR:
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
			SendMessage(ti->hParent, message, wParam, lParam);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}




// Slider.cpp: implementation of the Slider library.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Destinator.h"
#include "Slider.h"
#include "Button.h"
#include "commctrl.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


typedef struct _SliderWndInfo
{
	int			id;
	POINT		pWnd;
	HWND		hWnd, hParent;
	HWND		hMark;
	HDC			hdc;
	Graphics	*g;
	Graphics	*graph;
	Image		*image;
	int			pressed;
	Brush		*br[2];	/* 2 brushes, for white and blue */
	Pen			*pn;
	int			value, dt, range, min;
	BOOL		vDisabled;
	BYTE		vAlpha;
} SliderInfo;


LRESULT CALLBACK DestSliderProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void DestSlider(Bitmap ** btn, HINSTANCE hInstance, int width, int height);


#define IDLE			0
#define PRESSED			1
#define DISABLED		2
#define PRESSED_LEFT	10
#define PRESSED_RIGHT	11

// The brush colors
#define BR_GREEN		0
#define BR_BLUE			1


//////////////////////////////////////////////////////////////////////
int SliderInit(HINSTANCE hInstance)
{
	WNDCLASSEX     wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= DestSliderProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(LONG_PTR);
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("WB_DESTSLIDER");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

void Paint(SliderInfo * bi)
{
	int  pos;

	if(bi)
	{
		pos = bi->value * (bi->pWnd.x - 2 * bi->pWnd.y) / bi->range;

		bi->graph->FillRectangle(bi->br[BR_GREEN], 0, 0, bi->pWnd.x, bi->pWnd.y);
		bi->graph->FillRectangle(bi->br[BR_BLUE], 2, 2, bi->pWnd.x - 4, bi->pWnd.y - 4);
		bi->graph->DrawRectangle(bi->pn, 1, 1, bi->pWnd.x - 3, bi->pWnd.y - 3);

		bi->g->DrawImage(bi->image, 0, 0);
		SetWindowPos(bi->hMark, 0, pos, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		SendMessage(bi->hMark, WM_PAINT, 0, 0);
	}
}

LRESULT CALLBACK DestSliderProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT     ps;
	LPCREATESTRUCT  cs;
	POINTS          pt;
	SliderInfo    * bi;
	int             pos;

	bi = (SliderInfo *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (message)
	{
		case WM_CREATE:
			bi = (SliderInfo *)GlobalAlloc(GPTR, sizeof(SliderInfo));
			if(bi == NULL)
				return(1L);	// Error
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)bi);

			cs = (LPCREATESTRUCT)lParam;
			bi->hParent = cs->hwndParent;
			bi->hWnd = hWnd;
			bi->id = (int)cs->hMenu;

			// GDI+ drawing class
			bi->vAlpha = 255;
			bi->hdc = GetDC(hWnd);
			bi->g = new Graphics(hWnd, FALSE);
			bi->image = new Bitmap(cs->cx, cs->cy);
			bi->graph = new Graphics(bi->image);
			bi->vDisabled = FALSE;

			// Background brushes & pen
			bi->pn = new Pen(Color(255, 0, 0, 0), 1);
			bi->br[BR_BLUE] = new SolidBrush(Color(255, 201, 211, 227));
			bi->br[BR_GREEN] = new SolidBrush(Color(255, 200, 255, 200));

			// Destinator style buttons
			bi->hMark = CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("||"), WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_CLIPCHILDREN, 0, 0, 2 * cs->cy, cs->cy, hWnd, (HMENU)0, 0,0);

			bi->pWnd.x = cs->cx;
			bi->pWnd.y = cs->cy;
			bi->pressed = IDLE;
			bi->value = 0;
			bi->min = 0;
			bi->range = 65535;
			break;

		case WB_SETALPHA:
			bi->vAlpha = (BYTE)lParam;
			if(bi->br[BR_BLUE]) delete bi->br[BR_BLUE];
			if(bi->br[BR_GREEN]) delete bi->br[BR_GREEN];
			bi->br[BR_BLUE] = new SolidBrush(Color(bi->vAlpha, 201, 211, 227));
			bi->br[BR_GREEN] = new SolidBrush(Color(bi->vAlpha, 200, 255, 200));
			break;

		case WM_DESTROY:
			if(bi)
			{
				if(bi->g) delete bi->g;
				if(bi->graph) delete bi->graph;
				if(bi->image) delete bi->image;
				if(bi->br[0]) delete bi->br[0];
				if(bi->br[1]) delete bi->br[1];
				if(bi->pn) delete bi->pn;
				if(bi->hdc) ReleaseDC(hWnd, bi->hdc);
				GlobalFree((void *)bi);
			}
			bi = NULL;
			break;

		case WM_SIZE:
			// update GDI+ drawing class
			if(bi)
			{
				bi->pWnd.x = LOWORD(lParam);
				bi->pWnd.y = HIWORD(lParam);

				if(bi->g) delete bi->g;
				bi->g = new Graphics(hWnd, FALSE);
				if(bi->image) delete bi->image;
				bi->image = new Bitmap(bi->pWnd.x, bi->pWnd.y);
				if(bi->graph) delete bi->graph;
				bi->graph = new Graphics(bi->image);
			}
			break;

		case WM_LBUTTONDOWN:
			if(bi->range && !bi->vDisabled)
			{
				pt = MAKEPOINTS(lParam);
				pos = bi->value * (bi->pWnd.x - 2 * bi->pWnd.y) / bi->range;
				if(pt.x < pos)	// Pressed before the button
					bi->pressed = PRESSED_LEFT;
				else if(pt.x > (pos + 2 * bi->pWnd.y))	// Pressed behind the button
					bi->pressed = PRESSED_RIGHT;
				else	// Pressed on the button
				{
					bi->dt = pt.x - pos;
					bi->pressed = PRESSED;
				}
				SetCapture(hWnd);
				SendMessage(bi->hMark, BM_SETCHECK, BST_CHECKED, 0);
			}
			break;

		case WM_MOUSEMOVE:
			if(bi->range && (bi->pressed != IDLE) && !bi->vDisabled)
			{
				pt = MAKEPOINTS(lParam);
				if((pt.x < 0) || (pt.y < 0) || (pt.x >= bi->pWnd.x) || (pt.y >= bi->pWnd.y)) // out of range
				{
					SendMessage(bi->hMark, BM_SETCHECK, BST_UNCHECKED, 0);
					break;
				}
				else if(bi->pressed != IDLE)
					SendMessage(bi->hMark, BM_SETCHECK, BST_CHECKED, 0);

				if((wParam == MK_LBUTTON) && (bi->pressed == PRESSED)) // dragging
					bi->value = bi->range * (pt.x - bi->dt) / (bi->pWnd.x - 2 * bi->pWnd.y);
				else
					break;

				if(bi->value < 0)
					bi->value = 0;
				if(bi->value > bi->range)
					bi->value = bi->range;
				SendMessage(bi->hParent, WM_HSCROLL, MAKEWPARAM(TB_THUMBTRACK, bi->value + bi->min), (LPARAM)hWnd);
				Paint(bi);
			}
			break;

		case WM_LBUTTONUP:
			if(bi->range && !bi->vDisabled)
			{
				pt = MAKEPOINTS(lParam);
				pos = bi->value * (bi->pWnd.x - 2 * bi->pWnd.y) / bi->range;
				if((pt.x >= 0) && (pt.y >= 0) && (pt.x < bi->pWnd.x) && (pt.y < bi->pWnd.y)) // (back) in range?
				{
				//	if((pt.x < pos) && (bi->pressed == PRESSED_LEFT))	// Pressed before the button
				//		bi->value = bi->range * (pt.x - bi->pWnd.y) / (bi->pWnd.x - 2 * bi->pWnd.y);
				//	else if((pt.x > (pos + 2 * bi->pWnd.y)) && (bi->pressed == PRESSED_RIGHT))	// Pressed behind the button
				//		bi->value = bi->range * (pt.x - bi->pWnd.y) / (bi->pWnd.x - 2 * bi->pWnd.y);
				
					bi->value = bi->range * (pt.x - bi->pWnd.y) / (bi->pWnd.x - 2 * bi->pWnd.y);
					if(bi->value > bi->range)
						bi->value = bi->range;
					else if(bi->value < 0)
						bi->value = 0;
				}

				if(bi->pressed != IDLE)
					SendMessage(bi->hParent, WM_HSCROLL, MAKEWPARAM(TB_ENDTRACK, bi->value + bi->min), (LPARAM)hWnd);
				bi->pressed = IDLE;
				SendMessage(bi->hMark, BM_SETCHECK, BST_UNCHECKED, 0);
				ReleaseCapture();
				Paint(bi);
			}
			break;

		case BM_SETCHECK:
			bi->vDisabled = (wParam == BST_INDETERMINATE ? TRUE : FALSE);
			if(wParam != BST_INDETERMINATE) wParam = BST_UNCHECKED;
			SendMessage(bi->hMark, message, wParam, lParam);
			break;

		case WM_PAINT:
			BeginPaint(hWnd, &ps);
			Paint(bi);
			EndPaint(hWnd, &ps);
			break;

		case TBM_GETPOS:
			return(bi->value + bi->min);

		case TBM_SETPOS:
			lParam -= bi->min;
			if(lParam > bi->range)
				bi->value = bi->range;
			else if(lParam < 0)
				bi->value = 0;
			else
				bi->value = lParam;
			Paint(bi);
			break;

		case TBM_SETRANGE:
			bi->min = LOWORD(lParam);
			bi->range = HIWORD(lParam) - bi->min;
			if(bi->range == 0)
			{
				SendMessage(bi->hMark, BM_SETCHECK, BST_INDETERMINATE, 0);
				bi->pressed = IDLE;
				ReleaseCapture();
			}
			break;

		case TBM_GETRANGEMIN:
			return(bi->min);

		case TBM_GETRANGEMAX:
			return(bi->range + bi->min);

		case WM_MOUSEWHEEL:
			bi->value += -GET_WHEEL_DELTA_WPARAM(wParam);
			if(bi->value < 0) bi->value = 0;
			if(bi->value > bi->range) bi->value = bi->range;
			Paint(bi);
			SendMessage(bi->hParent, WM_HSCROLL, MAKEWPARAM(TB_ENDTRACK, bi->value + bi->min), (LPARAM)hWnd);
			return(0);

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}




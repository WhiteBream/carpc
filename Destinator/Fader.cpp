// Slider.cpp: implementation of the Slider library.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Fader.h"
#include "Button.h"
#include "commctrl.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


typedef struct _FaderWndInfo
{
	int				id;
	POINT			pWnd;
	HWND			hWnd, hParent;
	HWND			hMark;
	HDC				hdc;
	Graphics		*g;
	Graphics		*graph;
	Image			*image;
	int				pressed;
	Brush			*br[2];	/* 2 brushes, for white and blue */
	Pen				*pn;
	USHORT			ticFreq, ticCount;
	SHORT			dt;
	INT				value;
	USHORT			rangeLo, rangeHi;
	USHORT			range;
	BOOL			orientation;
	BOOL			vDisabled;
} FaderInfo;


LRESULT CALLBACK FaderProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


#define IDLE			0
#define PRESSED			1
#define DISABLED		2
#define PRESSED_LEFT	10
#define PRESSED_RIGHT	11

// The brush colors
#define BR_GREEN		0
#define BR_BLUE			1


//////////////////////////////////////////////////////////////////////
int FaderInit(HINSTANCE hInstance)
{
	WNDCLASSEX     wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= FaderProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(LONG_PTR);
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("WB_DESTFADER");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

void Paint(FaderInfo * bi)
{
	INT	pos, dX, dS;
	USHORT i;

	if(bi)
	{
		if(bi->orientation)
		{
			pos = (bi->value - bi->rangeLo) * (bi->pWnd.y - 2 * bi->pWnd.x / 3) / bi->range;

			bi->graph->FillRectangle(bi->br[BR_GREEN], 0, 0, bi->pWnd.x, bi->pWnd.y);
			bi->graph->FillRectangle(bi->br[BR_BLUE], 8, 8, bi->pWnd.x - 16, bi->pWnd.y - 16);
			bi->graph->DrawRectangle(bi->pn, 7, 7, bi->pWnd.x - 15, bi->pWnd.y - 15);

			if(bi->ticCount)
			{
				dX = (bi->pWnd.y - 28) / bi->ticCount;
				dS = (bi->pWnd.y - (bi->ticCount * dX)) / 2;
				for(i = 0; i <= bi->ticCount; i++)
					bi->graph->DrawLine(bi->pn, 14, dS + i * dX, bi->pWnd.x - 15, dS + i * dX);
			}
			else if(bi->ticFreq)
			{
				dX = bi->ticFreq * (bi->pWnd.y - 28) / bi->range;
				dS = (bi->pWnd.y - ((bi->range / bi->ticFreq) * dX)) / 2;
				for(i = 0; i <= bi->range / bi->ticFreq; i++)
					bi->graph->DrawLine(bi->pn, 14, dS + i * dX, bi->pWnd.x - 15, dS + i * dX);
			}
			bi->g->DrawImage(bi->image, 0, 0);
			SetWindowPos(bi->hMark, 0, 0, pos, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
		else
		{
			pos = (bi->value - bi->rangeLo) * (bi->pWnd.x - 2 * bi->pWnd.y / 3) / bi->range;

			bi->graph->FillRectangle(bi->br[BR_GREEN], 0, 0, bi->pWnd.x, bi->pWnd.y);
			bi->graph->FillRectangle(bi->br[BR_BLUE], 8, 8, bi->pWnd.x - 16, bi->pWnd.y - 16);
			bi->graph->DrawRectangle(bi->pn, 7, 7, bi->pWnd.x - 15, bi->pWnd.y - 15);

			if(bi->ticCount)
			{
				dX = (bi->pWnd.x - 28) / bi->ticCount;
				dS = (bi->pWnd.x - (bi->ticCount * dX)) / 2;
				for(i = 0; i <= bi->ticCount; i++)
					bi->graph->DrawLine(bi->pn, dS + i * dX, 14, dS + i * dX, bi->pWnd.y - 15);
			}
			else if(bi->ticFreq)
			{
				dX = bi->ticFreq * (bi->pWnd.x - 28) / bi->range;
				dS = (bi->pWnd.x - ((bi->range / bi->ticFreq) * dX)) / 2;
				for(i = 0; i < bi->range / bi->ticFreq; i++)
					bi->graph->DrawLine(bi->pn, dS + i * dX, 14, dS + i * dX, bi->pWnd.y - 15);
			}
			bi->g->DrawImage(bi->image, 0, 0);
			SetWindowPos(bi->hMark, 0, pos, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
		SendMessage(bi->hMark, WM_PAINT, 0, 0);
	}
}

LRESULT CALLBACK FaderProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT		ps;
	LPCREATESTRUCT	cs;
	POINTS			pt;
	FaderInfo		*bi;
	int				pos;

	bi = (FaderInfo *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (message)
	{
	//	case WM_NCCREATE:
	//		cs = (LPCREATESTRUCT)lParam;
	//		cs->dwExStyle |= WS_EX_TRANSPARENT;
	//		return(DefWindowProc(hWnd, message, wParam, lParam));

		case WM_ERASEBKGND:
			return(TRUE);

		case WM_CREATE:
			bi = (FaderInfo *)GlobalAlloc(GPTR, sizeof(FaderInfo));
			if(bi == NULL)
				return(1L);	// Error
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)bi);

			cs = (LPCREATESTRUCT)lParam;
			bi->hParent = cs->hwndParent;
			bi->hWnd = hWnd;
			bi->id = (int)cs->hMenu;

			// GDI+ drawing class
			bi->hdc = GetDC(hWnd);
			bi->g = new Graphics(hWnd, FALSE);
			bi->image = new Bitmap(cs->cx, cs->cy);
			bi->graph = new Graphics(bi->image);
			bi->vDisabled = FALSE;

			// Background brushes & pen
			bi->pn = new Pen(Color(255, 0, 0, 0), 1);
			bi->br[BR_BLUE] = new SolidBrush(Color(255, 201, 211, 227));
			bi->br[BR_GREEN] = new SolidBrush(Color(255, 200, 255, 200));

			bi->orientation = cs->style & TBS_VERT;
			bi->ticFreq = 0;
			bi->ticCount = 0;

			// Destinator style button
			if(bi->orientation)
				bi->hMark = CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("="), WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_CLIPCHILDREN,
										0, 0, cs->cx, (2 * cs->cx) / 3, hWnd, (HMENU)0, 0,0);
			else
				bi->hMark = CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("||"), WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_CLIPCHILDREN,
										0, 0, (2 * cs->cy) / 3, cs->cy, hWnd, (HMENU)0, 0,0);

			bi->pWnd.x = cs->cx;
			bi->pWnd.y = cs->cy;
			bi->pressed = IDLE;
			bi->value = 0;
			bi->rangeLo = 0;
			bi->rangeHi = 10000;
			bi->range = bi->rangeHi - bi->rangeLo;
			break;

		case WB_SETALPHA:
			if(bi->br[BR_BLUE]) delete bi->br[BR_BLUE];
			if(bi->br[BR_GREEN]) delete bi->br[BR_GREEN];
			bi->br[BR_BLUE] = new SolidBrush(Color((BYTE)lParam, 201, 211, 227));
			bi->br[BR_GREEN] = new SolidBrush(Color((BYTE)lParam, 200, 255, 200));
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
				if(bi->orientation)
				{
					pos = (bi->value - bi->rangeLo) * (bi->pWnd.y - (2 * bi->pWnd.x / 3)) / bi->range;
					if(pt.y < pos)	// Pressed before the button
						bi->pressed = PRESSED_LEFT;
					else if(pt.y > (pos + 2 * bi->pWnd.x / 3))	// Pressed behind the button
						bi->pressed = PRESSED_RIGHT;
					else	// Pressed on the button
					{
						bi->dt = pt.y - pos;
						bi->pressed = PRESSED;
					}
				}
				else
				{
					pos = (bi->value - bi->rangeLo) * (bi->pWnd.x - (2 * bi->pWnd.y / 3)) / bi->range;
					if(pt.x < pos)	// Pressed before the button
						bi->pressed = PRESSED_LEFT;
					else if(pt.x > (pos + 2 * bi->pWnd.y / 3))	// Pressed behind the button
						bi->pressed = PRESSED_RIGHT;
					else	// Pressed on the button
					{
						bi->dt = pt.x - pos;
						bi->pressed = PRESSED;
					}
				}
				SetCapture(hWnd);
				SendMessage(bi->hMark, BM_SETCHECK, BST_CHECKED, 0);
			}
			break;

		case WM_MOUSEMOVE:
			if(bi->range && (bi->pressed != IDLE) && !bi->vDisabled)
			{
				pt = MAKEPOINTS(lParam);
				if(bi->orientation)
				{
					if((pt.x < 0) || (pt.y < 0) || (pt.x >= bi->pWnd.x) || (pt.y >= bi->pWnd.y)) // out of range
					{
						SendMessage(bi->hMark, BM_SETCHECK, BST_UNCHECKED, 0);
						break;
					}
					else if(bi->pressed != IDLE)
						SendMessage(bi->hMark, BM_SETCHECK, BST_CHECKED, 0);

					if((wParam == MK_LBUTTON) && (bi->pressed == PRESSED)) // dragging
						bi->value = bi->rangeLo + bi->range * (pt.y - bi->dt) / (bi->pWnd.y - 2 * bi->pWnd.x / 3);
					else
						break;
				}
				else
				{
					if((pt.x < 0) || (pt.y < 0) || (pt.x >= bi->pWnd.x) || (pt.y >= bi->pWnd.y)) // out of range
					{
						SendMessage(bi->hMark, BM_SETCHECK, BST_UNCHECKED, 0);
						break;
					}
					else if(bi->pressed != IDLE)
						SendMessage(bi->hMark, BM_SETCHECK, BST_CHECKED, 0);

					if((wParam == MK_LBUTTON) && (bi->pressed == PRESSED)) // dragging
						bi->value = bi->rangeLo + bi->range * (pt.x - bi->dt) / (bi->pWnd.x - 2 * bi->pWnd.y / 3);
					else
						break;
				}

				if(bi->value > bi->rangeHi)
					bi->value = bi->rangeHi;
				else if(bi->value < bi->rangeLo)
					bi->value = bi->rangeLo;
				SendMessage(bi->hParent, WM_VSCROLL, MAKEWPARAM(TB_THUMBTRACK, (USHORT)(bi->value & 0xFFFF)), (LPARAM)hWnd);
				Paint(bi);
			}
			break;

		case WM_LBUTTONUP:
			if(bi->range && !bi->vDisabled)
			{
				pt = MAKEPOINTS(lParam);
				if(bi->orientation)
				{
					pos = (bi->value - bi->rangeLo) * (bi->pWnd.y - 2 * bi->pWnd.x / 3) / bi->range;
					if((pt.x >= 0) && (pt.y >= 0) && (pt.x < bi->pWnd.x) && (pt.y < bi->pWnd.y)) // (back) in range?
					{
						if((pt.y < pos) && (bi->pressed == PRESSED_LEFT))	// Pressed before the button
							bi->value = bi->rangeLo + bi->range * (pt.y - bi->pWnd.x / 3) / (bi->pWnd.y - 2 * bi->pWnd.x / 3);
						else if((pt.y > (pos + 2 * bi->pWnd.x / 3)) && (bi->pressed == PRESSED_RIGHT))	// Pressed behind the button
							bi->value = bi->rangeLo + bi->range * (pt.y - bi->pWnd.x / 3) / (bi->pWnd.y - 2 * bi->pWnd.x / 3);
					}
				}
				else
				{
				//	pos = bi->value * (bi->pWnd.x - 2 * bi->pWnd.y / 3) / bi->range;
					pos = (bi->value - bi->rangeLo) * (bi->pWnd.x - 2 * bi->pWnd.y / 3) / bi->range;
					if((pt.x >= 0) && (pt.y >= 0) && (pt.x < bi->pWnd.x) && (pt.y < bi->pWnd.y)) // (back) in range?
					{
						if((pt.x < pos) && (bi->pressed == PRESSED_LEFT))	// Pressed before the button
							bi->value = bi->rangeLo + bi->range * (pt.x - bi->pWnd.y / 3) / (bi->pWnd.x - 2 * bi->pWnd.y / 3);
						else if((pt.x > (pos + 2 * bi->pWnd.y / 3)) && (bi->pressed == PRESSED_RIGHT))	// Pressed behind the button
							bi->value = bi->rangeLo + bi->range * (pt.x - bi->pWnd.y / 3) / (bi->pWnd.x - 2 * bi->pWnd.y / 3);

					}
				}
				if(bi->value > bi->rangeHi)
					bi->value = bi->rangeHi;
				else if(bi->value < bi->rangeLo)
					bi->value = bi->rangeLo;

				if(bi->pressed != IDLE)
					SendMessage(bi->hParent, WM_VSCROLL, MAKEWPARAM(TB_ENDTRACK, (USHORT)(bi->value & 0xFFFF)), (LPARAM)hWnd);
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

		case WM_SETTEXT:
		case WB_GETTEXTSIZE:
		case WB_SETTEXTSIZE:
			return(SendMessage(bi->hMark, message, wParam, lParam));

		case WM_PAINT:
			BeginPaint(hWnd, &ps);
			Paint(bi);
			EndPaint(hWnd, &ps);
			break;

		case TBM_SETTICFREQ:
			bi->ticFreq = (INT)wParam;
			bi->ticCount = 0;
			break;

		case WB_SETTICKCOUNT:
			bi->ticCount = (INT)wParam;
			bi->ticFreq = 0;
			break;

		case TBM_SETPOS:
			if(lParam > bi->rangeHi)
				bi->value = bi->rangeHi;
			else if(lParam < bi->rangeLo)
				bi->value = bi->rangeLo;
			else
				bi->value = (USHORT)lParam;
			Paint(bi);
			break;

		case TBM_GETPOS:
			return(bi->value);

		case TBM_SETRANGE:
			bi->rangeLo = LOWORD(lParam);
			bi->rangeHi = HIWORD(lParam);
			bi->range = bi->rangeHi - bi->rangeLo;
			if(bi->rangeHi <= bi->rangeLo)
			{
				SendMessage(bi->hMark, BM_SETCHECK, BST_INDETERMINATE, 0);
				bi->pressed = IDLE;
				ReleaseCapture();
			}
			break;

		case TBM_GETRANGEMIN:
			return(bi->rangeLo);

		case TBM_GETRANGEMAX:
			return(bi->rangeHi);

		case WM_MOUSEWHEEL:
			bi->value += -GET_WHEEL_DELTA_WPARAM(wParam);
			if(bi->value < bi->rangeLo) bi->value = bi->rangeLo;
			if(bi->value > bi->rangeHi) bi->value = bi->rangeHi;
			Paint(bi);
			SendMessage(bi->hParent, WM_VSCROLL, MAKEWPARAM(TB_ENDTRACK, bi->value), (LPARAM)hWnd);
			return(0);

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}




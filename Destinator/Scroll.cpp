// Scroll.cpp: implementation of the Scroll library.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Scroll.h"
#include "Button.h"
#include "commctrl.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


typedef struct _ScrollWndInfo
{
	int        id;
	POINT      pWnd;
	HWND       hWnd, hParent;
	RECT       wr;
	int        offset;
	Graphics * g;
	int		   pressed;
	Brush    * br[2];	/* 2 brushes, for white and blue */
	Pen      * pn;
	INT        dlgItem;
	int        value, dt, range, pagesize, pos;
} ScrollInfo;


LRESULT CALLBACK DestScrollProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void DestScroll(Bitmap ** btn, HINSTANCE hInstance, int width, int height);


#define IDLE			0
#define PRESSED			1
#define DISABLED		2
#define PRESSED_LEFT	10
#define PRESSED_RIGHT	11

// The brush colors
#define BR_GREEN		0
#define BR_BLUE			1

#define IDC_MIN			1
#define IDC_MAX			2
#define IDC_CAP			3

//////////////////////////////////////////////////////////////////////
int ScrollInit(HINSTANCE hInstance)
{
	WNDCLASSEX     wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= DestScrollProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(LONG_PTR);
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("WB_DESTSCROLL");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

void Paint(ScrollInfo * bi)
{
	GraphicsContainer state;
	Rect *sb;

	if(bi->range)
		bi->pos = (bi->value * (bi->wr.bottom - bi->wr.top - bi->offset)) / bi->range;
	else
		bi->pos = 0;
	bi->pos += bi->wr.top;

	state = bi->g->BeginContainer();
	bi->g->ResetClip();
	sb = new Rect((INT)bi->wr.left, (INT)bi->pos, (INT)bi->offset, (INT)bi->offset);
	if(sb)
	{
		bi->g->ExcludeClip(*sb);
		delete sb;
	}
	SetWindowPos(GetDlgItem(bi->hWnd, IDC_CAP), NULL, 0, bi->pos, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	bi->g->FillRectangle(bi->br[BR_GREEN], bi->wr.left, bi->wr.top, bi->wr.right - bi->wr.left, bi->wr.bottom - bi->wr.top);
	bi->g->DrawRectangle(bi->pn, bi->wr.left + 2, bi->wr.top + 2, bi->wr.right - bi->wr.left - 5, bi->wr.bottom - bi->wr.top - 5);
	bi->g->FillRectangle(bi->br[BR_BLUE], bi->wr.left + 3, bi->wr.top + 3, bi->wr.right - bi->wr.left - 6, bi->wr.bottom - bi->wr.top - 6);
	bi->g->EndContainer(state);
}

LRESULT CALLBACK DestScrollProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static POINTS    pt;

	PAINTSTRUCT     ps;
	LPCREATESTRUCT  cs;
	ScrollInfo    * bi;

	bi = (ScrollInfo *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (message)
	{
		case WM_CREATE:
			bi = (ScrollInfo *)GlobalAlloc(GPTR, sizeof(ScrollInfo));
			if(bi == NULL)
				return(1L);	// Error
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)bi);

			cs = (LPCREATESTRUCT)lParam;
			bi->hParent = cs->hwndParent;
			bi->hWnd = hWnd;
			bi->id = (int)cs->hMenu;
			bi->offset = cs->cx;
			GetClientRect(hWnd, &bi->wr);
			/* Adjust slider area */
			bi->wr.top += bi->offset;
			bi->wr.bottom -= bi->offset;
			bi->dlgItem = (INT)cs->hMenu;

			// GDI+ drawing class
			bi->g = new Graphics(hWnd, FALSE);

			// Background brushes & pen
			bi->pn = new Pen(Color(255, 0, 0, 0), 1);
			bi->br[BR_BLUE] = new SolidBrush(Color(255, 201, 211, 227));
			bi->br[BR_GREEN] = new SolidBrush(Color(255, 200, 255, 200));

			// Destinator style buttons
			CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("^"), WS_CHILD | WS_VISIBLE, 0, 0, cs->cx, cs->cx, hWnd, (HMENU)IDC_MIN, 0,0);
			SendDlgItemMessage(hWnd, IDC_MIN, WM_SETREPEAT, 0, (LPARAM)100);
			CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("="), WS_CHILD | WS_VISIBLE | WS_DISABLED, 0, cs->cx, cs->cx, cs->cx, hWnd, (HMENU)IDC_CAP, 0,0);
			CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("v"), WS_CHILD | WS_VISIBLE, 0, cs->cy - cs->cx, cs->cx, cs->cx, hWnd, (HMENU)IDC_MAX, 0,0);
			SendDlgItemMessage(hWnd, IDC_MAX, WM_SETREPEAT, 0, (LPARAM)100);

			bi->pWnd.x = cs->cx;
			bi->pWnd.y = cs->cy;
			bi->pressed = IDLE;
			bi->value = 0;
			bi->range = 65535;
			break;

		case WM_DESTROY:
			if(bi->g) delete bi->g;
			if(bi->br[0]) delete bi->br[0];
			if(bi->br[1]) delete bi->br[1];
			if(bi->pn) delete bi->pn;
			GlobalFree((HGLOBAL)bi);
			bi = NULL;
			break;

		case WM_SIZE:
			/* Adjust slider area */
			GetClientRect(hWnd, &bi->wr);
			bi->wr.top += bi->offset;
			bi->wr.bottom -= bi->offset;
			/* update GDI+ drawing class */
			if(bi->g) delete bi->g;
			bi->g = new Graphics(hWnd, FALSE);
			/* Adjust button positions and sizes */
			SetWindowPos(GetDlgItem(hWnd, IDC_MIN), NULL, 0, 0, bi->offset, bi->offset, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, IDC_CAP), NULL, 0, bi->wr.top, bi->offset, bi->offset, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, IDC_MAX), NULL, 0, bi->wr.bottom, bi->offset, bi->offset, SWP_NOACTIVATE | SWP_NOZORDER);
			break;

		case WM_PARENTNOTIFY:
			pt = MAKEPOINTS(lParam);
			//pt.x += (SHORT)plc.rcNormalPosition.left;
			//pt.y += (SHORT)plc.rcNormalPosition.top;

			switch(LOWORD(wParam))
			{
				case WM_LBUTTONDOWN:
			//		SetTimer(hWnd, 1, 100, NULL);
					break;
				case WM_MOUSEMOVE:
					break;
				case WM_LBUTTONUP:
			//		KillTimer(hWnd, 1);
					break;
			}
//			SendMessage(hWnd, (UINT)wParam, 0, lParam);
			break;

		case WM_LBUTTONDOWN:
			if(bi->range)
			{
				pt = MAKEPOINTS(lParam);
				if(pt.y < bi->pos)	// Pressed before the button
					bi->pressed = PRESSED_LEFT;
				else if(pt.y > (bi->pos + bi->offset))	// Pressed behind the button
					bi->pressed = PRESSED_RIGHT;
				else	// Pressed on the button
				{
					bi->dt = pt.y - bi->pos;
					bi->pressed = PRESSED;
					SendDlgItemMessage(hWnd, IDC_CAP, BM_SETCHECK, BST_CHECKED, 0);
				}
				SetCapture(hWnd);
			}
			break;

		case WM_MOUSEMOVE:
			if(bi->range && (bi->pressed != IDLE))
			{
				pt = MAKEPOINTS(lParam);
				if((pt.y < (bi->wr.top - bi->offset)) || (pt.y >= (bi->wr.bottom + bi->offset))) // out of range
				{
					SendDlgItemMessage(hWnd, IDC_CAP, BM_SETCHECK, BST_UNCHECKED, 0);
					break;
				}
				else if(bi->pressed != IDLE)
					SendDlgItemMessage(hWnd, IDC_CAP, BM_SETCHECK, BST_CHECKED, 0);

				if((wParam == MK_LBUTTON) && (bi->pressed == PRESSED)) // dragging
					bi->value = bi->range * (pt.y - bi->dt - bi->offset) / (bi->wr.bottom -  bi->wr.top - bi->offset);
				else
					break;

				if(bi->value < 0)
					bi->value = 0;
				if(bi->value > bi->range)
					bi->value = bi->range;

				SendMessage(bi->hParent, WM_VSCROLL, MAKEWPARAM(TB_THUMBTRACK, bi->value), (LPARAM)hWnd);
				Paint(bi);
			}
			break;

		case WM_LBUTTONUP:
			if(bi->range)
			{
				pt = MAKEPOINTS(lParam);
				if((pt.x >= bi->wr.left) && (pt.y >= (bi->wr.top - bi->offset)) && (pt.x < bi->wr.right) && (pt.y < (bi->wr.bottom + bi->offset))) // (back) in range?
				{
					if((pt.y < bi->pos) && (bi->pressed == PRESSED_LEFT))	// Pressed before the button
					{
						if(bi->pagesize)
							bi->value -= bi->pagesize;
						else
							bi->value -= bi->range / 10;
					//	SendMessage(bi->hParent, TB_PAGEUP, 0, 0);
					}
					else if((pt.y > (bi->pos + bi->offset)) && (bi->pressed == PRESSED_RIGHT))	// Pressed behind the button
					{
						if(bi->pagesize)
							bi->value += bi->pagesize;
						else
							bi->value += bi->range / 10;
					//	SendMessage(bi->hParent, TB_PAGEDOWN, 0, 0);
					}
				}

				if(bi->value < 0)
					bi->value = 0;
				if(bi->value > bi->range)
					bi->value = bi->range;

				if(bi->pressed != IDLE)
					SendMessage(bi->hParent, WM_VSCROLL, MAKEWPARAM(TB_ENDTRACK, bi->value), (LPARAM)hWnd);
				bi->pressed = IDLE;
				SendDlgItemMessage(hWnd, IDC_CAP, BM_SETCHECK, BST_UNCHECKED, 0);
				ReleaseCapture();
				Paint(bi);
			}
			break;

		case WM_TIMER:
			if(bi->range)
			{
				if((pt.x >= bi->wr.left) && (pt.y >= (bi->wr.top - bi->offset)) && (pt.x < bi->wr.right) && (pt.y < (bi->wr.bottom + bi->offset))) // (back) in range?
				{
					if((pt.y < bi->pos) && (bi->pressed == PRESSED_LEFT))	// Pressed before the button
					{
						if(bi->pagesize)
							bi->value -= bi->pagesize;
						else
							bi->value -= bi->range / 10;
					//	SendMessage(bi->hParent, TB_PAGEUP, 0, 0);
					}
					else if((pt.y > (bi->pos + bi->offset)) && (bi->pressed == PRESSED_RIGHT))	// Pressed behind the button
					{
						if(bi->pagesize)
							bi->value += bi->pagesize;
						else
							bi->value += bi->range / 10;
					//	SendMessage(bi->hParent, TB_PAGEDOWN, 0, 0);
					}
				}

				if(bi->value < 0)
					bi->value = 0;
				if(bi->value > bi->range)
					bi->value = bi->range;

			//	if(bi->pressed != IDLE)
			//		SendMessage(bi->hParent, WM_VSCROLL, MAKEWPARAM(TB_ENDTRACK, bi->value), (LPARAM)hWnd);
			}
			break;

		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					switch(LOWORD(wParam))
					{
						case IDC_MIN:
							bi->value--;
							if(bi->value < 0)
								bi->value = 0;
							SendMessage(bi->hParent, WM_VSCROLL, MAKEWPARAM(TB_ENDTRACK, bi->value), (LPARAM)hWnd);
							Paint(bi);
							break;

						case IDC_MAX:
							bi->value++;
							if(bi->value > bi->range)
								bi->value = bi->range;
							SendMessage(bi->hParent, WM_VSCROLL, MAKEWPARAM(TB_ENDTRACK, bi->value), (LPARAM)hWnd);
							Paint(bi);
							break;
					}
					break;
			}
			break;

		case WM_PAINT:
			BeginPaint(hWnd, &ps);
			Paint(bi);
			EndPaint(hWnd, &ps);
			SendDlgItemMessage(hWnd, IDC_MIN, message, wParam, lParam);
			SendDlgItemMessage(hWnd, IDC_CAP, message, wParam, lParam);
			SendDlgItemMessage(hWnd, IDC_MAX, message, wParam, lParam);
			break;

		case TBM_GETPOS:
			return(bi->value);

		case TBM_SETPOS:
			if(lParam < bi->range)
                bi->value = lParam;
			else
                bi->value = bi->range;
			if(lParam < 0)
				bi->value = 0;

			if(wParam)
				Paint(bi);
			else if(bi->range)
				if(bi->pos - bi->wr.top != (bi->value * (bi->wr.bottom - bi->wr.top - bi->offset)) / bi->range)
					Paint(bi);
			break;

		case TBM_GETPAGESIZE:
			return(bi->pagesize);

		case TBM_SETPAGESIZE:
			bi->pagesize = lParam;
			break;

		case TBM_SETRANGE:
			if(HIWORD(lParam) == 0)
			{
				bi->range = 0;
				SendDlgItemMessage(hWnd, IDC_MIN, BM_SETCHECK, BST_INDETERMINATE, 0);
				SendDlgItemMessage(hWnd, IDC_CAP, BM_SETCHECK, BST_INDETERMINATE, 0);
				SendDlgItemMessage(hWnd, IDC_MAX, BM_SETCHECK, BST_INDETERMINATE, 0);
				bi->pressed = IDLE;
			//	ReleaseCapture();
			}
			else
			{
				if(bi->range == 0)
				{
					SendDlgItemMessage(hWnd, IDC_MIN, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hWnd, IDC_CAP, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hWnd, IDC_MAX, BM_SETCHECK, BST_UNCHECKED, 0);
				//	Paint(bi);
				}
				bi->range = HIWORD(lParam);
				if(bi->value)
					Paint(bi);
			}
			break;

		case TBM_GETRANGEMIN:
			return(0);

		case TBM_GETRANGEMAX:
			return(bi->range);

		case WM_MOUSEWHEEL:
//			bi->value += -GET_WHEEL_DELTA_WPARAM(wParam) / 40;
//			if(bi->value < 0) bi->value = 0;
//			if(bi->value > bi->range) bi->value = bi->range;
//			Paint(bi);
//			SendMessage(bi->hParent, WM_VSCROLL, MAKEWPARAM(TB_ENDTRACK, bi->value), (LPARAM)hWnd);
			return(0);
////			return(SendMessage(GetAncestor(hWnd, GA_PARENT), message, wParam, lParam));

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}




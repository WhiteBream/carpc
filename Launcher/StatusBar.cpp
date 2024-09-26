// StatusBar.cpp: implementation of the StatusBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StatusBar.h"
#include "Config.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

LRESULT CALLBACK StatusBarWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static StatusBar	*my;
static UINT			timerId = 1;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
StatusBar::StatusBar(HINSTANCE hInstance, HWND hParent)
{
	LONG  newStyle;
	int   width, height;

	my = this;
	hInst = hInstance;
	gr = NULL;
	graph = NULL;
	image = NULL;
	hDc = NULL;
	MyRegisterClass(hInstance);

	hBar = CreateWindow(TEXT("CRPC_STTSBR"), TEXT("CARGO statusbar"), WS_BORDER,
      0, 0, 0, 0, hParent, NULL, hInstance, NULL);

	newStyle = GetWindowLong(hBar, GWL_STYLE);
	newStyle &= ~(WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_OVERLAPPED | WS_TILED | WS_DLGFRAME | WS_CHILD);
	SetWindowLong(hBar, GWL_STYLE, newStyle);

	SetWindowLong(hBar, GWL_EXSTYLE, WS_EX_NOACTIVATE);

	ConfigGetInt(BUTTONHEIGHT, &height);
	ConfigGetInt(STARTBUTTONWIDTH, &width);
	SetWindowPos(hBar, HWND_NOTOPMOST, width, GetSystemMetrics(SM_CYSCREEN) - height, 
				GetSystemMetrics(SM_CXSCREEN) - width - height, height, SWP_SHOWWINDOW | SWP_FRAMECHANGED);

	GetClientRect(hBar, &sRect);

	image = new Bitmap(sRect.right, sRect.bottom);
	graph = new Graphics(image);

	solidWhite = new SolidBrush(Color(255, 255, 255, 255));
	solidGrey = new SolidBrush(Color(255, 80, 80, 80));
	solidYellow = new SolidBrush(Color(255, 255, 255, 80));
	bkGnd = new Color(255, 0, 0, 192);
	font = new Font(TEXT("Arial"), 32);
	hDc = GetDC(hBar);
	gr = new Graphics(hDc);

	ShowWindow(hBar, TRUE);
	UpdateWindow(hBar);
}

StatusBar::~StatusBar()
{
	if(gr) delete gr;
	if(graph) delete graph;
	if(image) delete image;
	if(font) delete font;
	if(bkGnd) delete bkGnd;
	if(solidWhite) delete solidWhite;
	if(solidGrey) delete solidGrey;
	if(solidYellow) delete solidYellow;
	ReleaseDC(hBar, hDc);
	DestroyWindow(hBar);
}

void StatusBar::UpdateGdi(void)
{
	if(gr) delete gr;
	if(graph) delete graph;
	if(image) delete image;
	if(hDc)	ReleaseDC(hBar, hDc);
	GetClientRect(hBar, &sRect);
	image = new Bitmap(sRect.right, sRect.bottom);
	graph = new Graphics(image);
	hDc = GetDC(hBar);
	gr = new Graphics(hDc);
}

ATOM StatusBar::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= 0;
	wcex.lpfnWndProc	= StatusBarWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_STTSBR");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

void StatusBar::Paint(void)
{
	TCHAR       tText[10];
	SYSTEMTIME  tVal;
	RectF       tRect;
    PointF      point;
	BOOL		vDemo = TRUE;
	CHAR		vInpTxt[10];

	GetLocalTime(&tVal);
	StringCchPrintf(tText, sizeof(tText), TEXT("%2d:%02d:%02d"), tVal.wHour, tVal.wMinute, tVal.wSecond);

	graph->Clear(*my->bkGnd);
	point.X = 0;
	point.Y = 0;
	graph->MeasureString(tText, -1, font, point, &tRect);
	point.X = 5 + sRect.right - tRect.Width - 3;
	point.Y = 5 + (sRect.bottom - tRect.Height) / 2;
	graph->DrawString(tText, -1, font, point, my->solidGrey);
	point.X -= 2;
	point.Y -= 2;
	graph->DrawString(tText, -1, font, point, my->solidWhite);

	if(PipeIo("SIrMI", 6, vInpTxt, sizeof(vInpTxt)))
		vDemo = FALSE;
	if(vDemo)
	{
		point.X = 5;
		graph->DrawString(L"Demo mode", -1, font, point, my->solidYellow);
	}

	gr->DrawImage(image, 0, 0);
}

LRESULT CALLBACK StatusBarWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC         hdc;
	int   width, height;

	switch (message) 
	{
		case WM_CREATE:
			SetTimer(hWnd, timerId, 1000, NULL);
			break;

		case WM_TIMER:
			my->Paint();
			break;

		case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
			my->Paint();
			EndPaint(hWnd, &ps);
			break;

		case WM_DISPLAYCHANGE:
		case WM_SIZE:
			ConfigGetInt(BUTTONHEIGHT, &height);
			ConfigGetInt(STARTBUTTONWIDTH, &width);
			SetWindowPos(hWnd, HWND_NOTOPMOST, width, GetSystemMetrics(SM_CYSCREEN) - height, 
						GetSystemMetrics(SM_CXSCREEN) - width - height, height, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
			my->UpdateGdi();
			break;

		case WM_DESTROY:
			KillTimer(hWnd, timerId);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}


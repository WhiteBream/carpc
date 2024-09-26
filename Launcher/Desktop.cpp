// Desktop.cpp: implementation of the StartButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Desktop.h"
#include "Config.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

LRESULT CALLBACK DesktopWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static Desktop * myPtr;
Bitmap * bmp = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Desktop::Desktop(HINSTANCE hInstance, HWND hParent)
{
	LONG  newStyle;
	TCHAR backGndFile[256] = {0};
	int   height;

	myPtr = this;
	myGraphics = NULL;
	hDc = NULL;
	destinationRect = NULL;

	hInst = hInstance;

	MyRegisterClass(hInstance);

	ConfigGetString(BACKGROUNDFILE, backGndFile, sizeof(backGndFile));
	bmp = new Bitmap(backGndFile);
	if(bmp->GetWidth() == 0)
	{
		delete bmp;
		bmp = new Bitmap(hInst, MAKEINTRESOURCE(IDB_BACKGROUND));
	}

	ConfigGetInt(BUTTONHEIGHT, &height);
	destinationRect = new Rect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) - height);

	hDesktop = CreateWindowEx(WS_EX_TOOLWINDOW, TEXT("CRPC_DSKTP"), TEXT("CarPC desktop"), WS_BORDER,
		0, 0, 0, 0, hParent, NULL, hInstance, NULL);

	newStyle = GetWindowLong(hDesktop, GWL_STYLE);
	newStyle &= ~(WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_OVERLAPPED | WS_TILED | WS_DLGFRAME );
	SetWindowLong(hDesktop, GWL_STYLE, newStyle);
	SetWindowLong(hDesktop, GWL_EXSTYLE, WS_EX_NOACTIVATE);

	SetWindowPos(hDesktop, HWND_BOTTOM, 0, 0, destinationRect->Width, destinationRect->Height, SWP_SHOWWINDOW | SWP_FRAMECHANGED);

	hDc = GetDC(hDesktop);
	myGraphics = new Graphics(hDc);

	ShowWindow(hDesktop, SW_SHOW);
	UpdateWindow(hDesktop);
}

Desktop::~Desktop()
{
	if(bmp) delete bmp;
	if(backGround) delete backGround;
	if(myGraphics) delete myGraphics;
	if(destinationRect) delete destinationRect;
	ReleaseDC(hDesktop, hDc);
	DestroyWindow(hDesktop);
}

void Desktop::UpdateGdi(void)
{
	int height;

	if(myGraphics) delete myGraphics;
	if(destinationRect) delete destinationRect;
	if(hDc)	ReleaseDC(hDesktop, hDc);
	ConfigGetInt(BUTTONHEIGHT, &height);
	destinationRect = new Rect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) - height);
	hDc = GetDC(hDesktop);
	myGraphics = new Graphics(hDc);
}
ATOM Desktop::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= 0;
	wcex.lpfnWndProc	= DesktopWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_DSKTP");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK DesktopWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int	height;

	switch (message)
	{
		case WM_ERASEBKGND:
			if(bmp && myPtr->myGraphics)
			{
				myPtr->myGraphics->DrawImage(bmp, *myPtr->destinationRect, 1, 1, 
					bmp->GetWidth() - 2, bmp->GetHeight() - 2, UnitPixel);
			}
			return(1);

		case WM_DISPLAYCHANGE:
		case WM_SIZE:
			ConfigGetInt(BUTTONHEIGHT, &height);
			SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) - height, SWP_NOMOVE);
			myPtr->UpdateGdi();
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return(0);
}


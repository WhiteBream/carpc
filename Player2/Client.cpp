// Settings.cpp: implementation of the StartButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Client.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

LRESULT CALLBACK ClientWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static Client * myPtr;

Brush    * ourBgnd2;
Graphics * ourGraphics2;
HWND       hPrnt2;
int        offset2;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Client::Client(HINSTANCE hInstance, HWND hParent, int bottomOffset)
{
   RECT pRect;

   myPtr = this;

   hInst = hInstance;
   hPrnt2 = hParent;
   offset2 = bottomOffset;

   MyRegisterClass(hInstance);

   GetClientRect(hParent, &pRect);

   hClient = CreateWindow(TEXT("CRPC_CLNT"), TEXT("CarPC client"), WS_CHILD,
      0, 0, pRect.right, pRect.bottom - bottomOffset, hParent, NULL, hInstance, NULL);

   ourBgnd2 = new SolidBrush(Color(255, 0, 255, 0));
}

Client::~Client()
{
   delete ourBgnd2;
   DestroyWindow(hClient);
}

Client::Show(BOOL yesNo)
{
   if(yesNo)
	   ShowWindow(hClient, SW_SHOW);
   else
	   ShowWindow(hClient, SW_HIDE);
}

ATOM Client::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= ClientWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_CLNT");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK ClientWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
    RECT ourRect;
	GetClientRect(hPrnt2, &ourRect);
	ourRect.bottom -= offset2;

	switch (message)
	{
		case WM_PAINT:
    		SetWindowPos(hWnd, 0, 0, 0, ourRect.right, ourRect.bottom, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
            BeginPaint(hWnd, &ps);
			ourGraphics2 = new Graphics(hWnd, FALSE);
			ourGraphics2->FillRectangle(ourBgnd2, 0, 0, ourRect.right, ourRect.bottom);
			EndPaint(hWnd, &ps);
			delete ourGraphics2;
			break;

		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
			break;

		case WM_LBUTTONUP:
			MessageBox(hWnd, TEXT("Click client2"), TEXT("info"), MB_OK);
			ReleaseCapture();
			break;

		case WM_DESTROY:
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}


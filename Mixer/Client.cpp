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

#define WINCLASS	TEXT("CRGMX_CLNT")

LRESULT CALLBACK ClientWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

Brush    * ourBgnd2;
Graphics * ourGraphics2;
HWND       hPrnt2;
int        offset2;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Client::Client(PAppInfo pAppInfo)
{
	MakeWindow(pAppInfo, WINCLASS, ClientWndProc, TEXT("CarGo client"));
}

Client::Client(PAppInfo pAppInfo, LPTSTR pClassName, WNDPROC pClassCb)
{
	MakeWindow(pAppInfo, pClassName, pClassCb, NULL);
}

Client::Client(PAppInfo pAppInfo, LPTSTR pClassName, WNDPROC pClassCb, LPTSTR pWinName)
{
	MakeWindow(pAppInfo, pClassName, pClassCb, pWinName);
}

Client::~Client()
{
   DestroyWindow(hWnd);
}

void Client::MakeWindow(PAppInfo pAppInfo, LPTSTR pClassName, WNDPROC pClassCb, LPTSTR pWinName)
{
	RECT pRect;

	pAi = pAppInfo;

	RegWinClass(pClassName, pClassCb);
	GetClientRect(pAi->hApp, &pRect);

	hWnd = CreateWindow(pClassName, pWinName, WS_CHILD | WS_CLIPCHILDREN,
		0, 0, pRect.right, pRect.bottom - pAi->vOffs, pAi->hApp, NULL, pAi->hInst, this);
}

Client::Show(BOOL yesNo)
{
   if(yesNo)
	   ShowWindow(hWnd, SW_SHOW);
   else
	   ShowWindow(hWnd, SW_HIDE);
}

ATOM Client::RegWinClass(LPTSTR pClassName, WNDPROC pClassCb)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= pClassCb;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= pAi->hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= pClassName;
	wcex.hIconSm		= NULL;

	return(RegisterClassEx(&wcex));
}

LRESULT CALLBACK ClientWndProc(HWND hWnd, UINT vMsg, WPARAM wParam, LPARAM lParam)
{
	Client		*my;
	PAINTSTRUCT	ps;
    RECT		ourRect;

	my = (Client*)GetWindowLong(hWnd, GWL_USERDATA);
	if((my == NULL) && (vMsg != WM_NCCREATE))
		return(DefWindowProc(hWnd, vMsg, wParam, lParam));

	switch(vMsg)
	{
		case WM_NCCREATE:
			my = (Client*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLong(hWnd, GWL_USERDATA, (LONG)my);
			return(my ? DefWindowProc(hWnd, vMsg, wParam, lParam) : FALSE);

		case WM_CREATE:
			ourBgnd2 = new SolidBrush(Color(255, 0, 255, 0));
			break;

		case WM_PAINT:
			GetClientRect(hWnd, &ourRect);
			BeginPaint(hWnd, &ps);
			ourGraphics2 = new Graphics(hWnd, FALSE);
			ourGraphics2->FillRectangle(ourBgnd2, my->pAi->vOffs, my->pAi->vOffs, 
										ourRect.right - 2 * my->pAi->vOffs, ourRect.bottom - 2 * my->pAi->vOffs);
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
			delete ourBgnd2;
			break;

		default:
			return(DefWindowProc(hWnd, vMsg, wParam, lParam));
   }
   return(0);
}


// MenuWindow.cpp: implementation of the MenuWindow Class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MenuWindow.h"
#include "Config.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif


LRESULT CALLBACK MenuWindowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static MenuWindow * myPtr;

MenuWindow::MenuWindow(HINSTANCE hInstance, HWND hParent)
{
   myPtr = this;

   hInst = hInstance;
   hMenu = NULL;
   hParentWin = hParent;

   MyRegisterClass(hInstance);

   ConfigGetInt(BUTTONHEIGHT, &height);
   ConfigGetInt(MENUBUTTONWIDTH, &width);
   
   Show();
}

MenuWindow::~MenuWindow()
{
	DestroyWindow(hMenu);
}

ATOM MenuWindow::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= 0;
	wcex.lpfnWndProc	= MenuWindowWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= TEXT("");
	wcex.lpszClassName	= TEXT("CRPC_MNWNDW");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

void MenuWindow::Show()
{
	LONG  newStyle;
	int   x, y;

	hMenu = CreateWindow(TEXT("CRPC_MNWNDW"), TEXT("CARGO menu"), WS_BORDER | WS_CLIPCHILDREN, 0, 0, 0, 0, NULL, NULL, hInst, NULL);

	newStyle = GetWindowLong(hMenu, GWL_STYLE);
	newStyle &= ~(WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_OVERLAPPED | WS_TILED | WS_DLGFRAME );
	SetWindowLong(hMenu, GWL_STYLE, newStyle);

	SetWindowLong(hMenu, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

	ConfigGetInt(MENUROWS, &y);
	ConfigGetInt(MENUCOLUMS, &x);

	SetWindowPos(hMenu, HWND_NOTOPMOST, 0, GetSystemMetrics(SM_CYSCREEN) - ((y + 1) * height), 
				x * width, (y * height), SWP_FRAMECHANGED);

	DrawButtons();
}

void MenuWindow::DrawButtons()
{
	int x, y, n;
	size_t pcb;

	ConfigGetInt(MENUROWS, &y);
	for(y-- ; y >= 0; y--)
	{
		ConfigGetInt(MENUCOLUMS, &x);
		for(x--; x >= 0; x--)
		{
			n = (y << 1) + x;

			if(sMenu[n].menuType == MENU_INT)
				sMenu[n].tValid = TRUE;
			if(StringCbLength(sMenu[n].exeName, sizeof(sMenu[n].exeName), &pcb) == S_OK)
				if(pcb != 0)
					sMenu[n].tValid = TRUE;

		   CreateWindow(TEXT("WB_DESTBUTTON"), sMenu[n].menuName, WS_CHILD | WS_VISIBLE, 
			   width * x, height * y, width, height, hMenu, (HMENU)(IDC_BUTTON01 + n), hInst, NULL);
		   SendDlgItemMessage(hMenu, IDC_BUTTON01 + n, WM_USERALIGN, (WPARAM)ALIGNLEFT, 0);

		   if(sMenu[n].tValid == TRUE)
			   SendDlgItemMessage(hMenu, IDC_BUTTON01 + n, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			else
			   SendDlgItemMessage(hMenu, IDC_BUTTON01 + n, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
		}
	}
}

LRESULT CALLBACK MenuWindowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int	x, y, height;

	switch (message) 
	{
		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				if((LOWORD(wParam) >= IDC_BUTTON01) && (LOWORD(wParam) <= IDC_BUTTON24))
					SendMessage(myPtr->hParentWin, WM_USER + 1, TRUE, LOWORD(wParam) - IDC_BUTTON01);
			}
			break;

		case WM_KILLFOCUS:
			if(IsWindowVisible(hWnd) && ((HWND)wParam != myPtr->hParentWin))
				PostMessage(myPtr->hParentWin, WM_USER + 1, 0, 0);
			break;

		case WM_DISPLAYCHANGE:
			ConfigGetInt(BUTTONHEIGHT, &height);
			ConfigGetInt(MENUROWS, &y);
			ConfigGetInt(MENUCOLUMS, &x);

			SetWindowPos(hWnd, HWND_NOTOPMOST, 0, GetSystemMetrics(SM_CYSCREEN) - ((y + 1) * height), 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE);
			break;

		case WM_DESTROY:
			myPtr->hMenu = NULL;
			SendMessage(myPtr->hParentWin, WM_USER + 1, 0, 0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   UpdateWindow(hWnd);
   return(0);
}


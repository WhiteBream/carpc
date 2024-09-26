// StartButton.cpp: implementation of the StartButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Launcher.h"
#include "StartButton.h"
#include "Config.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

#define TIMERDELAY	8000

LRESULT CALLBACK StartButtonWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static StartButton * my;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StartButton::StartButton(HINSTANCE hInstance, HWND hParent)
{
	LONG  newStyle;
	int   width, height;

	my = this;
	hInst = hInstance;
	MyRegisterClass(hInstance);

	hStart = CreateWindow(TEXT("CRPC_STRTBTTN"), TEXT("CARGO"), WS_BORDER,
			0, 0, 0, 0, hParent, NULL, hInstance, NULL);

	newStyle = GetWindowLong(hStart, GWL_STYLE);
	newStyle &= ~(WS_CAPTION | WS_BORDER | WS_THICKFRAME |
				 WS_OVERLAPPED | WS_TILED | WS_DLGFRAME );
	SetWindowLong(hStart, GWL_STYLE, newStyle);

	SetWindowLong(hStart, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

	ConfigGetInt(BUTTONHEIGHT, &height);
	ConfigGetInt(STARTBUTTONWIDTH, &width);
	SetWindowPos(hStart, HWND_TOPMOST, 0, GetSystemMetrics(SM_CYSCREEN) - height, 
			width, height, SWP_SHOWWINDOW | SWP_FRAMECHANGED);

	CreateWindow(TEXT("WB_DESTBUTTON"), NULL, WS_CHILD | WS_VISIBLE, 
		0, 0, width, height, hStart, (HMENU)IDC_START, hInstance, NULL);
	AssignIcon(hInstance, hStart, IDC_START, IDI_START);

	ShowWindow(hStart, SW_SHOW);
	UpdateWindow(hStart);

	hService = OpenPsuEvent(hStart, PSU_STATE);

	menuWnd = new MenuWindow(hInst, hStart);
}

StartButton::~StartButton()
{
	if(hService) ClosePsuEvent(hService, NULL);
	if(menuWnd) delete menuWnd;
	DestroyWindow(hStart);
}

void StartButton::Release()
{
	if(IsWindowVisible(menuWnd->hMenu))
	{
		KillTimer(hStart, 1);
		ShowWindow(menuWnd->hMenu, SW_HIDE);
		SendDlgItemMessage(hStart, IDC_START, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
		UpdateWindow(menuWnd->hMenu);
		SetFocus(hStart);
		UpdateWindow(hStart);
	}
	else
	{
		SendDlgItemMessage(hStart, IDC_START, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		SetTimer(hStart, 1, TIMERDELAY, NULL);
		ShowWindow(menuWnd->hMenu, SW_SHOW);
		SetForegroundWindow(menuWnd->hMenu);
	}
}

ATOM StartButton::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= StartButtonWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, TEXT("StartMenu.ico"));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= TEXT("");
	wcex.lpszClassName	= TEXT("CRPC_STRTBTTN");
	wcex.hIconSm		= LoadIcon(wcex.hInstance, TEXT("small.ico"));

	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK StartButtonWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static INT	vStartApplication;

	HDC			hdc;
	PAINTSTRUCT	ps;
	int			height;

	switch (message) 
	{
		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				if(LOWORD(wParam) == IDC_START)
					my->Release();
			}
			break;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
			break;

		case WM_DISPLAYCHANGE:
		case WM_SIZE:
			ConfigGetInt(BUTTONHEIGHT, &height);
			SetWindowPos(hWnd, HWND_TOPMOST, 0, GetSystemMetrics(SM_CYSCREEN) - height, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE);
			UpdateWindow(hWnd);
			break;

		case WM_TIMER:
			if(wParam == 1)
				my->Release();
			else if(wParam == 2)
			{
				KillTimer(hWnd, 2);
				StartApplication(vStartApplication);
				SetFocus(my->hStart);
			}
			break;

		case WM_USER + 1:
			my->Release();
			ShowWindow(my->hStart, SW_SHOW);
			if(wParam)
			{
				vStartApplication = lParam;
				SetTimer(hWnd, 2, 50, NULL);
			}
			break;

		case WB_PSUEVENT:
			if(wParam == PSU_STATE)
			{
				if(lParam == SYS_SHUTDOWN_EXTEND)
				{
					// Request shutdown time
					// Display notification dialog with time
					MessageBox(hWnd, TEXT("The power supply just entered extended\noperation time mode."), TEXT("Power supply"), MB_OK | MB_ICONINFORMATION);
				}
			}
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   UpdateWindow(hWnd);
   return(0);
}


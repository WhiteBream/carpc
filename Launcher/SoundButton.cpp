// StartButton.cpp: implementation of the StartButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Launcher.h"
#include "SoundButton.h"
#include "Config.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

#define TIMERDELAY	8000

LRESULT CALLBACK SoundButtonWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static SoundButton * my;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SoundButton::SoundButton(HINSTANCE hInstance, HWND hParent)
{
	LONG	newStyle;
	INT		size;

	my = this;
	volWnd = NULL;
	hInst = hInstance;
	MyRegisterClass(hInstance);

	hWnd = CreateWindow(TEXT("CRPC_SNDBTTN"), TEXT("Sound"), WS_BORDER,
								0, 0, 0, 0, hParent, NULL, hInstance, NULL);

	newStyle = GetWindowLong(hWnd, GWL_STYLE);
	newStyle &= ~(WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_OVERLAPPED | WS_TILED | WS_DLGFRAME );
	SetWindowLong(hWnd, GWL_STYLE, newStyle);

	SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

	ConfigGetInt(BUTTONHEIGHT, &size);
	SetWindowPos(hWnd, HWND_TOPMOST, GetSystemMetrics(SM_CXSCREEN) - size, GetSystemMetrics(SM_CYSCREEN) - size,
						size, size, SWP_SHOWWINDOW | SWP_FRAMECHANGED);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE,
						0, 0, size, size, hWnd, (HMENU)IDC_SOUND, hInstance, NULL);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	volWnd = new SoundWindow(hInst, hWnd);
}

SoundButton::~SoundButton()
{
	if(volWnd) delete volWnd;
	DestroyWindow(hWnd);
}

void SoundButton::Release()
{
	if(IsWindowVisible(volWnd->hWnd))
	{
		KillTimer(hWnd, 1);
		ShowWindow(volWnd->hWnd, SW_HIDE);
		SendDlgItemMessage(hWnd, IDC_SOUND, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
		UpdateWindow(volWnd->hWnd);
		SetFocus(hWnd);
		UpdateWindow(hWnd);
	}
	else
	{
		SendDlgItemMessage(hWnd, IDC_SOUND, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		SetTimer(hWnd, 1, TIMERDELAY, NULL);
		ShowWindow(volWnd->hWnd, SW_SHOW);
		SetForegroundWindow(volWnd->hWnd);
	}
}

ATOM SoundButton::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= 0;
	wcex.lpfnWndProc	= SoundButtonWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= TEXT("");
	wcex.lpszClassName	= TEXT("CRPC_SNDBTTN");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK SoundButtonWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	INT size;

	switch (message)
	{
		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				if(LOWORD(wParam) == IDC_SOUND)
					my->Release();
			}
			break;

		case WM_DISPLAYCHANGE:
		case WM_SIZE:
			ConfigGetInt(BUTTONHEIGHT, &size);
			SetWindowPos(hWnd, HWND_TOPMOST, GetSystemMetrics(SM_CXSCREEN) - size, GetSystemMetrics(SM_CYSCREEN) - size, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE);
			UpdateWindow(hWnd);
			break;

		case WM_USER + 1:
			my->Release();
			ShowWindow(my->hWnd, SW_SHOW);
			break;

		case WM_USER + 2:
			AssignIcon(my->hInst, hWnd, IDC_SOUND, (wParam ? IDI_SPK_OFF : IDI_SPK_ON));
			break;

		case WM_USER + 3:
			SetTimer(hWnd, 1, TIMERDELAY, NULL);
			break;

		case WM_TIMER:
			if(wParam == 1)
				my->Release();
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}


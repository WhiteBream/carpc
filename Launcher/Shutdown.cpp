// Shutdown.cpp: implementation of the Shutdown class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Shutdown.h"
#include "Config.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

LRESULT CALLBACK ShutdownWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK FilterWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static Shutdown * my;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Shutdown::Shutdown(HINSTANCE hInstance, HWND hParent)
{
	int		offset;
	MSG		msg;
	BOOL	bRet;
	LONG	newStyle;

	my = this;
	hInst = hInstance;
	MyRegisterClass(hInstance);
	vClicked = FALSE;

	hBlock = CreateWindowEx(WS_EX_TOOLWINDOW, TEXT("CRPC_SHTFLT"), TEXT("CARGO Shutdown filter"), WS_BORDER, 0, 0, 
			GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInstance, NULL);
	newStyle = GetWindowLong(hBlock, GWL_STYLE);
	newStyle &= ~(WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_OVERLAPPED | WS_TILED | WS_DLGFRAME );
	SetWindowLong(hBlock, GWL_STYLE, newStyle);
	SetWindowLong(hBlock, GWL_EXSTYLE, WS_EX_NOACTIVATE);

	SetWindowPos(hBlock, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);

	hBar = CreateWindow(TEXT("CRPC_SHTDWN"), TEXT("CARGO Shutdown"), WS_POPUP | WS_DLGFRAME,
      0, 0, 0, 0, hParent, NULL, hInstance, NULL);

	ConfigGetInt(BUTTONHEIGHT, &offset);

	SetWindowPos(hBar, HWND_TOPMOST, (GetSystemMetrics(SM_CXSCREEN) - 8 * offset - 6) / 2,
   			(GetSystemMetrics(SM_CYSCREEN) - 2 * offset) / 2, 8 * offset + 6, 5 * offset / 2, SWP_SHOWWINDOW | SWP_FRAMECHANGED);

	if(FindWindow(TEXT("Shell_TrayWnd"), TEXT("")))
	{
		CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("Exit"), WS_CHILD | WS_VISIBLE, offset / 2, offset / 2, 
			2 * offset, offset, hBar, (HMENU)IDC_EXIT, hInstance, NULL);
	}
	else
	{
		CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("Log off"), WS_CHILD | WS_VISIBLE, offset / 2, offset / 2, 
			2 * offset, offset, hBar, (HMENU)IDC_LOGOFF, hInstance, NULL);
	}

	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("Turn off"), WS_CHILD | WS_VISIBLE, 3 * offset, offset / 2, 
		2 * offset, offset, hBar, (HMENU)IDC_SHUTDOWN, hInstance, NULL);

	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("Restart"), WS_CHILD | WS_VISIBLE, 11 * offset / 2, offset / 2, 
		2 * offset, offset, hBar, (HMENU)IDC_REBOOT, hInstance, NULL);

	SetWindowPos(hBar, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
	UpdateWindow(hBar);

	while((bRet = GetMessage(&msg, NULL, 0, 0)))
	{
		if(bRet == -1)
			break;
		TranslateMessage(&msg);

		if((msg.hwnd == hBar) || IsChild(hBar, msg.hwnd))
			DispatchMessage(&msg);

		if((msg.hwnd == hBar) && (msg.message == WM_CLOSE))
			break;
	}
	if(bRet == 0)
		PostQuitMessage(0);
	PostMessage(hBlock, WM_CLOSE, 0, 0);
}

Shutdown::~Shutdown()
{
}

void ShutdownRequest(HWND hWnd)
{
	PostMessage(hWnd, WM_SYSKEYDOWN, VK_F4, 0);
}

ATOM Shutdown::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= 0;
	wcex.lpfnWndProc	= FilterWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_SHTFLT");
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);

	wcex.lpfnWndProc	= ShutdownWndProc;
	wcex.hbrBackground	= CreateSolidBrush(RGB(200, 255, 200));
	wcex.lpszClassName	= TEXT("CRPC_SHTDWN");

	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK ShutdownWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_DESTROY:
			break;

		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					HANDLE hToken; 
					TOKEN_PRIVILEGES tkp; 

					switch(LOWORD(wParam))
					{
						case IDC_EXIT:
							PostQuitMessage(0);
							break;

						case IDC_LOGOFF:
							ChangeDisplaySettings(NULL, 0);
							ExitWindowsEx(EWX_LOGOFF | EWX_FORCEIFHUNG, 0);
							break;

						case IDC_SHUTDOWN:
							OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
							LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 
 
							tkp.PrivilegeCount = 1;  // one privilege to set    
							tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
 
							AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); 
 							ExitWindowsEx(EWX_POWEROFF | EWX_FORCEIFHUNG, 0);
							break;

						case IDC_REBOOT:
							OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
							LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 
 
							tkp.PrivilegeCount = 1;  // one privilege to set    
							tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
 
							AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); 
 							ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG, 0);
							break;
					}
					break;
			}
			break;

		case WM_LBUTTONDOWN:
			my->vClicked = TRUE;
			break;

		case WM_LBUTTONUP:
			if(my->vClicked == TRUE)
				PostMessage(hWnd, WM_CLOSE, 1234567, 0);
			break;

		case WM_SYSKEYDOWN:
			if(wParam != VK_F4)
				return(DefWindowProc(hWnd, message, wParam, lParam));
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   UpdateWindow(hWnd);
   return(0);
}

LRESULT CALLBACK FilterWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_SYSKEYDOWN:
			if(wParam != VK_F4)
				return(DefWindowProc(hWnd, message, wParam, lParam));
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}

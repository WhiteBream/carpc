// Launcher.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "Launcher.h"
#include "StartButton.h"
#include "SoundButton.h"
#include "StatusBar.h"
#include "Desktop.h"
#include "LaunchOnce.h"
#include "Config.h"
#include "Shutdown.h"
#include "Settings.h"

#define MAX_LOADSTRING 100

// Global Variables:
TCHAR			szTitle[MAX_LOADSTRING];
StartButton		*startButton;
StatusBar		*statusBar;
Desktop			*desktopWin;
SoundButton		*soundButton;
HINSTANCE		hInst;
LPTSTR			pAppName = NULL;

// Foward declarations of functions included in this code module:
void InitInstance(HINSTANCE);
extern BOOL CheckXPIntegration(void);
extern void TouchCal(void);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	GdiplusStartupInput	gdiplusStartupInput;
	ULONG_PTR			gdiplusToken;
	HWND				oldWin;
	MSG					msg;
	BOOL				bRet;
	int					x, n;
	int					nArgs;
	LPTSTR				*pCommandLine;
	LPTSTR				pAppPath, pPathEnv;
	size_t				vLen;
	HANDLE				hShellReadyEvent;

	// Terminate any pending instance of our application
	oldWin = FindWindow(TEXT("CRPC_STRTBTTN"), NULL);
	if(oldWin)
		SendMessage(oldWin, WM_CLOSE, 0, 0);

	// Initialize GDI+.
	gdiplusStartupInput.DebugEventCallback = NULL;
	gdiplusStartupInput.GdiplusVersion = 1;
	gdiplusStartupInput.SuppressBackgroundThread = FALSE;
	gdiplusStartupInput.SuppressExternalCodecs = FALSE;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Get the commandline; use a supplied filename as the inifile
	pCommandLine = CommandLineToArgv(GetCommandLine(), &nArgs);
	ConfigRead(nArgs > 1 ? pCommandLine[1] : NULL);

	// ...and save our exec/path variable
	StringCchLength(pCommandLine[0], MAX_PATH, &vLen);
	vLen = (vLen + 1) * sizeof(TCHAR);
	pAppName = (LPTSTR)GlobalAlloc(GPTR, vLen);
	StringCbCopy(pAppName, vLen, pCommandLine[0]);
	GlobalFree(pCommandLine);

	// Include our startpath to PATH environment variable
	pAppPath = (LPTSTR)GlobalAlloc(GPTR, vLen);
	StringCbCopy(pAppPath, vLen, pAppName);
	pPathEnv = _tcsrchr(pAppPath, TEXT('\\'));
	if(pPathEnv)
		*pPathEnv = TEXT('\0');

	pPathEnv = _tgetenv(TEXT("PATH"));
	StringCchLength(pPathEnv, STRSAFE_MAX_CCH, (size_t*)&nArgs);
	nArgs = (nArgs + 1) * sizeof(TCHAR);
	vLen += nArgs + 5 * sizeof(TCHAR);
	pPathEnv = (LPTSTR)GlobalAlloc(GPTR, vLen);
	StringCbPrintf(pPathEnv, vLen, TEXT("PATH=%s;%s"), _tgetenv(TEXT("PATH")), pAppPath);
	_tputenv(pPathEnv);
	GlobalFree(pPathEnv);
	GlobalFree(pAppPath);

	// Try to register with windows
	if(CheckXPIntegration())
		return(1);

	hShellReadyEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("msgina: ShellReadyEvent"));
	if(hShellReadyEvent)
	{
		SetEvent(hShellReadyEvent);
		CloseHandle(hShellReadyEvent);
	}

	// Adjust screensize for small displays
	SetScreenSize();

	// Allow for toucscreen calibration
	TouchCal();

	// Perform application initialization:
	InitInstance(hInstance);

	// Try to start the autostart applications
	ConfigGetInt(MENUROWS, &n);
	ConfigGetInt(MENUCOLUMS, &x);
	n *= x;
	for(x = 0; x < n; x++)
	{
		if(sMenu[x].autoStart == TRUE)
			StartApplication(x);
		sMenu[x].minimize = FALSE;
	}

	// Main message loop:
	while((bRet = GetMessage(&msg, NULL, 0, 0)))
	{
		if(bRet == -1)
			break;
		TranslateMessage(&msg);
		if((msg.message == WM_SYSKEYDOWN) && (msg.wParam == VK_F4))
			Shutdown(hInst, startButton->hStart);
		else
			DispatchMessage(&msg);
	}

	// Ende; kill any pending applications started by us
	for(x = 0; x < n; x++)
	{
		if(sMenu[x].menuType == MENU_EXE)
			KillApp(sMenu[x].exeName);
	}

	ChangeDisplaySettings(NULL, 0);
	GlobalFree(pAppName);
	return(msg.wParam);
}

void InitInstance(HINSTANCE hInstance)
{
	hInst = hInstance;

	DestinatorInit(hInstance);

	desktopWin = new Desktop(hInstance, NULL);
	statusBar = new StatusBar(hInstance, desktopWin->hDesktop);
	startButton = new StartButton(hInstance, NULL);

	soundButton = new SoundButton(hInstance, NULL);
}

void SetScreenSize(void)
{
	HANDLE	hServerStopEvent = NULL;
	DEVMODE	vDevMode;
	INT vVal = 0;

	if(FindWindow(TEXT("Shell_TrayWnd"), TEXT("")) == NULL)
	{
		hServerStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if(hServerStopEvent)
		{
			ConfigGetInt(SETSCREENSIZE, &vVal);
			if(vVal == 1)
			{
				LONG vTest;
				ConfigGetInt(SCREENSIZE, &vVal);
				if(vVal == 1)
				{
					vDevMode.dmPelsWidth = 800;
					vDevMode.dmPelsHeight = 600;
				}
				else
				{
					vDevMode.dmPelsWidth = 640;
					vDevMode.dmPelsHeight = 480;
				}
				vDevMode.dmSize = sizeof(DEVMODE);
				vDevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
				while((vTest = ChangeDisplaySettings(&vDevMode, 0)) == DISP_CHANGE_FAILED)
					WaitForSingleObject(hServerStopEvent, 100);

			/*	switch(vTest)
				{
				//	case DISP_CHANGE_BADDUALVIEW:	MessageBox(NULL, TEXT("DISP_CHANGE_BADDUALVIEW"), TEXT("wait"), MB_OK);	break;
					case DISP_CHANGE_BADFLAGS:		MessageBox(NULL, TEXT("DISP_CHANGE_BADFLAGS"), TEXT("wait"), MB_OK);	break;
					case DISP_CHANGE_BADMODE:		MessageBox(NULL, TEXT("DISP_CHANGE_BADMODE"), TEXT("wait"), MB_OK);		break;
					case DISP_CHANGE_BADPARAM:		MessageBox(NULL, TEXT("DISP_CHANGE_BADPARAM"), TEXT("wait"), MB_OK);	break;
					case DISP_CHANGE_FAILED:		MessageBox(NULL, TEXT("DISP_CHANGE_FAILED"), TEXT("wait"), MB_OK);		break;
					case DISP_CHANGE_NOTUPDATED:	MessageBox(NULL, TEXT("DISP_CHANGE_NOTUPDATED"), TEXT("wait"), MB_OK);	break;
					case DISP_CHANGE_RESTART:		MessageBox(NULL, TEXT("DISP_CHANGE_RESTART"), TEXT("wait"), MB_OK);		break;
					case DISP_CHANGE_SUCCESSFUL:	break;
					default:  MessageBox(NULL, TEXT("DISP_CHANGE_????"), TEXT("wait"), MB_OK); break;
				}	*/
			}
			else
			{
				while(ChangeDisplaySettings(NULL, 0) == DISP_CHANGE_FAILED)
					WaitForSingleObject(hServerStopEvent, 200);
			}
			CloseHandle(hServerStopEvent);
		}
	}
}

void ShowDesktop(void)
{
	BringWindowToTop(desktopWin->hDesktop);
	BringWindowToTop(statusBar->hBar);
	BringWindowToTop(startButton->hStart);
}

void StartApplication(int n)
{
	PROCESS_INFORMATION	strProc;
	STARTUPINFO			supInfo;
	LONG                retVal;

	if(sMenu[n].menuType == MENU_EXE)
	{
		LaunchOnce(sMenu[n].exeName, sMenu[n].setSize, sMenu[n].minimize);
	}
	else if(sMenu[n].menuType == MENU_MAN)
	{
		supInfo.cb = sizeof(STARTUPINFO);
		supInfo.lpReserved = NULL;
		supInfo.lpDesktop = NULL;
		supInfo.lpTitle = NULL;  // console param
		supInfo.dwX = 0;
		supInfo.dwY = 0;
		supInfo.dwXSize = GetSystemMetrics(SM_CXSCREEN);
		supInfo.dwYSize = GetSystemMetrics(SM_CYSCREEN);
		supInfo.dwFlags = STARTF_USEPOSITION | STARTF_USESIZE | STARTF_USESHOWWINDOW;
		supInfo.wShowWindow = SW_SHOWDEFAULT;
		supInfo.cbReserved2 = 0;
		supInfo.lpReserved2 = NULL;

		retVal = CreateProcess(NULL,			// LPCTSTR lpApplicationName,
								sMenu[n].exeName,		// LPTSTR lpCommandLine,
								NULL,			// LPSECURITY_ATTRIBUTES lpProcessAttributes,
								NULL,			// LPSECURITY_ATTRIBUTES lpThreadAttributes,
								TRUE,			// BOOL bInheritHandles,
								0,				// DWORD dwCreationFlags,
								NULL,			// LPVOID lpEnvironment,
								NULL,			// LPCTSTR lpCurrentDirectory,
								&supInfo,		// LPSTARTUPINFO lpStartupInfo,
								&strProc);		// LPPROCESS_INFORMATION lpProcessInformation
		if(retVal == FALSE)
		{
			TCHAR vError[300];
			StringCbPrintf(vError, sizeof(vError), TEXT("Failed to start \"%s\"\n(%s)"), sMenu[n].menuName, sMenu[n].exeName);
			MessageBox(NULL, vError, TEXT("Error"), MB_OK | MB_ICONWARNING);
		}
	}
	else if(sMenu[n].menuType == MENU_INT)
	{
		if(StrCmp(sMenu[n].exeName, TEXT("SHUTDOWN")) == 0)
			ShutdownRequest(startButton->hStart);

		if(StrCmp(sMenu[n].exeName, TEXT("DESKTOP")) == 0)
			ShowDesktop();

		if(StrCmp(sMenu[n].exeName, TEXT("SETTINGS")) == 0)
			new Settings(hInst, startButton->hStart);
	}
}

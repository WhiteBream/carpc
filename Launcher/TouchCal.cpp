// TouchCal.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"


void TouchCal(void)
{
	HKEY	hReg;
	DWORD	vLength;
	LONG	result = 1;
	TCHAR	vPath[MAX_PATH];
	PROCESS_INFORMATION	strProc;
	STARTUPINFO			supInfo;

	// Find XCal in runonce

	if(RegCreateKey(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce"), &hReg) == ERROR_SUCCESS)
	{
		vLength = MAX_PATH;
		result = RegQueryValueEx(hReg, TEXT("AutoCalibration"), 0, NULL, (PBYTE)vPath, &vLength); 
		RegDeleteKey(hReg, TEXT("AutoCalibration"));
		RegCloseKey(hReg);
	}
	if(result == ERROR_SUCCESS)
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

		result = CreateProcess(NULL,			// LPCTSTR lpApplicationName,
								vPath,		// LPTSTR lpCommandLine,
								NULL,			// LPSECURITY_ATTRIBUTES lpProcessAttributes,
								NULL,			// LPSECURITY_ATTRIBUTES lpThreadAttributes,
								TRUE,			// BOOL bInheritHandles,
								0,				// DWORD dwCreationFlags,
								NULL,			// LPVOID lpEnvironment,
								NULL,			// LPCTSTR lpCurrentDirectory,
								&supInfo,		// LPSTARTUPINFO lpStartupInfo,
								&strProc);		// LPPROCESS_INFORMATION lpProcessInformation
		if(result == FALSE)
		{
			TCHAR vError[300];
			StringCbPrintf(vError, sizeof(vError), TEXT("Failed to start the application\n(%s)"), vPath);
			MessageBox(NULL, vError, TEXT("Error"), MB_OK | MB_ICONWARNING);
		}

		if(strProc.hProcess)
		{
			HANDLE hWaitEvent;
			hWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if(hWaitEvent)
			{
				WaitForSingleObject(hWaitEvent, 100);
				CloseHandle(hWaitEvent);
			}
			//WaitForInputIdle(strProc.hProcess, 2500);
			WaitForSingleObject(strProc.hProcess, 60000);
			CloseHandle(strProc.hProcess);
		}
		if(strProc.hThread)
			CloseHandle(strProc.hThread);
	}
}

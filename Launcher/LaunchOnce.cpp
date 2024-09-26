/*
 $Id:  $
***************************************************************************
 White Bream
 Terborchdreef 26
 3262NB  Oud-Beijerland
 The Netherlands
 www.whitebream.com
 info@whitebream.nl
***************************************************************************

 Launchonce

***************************************************************************
 Copyright (c) 2003, White Bream
***************************************************************************
 File:          launchonce.cpp
 Project id.:   -

 Version:       0.10
 Creation date: May 18, 2003
 Revison date:  May 18, 2003
 Author:        Henk Bliek
***************************************************************************
 Revisionlog:
 0.10     May 18, 2003
          * Initial
**************************************************************************/

#include "stdafx.h"
#include <stdio.h>
#include <winbase.h>
#include "LaunchOnce.h"


typedef struct _EnumCallbackData
{
	DWORD	processId;
	HWND	topWindow;
	BOOL	alreadyRunning;
	BOOL	setSize;
	BOOL	minimize;
	INT		success;
} EnumCallbackData;


BOOL ModifyProcess(DWORD processId, DWORD threadId, BOOL alreadyRunning, BOOL setSize, BOOL minimize);
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
void InformationDialog(void);


int LaunchOnce(LPTSTR lpCmdLine, BOOL setSize, BOOL minimize)
{
	PROCESS_INFORMATION	strProc;
	STARTUPINFO			supInfo;
	DWORD               registryResult, tmpStatus, idSize, regValueType;
	HANDLE              tmpHandle;
	HKEY                regKeyHandle, processRegHandle, regAppKeyHandle;
	LONG                retVal;
	BOOL                isRunning;
	size_t				pcb;
	INT					i;
	union
	{
		DWORD	dwid;
		BYTE	bid[4];
	} process;

	/* Read the registry stuff */
	retVal = RegCreateKeyEx(HKEY_CURRENT_USER,		//HKEY hKey,                        // handle to open key
							HKEY_WHITEBREAM,		//LPCTSTR lpSubKey,                 // subkey name
							0,						//DWORD Reserved,                   // reserved
							TEXT(""),				//LPTSTR lpClass,                   // class string
							REG_OPTION_NON_VOLATILE,//DWORD dwOptions,                  // special options
							KEY_ALL_ACCESS,			//REGSAM samDesired,                // desired security access
							NULL,					//LPSECURITY_ATTRIBUTES lpSecAttr,	// inheritance
							&regKeyHandle,			//PHKEY phkResult,                  // key handle 
							&registryResult);		//LPDWORD lpdwDisposition           // disposition value buffer

	retVal = RegCreateKeyEx(regKeyHandle,			//HKEY hKey,                        // handle to open key
							HKEY_PROCESS,			//LPCTSTR lpSubKey,                 // subkey name
							0,						//DWORD Reserved,                   // reserved
							TEXT(""),				//LPTSTR lpClass,                   // class string
							REG_OPTION_NON_VOLATILE,//DWORD dwOptions,                  // special options
							KEY_ALL_ACCESS,			//REGSAM samDesired,                // desired security access
							NULL,					//LPSECURITY_ATTRIBUTES lpSecAttr,	// inheritance
							&processRegHandle,		//PHKEY phkResult,                  // key handle 
							&registryResult);		//LPDWORD lpdwDisposition           // disposition value buffer

	if(registryResult == REG_CREATED_NEW_KEY)
	{
		RegSetValueEx(processRegHandle,				//HKEY hKey,            // handle to key
					  NULL,							//LPCTSTR lpValueName,  // value name
					  0,							//DWORD Reserved,       // reserved
					  REG_SZ,						//DWORD lpType,         // type buffer
		(CONST BYTE *)PROCESS_INFOSTR,				//CONST BYTE * lpData,  // data buffer
					  sizeof(PROCESS_INFOSTR)-1);	//DWORD lpcbData        // size of data buffer
	}

	retVal = RegCreateKeyEx(processRegHandle,		//HKEY hKey,                        // handle to open key
							lpCmdLine,				//LPCTSTR lpSubKey,                 // subkey name
							0,						//DWORD Reserved,                   // reserved
							TEXT(""),				//LPTSTR lpClass,                   // class string
							REG_OPTION_VOLATILE,	//DWORD dwOptions,                  // special options
							KEY_ALL_ACCESS,			//REGSAM samDesired,                // desired security access
							NULL,					//LPSECURITY_ATTRIBUTES lpSecAttr,	// inheritance
							&regAppKeyHandle,		//PHKEY phkResult,                  // key handle 
							&registryResult);		//LPDWORD lpdwDisposition           // disposition value buffer

	idSize = sizeof(process);
	regValueType = REG_DWORD;
	process.dwid = 0;

	retVal = RegQueryValueEx(regAppKeyHandle,		//HKEY hKey,            // handle to key
							HKEY_PROCESS_ID,		//LPCTSTR lpValueName,  // value name
							NULL,					//LPDWORD lpReserved,   // reserved
							&regValueType,			//LPDWORD lpType,       // type buffer
							process.bid,			//LPBYTE lpData,        // data buffer
							&idSize);				//LPDWORD lpcbData      // size of data buffer

	isRunning = FALSE;

	if((retVal == ERROR_SUCCESS) && (registryResult == REG_OPENED_EXISTING_KEY) && ((process.dwid != 0)))
	{
		/* Program seems to be running already */
		tmpHandle = OpenProcess(PROCESS_QUERY_INFORMATION,	//DWORD dwDesiredAccess,
								FALSE,						//BOOL bInheritHandle,
								process.dwid);				//DWORD dwProcessId

		if(tmpHandle != NULL)
		{
			/* Test if process not exited */
			if(GetExitCodeProcess(tmpHandle, &tmpStatus))
			{
				if(tmpStatus == STILL_ACTIVE)
				{
					isRunning = TRUE;
				}
			}
			CloseHandle(tmpHandle);
		}
	}

	if(StringCchLength(lpCmdLine, STRSAFE_MAX_CCH, &pcb) == S_OK)
	{
		if(pcb == 0)
		{
			RegCloseKey(processRegHandle);
			RegCloseKey(regKeyHandle);
			RegCloseKey(regAppKeyHandle);
			return 1;
		}
	}

	if(isRunning == FALSE)
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
		if(minimize || setSize)
			supInfo.wShowWindow = SW_HIDE;  
		else
			supInfo.wShowWindow = SW_SHOWDEFAULT;  
		supInfo.cbReserved2 = 0;  
		supInfo.lpReserved2 = NULL;  

		retVal = CreateProcess(NULL,			// LPCTSTR lpApplicationName,
								lpCmdLine,		// LPTSTR lpCommandLine,
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
			RegCloseKey(processRegHandle);
			RegCloseKey(regKeyHandle);
			RegCloseKey(regAppKeyHandle);

			TCHAR vError[300];
			StringCbPrintf(vError, sizeof(vError), TEXT("Failed to start the application\n(%s)"), lpCmdLine);
			MessageBox(NULL, vError, TEXT("Error"), MB_OK | MB_ICONWARNING);
			return 2;
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
			process.dwid = strProc.dwProcessId;
			WaitForInputIdle(strProc.hProcess, 2500);
			CloseHandle(strProc.hProcess);
		}
		if(strProc.hThread)
			CloseHandle(strProc.hThread);

		/* Store the new id's in the registry */
		retVal = RegSetValueEx(regAppKeyHandle,		//HKEY hKey,            // handle to key
								HKEY_PROCESS_ID,	//LPCTSTR lpValueName,  // value name
								0,					//DWORD Reserved,       // reserved
								regValueType,		//DWORD lpType,         // type buffer
				  (CONST BYTE *)process.bid,		//CONST BYTE * lpData,  // data buffer
								idSize);			//DWORD lpcbData        // size of data buffer
	}

	i = 0;
	do
	{
		if(ModifyProcess(process.dwid, 0, isRunning, setSize, minimize))
			break;
		else
		{
			HANDLE hWaitEvent;
			hWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if(hWaitEvent)
			{
				WaitForSingleObject(hWaitEvent, 100);
				CloseHandle(hWaitEvent);
			}
		}
	} while(i++ < 25);

	RegCloseKey(processRegHandle);
	RegCloseKey(regKeyHandle);
	RegCloseKey(regAppKeyHandle);
	return 0;
}


BOOL ModifyProcess(DWORD processId, DWORD threadId, BOOL alreadyRunning, BOOL setSize, BOOL minimize)
{
	LONG				newStyle;
	POINT				testPoint;
	BOOL				isVisible;
	WINDOWPLACEMENT		winPlacement;
	DWORD				winProcess;
	DWORD				winThread;
	EnumCallbackData	cbIo;

	cbIo.processId = processId;
	cbIo.topWindow = 0;
	cbIo.alreadyRunning = alreadyRunning;
	cbIo.setSize = setSize;
	cbIo.minimize = minimize;
	cbIo.success = 0;

	EnumWindows(EnumWindowsProc, (LPARAM)&cbIo);
	return(cbIo.success ? TRUE : FALSE);

	/* Now the window is found or all windows checked */

	if(cbIo.topWindow == 0)
	{
		//MessageBox(NULL, TEXT("No window..."), TEXT("Start"), MB_OK);
		return(FALSE);
	}

	testPoint.x = GetSystemMetrics(SM_CXSCREEN) - 50;
	testPoint.y = 50;

	isVisible = FALSE;
	winThread = GetWindowThreadProcessId(WindowFromPoint(testPoint), &winProcess);
	if(winProcess == cbIo.processId)
	{
		isVisible = TRUE;
	}

	if(isVisible && alreadyRunning)
	{
		CloseWindow(cbIo.topWindow);
		SetForegroundWindow(WindowFromPoint(testPoint));
	}
	else
	{
		if(setSize)
		{
			/* Fill Windowplacement vars */
			memset(&winPlacement, 0, sizeof(WINDOWPLACEMENT));
			winPlacement.length = sizeof(WINDOWPLACEMENT);
			winPlacement.flags = WPF_SETMINPOSITION | WPF_RESTORETOMAXIMIZED;
			if(minimize)
				winPlacement.showCmd = SW_SHOWMINNOACTIVE;
			else
				winPlacement.showCmd = SW_SHOWMAXIMIZED;
			winPlacement.ptMinPosition.y = GetSystemMetrics(SM_CYSCREEN) - 1;
			SetWindowPlacement(cbIo.topWindow, &winPlacement);

			newStyle = GetWindowLong(cbIo.topWindow, GWL_STYLE);
			newStyle &= ~(WS_CAPTION | WS_THICKFRAME);
			newStyle &= ~(WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
			SetWindowLongPtr(cbIo.topWindow, GWL_STYLE, newStyle);
			SetWindowLongPtr(cbIo.topWindow, GWL_EXSTYLE, WS_EX_APPWINDOW);
			SetWindowPos(cbIo.topWindow, HWND_NOTOPMOST, 0, 0, 
						GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_NOACTIVATE | SWP_FRAMECHANGED);

			newStyle = GetWindowLong(cbIo.topWindow, GWL_STYLE);
			if(!newStyle)
				return(FALSE);

			if(minimize)
				ShowWindow(cbIo.topWindow, SW_SHOWMINNOACTIVE);
			else
			{
				ShowWindow(cbIo.topWindow, SW_SHOWMAXIMIZED);

				SetForegroundWindow(cbIo.topWindow);
				SetFocus(cbIo.topWindow);
				BringWindowToTop(cbIo.topWindow);
			}
		}
		else
		{
			ShowWindow(cbIo.topWindow, SW_SHOWMAXIMIZED);

			SetForegroundWindow(cbIo.topWindow);
			SetFocus(cbIo.topWindow);
			BringWindowToTop(cbIo.topWindow);
		}
	}
	return(TRUE);
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD				windowProcess;
	DWORD				threadId;
	TCHAR				strWinName[21];
	WINDOWINFO			sWinInfo;
	EnumCallbackData	*cbIo = (EnumCallbackData*)lParam;

	LONG			newStyle;
	POINT			testPoint;
	BOOL			isVisible;
	WINDOWPLACEMENT winPlacement;
	DWORD			winProcess;
	DWORD			winThread;

	/* Get the process Id */
	threadId = GetWindowThreadProcessId(hwnd, &windowProcess);

	if(windowProcess == cbIo->processId)
	{
		// Leave child windows alone
		if(GetParent(hwnd) != NULL)
			return(TRUE);

		// Leave popup & child windows alone
		GetWindowInfo(hwnd, &sWinInfo);
		if(sWinInfo.dwStyle & (WS_POPUP | WS_CHILDWINDOW))
			return(TRUE);

		// Leave owned windows alone
		if(GetWindow(hwnd, GW_OWNER) != NULL)
			return(TRUE);

		// Do not mess with Direct X windows
		GetWindowText(hwnd, strWinName, sizeof(strWinName)-1);
		if(StrCmp(strWinName, TEXT("GDI+ Window")) == 0)
			return(TRUE);

		if(StrCmp(strWinName, TEXT("ActiveMovie Window")) == 0)
			return(TRUE);

		GetClassName(hwnd, strWinName, sizeof(strWinName)-1);
		if(StrCmp(strWinName, TEXT("TThreadWindow")) == 0)
			return(TRUE);

		cbIo->topWindow = hwnd;

		// Modify the window
		testPoint.x = GetSystemMetrics(SM_CXSCREEN) - 50;
		testPoint.y = 50;

		isVisible = FALSE;
		winThread = GetWindowThreadProcessId(WindowFromPoint(testPoint), &winProcess);
		if(winProcess == cbIo->processId)
			isVisible = TRUE;

		if(isVisible && cbIo->alreadyRunning)
		{
			CloseWindow(cbIo->topWindow);
			SetForegroundWindow(WindowFromPoint(testPoint));
			cbIo->success++;
		}
		else
		{
			if(cbIo->setSize)
			{
				/* Fill Windowplacement vars */
				memset(&winPlacement, 0, sizeof(WINDOWPLACEMENT));
				winPlacement.length = sizeof(WINDOWPLACEMENT);
				winPlacement.flags = WPF_SETMINPOSITION | WPF_RESTORETOMAXIMIZED;
				if(cbIo->minimize)
					winPlacement.showCmd = SW_SHOWMINNOACTIVE;
				else
					winPlacement.showCmd = SW_SHOWMAXIMIZED;
				winPlacement.ptMinPosition.y = GetSystemMetrics(SM_CYSCREEN) - 1;
				SetWindowPlacement(cbIo->topWindow, &winPlacement);

				newStyle = GetWindowLong(cbIo->topWindow, GWL_STYLE);
				newStyle &= ~(WS_CAPTION | WS_THICKFRAME);
				newStyle &= ~(WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
				SetWindowLongPtr(cbIo->topWindow, GWL_STYLE, newStyle);
				SetWindowLongPtr(cbIo->topWindow, GWL_EXSTYLE, WS_EX_APPWINDOW);
				SetWindowPos(cbIo->topWindow, HWND_NOTOPMOST, 0, 0, 
							GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_NOACTIVATE | SWP_FRAMECHANGED);

				newStyle = GetWindowLong(cbIo->topWindow, GWL_STYLE);
				if(!newStyle)
				{
					cbIo->success = 0;
					return(FALSE);
				}

				if(cbIo->minimize)
					ShowWindow(cbIo->topWindow, SW_SHOWMINNOACTIVE);
				else
				{
					ShowWindow(cbIo->topWindow, SW_SHOWMAXIMIZED);

					SetForegroundWindow(cbIo->topWindow);
					SetFocus(cbIo->topWindow);
					BringWindowToTop(cbIo->topWindow);
				}
			}
			else
			{
				ShowWindow(cbIo->topWindow, SW_SHOWMAXIMIZED);

				SetForegroundWindow(cbIo->topWindow);
				SetFocus(cbIo->topWindow);
				BringWindowToTop(cbIo->topWindow);
			}
		}
		cbIo->success++;
	}
	return(TRUE);
}


void KillApp(LPTSTR lpCmdLine)
{
	DWORD               registryResult, idSize, regValueType;
	HKEY                regKeyHandle, processRegHandle, regAppKeyHandle;
	LONG                retVal;
	EnumCallbackData	cbIo;
	union
	{
		DWORD	dwid;
		BYTE	bid[4];
	} process;

	/* Read the registry stuff */
	retVal = RegCreateKeyEx(HKEY_CURRENT_USER,		//HKEY hKey,                        // handle to open key
							HKEY_WHITEBREAM,		//LPCTSTR lpSubKey,                 // subkey name
							0,						//DWORD Reserved,                   // reserved
							TEXT(""),				//LPTSTR lpClass,                   // class string
							REG_OPTION_NON_VOLATILE,//DWORD dwOptions,                  // special options
							KEY_ALL_ACCESS,			//REGSAM samDesired,                // desired security access
							NULL,					//LPSECURITY_ATTRIBUTES lpSecAttr,	// inheritance
							&regKeyHandle,			//PHKEY phkResult,                  // key handle 
							&registryResult);		//LPDWORD lpdwDisposition           // disposition value buffer

	retVal = RegCreateKeyEx(regKeyHandle,			//HKEY hKey,                        // handle to open key
							HKEY_PROCESS,			//LPCTSTR lpSubKey,                 // subkey name
							0,						//DWORD Reserved,                   // reserved
							TEXT(""),				//LPTSTR lpClass,                   // class string
							REG_OPTION_NON_VOLATILE,//DWORD dwOptions,                  // special options
							KEY_ALL_ACCESS,			//REGSAM samDesired,                // desired security access
							NULL,					//LPSECURITY_ATTRIBUTES lpSecAttr,	// inheritance
							&processRegHandle,		//PHKEY phkResult,                  // key handle 
							&registryResult);		//LPDWORD lpdwDisposition           // disposition value buffer

	if(registryResult == REG_CREATED_NEW_KEY)
	{
		RegSetValueEx(processRegHandle,				//HKEY hKey,            // handle to key
					  NULL,							//LPCTSTR lpValueName,  // value name
					  0,							//DWORD Reserved,       // reserved
					  REG_SZ,						//DWORD lpType,         // type buffer
		(CONST BYTE *)PROCESS_INFOSTR,				//CONST BYTE * lpData,  // data buffer
					  sizeof(PROCESS_INFOSTR)-1);	//DWORD lpcbData        // size of data buffer
	}

	retVal = RegCreateKeyEx(processRegHandle,		//HKEY hKey,                        // handle to open key
							lpCmdLine,				//LPCTSTR lpSubKey,                 // subkey name
							0,						//DWORD Reserved,                   // reserved
							TEXT(""),				//LPTSTR lpClass,                   // class string
							REG_OPTION_VOLATILE,	//DWORD dwOptions,                  // special options
							KEY_ALL_ACCESS,			//REGSAM samDesired,                // desired security access
							NULL,					//LPSECURITY_ATTRIBUTES lpSecAttr,	// inheritance
							&regAppKeyHandle,		//PHKEY phkResult,                  // key handle 
							&registryResult);		//LPDWORD lpdwDisposition           // disposition value buffer

	idSize = sizeof(process);
	regValueType = REG_DWORD;
	process.dwid = 0;

	retVal = RegQueryValueEx(regAppKeyHandle,		//HKEY hKey,            // handle to key
							HKEY_PROCESS_ID,		//LPCTSTR lpValueName,  // value name
							NULL,					//LPDWORD lpReserved,   // reserved
							&regValueType,			//LPDWORD lpType,       // type buffer
							process.bid,			//LPBYTE lpData,        // data buffer
							&idSize);				//LPDWORD lpcbData      // size of data buffer

	if((retVal == ERROR_SUCCESS) && (registryResult == REG_OPENED_EXISTING_KEY) && ((process.dwid != 0)))
	{
		cbIo.processId = process.dwid;
		cbIo.topWindow = 0;
		cbIo.alreadyRunning = TRUE;
		cbIo.setSize = FALSE;
		cbIo.minimize = FALSE;

		EnumWindows(EnumWindowsProc, (LPARAM)&cbIo);
		/* Now the window is found or all windows checked */

		if(cbIo.topWindow)
			SendMessage(cbIo.topWindow, WM_CLOSE, 0, 0);
	}

	RegCloseKey(processRegHandle);
	RegCloseKey(regKeyHandle);
	RegCloseKey(regAppKeyHandle);
}



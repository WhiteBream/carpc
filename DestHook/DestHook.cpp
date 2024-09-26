// DestHook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "DestHook.h"


HHOOK hDestHook;
DWORD dwDestProcess;
DWORD dwDestThread;


BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			hDestHook = (HHOOK)hModule;
			dwDestProcess = 0;
			dwDestThread = 0;
			break;

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			if(hDestHook)
				UnhookWindowsHookEx(hDestHook);
			hDestHook = NULL;
			break;
	}
	return TRUE;
}


DESTHOOK_API CDestSetHook(HHOOK hHook)
{
}

DESTHOOK_API CDestSetIds(DWORD dwThread, DWORD dwProcess)
{
	dwDestProcess = dwProcess;
	dwDestThread = dwThread;
}

DESTHOOK_API LRESULT CALLBACK CDestProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	DWORD nProcess, nThread;
	LONG  newStyle;
	char  szClass[15] = {0};
	RECT  wDims;

	if (nCode < 0)  // do not process message
		return CallNextHookEx(hDestHook, nCode, wParam, lParam);

	if(nCode == HCBT_ACTIVATE)
	{
		nThread = GetWindowThreadProcessId((HWND)wParam, &nProcess);
		if((nThread == dwDestThread) || (nProcess == dwDestProcess))
		{
			// Prevent some intermediate window from being blown
			GetClassName((HWND)wParam, szClass, sizeof(szClass) - 1);
			if(strcmp(szClass, "TThreadWindow") == 0) return(0);

			// Show a debug dialog, but prevent it from being changed
			//GetWindowText((HWND)wParam, szClass, sizeof(szClass) - 1);
			//if(strcmp(szClass, "HCBT_ACTIVATE") == 0) return(0);
			//MessageBox(NULL, "All right", "HCBT_ACTIVATE", MB_OK);

			// Get the current dimensions
			GetWindowRect((HWND)wParam, &wDims);
			wDims.bottom -= wDims.top;
			wDims.right -= wDims.left;

			// Clear some extended window styles
			newStyle = GetWindowLong((HWND)wParam, GWL_EXSTYLE);
			if(newStyle)
			{
				newStyle &= ~(WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME);
				SetWindowLong((HWND)wParam, GWL_EXSTYLE, newStyle);
			}

			// Clear the normal window styles
			newStyle = GetWindowLong((HWND)wParam, GWL_STYLE);
			if(newStyle)
			{
				newStyle &= ~(WS_CAPTION | WS_BORDER | WS_THICKFRAME |
							WS_OVERLAPPED | WS_TILED | WS_DLGFRAME |
							DS_3DLOOK | DS_MODALFRAME | WS_POPUP | WS_SYSMENU);
				SetWindowLong((HWND)wParam, GWL_STYLE, newStyle);
			}

			// Clear the normal window styles
			newStyle = GetWindowLong((HWND)wParam, GWL_STYLE);
			if(newStyle)
			{
				newStyle &= ~(WS_OVERLAPPED);
				SetWindowLong((HWND)wParam, GWL_STYLE, newStyle);
			}
			return(0L);
		}
	}
	return(CallNextHookEx(hDestHook, nCode, wParam, lParam));
}

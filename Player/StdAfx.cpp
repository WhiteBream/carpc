// stdafx.cpp : source file that includes just the standard includes
//	player.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

void StripA(LPSTR lpString)
{
	LPSTR  pTmp;
	size_t vLen;

	while(*lpString == ' ')
	{
		pTmp = lpString;
		for(vLen = strlen(lpString); vLen; vLen--)
            *pTmp++ = *(pTmp + 1);
	}

	pTmp = &lpString[strlen(lpString)];
	while((*pTmp == '\0') || (*pTmp == ' ')) 
	{
		*pTmp = '\0';
		if(pTmp == lpString) break;
		pTmp--;
	}
}

void StripW(LPWSTR lpString)
{
	LPWSTR  pTmp;
	size_t vLen;

	while(*lpString == L' ')
	{
		pTmp = lpString;
		for(vLen = wcslen(lpString); vLen; vLen--)
            *pTmp++ = *(pTmp + 1);
	}

	pTmp = &lpString[wcslen(lpString)];
	while((*pTmp == L'\00') || (*pTmp == L' ')) 
	{
		*pTmp = L'\00';
		if(pTmp == lpString) break;
		pTmp--;
	}
}

void AssignIcon(HINSTANCE hInstance, HWND hWnd, int vControl, int vIcon)
{
	HICON ico;

	if(vIcon)
	{
		ico = LoadIcon(hInstance, (LPCTSTR)vIcon);
		SendDlgItemMessage(hWnd, vControl, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
		DestroyIcon(ico);
	}
	else
		SendDlgItemMessage(hWnd, vControl, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)NULL);
}

#ifdef LOCALPIPEIO
const LPTSTR lpszPipeName = TEXT("\\\\.\\pipe\\whitebreamsysaccess");
#define PIPE_WAIT_TIME		250
BOOL PipeIo(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize)
{
	BOOL	vResult;
	DWORD	byteCnt;
	CHAR	vRetry = 0;
	SECURITY_ATTRIBUTES	sa;
	HANDLE				hMutex;

	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR)GlobalAlloc(GPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
	SetSecurity(&sa);

	do
	{
		hMutex = CreateMutex(&sa, FALSE, lpszMutexName);
		if(hMutex)
		{
			vResult = WaitForSingleObject(hMutex, MUTEX_WAIT_TIME);
			if(vResult != WAIT_TIMEOUT)
			{
				vResult = CallNamedPipe(lpszPipeName, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, &byteCnt, PIPE_WAIT_TIME);
				if(!vResult)
				{
					if(GetLastError() == ERROR_TOO_MANY_OPEN_FILES)
					{
						HANDLE hWaitEvent;
						hWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
						if(hWaitEvent)
						{
							WaitForSingleObject(hWaitEvent, 1);
							CloseHandle(hWaitEvent);
						}
					}
					if(vRetry++ == 25)
					{
						ReleaseMutex(hMutex);
						CloseHandle(hMutex);
						GlobalFree(sa.lpSecurityDescriptor);
						return(vResult);
					}
				}
				else if((((LPSTR)lpOutBuffer)[0] == 'E') && (((LPSTR)lpOutBuffer)[1] == 'R') && (((LPSTR)lpOutBuffer)[2] == 'R'))
				{
					vResult = FALSE;
					if(vRetry++ == 5)
					{
						ReleaseMutex(hMutex);
						CloseHandle(hMutex);
						GlobalFree(sa.lpSecurityDescriptor);
						return(TRUE);//vResult);
					}
				}
			}
			ReleaseMutex(hMutex);
			CloseHandle(hMutex);
		}
		else
			break;
	} while(!vResult);
	GlobalFree(sa.lpSecurityDescriptor);
	return(vResult);
}
#endif //LOCALPIPEIO


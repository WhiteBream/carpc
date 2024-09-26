// stdafx.cpp : source file that includes just the standard includes
//	player.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

void Strip(LPSTR lpString)
{
	LPSTR pTmp = &lpString[strlen(lpString)];
	while((*pTmp == '\0') || (*pTmp == ' ')) 
	{
		*pTmp = '\0';
		if(pTmp == lpString) break;
		pTmp--;
	}
}

void AssignIcon(HINSTANCE hInstance, HWND hWnd, int vControl, int vIcon)
{
	HICON ico;

	ico = LoadIcon(hInstance, (LPCTSTR)vIcon);
	SendDlgItemMessage(hWnd, vControl, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
	DestroyIcon(ico);
}

const LPTSTR lpszPipeName = TEXT("\\\\.\\pipe\\whitebreamsysaccess");
const LPTSTR lpszPipeMutex = TEXT("Global\\whitebreamsysaccesswait");
#define PIPE_WAIT_TIME		100
#define MUTEX_WAIT_TIME		250

BOOL PipeIo(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize)
{
	BOOL	vResult;
	DWORD	byteCnt;
	CHAR	vRetry = 0;
	HANDLE	hMutex;

	do
	{
		hMutex = CreateMutex(NULL, FALSE, lpszPipeMutex);
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
	return(vResult);
}

void 
restore_data(LPSTR pData, CHAR vCount)
{
	CHAR i;

	if(vCount > 8)
		printf("restore_data handles max 8 chars");

	for(i = 0; i < vCount; i++)
	{
		if((pData[i] == (CHAR)0xFF) && (pData[vCount] & (0x01 << i)))
			pData[i] = 0;
	}
	pData[vCount] = 0;
}


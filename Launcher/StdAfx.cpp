// stdafx.cpp : source file that includes just the standard includes
//	startbtn1.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

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

PCHAR* CommandLineToArgvA(PCHAR CmdLine, int* _argc)
{
	PCHAR* argv;
	PCHAR  _argv;
	ULONG   len;
	ULONG   argc;
	CHAR   a;
	ULONG   i, j;

	BOOLEAN  in_QM;
	BOOLEAN  in_TEXT;
	BOOLEAN  in_SPACE;

	len = strlen(CmdLine);
	i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

	argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
		i + (len+2)*sizeof(CHAR));

	_argv = (PCHAR)(((PUCHAR)argv)+i);

	argc = 0;
	argv[argc] = _argv;
	in_QM = FALSE;
	in_TEXT = FALSE;
	in_SPACE = TRUE;
	i = 0;
	j = 0;

	while(a = CmdLine[i])
	{
		if(in_QM)
		{
            if(a == '\"')
                in_QM = FALSE;
            else
			{
                _argv[j] = a;
                j++;
            }
        }
		else
		{
            switch(a)
			{
				case '\"':
					in_QM = TRUE;
					in_TEXT = TRUE;
					if(in_SPACE)
					{
						argv[argc] = _argv+j;
						argc++;
					}
					in_SPACE = FALSE;
					break;
				case ' ':
				case '\t':
				case '\n':
				case '\r':
					if(in_TEXT)
					{
						_argv[j] = '\0';
						j++;
					}
					in_TEXT = FALSE;
					in_SPACE = TRUE;
					break;
				default:
					in_TEXT = TRUE;
					if(in_SPACE)
					{
						argv[argc] = _argv+j;
						argc++;
					}
					_argv[j] = a;
					j++;
					in_SPACE = FALSE;
					break;
            }
        }
        i++;
	}
	_argv[j] = '\0';
	argv[argc] = NULL;

	(*_argc) = argc;
	return(argv);
}


#ifdef LOCALPIPEIO
const LPTSTR lpszPipeName = TEXT("\\\\.\\pipe\\whitebreamsysaccess");
#define PIPE_WAIT_TIME		100
BOOL PipeIo(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize)
{
	BOOL	vResult;
	DWORD	byteCnt;
	CHAR	vRetry = 0;

	memset(lpOutBuffer, 0, nOutBufferSize);
	do
	{
		vResult = CallNamedPipe(lpszPipeName, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, &byteCnt, PIPE_WAIT_TIME);
		if(!vResult)
		{
			byteCnt = GetLastError();
			if(byteCnt == ERROR_TOO_MANY_OPEN_FILES)
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
				return(vResult);
		}
	} while(!vResult);
	return(vResult);
}
#endif
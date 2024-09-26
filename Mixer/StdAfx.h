// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__C6DCE3F2_FA9B_48AF_AD0C_39BCC4FDFF4B__INCLUDED_)
#define AFX_STDAFX_H__C6DCE3F2_FA9B_48AF_AD0C_39BCC4FDFF4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0501
#define UNICODE
#define _UNICODE

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>

#include <tchar.h>
#include <shlwapi.h>
#include <strsafe.h>

#pragma comment(lib, "shlwapi.lib")
#include <commctrl.h>

// Local Header Files
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

// TODO: reference additional headers your program requires here
#include "resource.h"
#include "..\Destinator\Destinator.h"
#pragma comment(lib, "..\\Destinator\\Release\\Destinator.lib")

#define WM_SETVIDEOWINDOW		(WM_USER + 1)
#define WM_SETREGISTERFINALWND		(WM_USER + 2)
#define WM_PAINTVIDEOWINDOW		(WM_USER + 3)
#define WM_REALIGNVIDEOWINDOW	(WM_USER + 4)
#define WM_SHOWPLAYERWINDOW		(WM_USER + 5)
#define WM_SHOWVIDEOWINDOW		(WM_USER + 6)
#define WM_DIRECTSHOW			(WM_USER + 7)
#define WM_DIRECTSHOWMUTE		(WM_USER + 8)
#define WM_FINDFIRSTFILE		(WM_USER + 9)
#define WM_FINDNEXTFILE			(WM_USER + 10)
#define WM_SAVEDATA				(WM_USER + 11)
#define WM_RECEIVESTRING		(WM_USER + 12)	/* wParam=size(INT),lParam=globalheap(LONG_PTR) */
#define WM_PLAYNOW				(WM_USER + 100)
#define WM_PLAYNEXT				(WM_USER + 101)
#define WM_PLAYQUEUE			(WM_USER + 102)

#define WBBUTTON	TEXT("WB_DESTBUTTON")

#undef LOWORD
#define LOWORD(l)		((signed short)((INT)(l) & 0xffff))
#undef HIWORD
#define HIWORD(l)		((INT)(l) >> 16)

#include "..\..\..\C401\Drivers\Cargo\MixerDll\MixerDll.h"

typedef struct _AppInfo
{
	HINSTANCE	hInst;
	HWND		hApp;
	INT			vOffs;
	HFNSET		hSetValue;
} AppInfo, *PAppInfo;

extern AppInfo sAi;

void Strip(LPSTR lpString);
void AssignIcon(HINSTANCE hInstance, HWND hWnd, int vControl, int vIcon);

BOOL PipeIo(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize);
void restore_data(LPSTR pData, CHAR vCount);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C6DCE3F2_FA9B_48AF_AD0C_39BCC4FDFF4B__INCLUDED_)

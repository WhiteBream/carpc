// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_4445535400001__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_4445535400001__INCLUDED_

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

#pragma warning(disable : 4995)

// Local Header Files
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

// TODO: reference additional headers your program requires here
#include "resource.h"
#include "..\Destinator\Destinator.h"
#pragma comment(lib, "..\\Destinator\\Release\\Destinator.lib")

#include "..\..\..\C401\Drivers\Cargo\ServiceLib\ServiceLib.h"
//#ifdef _DEBUG
//#pragma comment(lib, "..\\..\\..\\C401\\Drivers\\Cargo\\ServiceLib\\Debug\\ServiceLib.lib")
//#else
#pragma comment(lib, "..\\..\\..\\C401\\Drivers\\Cargo\\ServiceLib\\Release\\ServiceLib.lib")
//#endif

//#define TDA7416SOURCE

#define WM_SETVIDEOWINDOW		(WM_USER + 1)
#define WM_SETREGISTERFINALWND	(WM_USER + 2)
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
#define WM_GARBAGECOLLECTION	(WM_USER + 13)
#define WM_MUTERADIO			(WM_USER + 14)
#define WM_REMOVEDELETE			(WM_USER + 15)
#define WM_SEARCHPATH			(WM_USER + 16)
#define WM_SEARCHCOUNT			(WM_USER + 17)
#define WM_PAINTOWNERDRAWN		(WM_USER + 18)
#define WM_GETREGISTERFINALWND	(WM_USER + 19)
#define WM_INITDONE				(WM_USER + 20)
#define WM_SAVESEARCHQUEUE		(WM_USER + 21)
#define WM_SAVESEARCHPATH		(WM_USER + 22)
#define WM_RADIOTA				(WM_USER + 23)
#define WM_PAUSE				(WM_USER + 24)
#define WM_DSHOWTIME			(WM_USER + 25)
#define WM_DSHOWPOS				(WM_USER + 26)
#define WM_SETPATH				(WM_USER + 27)
#define WM_FINDNEXTVOLUME		(WM_USER + 28)
#define WM_MMBUTTON				(WM_USER + 29)
#define WM_RESETTIMER			(WM_USER + 30)
#define WM_UPDATERDS			(WM_USER + 31)
#define WM_RESETRDS				(WM_USER + 32)

#define WM_PLAYNOW				(WM_USER + 100)
#define WM_PLAYNEXT				(WM_USER + 101)
#define WM_PLAYQUEUE			(WM_USER + 102)

#include "Database.h"

typedef struct _AppInfo
{
	Database	*hDb;
	HINSTANCE	hInst;
	HWND		hApp;
	INT			vOffs;
	BOOL		vTA;
} AppInfo, *PAppInfo;

extern AppInfo sAi;

void StripA(LPSTR lpString);
void StripW(LPWSTR lpString);
void AssignIcon(HINSTANCE hInstance, HWND hWnd, int vControl, int vIcon);

#ifdef UNICODE
#define Strip	StripW
#else
#define Strip	StripA
#endif

//BOOL PipeIo(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_4445535400001__INCLUDED_)

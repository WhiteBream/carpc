// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_IMAGER_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_IMAGER_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0500
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

#include "ConfigDb.h"

// TODO: reference additional headers your program requires here
#include "resource.h"
#include "..\Destinator\Destinator.h"
#pragma comment(lib, "..\\Destinator\\Debug\\Destinator.lib")

#define WM_REGISTERFINALWND		(WM_USER + 2)
#define WM_FINDFIRSTFILE		(WM_USER + 8)
#define WM_FINDNEXTFILE			(WM_USER + 9)
#define WM_SAVEDATA				(WM_USER + 11)
#define WM_RECEIVESTRING		(WM_USER + 12)
#define WM_SETPATH				(WM_USER + 27)
#define WM_FINDNEXTVOLUME		(WM_USER + 28)

void StripA(LPSTR lpString);
void StripW(LPWSTR lpString);
void AssignIcon(HINSTANCE hInstance, HWND hWnd, int vControl, int vIcon);

#ifdef UNICODE
#define Strip	StripW
#else
#define Strip	StripA
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGER_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)

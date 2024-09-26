// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0500
#define UNICODE

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <wchar.h>
#include <stdio.h>

// Local Header Files
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

#include "resource.h"
#include "..\Destinator\Destinator.h"
#pragma comment(lib, "..\\Destinator\\Debug\\Destinator.lib")

// TODO: reference additional headers your program requires here
#include "resource.h"


#define WM_SETVIDEOWINDOW		(WM_USER + 1)
#define WM_REGISTERFINALWND		(WM_USER + 2)
#define WM_PAINTVIDEOWINDOW		(WM_USER + 3)
#define WM_REALIGNVIDEOWINDOW	(WM_USER + 4)
#define WM_SHOWVIDEOWINDOW		(WM_USER + 5)
#define WM_DIRECTSHOW			(WM_USER + 6)
#define WM_DIRECTSHOWMUTE		(WM_USER + 7)
#define WM_FINDFIRSTFILE		(WM_USER + 8)
#define WM_FINDNEXTFILE			(WM_USER + 9)
#define WM_SAVEDATA				(WM_USER + 11)
#define WM_PLAYNOW				(WM_USER + 100)
#define WM_PLAYNEXT				(WM_USER + 101)
#define WM_PLAYQUEUE			(WM_USER + 102)

//#include "Database.h"

void Strip(LPSTR lpString);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__CE96BCF6_0AFB_4D7C_AD59_A770C02A2B7D__INCLUDED_)
#define AFX_STDAFX_H__CE96BCF6_0AFB_4D7C_AD59_A770C02A2B7D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0500
#define UNICODE

// Windows Header Files:
#include <windows.h>
#include <commctrl.h>

#include <tchar.h>
#include <shlwapi.h>
#include <strsafe.h>

// Local Header Files
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

#include "resource2.h"
#include "destinator.h"

#define SRC_SIZE	36
#define HKEY_WHITEBREAM		TEXT("SOFTWARE\\White Bream\\CarPC\\Buttons")
#define PROCESS_INFOSTR		TEXT("Contains alternative bitmaps for DestButtons")

// The brush colors
#define BR_WHITE		0
#define BR_BLUE			1

#define WM_PAINTOWNERDRAWN		(WM_USER + 18)

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__CE96BCF6_0AFB_4D7C_AD59_A770C02A2B7D__INCLUDED_)

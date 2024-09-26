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
#define _UNICODE

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

#include <tchar.h>
#include <shlwapi.h>
#include <strsafe.h>

// Local Header Files
#include <gdiplus.h>
using namespace Gdiplus; 
#pragma comment(lib, "gdiplus.lib")

#include "ConfigDb.h"

// TODO: reference additional headers your program requires here
#include "resource.h"
#include "..\Destinator\Destinator.h"
#pragma comment(lib, "..\\Destinator\\Release\\Destinator.lib")

#include "..\..\..\C401\Drivers\Cargo\ServiceLib\ServiceLib.h"
#ifdef _DEBUG
#pragma comment(lib, "..\\..\\..\\C401\\Drivers\\Cargo\\ServiceLib\\Debug\\ServiceLib.lib")
#else
#pragma comment(lib, "..\\..\\..\\C401\\Drivers\\Cargo\\ServiceLib\\Release\\ServiceLib.lib")
#endif

void AssignIcon(HINSTANCE hInstance, HWND hWnd, int vControl, int vIcon);
PCHAR* CommandLineToArgvA(PCHAR CmdLine, int* _argc);

#ifdef UNICODE
#define CommandLineToArgv	CommandLineToArgvW
#else
#define CommandLineToArgv	CommandLineToArgvA
#endif

//BOOL PipeIo(LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)

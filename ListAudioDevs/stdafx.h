// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0501
//#define UNICODE
//#define _UNICODE

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

// TODO: reference additional headers your program requires here

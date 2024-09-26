// Destinator.h: interface for the Destinator library.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DESTINATOR_H__8F526B1F_CF81_469D_A9B7_DAC37B22DDAF__INCLUDED_)
#define AFX_DESTINATOR_H__8F526B1F_CF81_469D_A9B7_DAC37B22DDAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

int DestinatorInit(HINSTANCE hInstance);
int DestinatorFree(HINSTANCE hInstance);

#define WBBUTTON	TEXT("WB_DESTBUTTON")
#define WBVSCROLL	TEXT("WB_DESTSCROLL")
#define WBHSCROLL	TEXT("WB_DESTHSCROLL")
#define WBFADER		TEXT("WB_DESTFADER")


// Message for alignment
#define WM_USERALIGN	(WM_USER + 1001)
#define WM_SETOSKTARGET	(WM_USER + 1002)
#define WM_SETREPEAT	(WM_USER + 1003)

#define WB_GETTEXTSIZE	(WM_USER + 1004)
#define WB_SETTEXTSIZE	(WM_USER + 1005)
#define WB_SETTICKCOUNT	(WM_USER + 1006)

#define WB_SETALPHA		(WM_USER + 1007)

#define ALIGNLEFT		0
#define ALIGNCENTER		1
#define ALIGNRIGHT		2

#endif // !defined(AFX_DESTINATOR_H__8F526B1F_CF81_469D_A9B7_DAC37B22DDAF__INCLUDED_)

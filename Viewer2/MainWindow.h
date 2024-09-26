// MainWindow.h: interface for the MainWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINWINDOW_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_MAINWINDOW_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Destinator\Destinator.h"

class MainWindow
{
private:
	Graphics	*myGraphics;
	Brush		*bkGnd1, *bkGnd2;
	ULONG_PTR	gdiplusToken;
	RECT		ourRect;

	ATOM MyRegisterClass(HINSTANCE hInstance);

public:
	MainWindow(HINSTANCE hInstance, HWND hParent);
	virtual ~MainWindow();

	void InitDC(BOOL updateDC);
	void Paint();

	HINSTANCE  hInst;
	HWND       hWnd, hCbWnd;
	int        offs;
	RECT       startRect;
    INT			curtop;
	INT			visibleCount;
	INT			range;
};


#endif // !defined(AFX_MAINWINDOW_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

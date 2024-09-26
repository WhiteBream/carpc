// MainWindow.h: interface for the MainWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINWINDOW_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_MAINWINDOW_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Destinator\Destinator.h"

#include "Call.h"

class MainWindow
{
private:
	Graphics   * myGraphics;
	Brush      * bkGnd;
	ULONG_PTR    gdiplusToken;
	int          lastclicked;
	RECT         ourRect;

	ATOM MyRegisterClass(HINSTANCE hInstance);

public:
	MainWindow(HINSTANCE hInstance, HWND hParent);
	virtual ~MainWindow();

	void InitDC(BOOL updateDC);
	void GdiInit(HINSTANCE hInstance, HWND hSrc);
	void Paint();
	void Client(int n);

	HINSTANCE  hInst;
	HWND       hPlay, hCbWnd;
	int        current;
	int        offs;
	RECT       startRect;

	Call	 * callWin;
};


#endif // !defined(AFX_MAINWINDOW_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

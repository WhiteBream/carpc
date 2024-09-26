// MainWindow.h: interface for the MainWindow class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "StdAfx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define ALPHA	150
#define ALPHA	255
#include "..\Destinator\Destinator.h"

#include "Client.h"
//#include "MixWin.h"
//#include "Radio.h"
//#include "Playlist.h"
//#include "Sorted.h"
//#include "MediaDB.h"
//#include "Video.h"
//#include "Settings.h"


typedef struct _Clients
{
	Rect	bRect;
	HWND	hWnd, hCbWnd;
	BOOL	enabled;
	INT		vControl, vIcon;
	Client	*pClient;
} Clients;



class MainWindow
{
private:
	Graphics	*myGraphics;
	Brush		*bkGnd;
	Bitmap		*myBackground;
	ULONG_PTR	gdiplusToken;
	int			lastclicked;
	HWND		hWnd;
	RECT		ourRect;
	HMODULE		hMixer;

	ATOM MyRegisterClass(HINSTANCE hInstance);

public:
	MainWindow(HINSTANCE hInstance, HWND hParent);
	virtual ~MainWindow();

	void InitDC(BOOL updateDC);
	void GdiInit(HINSTANCE hInstance, HWND hSrc);
	void Paint();
	void ShowClient(int n);
	void SendClientMessage(UINT vMsg, WPARAM wParam, LPARAM lParam);

	PAppInfo	pAi;
	HWND		hCbWnd;
	int			current;
	RECT		startRect;
	Clients     clients[IDW_max+1];

//	MixWin		*mixWin;
//	Radio    * radioWin;
//	Playlist * listWin;
//	Sorted   * sortWin;
//	MediaDB  * mediaDB;
//	Video    * video;
//	Settings * settWin;
};


#endif // MAINWINDOW_H

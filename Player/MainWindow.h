// MainWindow.h: interface for the MainWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINWINDOW_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_MAINWINDOW_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Destinator\Destinator.h"
#include "Database.h"

#include "Client.h"
#include "PlayWin.h"
#include "Radio.h"
#include "Playlist.h"
#include "Sorted.h"
#include "MediaDB.h"
#include "Video.h"
#include "Settings.h"

typedef enum _RUNMODE
{
	MODE_MP3,
	MODE_AMFM,
	MODE_CD,
	MODE_DVD
} RUNMODE;

class MainWindow
{
private:
	Graphics	*myGraphics;
	Brush		*bkGnd;
	Font		*font;
	Brush		*yellow;
	ULONG_PTR	gdiplusToken;
	int			lastclicked;
	HWND		hMain;
	RECT		ourRect;

	ATOM MyRegisterClass(HINSTANCE hInstance);

public:
	MainWindow(HINSTANCE hInstance, HWND hParent);
	virtual ~MainWindow();

	void InitDC(BOOL updateDC);
	void GdiInit(HINSTANCE hInstance, HWND hSrc);
	void Paint();
	void Client(UINT n);
	void SaveMode(void);
	void Source(BOOL vTuner);

	HWND		hCbWnd;
	UINT		current;
	RECT		startRect;
	RUNMODE		vRunMode;
	HMODULE		hAppKeys;
	HMODULE		hMixer;
	FARPROC		hSetValue;

	UINT		vTaIntercept;

	PlayWin		*playWin;
	Radio		*radioWin;
	Playlist	*listWin;
	Sorted		*sortWin;
	MediaDB		*mediaDB;
	Video		*video;
	Settings	*settWin;
};


#endif // !defined(AFX_MAINWINDOW_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

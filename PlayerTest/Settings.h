// Settings.h: interface for the Settings class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SETTINGS_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_SETTINGS_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Search.h"

class Settings
{
private:
	Graphics		*myGraphics;
	Brush			*bkGnd;
	RECT			ourRect;
	Brush			*br_black;
	Brush			*br_white;
	Pen				*pn;
	Font			*fn;
	RectF			*statusRect;
	StringFormat	*statusFormat;
	HBITMAP			hBmp;
	HDC				hDc;

	ATOM MyRegisterClass(void);

public:
	Settings(PAppInfo pAppInfo);
	virtual ~Settings();

	HWND		hSett, hCbWnd;
    BOOL		fixedDrive;
	Search		*cSearch;
	PAppInfo	pAi;
	Rect        vHelpRect;
	RectF		*vHelpTextRect;
	LPCTSTR		vHelpText;

	virtual Show(BOOL yesNo);
	void InitDC(BOOL updateDC);
    void UpdateCurrentDirectory(LPCTSTR lpDir);
	void UpdateFindCounter(void);
};


#endif // !defined(AFX_SETTINGS_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

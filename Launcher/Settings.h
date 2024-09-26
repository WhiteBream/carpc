// Settings.h: interface for the Settings class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SETTINGS_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_SETTINGS_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
	Settings(HINSTANCE hInstance, HWND hParent);
	virtual ~Settings();

	HWND		hSett, hCbWnd;
	HWND		parent;
	HINSTANCE	instance;
	INT			offset;

	ConfigDb	*dBase;
	Rect        vHelpRect;
	RectF		*vHelpTextRect;
	LPCWSTR		vHelpText;
	RectF		*vPsuStatusRect;
	WCHAR		vPsuStatus[64];

	virtual Show(BOOL yesNo);
	void InitDC(BOOL updateDC);
	void Infobox(void);
};


#endif // !defined(AFX_SETTINGS_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

// StatusBar.h: interface for the StatusBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STATUSBAR_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_STATUSBAR_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class StatusBar  
{
private:
	HINSTANCE	hInst;
	RECT		sRect;
	Graphics	*gr;
	Image		*image;
	Graphics	*graph;
	Color		*bkGnd;
	SolidBrush	*solidWhite;
	SolidBrush	*solidGrey;
	SolidBrush	*solidYellow;
	Font		*font;
	HDC			hDc;

	ATOM MyRegisterClass(HINSTANCE hInstance);

public:
	StatusBar(HINSTANCE hInstance, HWND hParent);
	virtual ~StatusBar();
	void Paint();
	void UpdateGdi(void);

	HWND		hBar;
};


#endif // !defined(AFX_STATUSBAR_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

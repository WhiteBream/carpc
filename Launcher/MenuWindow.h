// MenuWindow.h: interface for the MenuWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUWINDOW_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_MENUWINDOW_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class MenuWindow  
{
private:
	HINSTANCE    hInst;
	int xn, yn, lastpressed;
	int width, height;

	ATOM MyRegisterClass(HINSTANCE hInstance);

    PointF pStart;

public:
	MenuWindow(HINSTANCE hInstance, HWND hParent);
	virtual ~MenuWindow();

	HWND         hMenu;
	HWND         hParentWin;

	void Show();
	void DrawButtons();
};


#endif // !defined(AFX_MENUWINDOW_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

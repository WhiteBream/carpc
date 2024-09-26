// Popup.h: interface for the Popup class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_Popup_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_Popup_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Popup
{
private:
	Graphics	*myGraphics;
	Brush		*bg[4];
	Pen			*pn;
	RECT		ourRect;
	Font		*fn[2];

	ATOM MyRegisterClass(HINSTANCE hInstance);

public:
	Popup(HINSTANCE hInstance, HWND hParent, int bottomOffset, HWND * pWnd, Database * dBase);
	virtual ~Popup();

	POINT		vPos;
	int			offset;
	HINSTANCE	hInst;
	HWND		hWnd, hCbWnd, hPrnt;
	int			vProgress;
	BOOL		vChanging;
	BOOL        vMoving;
	POINTS      vMoveStart;
	Database	*dB;

	void Show(BOOL yesNo);
	void InitDC(BOOL updateDC);
	void Paint();
	BOOL Progress();
};


#endif // !defined(AFX_Popup_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

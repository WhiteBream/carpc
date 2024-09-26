// Edit.h: interface for the MediaDB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDIT_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_EDIT_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Edit
{
private:
	Graphics   * myGraphics;
	Brush      * bkGnd;
	Pen        * pn;
	RECT         ourRect;
	Font       * font;

	ATOM MyRegisterClass(void);

public:
	Edit(PAppInfo pAppInfo);
	virtual ~Edit();

	HWND       hEdit, hCbWnd;
	BOOL       vOsk;
	PAppInfo   pAi;

	void InitDC(BOOL updateDC);
	void Paint();
	virtual Show(BOOL yesNo);
};


#endif // !defined(AFX_EDIT_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

// Desktop.h: interface for the Desktop class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DESKTOP_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_DESKTOP_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Desktop
{
private:
	HINSTANCE	hInst;
	HDC			hDc;

	ATOM MyRegisterClass(HINSTANCE hInstance);

public:
	Desktop(HINSTANCE hInstance, HWND hParent);
	virtual ~Desktop();
	void UpdateGdi(void);

	Graphics	*myGraphics;
	Bitmap		*backGround;
	Rect		*destinationRect;
	HWND		hDesktop;
};


#endif // !defined(AFX_DESKTOP_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

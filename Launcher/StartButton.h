// StartButton.h: interface for the StartButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STARTBUTTON_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_STARTBUTTON_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Destinator\Destinator.h"
#include "MenuWindow.h"

class StartButton  
{
private:
	HINSTANCE	hInst;
	BOOL		active;
	HANDLE		hService;

	ATOM MyRegisterClass(HINSTANCE hInstance);

public:
	StartButton(HINSTANCE hInstance, HWND hParent);
	virtual ~StartButton();

	MenuWindow	*menuWnd;
	HWND		hStart;

	void Release();
};


#endif // !defined(AFX_STARTBUTTON_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

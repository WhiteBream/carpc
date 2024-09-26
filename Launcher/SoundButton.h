// SoundButton.h: interface for the SoundButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOUNDBUTTON_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_SOUNDBUTTON_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Destinator\Destinator.h"
#include "SoundWindow.h"

class SoundButton
{
private:

	ATOM MyRegisterClass(HINSTANCE hInstance);

public:
	SoundButton(HINSTANCE hInstance, HWND hParent);
	virtual ~SoundButton();

	HINSTANCE	hInst;
	HWND		hWnd;
	SoundWindow	*volWnd;

	void Release();
};


#endif // !defined(AFX_SOUNDBUTTON_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

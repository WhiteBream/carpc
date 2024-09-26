// SoundWindow.h: interface for the SoundWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOUNDWINDOW_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_SOUNDWINDOW_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class SoundWindow
{
private:
	HMODULE     hinstDLL;

	ATOM MyRegisterClass(HINSTANCE hInstance);

public:
	SoundWindow(HINSTANCE hInstance, HWND hParent);
	virtual ~SoundWindow();

	HINSTANCE	hInst;
	HWND		hWnd;
	HWND		hPrnt;
	HMIXER		hMixer;
	DWORD		vMuteControl;
	DWORD		vVolumeControl;
	DWORD		vVolMin, vVolMax;

	void Show();
	void DrawButtons();
};


#endif // !defined(AFX_SOUNDWINDOW_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

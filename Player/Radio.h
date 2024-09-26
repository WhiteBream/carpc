// Radio.h: interface for the PlayWin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RADIO_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_RADIO_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "..\..\..\C401\Drivers\Cargo\TunerLib\TunerLib.h"
//#ifdef _DEBUG
//#pragma comment(lib, "..\\..\\..\\C401\\Drivers\\Cargo\\TunerLib\\Debug\\TunerLib.lib")
//#else
//#pragma comment(lib, "..\\..\\..\\C401\\Drivers\\Cargo\\TunerLib\\Release\\TunerLib.lib")
//#endif

class Radio
{
private:
	Graphics	*myGraphics;
	Brush		*bg[4];
	Pen			*pn;
	RECT		ourRect;
	Font		*fn[2];
	BOOL		saved;
    Bitmap		*bitmap[6];
	LONG		autoLast;
	Graphics	*graph;
	Image		*image;
	HMODULE		hTuner;

	BOOL		vTunerOn;
	INT			vChannelMax;

	ATOM MyRegisterClass(HINSTANCE hInstance);

	void	LoadTunerDll(void);
	void	OpenTuner(void);

public:
	Radio(PAppInfo pAppInfo);
	virtual ~Radio();

	HINSTANCE	hInst;
	int			row1, row2, offset;
	HWND		hWnd, hCbWnd, hPrnt;
	PAppInfo	pAi;
	HANDLE		hKillThread;
	UINT		vFrequency;
	INT			vStereoMode;

	INT			vRssi;
	BOOL		vStereo;
	BOOL		vHasAM;

	Tuner		*pTuner;

	void	InitDC(BOOL updateDC);
	HRESULT	DShowInit(BOOL loadUnload);
	void	Paint();
	INT		GetFrequency(void);
	void	ResetTexts(void);
	void	PaintTime();
	void	ShowMode(int vMode);
	void	SaveSettings();
	void	RetrieveSettings();
	virtual	Show(BOOL yesNo);
	void	Mute(BOOL vMute);
	void	CloseTuner(void);
	void	UpdateTuner();
};


#endif // !defined(AFX_RADIO_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

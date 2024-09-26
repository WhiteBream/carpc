// Radio.h: interface for the PlayWin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RADIO_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_RADIO_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef enum _REGION
{
	EUROPE = 0x00,
	JAPANWB = 0x40,
	JAPAN = 0x80,
	REGMASK = 0xC0
} REGION;

typedef enum _SPACING
{
	SP200 = 0x00,
	SP100 = 0x10,
	SP50 = 0x20,
	SPMASK = 0x30
} SPACING;


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

	USHORT		vRadioRegisters[16];
	BOOL		vTunerOn;
	INT			vChannelMax;
	INT			vRssi;
	WCHAR		vStereo[10];
	INT			vRefresh;
	INT			vTP;
	BOOL		vUpdateFreq;
	REGION		vRegion;
	SPACING		vSpacing;
	CHAR		vTAFilter;
	HANDLE		hThread;
	CHAR		vTunersel;
	BOOL		vAntenna;
	DWORD		vTimerRes;

	ATOM MyRegisterClass(HINSTANCE hInstance);

	USHORT	ReadRegister(INT vRegister);
	void	WriteRegister(INT vRegister, USHORT vMask, USHORT vData);
	void	SetParams(REGION vReg, SPACING vSpace);
	BOOL	ScanTimeout(void);
	void	OpenTuner(void);
	void	SetAntenna(BOOL vOnOff);

public:
	Radio(PAppInfo pAppInfo);
	virtual ~Radio();

	HINSTANCE	hInst;
	int			row1, row2, offset;
	HWND		hWnd, hCbWnd, hPrnt;
	PAppInfo	pAi;
	HANDLE		hKillThread;
	INT			vChannel;
	double		vFrequency;
	INT			vStereoMode;

	void	InitDC(BOOL updateDC);
	HRESULT	DShowInit(BOOL loadUnload);
	void	Paint();
	void	PaintTime();
	void	ShowMode(int vMode);
	void	SaveSettings();
	void	RetrieveSettings();
	virtual	Show(BOOL yesNo);
	void	MonoStereo(BOOL vMono);
	void	Sensitivity(BOOL vHigh);
	void	Mute(BOOL vMute);
	void	CloseTuner(void);
	void	GetStatus(void);
	void	GetStatusFromThread(void);
	void	UpdateRds(WPARAM wParam, LPARAM lParam);
	void	ResetTexts(void);
	void	UpdateTuner();
	void	Scan(BOOL vDirection);
	void	Tune(BOOL vDirection);
	void	SetFrequency(INT vChan);
};


#endif // !defined(AFX_RADIO_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

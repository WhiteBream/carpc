// VideoBar.h: interface for the PlayWin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIDEOBAR_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_VIDEOBAR_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DShow.h"

typedef enum _VideoMode
{
	VB_IDLE,
	VB_NORMAL,
	VB_FULLSCREEN,
	VB_BLOCKED
} VideoMode;

class VideoBar
{
private:
	Graphics   * myGraphics;
	Brush      * bg[4];
	Pen        * pn;
	RECT         ourRect;
	Font       * fn[2];
	HANDLE       logFile;

	ATOM MyRegisterClass(HINSTANCE hInstance);

public:
	VideoBar(HINSTANCE hInstance, HWND hParent, int bottomOffset, Database * pDB);
	virtual ~VideoBar();

	int            offset, row1, row2;
	HWND           hBar, hCbWnd, hPrnt;
	HWND           hVideo;
	HDC            dcVid;
	REFERENCE_TIME tTime;
	BOOL           muted;
	HINSTANCE      hInst;
	VideoMode      vMode;

	// DirectShow stuff
	IGraphBuilder         * pGraph;
	IGraphConfig		  * pConfig;
	IMediaControl         * pControl;
	IMediaEvent           * pEvent;
	IMediaSeeking		  * pSeek;
    IVMRWindowlessControl * pWcontrol;
	IFilterGraph		  * pFilter;
	IBaseFilter			  * pSource;
    IBaseFilter           * pVmr;
	IBasicAudio			  * pAudio;

	void InitDC(BOOL updateDC);
	HRESULT DShowInit(BOOL loadUnload);
	HRESULT InitWindowlessVMR(HWND hwndApp, IVMRWindowlessControl * pWc);
	void Play(PMediaDescr pMedia);
	void Paint();
	void PaintTime();
	void ShowMode(int vMode);
	virtual Show(BOOL yesNo);
	void Pause(BOOL vPause);
	void SetWindowLayout(HWND hWnd);
};


#endif // !defined(AFX_VIDEOBAR_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

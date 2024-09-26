// VideoWin.h: interface for the VideoWin class.
//
//////////////////////////////////////////////////////////////////////

#ifndef VIDEOWIN_H
#define VIDEOWIN_H

#include "StdAfx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class VideoWin
{
private:
	ATOM RegWinClass(LPTSTR pClassName, WNDPROC pClassCb);

	IMediaControl			*pControl;
	IMediaEventEx			*pEvent;
	IGraphBuilder			*pGraph;
	ICaptureGraphBuilder2	*pCapture;
	IBaseFilter				*pVmr;
	IVMRWindowlessControl	*pWcontrol;
	IVMRMixerControl		*pWmixer;

	INT						vNumber;
	BOOL					vMirrorCurrent;

public:
	VideoWin(PAppInfo pAppInfo, IBaseFilter *pSrcFilter, INT vNum);
	virtual ~VideoWin();

	PAppInfo	pAi;
	HWND		hWnd, hCbWnd;
	HDC			hDcVid;

	virtual Show(BOOL yesNo);
	HRESULT SetupVideoWindow(HWND hWnd);
	void PaintVideoWindow(HWND hWnd);
	void ChangeVideoWindow(void);
	void ResizeVideoWindow(HWND hWnd);
	HRESULT ChangePreviewState(HWND hWnd, BOOL vShow);
	HRESULT SetMirrorMode(BOOL vMirror);
	HRESULT Record(BOOL vYesNo);
};


#endif // VIDEOWIN_H

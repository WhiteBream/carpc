// PlayWin.h: interface for the PlayWin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYWIN_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_PLAYWIN_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DShow.h"
#include "Popup.h"

typedef struct _DShow
{
	IGraphBuilder			*pGraph;
	IGraphConfig			*pConfig;
	IMediaControl			*pControl;
	IMediaEvent				*pEvent;
	IMediaSeeking			*pSeek;
    IVMRWindowlessControl	*pWcontrol;
	IFilterGraph			*pFilter;
	IBaseFilter				*pSource;
    IBaseFilter				*pVmr;
	IBasicAudio				*pAudio;
	BOOL					vVideo;
	HWND					hVideo;
	HDC						dcVid;
} DShow;

class PlayWin
{
private:
	Graphics   * myGraphics;
	Brush      * bg[3];
	Pen        * pn;
	RECT         ourRect;
	Font       * fn[2];
	BOOL         saved;
    Bitmap	   * bitmap[3];
	Graphics	*graph;
	Image		*image;

	Popup      * pPopup;
	LONG         autoLast;
	HANDLE       logFile;

	ATOM MyRegisterClass(HINSTANCE hInstance);

public:
	PlayWin(PAppInfo pAppInfo);
	virtual ~PlayWin();

	int				row1, row2, offset;
	HWND			hWnd, hCbWnd, hPrnt;
	HWND			hPopup;
	HWND			hCallBack;
//	BOOL			vVideo;
//	HWND			hVideo;
//	HDC				dcVid;
	REFERENCE_TIME	tTime;
	BOOL			muted;
	HANDLE			hHeap;
	HINSTANCE		hInst;
	BOOL			vHelpActive;
	HCURSOR			vOldCursor;
	HWND			hWndHelp;
	PAppInfo		pAi;
	INT				vNextIndex;
	DShow			dShow[2];
	DShow			*pShow;

	// DirectShow stuff
//	IGraphBuilder         * pGraph;
//	IGraphConfig		  * pConfig;
//	IMediaControl         * pControl;
//	IMediaEvent           * pEvent;
//	IMediaSeeking		  * pSeek;
 //   IVMRWindowlessControl * pWcontrol;
//	IFilterGraph		  * pFilter;
//	IBaseFilter			  * pSource;
 //   IBaseFilter           * pVmr;
//	IBasicAudio			  * pAudio;

//	void CrossfadeStart(PMediaDescr pMedia);
//	void CrossfadeDone();

	void InitDC(BOOL updateDC);
	HRESULT DShowInit(BOOL loadUnload);
	HRESULT InitWindowlessVMR(HWND hwndApp);
	void Paint();
	void ShowMode(int vMode);
	void SaveSettings();
	void RetrieveSettings();
	virtual Show(BOOL yesNo);
	void FillPlayList();
	void Play(PMediaDescr pMedia, BOOL vStart);
	void Pause(BOOL vPause);
};


#endif // !defined(AFX_PLAYWIN_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

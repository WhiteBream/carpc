// PlayWin.h: interface for the PlayWin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYWIN_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_PLAYWIN_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DShow.h"

class Call
{
private:
	Graphics   * myGraphics;
	Brush      * bg[4];
	Pen        * pn;
	RECT         ourRect;
	HINSTANCE    hInst;
	Font       * fn[2];

	LONG         autoLast;

	ATOM MyRegisterClass(HINSTANCE hInstance);
	HRESULT DShowInit(BOOL loadUnload);

public:
	Call(HINSTANCE hInstance, HWND hParent, int bottomOffset);
	virtual ~Call();

	int        offset;
	HWND       hCall, hCbWnd, hPrnt;

	BOOL     muted, saved;
	HANDLE   hHeap;

	// DirectShow stuff
	IGraphBuilder         * pGraph;
	IGraphConfig		  * pConfig;
	IBasicAudio			  * pAudio;

	void InitDC(BOOL updateDC);
	void SetKeypadPositions();
	void Paint();
	void PaintTime();
	void SaveSettings();
	void RetrieveSettings();
	virtual Show(BOOL yesNo);
};


#endif // !defined(AFX_PLAYWIN_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

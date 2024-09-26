// Video.h: interface for the Video class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIDEO_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_VIDEO_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"

class Video
{
private:
	Graphics	*myGraphics;
	Brush		*bkGnd, *textClr;
	RECT		ourRect;
	Font		*font;
	HANDLE		hService;

	Graphics	*barGraphics;
	Graphics	*barGraph;
	Image		*barImage;
	Brush		*barBg;
	Brush		*barText;
	Pen			*barPen;
	Font		*barFont;

	ATOM MyRegisterClass(void);

public:
	Video(PAppInfo pAppInfo);
	virtual ~Video();

	HWND				hVideo, hCbWnd;
	BOOL				vCursor;
	BOOL				vVideoActive;
	PAppInfo			pAi;
	HWND				hBar;
	INT					vDshowPos, vDshowTime;
	BOOL				vBlockCheckNeeded, vBlockNeeded;
	CHAR				vDblClickEnable;
	LPARAM				vMousemove;

	void InitDC(BOOL updateDC);
	void Paint();
	virtual Show(BOOL yesNo);

	void BarGdi(BOOL vStart);
	void InitBarDC(BOOL updateDC, HWND hWnd);
	void PaintTime();
};


#endif // !defined(AFX_VIDEO_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

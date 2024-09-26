// WinMixer.h: interface for the WinMixer class.
//
//////////////////////////////////////////////////////////////////////

#ifndef WINMIXER_H
#define WINMIXER_H

#include "Client.h"


class WinMixer : public Client
{
public:
	WinMixer(PAppInfo pAppInfo);
	virtual ~WinMixer();

	Graphics	*myGraphics;
	Brush    * ourBgnd2;
	Graphics * ourGraphics2;

//	virtual Show(BOOL yesNo);
	void Paint(HDC hDc);
	void PaintBkgnd(HDC hDc);
};


#endif // WINMIXER_H

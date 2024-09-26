// MixWin.h: interface for the Client class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MIXWIN_H
#define MIXWIN_H

#include "Client.h"


class MixWin : public Client
{
public:
	MixWin(PAppInfo pAppInfo);
	virtual ~MixWin();

	Graphics	*myGraphics;
	Brush    * ourBgnd2;
	Graphics * ourGraphics2;

//	virtual Show(BOOL yesNo);
	void Paint(HDC hDc);
	void PaintBkgnd(HDC hDc);
};


#endif // MIXWIN_H

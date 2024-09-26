// Client.h: interface for the Client class.
//
//////////////////////////////////////////////////////////////////////

#ifndef CLIENT_H
#define CLIENT_H

#include "StdAfx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Client
{
private:
	void MakeWindow(PAppInfo pAppInfo, LPTSTR pClassName, WNDPROC pClassCb, LPTSTR pWinName);

protected:
	ATOM RegWinClass(LPTSTR pClassName, WNDPROC pClassCb);

public:
	Client(PAppInfo pAppInfo);
	Client(PAppInfo pAppInfo, LPTSTR pClassName, WNDPROC pClassCb);
	Client(PAppInfo pAppInfo, LPTSTR pClassName, WNDPROC pClassCb, LPTSTR pWinName);
	virtual ~Client();

	PAppInfo	pAi;
	HWND		hWnd, hCbWnd;

	Graphics	*myGraphics;

	virtual Show(BOOL yesNo);
};


#endif // CLIENT_H

// Shutdown.h: interface for the Shutdown class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHUTDOWN_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_SHUTDOWN_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Shutdown
{
private:
	Graphics	*myGraphics;
	HINSTANCE	hInst;

	ATOM MyRegisterClass(HINSTANCE hInstance);

public:
	Shutdown(HINSTANCE hInstance, HWND hParent);
	virtual ~Shutdown();

	HWND	hBar, hBlock;
	BOOL	vClicked;
};

void ShutdownRequest(HWND hWnd);


#endif // !defined(AFX_SHUTDOWN_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

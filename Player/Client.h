// Client.h: interface for the Client class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENT_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_CLIENT_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Client
{
private:
	HINSTANCE    hInst;

	ATOM MyRegisterClass(HINSTANCE hInstance);

public:
	Client(HINSTANCE hInstance, HWND hParent, int bottomOffset);
	virtual ~Client();

	Graphics   * myGraphics;
	HWND         hClient;

	virtual Show(BOOL yesNo);
};


#endif // !defined(AFX_CLIENT_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

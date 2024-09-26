// MediaDB.h: interface for the MediaDB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEDIADB_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_MEDIADB_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class MediaDB
{
private:
	Graphics   * myGraphics;
	Brush      * bkGnd;
	Pen        * pn;
	RECT         ourRect;
	Font       * font;

	ATOM MyRegisterClass(void);

public:
	MediaDB(PAppInfo pAppInfo);
	virtual ~MediaDB();

	HWND       hMedia, hCbWnd;
	BOOL       vOsk;
	PAppInfo   pAi;
	INT        vNextIndex;

	void InitDC(BOOL updateDC);
	void Paint();
	virtual Show(BOOL yesNo);
};


#endif // !defined(AFX_MEDIADB_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

// Sorted.h: interface for the Sorted class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SORTED_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_SORTED_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Sorted
{
private:
	Graphics *myGraphics;
	Brush    *bkGnd;
	RECT      ourRect;
	Font     *font;
	Pen      *pn;
	HWND      hList;

	ATOM MyRegisterClass(void);

public:
	Sorted(PAppInfo pAppInfo);
	virtual ~Sorted();

	HWND       hCbWnd, hText;
	PAppInfo   pAi;
	INT        vNextIndex;

	void InitDC(BOOL updateDC);
	void Paint();
	virtual Show(BOOL yesNo);
};

#endif // !defined(AFX_SORTED_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

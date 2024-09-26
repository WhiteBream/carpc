// DestButton.h: interface for the DestButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DESTBUTTON_H__8F526B1F_CF81_469D_A9B7_DAC37B22DDAF__INCLUDED_)
#define AFX_DESTBUTTON_H__8F526B1F_CF81_469D_A9B7_DAC37B22DDAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gdiplus.h>
using namespace Gdiplus; 
#pragma comment(lib, "gdiplus.lib")


typedef struct _DbDescr
{
	int      width, height;
	int      rcBmp;
	WCHAR  * srcFile;
	Rect     srcRect;
} DbDescr;

#define HKEY_WHITEBREAM		"SOFTWARE\\White Bream\\CarPC\\Buttons"
#define PROCESS_INFOSTR		"Contains alternative bitmaps for DestButtons"

class DestButton
{
private:
	Bitmap    * btn[3];
	HINSTANCE   hInst;

	CreateButtons(int width, int height);
	Bitmap * CreateButton(DbDescr * dbDescr);

public:
	DestButton(HINSTANCE hInstance, int width, int height);
	virtual ~DestButton();

	Bitmap * Idle();
	Bitmap * Pressed();
	Bitmap * Disable();

};

#endif // !defined(AFX_DESTBUTTON_H__8F526B1F_CF81_469D_A9B7_DAC37B22DDAF__INCLUDED_)

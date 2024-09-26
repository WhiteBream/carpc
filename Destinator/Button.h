// Button.h: interface for the Button library.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUTTON_H__8F526B1F_CF81_469D_A9B7_DAC37B22DDAF__INCLUDED_)
#define AFX_BUTTON_H__8F526B1F_CF81_469D_A9B7_DAC37B22DDAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdio.h"

int ButtonInit(HINSTANCE hInstance, BOOL vLoadUnload);

void DestButton(Bitmap ** btn, HINSTANCE hInstance, HWND hwnd, int width, int height);

#endif // !defined(AFX_BUTTON_H__8F526B1F_CF81_469D_A9B7_DAC37B22DDAF__INCLUDED_)

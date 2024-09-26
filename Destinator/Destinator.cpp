// Destinator.cpp: implementation of the Destinator library.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Destinator.h"
#include "Listbox.h"
#include "Button.h"
#include "Slider.h"
#include "Fader.h"
#include "Scroll.h"
#include "Keyboard.h"
#include "Edit.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
int DestinatorInit(HINSTANCE hInstance)
{
	ButtonInit(hInstance, TRUE);
	ListboxInit(hInstance);
	SliderInit(hInstance);
	FaderInit(hInstance);
	ScrollInit(hInstance);
	KeyboardInit(hInstance, TRUE);
	EditInit(hInstance, TRUE);
	return 0;
}

int DestinatorFree(HINSTANCE hInstance)
{
	ButtonInit(hInstance, FALSE);
	KeyboardInit(hInstance, FALSE);
	return 0;
}


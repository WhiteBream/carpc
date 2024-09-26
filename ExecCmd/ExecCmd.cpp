#include "stdafx.h"
#include "Windows.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	WinExec(lpCmdLine, SW_HIDE);
//	WinExec(lpCmdLine, nCmdShow);
	return(0);
}


// player.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MainWindow.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG          msg;
	HWND         oldWin;
	BOOL         bRet;
	MainWindow * mainWin;
	CHAR	vInpTxt[10];

	oldWin = FindWindow(TEXT("CRGMX_MN"), NULL);
	if(oldWin)
		SendMessage(oldWin, WM_CLOSE, 0, 0);

	if(PipeIo("SIrMI", 6, vInpTxt, sizeof(vInpTxt)))
	{
		if((vInpTxt[0] != 0x01)	&& (vInpTxt[0] != 0x05)) //CARGOICE or CARGOBUS?
			return(1);
	}

	mainWin = new MainWindow(hInstance, NULL);

	while((bRet = GetMessage(&msg, NULL, 0, 0)))
	{
		if(bRet == -1)
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}


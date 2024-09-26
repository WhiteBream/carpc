// Player.cpp : Defines the entry point for the application.
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

	oldWin = FindWindow(TEXT("CRPCPLR_MN"), NULL);
	if(oldWin)
	{
		if(lpCmdLine[0])
		{
			// Send the file to the existing instance
			//SendMessage(oldWin, WM_COMMANDLINE, 0, lpCmdLine)
		}
		else
			SendMessage(oldWin, WM_CLOSE, 0, 0);
	}

	mainWin = new MainWindow(hInstance, NULL);

	if(lpCmdLine[0])
	{
		// Post the file to the new instance
		//PostMessage(oldWin, WM_COMMANDLINE, 0, lpCmdLine)
	}

	while((bRet = GetMessage(&msg, NULL, 0, 0)))
	{
		if(bRet == -1)
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}


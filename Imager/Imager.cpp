// Imager.cpp : Defines the entry point for the application.
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

   oldWin = FindWindow(TEXT("CRGIMG_MN"), NULL);
   if(oldWin)
	   SendMessage(oldWin, WM_CLOSE, 0, 0);

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


// Playlist.cpp: implementation of the Playlist class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Playlist.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

LRESULT CALLBACK PlaylistWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static Playlist * myPtr;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
static HWND hBtn[10];

Playlist::Playlist(PAppInfo pAppInfo)
{
	RECT pRect;

	myPtr = this;
	pAi = pAppInfo;
	hInst = pAi->hInst;
	hPrnt = pAi->hApp;
	offset = pAi->vOffs;
//	dBase = pAi->hDb;
	myGraphics = NULL;

   MyRegisterClass(hInst);

   GetClientRect(hPrnt, &pRect);

   bkGnd = new SolidBrush(Color(255, 200, 255, 200));
   font = new Font(L"Arial", 16);

   hList = CreateWindow(L"CRPC_PLLST", L"CarPC playlists", WS_CHILD | WS_CLIPCHILDREN,
      0, 0, pRect.right, pRect.bottom - offset, hPrnt, NULL, hInst, NULL);

   hBtn[0] = CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, 0 * offset, 0,
	  offset, offset, hList, NULL, hInst, NULL);

   hBtn[1] = CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, 1 * offset, 0,
	  offset, offset, hList, NULL, hInst, NULL);

   hBtn[2] = CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, 2 * offset, 0,
	  offset, offset, hList, NULL, hInst, NULL);

   hBtn[3] = CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, 3 * offset, 0,
	  offset, offset, hList, NULL, hInst, NULL);

   hBtn[4] = CreateWindow(L"WB_DESTLISTBOX", NULL, WS_CHILD | WS_VISIBLE, 0, offset,
	  (pRect.right >> 1) - 2, pRect.bottom - 2 * offset, hList, NULL, hInst, NULL);

   hBtn[5] = CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, (pRect.right >> 1) + 0 * offset + 2, 0,
	  offset, offset, hList, NULL, hInst, NULL);

   hBtn[6] = CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, (pRect.right >> 1) + 1 * offset + 2, 0,
	  offset, offset, hList, NULL, hInst, NULL);

   hBtn[7] = CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, (pRect.right >> 1) + 2 * offset + 2, 0,
	  offset, offset, hList, NULL, hInst, NULL);

   hBtn[8] = CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, (pRect.right >> 1) + 3 * offset + 2, 0,
	  offset, offset, hList, NULL, hInst, NULL);

   hBtn[9] = CreateWindow(L"WB_DESTLISTBOX", NULL, WS_CHILD | WS_VISIBLE, (pRect.right >> 1) + 2, offset,
	  (pRect.right >> 1) - 2, pRect.bottom - 2 * offset, hList, NULL, hInst, NULL);
}

Playlist::~Playlist()
{
   delete font;
   delete bkGnd;
   DestroyWindow(hList);
}

Playlist::Show(BOOL yesNo)
{
   if(yesNo)
   {
	   ShowWindow(hList, SW_SHOW);
	   UpdateWindow(hList);
	   PostMessage(hList, WM_PAINT, 0, 0);
   }
   else
   {
	   ShowWindow(hList, SW_HIDE);
   }
}

ATOM Playlist::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= PlaylistWinProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= L"CRPC_PLLST";
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

void Playlist::InitDC(BOOL updateDC)
{
	if(myGraphics == NULL)
	{
	   myGraphics = new Graphics(hCbWnd, FALSE);
	}
	else if(updateDC == TRUE)
	{
	   delete myGraphics;
	   myGraphics = new Graphics(hCbWnd, FALSE);
	}
	GetClientRect(hCbWnd, &ourRect);
}

void Playlist::Paint()
{
	if(myGraphics == NULL)
		return;

	myGraphics->FillRectangle(bkGnd, 0, 0, ourRect.right, ourRect.bottom);
}

LRESULT CALLBACK PlaylistWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
    RECT ourRect;
	int  x;

	GetClientRect(hWnd, &ourRect);

	switch (message)
	{
		case WM_CREATE:
			myPtr->hCbWnd = hWnd;
			myPtr->InitDC(FALSE);
			SendMessage(myPtr->hPrnt, WM_SETREGISTERFINALWND, (WPARAM)IDW_LST, (LPARAM)hWnd);
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			myPtr->Paint();
			EndPaint(hWnd, &ps);
			break;

		case WM_SIZE:
			x = ourRect.right - (8 * myPtr->offset);
			if(x > 2) x = 2;
			SetWindowPos(hBtn[0], 0, 0 * myPtr->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(hBtn[1], 0, 1 * myPtr->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(hBtn[2], 0, 2 * myPtr->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(hBtn[3], 0, 3 * myPtr->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

			SetWindowPos(hBtn[4], 0, 0, myPtr->offset, (ourRect.right >> 1) - 2, ourRect.bottom - myPtr->offset, SWP_NOACTIVATE | SWP_NOZORDER);

			SetWindowPos(hBtn[5], 0, (ourRect.right >> 1) + 0 * myPtr->offset + x, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(hBtn[6], 0, (ourRect.right >> 1) + 1 * myPtr->offset + x, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(hBtn[7], 0, (ourRect.right >> 1) + 2 * myPtr->offset + x, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(hBtn[8], 0, (ourRect.right >> 1) + 3 * myPtr->offset + x, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

			SetWindowPos(hBtn[9], 0, (ourRect.right >> 1) + 2, myPtr->offset, (ourRect.right >> 1) - 2, ourRect.bottom - myPtr->offset, SWP_NOACTIVATE | SWP_NOZORDER);

			myPtr->InitDC(TRUE);
			myPtr->Paint();
			break;

		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
			break;

		case WM_LBUTTONUP:
			ReleaseCapture();
			break;

		case WM_DESTROY:
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}


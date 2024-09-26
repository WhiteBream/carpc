// player.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MainWindow.h"

#define DEFAULTSIZE		48

typedef struct _Clients
{
	Rect   bRect;
	HWND   hWnd, hCbWnd;
	BOOL   enabled;
} Clients;


LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static MainWindow * myPtr;
int          winH, winW;

Clients      clients[7];
Clients      mute;

MainWindow::MainWindow(HINSTANCE hInstance, HWND hParent)
{
   HWND  hPlay, hStart;

   MyRegisterClass(hInstance);
   hInst = hInstance;
   myPtr = this;

//   playWin = NULL;
//   listWin = NULL;
///   sortWin = NULL;
//   mediaDB = NULL;
//   video = NULL;
//   settWin = NULL;

   hStart = FindWindow(L"CRPC_STRTBTTN", NULL);
   if(hStart)
	   GetClientRect(hStart, &startRect);
   else
	   startRect.bottom = startRect.left = startRect.right = startRect.top = 0;

   if(!startRect.bottom) startRect.bottom = DEFAULTSIZE;

   winH = 8 * startRect.bottom;
   winW = (4 * winH) / 3;

   myGraphics = NULL;
   current = 0;
   lastclicked = -1;

//   dBase = new Database();
//   if(dBase->Initialize())
//	   PostQuitMessage(1);

   hPlay = CreateWindow(L"CRPCKIT_MN", L"CarPC Carkit", WS_TILEDWINDOW | WS_CLIPCHILDREN,
						CW_USEDEFAULT, CW_USEDEFAULT, winW, winH, NULL, NULL, hInstance, NULL);

   ShowWindow(hPlay, SW_SHOW);
}

MainWindow::~MainWindow()
{
   DestroyWindow(hPlay);
   if(myGraphics != NULL) delete myGraphics;
   if(bkGnd != NULL) delete bkGnd;
   GdiplusShutdown(gdiplusToken);
//   delete dBase;
}

ATOM MainWindow::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= MainWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PLR));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= L"CRPCKIT_MN";
	wcex.hIconSm		= (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_PLR), IMAGE_ICON, 16, 16, 0);

	return RegisterClassEx(&wcex);
}

void MainWindow::InitDC(BOOL updateDC)
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

void MainWindow::GdiInit(HINSTANCE hInstance, HWND hSrc)
{
   Rect  btnPlace;
   HICON ico;
   GdiplusStartupInput   gdiplusStartupInput;

   // Initialize GDI+.
   GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

   bkGnd = new SolidBrush(Color(255, 0, 0, 192));

   DestinatorInit(hInstance);

   GetClientRect(hSrc, &ourRect);

   btnPlace.X = startRect.right;
   btnPlace.Y = ourRect.bottom - startRect.bottom;
   btnPlace.Width = startRect.bottom;
   btnPlace.Height = startRect.bottom;

   clients[0].hWnd = CreateWindow(L"WB_DESTBUTTON", NULL, WS_CHILD | WS_VISIBLE, 
	   btnPlace.X, btnPlace.Y, btnPlace.Width, btnPlace.Height, hSrc, (HMENU)IDC_PLR, hInstance, NULL);
   ico = LoadIcon(hInstance, (LPCTSTR)IDI_PLR);
   SendMessage(clients[0].hWnd, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
   DestroyIcon(ico);

   btnPlace.X += startRect.bottom;
   clients[1].hWnd = CreateWindow(L"WB_DESTBUTTON", NULL, WS_CHILD | WS_VISIBLE, 
	   btnPlace.X, btnPlace.Y, btnPlace.Width, btnPlace.Height, hSrc, (HMENU)IDC_LST, hInstance, NULL);
   ico = LoadIcon(hInstance, (LPCTSTR)IDI_LIB);
   SendMessage(clients[1].hWnd, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
   DestroyIcon(ico);
   SendMessage(clients[1].hWnd, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

   btnPlace.X += startRect.bottom;
   clients[2].hWnd = CreateWindow(L"WB_DESTBUTTON", NULL, WS_CHILD | WS_VISIBLE, 
	   btnPlace.X, btnPlace.Y, btnPlace.Width, btnPlace.Height, hSrc, (HMENU)IDC_SRT, hInstance, NULL);
   ico = LoadIcon(hInstance, (LPCTSTR)IDI_OSK);
   SendMessage(clients[2].hWnd, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
   DestroyIcon(ico);

   btnPlace.X += startRect.bottom;
   clients[3].hWnd = CreateWindow(L"WB_DESTBUTTON", NULL, WS_CHILD | WS_VISIBLE, 
	   btnPlace.X, btnPlace.Y, btnPlace.Width, btnPlace.Height, hSrc, (HMENU)IDC_MDB, hInstance, NULL);
   ico = LoadIcon(hInstance, (LPCTSTR)IDI_MXR);
   SendMessage(clients[3].hWnd, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
   DestroyIcon(ico);

   btnPlace.X += startRect.bottom;
   mute.hWnd = CreateWindow(L"WB_DESTBUTTON", NULL, WS_CHILD | WS_VISIBLE, 
	   btnPlace.X, btnPlace.Y, btnPlace.Width, btnPlace.Height, hSrc, (HMENU)IDC_SPK, hInstance, NULL);
   ico = LoadIcon(hInstance, (LPCTSTR)IDI_SPK_ON);
   SendMessage(mute.hWnd, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
   DestroyIcon(ico);

}

void MainWindow::Paint()
{
	if(myGraphics == NULL)
		return;

	myGraphics->FillRectangle(bkGnd, 0, 0, ourRect.right, ourRect.bottom);
}


void MainWindow::Client(int n)
{
	if(n != current)
	{
		if(n == -1) n = current;

		switch(n)
		{
			case 0: if(callWin) callWin->Show(TRUE); break;
//			case 1: if(listWin) listWin->Show(TRUE); break;
//			case 2: if(mediaDB) sortWin->Show(TRUE); break;
//			case 6: if(settWin) settWin->Show(TRUE); break;
		}
		if(n != current)
		{
			switch(current)
			{
				case 0: if(callWin) callWin->Show(FALSE); break;
//				case 1: if(listWin) listWin->Show(FALSE); break;
//				case 2: if(mediaDB) sortWin->Show(FALSE); break;
//				case 6: if(settWin) settWin->Show(FALSE); break;
			}
			current = n;
		}
		SendMessage(clients[0].hWnd, BM_SETCHECK, (WPARAM)((current == 0) ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(clients[1].hWnd, BM_SETCHECK, (WPARAM)((current == 1) ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(clients[2].hWnd, BM_SETCHECK, (WPARAM)((current == 2) ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(clients[3].hWnd, BM_SETCHECK, (WPARAM)((current == 3) ? BST_CHECKED : BST_UNCHECKED), 0);
	}
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	int    h, w, n;
	RECT   ourRect, wr;
	HICON  ico;

	h = myPtr->startRect.bottom ? myPtr->startRect.bottom : DEFAULTSIZE;
	w = myPtr->startRect.right;

	switch (message)
	{
		case WM_CREATE:
			myPtr->hCbWnd = hWnd;
			myPtr->InitDC(FALSE);
			myPtr->GdiInit(myPtr->hInst, hWnd);
			myPtr->callWin = new Call(myPtr->hInst, hWnd, myPtr->startRect.bottom);
//			myPtr->listWin = new Playlist(myPtr->hInst, hWnd, myPtr->startRect.bottom);
//			myPtr->sortWin = new Sorted(myPtr->hInst, hWnd, myPtr->startRect.bottom, myPtr->dBase);
//			myPtr->settWin = new Settings(myPtr->hInst, hWnd, myPtr->startRect.bottom, myPtr->dBase);
			break;

		case WM_ENDSESSION:
			if(((BOOL)wParam == TRUE) && (lParam == 0))
				SendMessage(clients[0].hCbWnd, WM_SAVEDATA, 0, 0);
			break;

		case WM_CLOSE:
			SendMessage(clients[0].hCbWnd, WM_SAVEDATA, 0, 0);
			return(DefWindowProc(hWnd, message, wParam, lParam));

		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				switch(LOWORD(wParam))
				{
					case IDC_PLR:
						n = 0;
						break;

					case IDC_LST:
						n = 1;
						break;

					case IDC_SRT:
						n = 2;
						break;

					case IDC_MDB:
						n = 3;
						break;

					case IDC_MXR:
						n = 6;
						break;
					default:
						n = myPtr->current;
						break;
				}
				myPtr->Client(n);

				switch(LOWORD(wParam))
				{
					case IDC_SPK:
						if(SendMessage(mute.hWnd, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
						{
							ico = LoadIcon(myPtr->hInst, (LPCTSTR)IDI_SPK_OFF);
							SendMessage(mute.hWnd, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
							PostMessage(clients[0].hCbWnd, WM_DIRECTSHOWMUTE, (WPARAM)TRUE, 0);
						}
						else
						{
							ico = LoadIcon(myPtr->hInst, (LPCTSTR)IDI_SPK_ON);
							SendMessage(mute.hWnd, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							PostMessage(clients[0].hCbWnd, WM_DIRECTSHOWMUTE, (WPARAM)FALSE, 0);
						}
						SendMessage(mute.hWnd, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
						DestroyIcon(ico);
						break;
				}
				// Pass the command to the playwindow
				SendMessage(clients[0].hCbWnd, message, wParam, lParam);
			}
			break;

		case WM_SYSCOMMAND:
			if(wParam == SC_CLOSE)
			{
				SendMessage(clients[0].hCbWnd, WM_SAVEDATA, 0, 0);
			}
			return(DefWindowProc(hWnd, message, wParam, lParam));
			break;

		case WM_SIZE:
			GetClientRect(hWnd, &ourRect);
			if((ourRect.right < (w + (8 * h))) || (ourRect.bottom < (3 * (w + (8 * h)) / 4)))
			{
				GetWindowRect(hWnd, &wr);
				wr.right -= wr.left;
				wr.right -= ourRect.right; // result is width overhead
				wr.bottom -= wr.top;
				wr.bottom -= ourRect.bottom; // result is height overhead

				if(ourRect.right < (w + (8 * h))) ourRect.right = w + (8 * h);
				if(ourRect.bottom < (3 * (w + (8 * h)) / 4)) ourRect.bottom = 3 * (w + (8 * h)) / 4;

				SetWindowPos(hWnd, 0, 0, 0, ourRect.right + wr.right, ourRect.bottom + wr.bottom,
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
				GetClientRect(hWnd, &ourRect);
			}
			else
			{
				SetWindowPos(clients[0].hWnd, 0, w + 0 * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
				SetWindowPos(clients[1].hWnd, 0, w + 1 * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
				SetWindowPos(clients[2].hWnd, 0, w + 2 * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
				SetWindowPos(clients[3].hWnd, 0, w + 3 * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
				SetWindowPos(mute.hWnd, 0, ourRect.right - h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			}

			myPtr->InitDC(TRUE);
			myPtr->Paint();
			myPtr->Client(-1);
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			myPtr->Paint();
			EndPaint(hWnd, &ps);
			break;

		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
			break;

		case WM_LBUTTONUP:
			ReleaseCapture();
			break;

		case WM_SETREGISTERFINALWND:
			clients[LOWORD(wParam)].hCbWnd = (HWND)lParam;
			break;

		case WM_REALIGNVIDEOWINDOW:
			GetClientRect(hWnd, &ourRect);
			SetWindowPos(clients[4].hWnd, 0, w + 4 * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			break;

		case WM_DESTROY:
//			delete myPtr->settWin;
//			delete myPtr->listWin;
//			delete myPtr->sortWin;
			delete myPtr->callWin;
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}



// MainWindow.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MainWindow.h"
#include "PlayCap.h"

#include "..\..\..\C401\Drivers\Cargo\ServiceLib\ServiceLib.h"
#ifdef _DEBUG
#pragma comment(lib, "..\\..\\..\\C401\\Drivers\\Cargo\\ServiceLib\\Debug\\ServiceLib.lib")
#else
#pragma comment(lib, "..\\..\\..\\C401\\Drivers\\Cargo\\ServiceLib\\Release\\ServiceLib.lib")
#endif

HANDLE hService;

#define DEFAULTSIZE		48

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK VideoProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static MainWindow * my;
INT          winH, winW, winX, winY;


MainWindow::MainWindow(HINSTANCE hInstance, HWND hParent)
{
	GdiplusStartupInput   gdiplusStartupInput;
	HWND  hStart;
	Rect  btnPlace;
	STARTUPINFO	supInfo;

	MyRegisterClass(hInstance);
	hInst = hInstance;
	my = this;

	hStart = FindWindow(TEXT("CRPC_STRTBTTN"), NULL);
	if(hStart)
		GetClientRect(hStart, &startRect);
	else
		startRect.bottom = startRect.left = startRect.right = startRect.top = 0;
	if(!startRect.bottom)
		startRect.bottom = DEFAULTSIZE;
	offs = startRect.bottom;

	memset(&supInfo, 0, sizeof(supInfo));
	GetStartupInfo(&supInfo);
	if(supInfo.dwFlags & STARTF_USESIZE)
	{
		winH = supInfo.dwYSize;
		winW = supInfo.dwXSize;
	}
	else
	{
		winH = 10 * startRect.bottom;
		winW = (4 * winH) / 3;
	}
	if(supInfo.dwFlags & STARTF_USEPOSITION)
	{
		winX = supInfo.dwX;
		winY = supInfo.dwY;
	}
	else
	{
		winX = CW_USEDEFAULT;
		winY = CW_USEDEFAULT;
	}

	// Initialize GDI+.
//	gdiplusStartupInput.DebugEventCallback = NULL;
//	gdiplusStartupInput.GdiplusVersion = 1;
//	gdiplusStartupInput.SuppressBackgroundThread = FALSE;
//	gdiplusStartupInput.SuppressExternalCodecs = FALSE;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	myGraphics = NULL;
	bkGnd1 = new SolidBrush(Color(255, 0, 0, 192));
	bkGnd2 = new SolidBrush(Color(255, 0, 0, 0));

	DestinatorInit(hInstance);

	hWnd = CreateWindow(TEXT("CRPCVWR_MN"), TEXT("Video viewer"), WS_TILEDWINDOW | WS_CLIPCHILDREN,
						winX, winY, winW, winH, NULL, NULL, hInstance, NULL);

	GetClientRect(hWnd, &ourRect);
	btnPlace.X = startRect.right;
	btnPlace.Y = ourRect.bottom - startRect.bottom;
	btnPlace.Width = startRect.bottom;
	btnPlace.Height = startRect.bottom;

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y, 
				btnPlace.Width, btnPlace.Height, hWnd, (HMENU)IDC_FOLDER, hInstance, NULL);
	AssignIcon(hInstance, hWnd, IDC_FOLDER, IDI_FOLDER);

	btnPlace.X += startRect.bottom;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y, 
				btnPlace.Width, btnPlace.Height, hWnd, (HMENU)IDC_ZOOMIN, hInstance, NULL);
	AssignIcon(hInstance, hWnd, IDC_ZOOMIN, IDI_ZOOMIN);
	SendDlgItemMessage(hWnd, IDC_ZOOMIN, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

	btnPlace.X += startRect.bottom;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y, 
				btnPlace.Width, btnPlace.Height, hWnd, (HMENU)IDC_ZOOMOUT, hInstance, NULL);
	AssignIcon(hInstance, hWnd, IDC_ZOOMOUT, IDI_ZOOMOUT);
	SendDlgItemMessage(hWnd, IDC_ZOOMOUT, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

	btnPlace.X += startRect.bottom;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y, 
					btnPlace.Width, btnPlace.Height, hWnd, (HMENU)IDC_PREV, hInstance, NULL);
	AssignIcon(hInstance, hWnd, IDC_PREV, IDI_PREV);
//	SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

	btnPlace.X += startRect.bottom;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y, 
				btnPlace.Width, btnPlace.Height, hWnd, (HMENU)IDC_NEXT, hInstance, NULL);
	AssignIcon(hInstance, hWnd, IDC_NEXT, IDI_NEXT);
//	SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

	btnPlace.X += startRect.bottom;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y, 
					btnPlace.Width, btnPlace.Height, hWnd, (HMENU)IDC_EXIT, hInstance, NULL);
	AssignIcon(hInstance, hWnd, IDC_EXIT, IDI_EXIT);

	CreateWindow(TEXT("CRPCVWR_VD"), NULL, WS_CHILD | WS_VISIBLE, 0, 0, 
					ourRect.right, btnPlace.Y, hWnd, (HMENU)IDC_VIDEO, hInstance, NULL);

    if(FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
    {
		MessageBox(NULL, TEXT("CoInit failed"), TEXT("Error"), MB_OK);
        exit(1);
    } 
	else
	{
        if(FAILED(CaptureVideo(GetDlgItem(hWnd, IDC_VIDEO))))
        {
            CloseInterfaces();        
			DestroyWindow(hWnd);
        }
        else
        {
            ShowWindow(hWnd, SW_SHOW);
        }       
	}

	hService = OpenPsuEvent(hWnd, INP_REVERSE);
}

MainWindow::~MainWindow()
{
	if(hService) ClosePsuEvent(hService, NULL);

	CoUninitialize();
	DestroyWindow(hWnd);
	if(myGraphics) delete myGraphics;
	if(bkGnd1) delete bkGnd1;
	if(bkGnd2) delete bkGnd2;
	GdiplusShutdown(gdiplusToken);
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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIEWER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= CreateSolidBrush(RGB(0, 0, 192));
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPCVWR_MN");
	wcex.hIconSm		= (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_VIEWER), IMAGE_ICON, 16, 16, 0);
	RegisterClassEx(&wcex);

	wcex.style			= 0;
	wcex.lpfnWndProc	= VideoProc;
	wcex.hbrBackground	= NULL;//CreateSolidBrush(RGB(200, 255, 200));
	wcex.lpszClassName	= TEXT("CRPCVWR_VD");

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

void MainWindow::Paint()
{
	if(myGraphics == NULL)
		return;

//	myGraphics->FillRectangle(bkGnd1, 0, ourRect.bottom - offs, ourRect.right, offs);
//	myGraphics->FillRectangle(bkGnd2, 0, 0, ourRect.right, ourRect.bottom - offs);
}

HWND	hOldWin = NULL;
UINT	hShowPrev;

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	int    h, w;
	RECT   ourRect, wr;

	h = my->startRect.bottom ? my->startRect.bottom : DEFAULTSIZE;
	w = my->startRect.right;

	switch (message)
	{
		case WM_CREATE:
			my->hCbWnd = hWnd;
			my->InitDC(FALSE);
			ShowWindow(hWnd, SW_SHOW);
			break;

		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				switch(LOWORD(wParam))
				{
					case IDC_FOLDER:
						break;

					case IDC_ZOOMIN:
						break;

					case IDC_ZOOMOUT:
						break;

					case IDC_PREV:
						SetMirrorMode(TRUE);
						break;

					case IDC_NEXT:
						SetMirrorMode(FALSE);
						break;

					case IDC_EXIT:
						PostQuitMessage(0);
						break;

					default:
						break;
				}
			}
			break;

		case WB_PSUEVENT:
			break;
			if(wParam == INP_REVERSE)
			{
				WINDOWPLACEMENT sPlacem;
				sPlacem.length = sizeof(WINDOWPLACEMENT);
				if(lParam)
				{
					hOldWin = GetForegroundWindow();

					GetWindowPlacement(hWnd, &sPlacem);
					hShowPrev = sPlacem.showCmd;
					if(sPlacem.showCmd == SW_SHOWMINIMIZED)
						ShowWindow(hWnd, SW_RESTORE);

					SetForegroundWindow(hWnd);
				}
				else if(hOldWin && (hOldWin != hWnd))
					SetForegroundWindow(hOldWin);

				if(!lParam && (hShowPrev == SW_SHOWMINIMIZED))
					ShowWindow(hWnd, SW_SHOWMINNOACTIVE);
			}
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
			SetWindowPos(GetDlgItem(hWnd, IDC_FOLDER), 0, w + 0 * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_ZOOMIN), 0, w + 1 * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_ZOOMOUT), 0, w + 2 * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_PREV), 0, w + 3 * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_NEXT), 0, w + 4 * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_EXIT), 0, w + 5 * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_VIDEO), 0, 0, 0, ourRect.right, ourRect.bottom - h, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);

			my->InitDC(TRUE);
			my->Paint();
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			my->Paint();
			EndPaint(hWnd, &ps);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return(0);
}


LRESULT CALLBACK VideoProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT     ps;
	switch (message)
	{
		case WM_CLOSE:
			CloseInterfaces();  // Stop capturing and release interfaces
			return(DefWindowProc(hWnd, message, wParam, lParam));

		case WM_SIZE:
		case WM_WINDOWPOSCHANGING:
			ResizeVideoWindow(hWnd);
			break;

		case WM_PAINT:
			BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			PaintVideoWindow(hWnd);
			break;

		case WM_GRAPHNOTIFY:
      //	HandleGraphEvent();
			break;

//		case WM_SHOWWINDOW:
//			ChangePreviewState(hWnd, IsWindowVisible(hWnd));
//			break;

		case WM_WINDOWPOSCHANGED:
			if(((WINDOWPOS*)lParam)->cx > 100)
			{
				ChangePreviewState(hWnd, TRUE);
				ResizeVideoWindow(hWnd);
			}
			else
				ChangePreviewState(hWnd, FALSE);
			break;

		case WM_DISPLAYCHANGE:
			ChangeVideoWindow();
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	// Pass this message to the video window for notification of system changes
	//GraphNotify(hWnd, message, wParam, lParam);
	return(0);
}

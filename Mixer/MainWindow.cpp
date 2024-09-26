// player.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MainWindow.h"
#include <shlobj.h>

#define DEFAULTSIZE		48

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

AppInfo sAi;


#include "MixWin.h"
#include "WinMixer.h"


MainWindow::MainWindow(HINSTANCE hInstance, HWND hParent)
{
	HWND		hStart;
	STARTUPINFO	supInfo;
	INT			X, Y, W, H;
	TCHAR		vPath[MAX_PATH];
	CHAR		vInpTxt[10];

	sAi.hInst = hInstance;
	sAi.hSetValue = NULL;
	MyRegisterClass(sAi.hInst);
	pAi = &sAi;
	memset(clients, 0, sizeof(clients));

	/* Get the desired size of the buttons */
	hStart = FindWindow(TEXT("CRPC_STRTBTTN"), NULL);
	if(hStart)
		GetClientRect(hStart, &startRect);
	else
		startRect.bottom = startRect.left = startRect.right = startRect.top = 0;
	if(!startRect.bottom)
		startRect.bottom = DEFAULTSIZE;
	sAi.vOffs = startRect.bottom;

	H = 8 * startRect.bottom;
	W = (4 * H) / 3;

	memset(&supInfo, 0, sizeof(supInfo));
	GetStartupInfo(&supInfo);
	if(supInfo.dwFlags & STARTF_USESIZE)
	{
		H = supInfo.dwYSize;
		W = supInfo.dwXSize;
	}
	else
	{
		H = 8 * startRect.bottom;
		W = (4 * H) / 3;
	}

	if(supInfo.dwFlags & STARTF_USEPOSITION)
	{
		X = supInfo.dwX;
		Y = supInfo.dwY;
	}
	else
	{
		X = CW_USEDEFAULT;
		Y = CW_USEDEFAULT;
	}

	myGraphics = NULL;
	myBackground = NULL;
	lastclicked = -1;
	current = IDW_PLR;

	/* Create main window */
	hWnd = CreateWindow(TEXT("CRGMX_MN"), TEXT("CARGO Audio Mixer"), WS_TILEDWINDOW | WS_CLIPCHILDREN,
						X, Y, W, H, hParent, NULL, sAi.hInst, this);

	SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, SHGFP_TYPE_CURRENT, vPath);
	PathAppend(vPath, TEXT("White Bream\\Mixer.dll"));
	hMixer = LoadLibrary(vPath);
	if(hMixer)
	{
		HFNCB dllIds;
		dllIds = (HFNCB)GetProcAddress(hMixer, "SetCallback");
		if(dllIds)
			(dllIds)(hWnd);

		sAi.hSetValue = (HFNSET)GetProcAddress(hMixer, "SetMixer");
	}

	PipeIo("MXrM", 5, vInpTxt, sizeof(vInpTxt));
	SendDlgItemMessage(hWnd, IDC_MUTE, BM_SETCHECK, (vInpTxt[0] - '0' ? BST_UNCHECKED : BST_CHECKED), 0);
}

MainWindow::~MainWindow()
{
//	CHAR		vInpTxt[10];
//	PipeIo("MXwA", 5, vInpTxt, sizeof(vInpTxt));	// Save All

	DestroyWindow(hWnd);
	if(myGraphics != NULL)
		delete myGraphics;
	if(bkGnd != NULL)
		delete bkGnd;
	GdiplusShutdown(gdiplusToken);

	DestinatorFree(sAi.hInst);
	sAi.hSetValue = NULL;
	FreeLibrary(hMixer);
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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MIXER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRGMX_MN");
	wcex.hIconSm		= (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_MIXER), IMAGE_ICON, 16, 16, 0);

	return RegisterClassEx(&wcex);
}

void MainWindow::InitDC(BOOL updateDC)
{
	if(updateDC && myGraphics)
	   delete myGraphics;
//	if(updateDC && myBackground)
//	   delete myBackground;

	myGraphics = NULL;
//	myBackground = NULL;

	if(myGraphics == NULL)
	{
		myGraphics = new Graphics(hCbWnd, FALSE);
	//	myBackground = new Bitmap(sAi.hInst, MAKEINTRESOURCE(IDB_BACKGROUND));
	}

	GetClientRect(hCbWnd, &ourRect);
}

void MainWindow::GdiInit(HINSTANCE hInstance, HWND hSrc)
{
	GdiplusStartupInput gdiplusStartupInput;
	Rect                btnPlace;
	INT                 i;

	// Initialize GDI+.
	gdiplusStartupInput.DebugEventCallback = NULL;
	gdiplusStartupInput.GdiplusVersion = 1;
	gdiplusStartupInput.SuppressBackgroundThread = FALSE;
	gdiplusStartupInput.SuppressExternalCodecs = FALSE;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	bkGnd = new SolidBrush(Color(ALPHA, 0, 0, 192));

	DestinatorInit(hInstance);

	GetClientRect(hSrc, &ourRect);

	btnPlace.X = startRect.right;
	btnPlace.Y = ourRect.bottom - sAi.vOffs;

	clients[0].vControl = IDC_MIXER;
	clients[0].vIcon = IDI_MIXER;
	clients[1].vControl = IDC_RAD;
	clients[1].vIcon = IDI_RAD;
	clients[2].vControl = IDC_LST;
	clients[2].vIcon = IDI_LST;
	clients[3].vControl = IDC_SRT;
	clients[3].vIcon = IDI_SRT;
	clients[4].vControl = IDC_EXIT;
	clients[4].vIcon = IDI_EXIT;
	clients[5].vControl = 0;

	for(i = 0; clients[i].vControl != 0; i++)
	{
		clients[i].hWnd = CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y, 
										sAi.vOffs, sAi.vOffs, hSrc, (HMENU)clients[i].vControl, hInstance, NULL);
		AssignIcon(hInstance, hSrc, clients[i].vControl, clients[i].vIcon);
		btnPlace.X += sAi.vOffs;
	}

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y, 
				sAi.vOffs, sAi.vOffs, hSrc, (HMENU)IDC_MUTE, hInstance, NULL);
	AssignIcon(hInstance, hSrc, IDC_MUTE, IDI_SPK_ON);

	/* Some (temporary) default disabled */
	SendDlgItemMessage(hSrc, IDC_RAD, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
	SendDlgItemMessage(hSrc, IDC_LST, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
	SendDlgItemMessage(hSrc, IDC_SRT, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
}

void MainWindow::Paint()
{
//	Rect *rect = new Rect(0, 0, ourRect.right, ourRect.bottom);

	if(myGraphics == NULL)
		return;

//	myGraphics->DrawImage(myBackground, *rect, 0, 0, 
//		myBackground->GetWidth(), myBackground->GetHeight(), UnitPixel);
//	delete rect;

	myGraphics->FillRectangle(bkGnd, 0, ourRect.bottom - sAi.vOffs, ourRect.right, sAi.vOffs);
}

void MainWindow::SendClientMessage(UINT vMsg, WPARAM wParam, LPARAM lParam)
{
	INT i;

	for(i = 0; clients[i].vControl != 0; i++)
		if(clients[i].hCbWnd)
			SendMessage(clients[i].hCbWnd, vMsg, wParam, lParam);
}

void MainWindow::ShowClient(int n)
{
	if(n != current)
	{
		if(n == -1)
			n = current;

		switch(n)
		{
			case 0: if(clients[n].pClient)  clients[n].pClient->Show(TRUE);  break;
		//	case 1: if(clients[n].pClient)  clients[n].pClient->Show(TRUE);  break;
		//	case 2: if(clients[n].pClient)  clients[n].pClient->Show(TRUE);  break;
		//	case 3: if(sortWin)  sortWin->Show(TRUE);  break;
		}
		if(n != current)
		{
			switch(current)
			{
				case 0: if(clients[current].pClient)  clients[current].pClient->Show(FALSE);  break;
		//		case 1: if(clients[current].pClient)  clients[current].pClient->Show(FALSE);  break;
		//		case 2: if(clients[current].pClient)  clients[current].pClient->Show(FALSE);  break;
		//		case 3: if(sortWin)  sortWin->Show(FALSE);  break;
			}
			current = n;
		}
		SendMessage(clients[0].hWnd, BM_SETCHECK, (WPARAM)((current == 0) ? BST_CHECKED : BST_UNCHECKED), 0);
//		SendMessage(clients[1].hWnd, BM_SETCHECK, (WPARAM)((current == 1) ? BST_CHECKED : BST_UNCHECKED), 0);
//		SendMessage(clients[2].hWnd, BM_SETCHECK, (WPARAM)((current == 2) ? BST_CHECKED : BST_UNCHECKED), 0);
//		SendMessage(clients[3].hWnd, BM_SETCHECK, (WPARAM)((current == 3) ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(clients[4].hWnd, BM_SETCHECK, (WPARAM)((current == 4) ? BST_CHECKED : BST_UNCHECKED), 0);
	}
}

bool TestCargo(void)
{
	CHAR	vInpTxt[10];

	if(PipeIo("SIrMI", 5, vInpTxt, sizeof(vInpTxt)))
	{
		// CARGO system ?
		if((vInpTxt[0] == 0x01) || (vInpTxt[0] == 0x05))
			return(TRUE);
	}
	return(FALSE);
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT vMsg, WPARAM wParam, LPARAM lParam)
{
	MainWindow *my;
	PAINTSTRUCT ps;
	int    h, w, n, i;
	RECT   ourRect, wr;
	CHAR	vInpTxt[10];
	CHAR	vOutTxt[10];

	my = (MainWindow*)GetWindowLong(hWnd, GWL_USERDATA);
	if((my == NULL) && (vMsg != WM_NCCREATE))
		return DefWindowProc(hWnd, vMsg, wParam, lParam);

	switch(vMsg)
	{
		case WM_NCCREATE:
			my = (MainWindow*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLong(hWnd, GWL_USERDATA, (LONG)my);
			return(my ? DefWindowProc(hWnd, vMsg, wParam, lParam) : FALSE);

		case WM_CREATE:
			my = (MainWindow*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLong(hWnd, GWL_USERDATA, (LONG)my);

			my->hCbWnd = hWnd;
			sAi.hApp = hWnd;
			my->InitDC(FALSE);
			my->GdiInit(sAi.hInst, hWnd);
			i = 0;
//			if(TestCargo())
			my->clients[i++].pClient = new MixWin(my->pAi);
//			my->clients[i++].pClient = new WinMixer(my->pAi);
//			my->clients[i++].pClient = new WinMixer(my->pAi);
			ShowWindow(hWnd, SW_SHOW);
			break;

		case WM_ENDSESSION:
			if(((BOOL)wParam == TRUE) && (lParam == 0))
			{
				if(sAi.hSetValue) sAi.hSetValue("MXwA", 5);
				else PipeIo("MXwA", 5, vInpTxt, sizeof(vInpTxt));	// Save All
				my->SendClientMessage(WM_SAVEDATA, 0, 0);
			}
			break;

		case WM_CLOSE:
			if(sAi.hSetValue) sAi.hSetValue("MXwA", 5);
			else PipeIo("MXwA", 5, vInpTxt, sizeof(vInpTxt));	// Save All
			my->SendClientMessage(WM_SAVEDATA, 0, 0);
			return(DefWindowProc(hWnd, vMsg, wParam, lParam));

		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				switch(LOWORD(wParam))
				{
					case IDC_MIXER:	n = 0;	break;
					case IDC_RAD:	n = 1;	break;
					case IDC_LST:	n = 2;	break;
					case IDC_SRT:	n = 3;	break;
//					case IDC_EXIT:	n = my->current; PostQuitMessage(0); 	break;
					case IDC_EXIT:	n = my->current; PostMessage(hWnd, WM_CLOSE, 0, 0); 	break;
					default:
						n = my->current;
						break;
				}
				my->ShowClient(n);

				switch(LOWORD(wParam))
				{
					case IDC_MUTE:
						BOOL mute;
						mute = SendDlgItemMessage(hWnd, IDC_MUTE, BM_GETCHECK, 0, 0) == BST_CHECKED ? FALSE : TRUE;
						StringCbPrintfA(vOutTxt, sizeof(vOutTxt), "MXwM%c", mute ? '1' : '0');
						if(sAi.hSetValue) sAi.hSetValue(vOutTxt, (INT)strlen(vOutTxt) + 1);
						else
						{
							PipeIo(vOutTxt, (DWORD)strlen(vOutTxt) + 1, vInpTxt, sizeof(vInpTxt));
							SendDlgItemMessage(hWnd, IDC_MUTE, BM_SETCHECK, (mute ? BST_CHECKED : BST_UNCHECKED), 0);
							AssignIcon(sAi.hInst, hWnd, IDC_MUTE, (mute ? IDI_SPK_OFF : IDI_SPK_ON));
						}
						break;

					default:
						if(IsChild(hWnd, (HWND)lParam))
						{
							// Pass the command to the playwindow
							SendMessage(my->clients[IDW_PLR].hCbWnd, vMsg, wParam, lParam);
						}
						break;
				}
			}
			break;

		case WM_SYSCOMMAND:
			if(wParam == SC_CLOSE)
				my->SendClientMessage(WM_SAVEDATA, 0, 0);
			return(DefWindowProc(hWnd, vMsg, wParam, lParam));

		case WM_MIXERCHANGED:
			if(wParam == 'M')
			{
				SendDlgItemMessage(hWnd, IDC_MUTE, BM_SETCHECK, (lParam - '0' ? BST_CHECKED : BST_UNCHECKED), 0);
				AssignIcon(sAi.hInst, hWnd, IDC_MUTE, (lParam - '0' ? IDI_SPK_OFF : IDI_SPK_ON));
			}
			my->SendClientMessage(vMsg, wParam, lParam);
			break;

		case WM_SIZE:
			h = sAi.vOffs ? sAi.vOffs : DEFAULTSIZE;
			w = my->startRect.right;
			GetClientRect(hWnd, &ourRect);
			if((ourRect.right < (w + (8 * h))) || (ourRect.right < (10 * h)) || (ourRect.bottom < (3 * (w + (8 * h)) / 4)))
			{
				GetWindowRect(hWnd, &wr);
				wr.right -= wr.left;
				wr.right -= ourRect.right; // result is width overhead
				wr.bottom -= wr.top;
				wr.bottom -= ourRect.bottom; // result is height overhead

				if(ourRect.right < (w + (8 * h))) ourRect.right = w + (8 * h);
				if(ourRect.right < (10 * h)) ourRect.right = 10 * h;
				if(ourRect.bottom < (3 * (w + (8 * h)) / 4)) ourRect.bottom = 3 * (w + (8 * h)) / 4;
				if(ourRect.bottom < (3 * (10 * h) / 4)) ourRect.bottom = 3 * (10 * h) / 4;

				SetWindowPos(hWnd, 0, 0, 0, ourRect.right + wr.right, ourRect.bottom + wr.bottom,
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
				GetClientRect(hWnd, &ourRect);
			}
			else
			{
				for(i = 0; my->clients[i].vControl != 0; i++)
					SetWindowPos(my->clients[i].hWnd, NULL, w + i * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
				SetWindowPos(GetDlgItem(hWnd, IDC_MUTE), 0, ourRect.right - h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			}
			for(i = 0; my->clients[i].vControl != 0; i++)
				SetWindowPos(my->clients[i].hCbWnd, NULL, 0, 0, ourRect.right, ourRect.bottom - h, SWP_NOACTIVATE | SWP_NOZORDER);

			my->InitDC(TRUE);
			my->Paint();
			my->ShowClient(-1);
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			my->Paint();
			EndPaint(hWnd, &ps);
			break;

		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
			break;

		case WM_LBUTTONUP:
			ReleaseCapture();
			break;

		case WM_SETREGISTERFINALWND:
			my->clients[LOWORD(wParam)].hCbWnd = (HWND)lParam;
			break;

		case WM_DESTROY:
			if(my->clients[0].pClient)  delete my->clients[0].pClient;
			if(my->clients[1].pClient)  delete my->clients[1].pClient;
			if(my->clients[2].pClient)  delete my->clients[2].pClient;
			PostQuitMessage(0);
			break;

		default:
			return(DefWindowProc(hWnd, vMsg, wParam, lParam));
   }
   return(0);
}

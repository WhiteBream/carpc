// player.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MainWindow.h"
#include <shlobj.h>

#define DEFAULTSIZE		48

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

AppInfo sAi;

#include "VideoWin.h"


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
	hWnd = CreateWindow(TEXT("CRGVD_MN"), TEXT("CARGO Camera Viewer"), WS_TILEDWINDOW | WS_CLIPCHILDREN,
						X, Y, W, H, hParent, NULL, sAi.hInst, this);
}

MainWindow::~MainWindow()
{
	DestroyWindow(hWnd);
	if(myGraphics != NULL)
		delete myGraphics;
	if(bkGnd != NULL)
		delete bkGnd;
	GdiplusShutdown(gdiplusToken);

	DestinatorFree(sAi.hInst);
	sAi.hSetValue = NULL;
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
	wcex.lpszClassName	= TEXT("CRGVD_MN");
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


HRESULT MainWindow::FindCaptureDevice(IBaseFilter ** ppSrcFilter)
{
    HRESULT hr;
    IBaseFilter * pSrc = NULL;
    CComPtr <IMoniker> pMoniker =NULL;
    ULONG cFetched;

    if (!ppSrcFilter)
        return E_POINTER;

    // Create the system device enumerator
    CComPtr <ICreateDevEnum> pDevEnum =NULL;

    hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
                           IID_ICreateDevEnum, (void **) &pDevEnum);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't create system enumerator!  hr=0x%x"), hr);
        return hr;
    }

    // Create an enumerator for the video capture devices
    CComPtr <IEnumMoniker> pClassEnum = NULL;

    hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't create class enumerator!  hr=0x%x"), hr);
        return hr;
    }

    // If there are no enumerators for the requested type, then
    // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
    if (pClassEnum == NULL)
    {
        MessageBox(hWnd,TEXT("No video capture device was detected.\r\n\r\n")
                   TEXT("This sample requires a video capture device, such as a USB WebCam,\r\n")
                   TEXT("to be installed and working properly.  The sample will now close."),
                   TEXT("No Video Capture Hardware"), MB_OK | MB_ICONINFORMATION);
        return E_FAIL;
    }

    // Use the first video capture device on the device list.
    // Note that if the Next() call succeeds but there are no monikers,
    // it will return S_FALSE (which is not a failure).  Therefore, we
    // check that the return code is S_OK instead of using SUCCEEDED() macro.
    if (S_OK == (pClassEnum->Next (1, &pMoniker, &cFetched)))
    {
        // Bind Moniker to a filter object
        hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);
        if (FAILED(hr))
        {
            Msg(TEXT("Couldn't bind moniker to filter object!  hr=0x%x"), hr);
            return hr;
        }
    }
    else
    {
        Msg(TEXT("Unable to access video capture device!"));
        return E_FAIL;
    }

    // Copy the found filter pointer to the output parameter.
    // Do NOT Release() the reference, since it will still be used
    // by the calling function.
    *ppSrcFilter = pSrc;

    return hr;
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
			case IDW_PLR: if(clients[n].pClient)  clients[n].pClient->Show(TRUE);  break;
			case IDW_RAD: if(clients[n].pClient)  clients[n].pClient->Show(TRUE);  break;
		//	case IDW_LST: if(listWin)  listWin->Show(TRUE);  break;
		//	case IDW_SRT: if(sortWin)  sortWin->Show(TRUE);  break;
			case IDW_BYE: PostQuitMessage(0);  break;
		}
		if(n != current)
		{
			switch(current)
			{
				case IDW_PLR: if(clients[current].pClient)  clients[current].pClient->Show(FALSE);  break;
				case IDW_RAD: if(clients[current].pClient)  clients[current].pClient->Show(FALSE);  break;
		//		case IDW_LST: if(listWin)  listWin->Show(FALSE);  break;
		//		case IDW_SRT: if(sortWin)  sortWin->Show(FALSE);  break;
			}
			current = n;
		}
		SendMessage(clients[0].hWnd, BM_SETCHECK, (WPARAM)((current == 0) ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(clients[1].hWnd, BM_SETCHECK, (WPARAM)((current == 1) ? BST_CHECKED : BST_UNCHECKED), 0);
//		SendMessage(clients[2].hWnd, BM_SETCHECK, (WPARAM)((current == 2) ? BST_CHECKED : BST_UNCHECKED), 0);
//		SendMessage(clients[3].hWnd, BM_SETCHECK, (WPARAM)((current == 3) ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(clients[4].hWnd, BM_SETCHECK, (WPARAM)((current == 4) ? BST_CHECKED : BST_UNCHECKED), 0);
	}
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

		//	my->clients[0].pClient = new MixWin(&sAi);
		//	my->clients[1].pClient = new Client(my->pAi);
			ShowWindow(hWnd, SW_SHOW);
			break;

		case WM_ENDSESSION:
			if(((BOOL)wParam == TRUE) && (lParam == 0))
				my->SendClientMessage(WM_SAVEDATA, 0, 0);
			break;

		case WM_CLOSE:
			my->SendClientMessage(WM_SAVEDATA, 0, 0);
			return(DefWindowProc(hWnd, vMsg, wParam, lParam));

		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				switch(LOWORD(wParam))
				{
					case IDC_MIXER:	n = IDW_PLR;	break;
					case IDC_RAD:	n = IDW_RAD;	break;
					case IDC_LST:	n = IDW_LST;	break;
					case IDC_SRT:	n = IDW_SRT;	break;
					case IDC_EXIT:	n = IDW_BYE;	break;
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
		//	if(my->listWin)  delete my->listWin;
		//	if(my->sortWin)  delete my->sortWin;
		//	if(my->radioWin) delete my->radioWin;
//			if(my->mixWin)  delete my->mixWin;
			if(my->clients[0].pClient)  delete my->clients[0].pClient;
			PostQuitMessage(0);
			break;

		default:
			return(DefWindowProc(hWnd, vMsg, wParam, lParam));
   }
   return(0);
}

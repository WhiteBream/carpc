// VideoBar.cpp: implementation of the PlayWin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VideoBar.h"
#include "Ticker.h"
//#include "vmr9.h"

#pragma comment(lib, "Strmiids.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define BG_WHITE		0
#define BG_GREEN		1
#define BG_LBLUE		2
#define BG_DBLUE		3

LRESULT CALLBACK PlayWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static VideoBar * my;

static Database * dBase;
static WCHAR      szCurrent[66];
static WCHAR      szInfo[66];
static WCHAR      szFile[1024];
static MediaDescr sCurrentMedia;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VideoBar::VideoBar(HINSTANCE hInstance, HWND hParent, int bottomOffset, Database * pDB)
{
	RECT        pRect;
	HICON       ico;
	FontFamily *fm;
//	INT         x, w;

	my = this;

	hInst = hInstance;
	hPrnt = hParent;
	offset = bottomOffset;
	row1 = (2 * bottomOffset) / 3;
	row2 = (5 * bottomOffset) / 2;
	myGraphics = NULL;
	dBase = pDB;
	muted = FALSE;
	vMode = VB_IDLE;

	pGraph = NULL;
	pConfig = NULL;
	pControl = NULL;
	pEvent = NULL;
	pSeek = NULL;
	pFilter = NULL;
	pWcontrol = NULL;
	pVmr = (IBaseFilter *)-1;
	pAudio = NULL;

	MyRegisterClass(hInstance);
	TickerInit(hInstance);
	GetClientRect(hParent, &pRect);

	bg[BG_WHITE] = new SolidBrush(Color(255, 255, 255, 255));
	bg[BG_GREEN] = new SolidBrush(Color(255, 200, 255, 200));
	bg[BG_LBLUE] = new SolidBrush(Color(255, 201, 211, 227));
	bg[BG_DBLUE] = new SolidBrush(Color(255, 0, 0, 192));
	pn = new Pen(Color(255, 0, 0, 0), 1);

	fm = new FontFamily(L"Arial");
	fn[0] = new Font(fm, (float)row1 + 2, FontStyleRegular, UnitPixel);
	fn[1] = new Font(fm, (float)(offset >> 1) - 7, FontStyleRegular, UnitPixel);
	delete fm;

	hBar = CreateWindow(TEXT("CRPC_VBAR"), NULL, WS_CHILD | WS_CLIPCHILDREN | WS_BORDER, 0, 0, 0, 0, hParent, NULL, hInstance, NULL);

	CreateWindow(WBBUTTON, NULL, WS_CHILD, 0, 0, 0, offset, hBar, (HMENU)IDC_PLAY, hInstance, NULL);
	ico = LoadIcon(hInstance, (LPCTSTR)IDI_PLAY);
	SendDlgItemMessage(hBar, IDC_PLAY, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
	DestroyIcon(ico);

	CreateWindow(WBBUTTON, NULL, WS_CHILD, 0, 0, 0, offset, hBar, (HMENU)IDC_STOP, hInstance, NULL);
	ico = LoadIcon(hInstance, (LPCTSTR)IDI_STOP);
	SendDlgItemMessage(hBar, IDC_STOP, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
	DestroyIcon(ico);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, offset, hBar, (HMENU)IDC_PAUSE, hInstance, NULL);
	ico = LoadIcon(hInstance, (LPCTSTR)IDI_PAUSE);
	SendDlgItemMessage(hBar, IDC_PAUSE, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
	DestroyIcon(ico);

	CreateWindow(TEXT("WB_DESTSLIDER"), NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, (offset >> 1) + 2, hBar, (HMENU)IDC_SEEK, hInstance, NULL);
	SendDlgItemMessage(hBar, IDC_SEEK, TBM_SETRANGE, TRUE, MAKELONG(0, 10000));

	CreateWindow(TEXT("WB_TICKERBOX"), NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hBar, (HMENU)IDC_TICKER, hInstance, NULL);

	CreateWindow(TEXT("WB_TICKERBOX"), NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hBar, (HMENU)IDC_SUBTICKER, hInstance, NULL);

	//HRESULT hr = CoInitialize(NULL);
	//if(FAILED(hr))
	//{
	//	MessageBox(NULL, TEXT("CoInit failed"), TEXT("Error"), MB_OK);
	//}
	//else
	//{
		logFile = CreateFile(TEXT("MediaVideo.log"), FILE_ALL_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		tTime = 0;
	//	pSource = NULL;

	//	// By now every should have been loaded...
	//	RetrieveSettings();
	//}
}

VideoBar::~VideoBar()
{
	DestroyWindow(hBar);
	delete bg[BG_GREEN];
	delete bg[BG_DBLUE];
	delete bg[BG_LBLUE];
	delete bg[BG_WHITE];
	delete pn;
	delete fn[0];
	delete fn[1];
	DShowInit(FALSE);
	//CoUninitialize();
}

VideoBar::Show(BOOL yesNo)
{
	ShowWindow(hBar, yesNo ? SW_SHOW : SW_HIDE);
}

ATOM VideoBar::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= PlayWinProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_VBAR");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

void VideoBar::InitDC(BOOL updateDC)
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

//IVMRMonitorConfig 

HRESULT VideoBar::DShowInit(BOOL loadUnload)
{
	IMediaEventEx * pEventEx;
    IVMRFilterConfig* pVmrConf; 
	HRESULT hr;

	if(pControl)  pControl->Stop();

	if(pAudio)    pAudio->Release();	pAudio = NULL;
	if(pSource)   pSource->Release();	pSource = NULL;
	if(pWcontrol) pWcontrol->Release(); pWcontrol = NULL;
	if(pEvent)    pEvent->Release();    pEvent = NULL;
	if(pSeek)     pSeek->Release();     pSeek = NULL;
	if(pControl)  pControl->Release();  pControl = NULL;
	if(pFilter)   pFilter->Release();	pFilter = NULL;
	if(pConfig)   pConfig->Release();	pConfig = NULL;
	if(pGraph)    pGraph->Release();    pGraph = NULL;

	if(loadUnload == FALSE)
		return(0);
	else
	{
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
				IID_IGraphBuilder, (void **)&pGraph);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IGraphBuilder failed"), TEXT("Error"), MB_OK);
			return hr;
		}
		pGraph->SetLogFile((DWORD_PTR)logFile);

		hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IMediaControl failed"), TEXT("Error"), MB_OK);
		}
		hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IMediaEvent failed"), TEXT("Error"), MB_OK);
			return hr;
		}
		hr = pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pSeek);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IMediaSeeking failed"), TEXT("Error"), MB_OK);
			return hr;
		}
		hr = pGraph->QueryInterface(IID_IGraphConfig, (void **)&pConfig);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IGraphConfig failed"), TEXT("Error"), MB_OK);
			return hr;
		}
		hr = pGraph->QueryInterface(IID_IFilterGraph, (void **)&pFilter);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IFilterGraph failed"), TEXT("Error"), MB_OK);
			return hr;
		}
		hr = pGraph->QueryInterface(IID_IMediaEventEx, (void **)&pEventEx);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IMediaEventEx failed"), TEXT("Error"), MB_OK);
			return hr;
		}
		pEventEx->SetNotifyWindow((long)hBar, WM_DIRECTSHOW, NULL);
		pEventEx->Release();
		hr = pGraph->QueryInterface(IID_IBasicAudio, (void **)&pAudio);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IBasicAudio failed"), TEXT("Error"), MB_OK);
			return hr;
		}
		pAudio->put_Volume(0);

		// Create the VMR. 
		hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void**)&pVmr); 
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("CLSID_VideoMixingRenderer failed"), TEXT("Error"), MB_OK);
			return hr;
		}
		// Add the VMR to the filter graph.
		hr = pGraph->AddFilter(pVmr, TEXT("Video Mixing Renderer")); 
		if(FAILED(hr)) 
		{
			MessageBox(NULL, TEXT("AddFilter on VMR failed"), TEXT("Error"), MB_OK);
			pVmr->Release();
			return hr;
		}
		// Set the rendering mode.  
		hr = pVmr->QueryInterface(IID_IVMRFilterConfig, (void**)&pVmrConf); 
		if(SUCCEEDED(hr)) 
		{ 
			hr = pVmrConf->SetRenderingMode(VMRMode_Windowless); 
			if(FAILED(hr))
			{
				MessageBox(NULL, TEXT("SetRenderingMode failed"), TEXT("Error"), MB_OK);
				return(hr);
			}
			pVmrConf->Release(); 
		}
		else
		{
			MessageBox(NULL, TEXT("IID_IVMRFilterConfig failed"), TEXT("Error"), MB_OK);
			return(hr);
		}

		hr = pVmr->QueryInterface(IID_IVMRWindowlessControl, (void**)&pWcontrol);
        if(FAILED(hr)) 
		{
			MessageBox(NULL, TEXT("IID_IVMRWindowlessControl failed"), TEXT("Error"), MB_OK);
			return(hr);
		}
		// Set the target window temporary to ours
		hr = pWcontrol->SetVideoClippingWindow(hBar); 
        if(FAILED(hr)) 
		{
			MessageBox(NULL, TEXT("SetVideoClippingWindow failed"), TEXT("Error"), MB_OK);
			return(hr);
		}
		hr = pWcontrol->SetAspectRatioMode(VMR_ARMODE_LETTER_BOX); 
        if(FAILED(hr)) 
		{
			MessageBox(NULL, TEXT("SetAspectRatioMode failed"), TEXT("Error"), MB_OK);
			return(hr);
		}
	    pVmr->Release();
		pVmr = NULL;
		return(hr);
	}
}

HRESULT VideoBar::InitWindowlessVMR(HWND hwndApp, IVMRWindowlessControl * pWc)
{ 
	RECT rcSrc, rcDest; 
	long lWidth, lHeight; 
	HRESULT hr;

    if(!pWc)
		return E_POINTER;

    // Set the window. 
    hr = pWc->SetVideoClippingWindow(hwndApp); 
    if (SUCCEEDED(hr))
    {
		// Find the native video size.
		if(pWcontrol)
		{
			hr = pWcontrol->GetNativeVideoSize(&lWidth, &lHeight, NULL, NULL); 
			if(SUCCEEDED(hr))
			{
				// Set the source rectangle.
				SetRect(&rcSrc, 0, 0, lWidth, lHeight); 

				// Get the window client area.
				GetClientRect(hVideo, &rcDest); 
				//myPtr->pWcontrol->SetAspectRatioMode(VMR_ARMODE_NONE);
				// Set the video position.
				hr = pWcontrol->SetVideoPosition(&rcSrc, &rcDest); 
				if(FAILED(hr))
					MessageBox(NULL, TEXT("SetVideoPosition failed"), TEXT("Error"), MB_OK);
			}
			else
				MessageBox(NULL, TEXT("GetNativeVideoSize failed"), TEXT("Error"), MB_OK);
		}
    }
	else
		MessageBox(NULL, TEXT("SetVideoClippingWindow failed"), TEXT("Error"), MB_OK);

    hr = pWc->SetBorderColor(RGB(0,0,0));
	if(FAILED(hr))
	{
		MessageBox(NULL, TEXT("SetBorderColor failed"), TEXT("Error"), MB_OK);
		return(hr);
	}
    return hr; 
} 

void VideoBar::Play(PMediaDescr pMedia)
{
	REFERENCE_TIME pos = 0;
	HRESULT        hr;
	INT            vLen;
	LPSTR          sFile;
	LONG           lWidth, lHeight;
	LPTSTR         pFile = NULL;
	HRESULT        vStat;
	DWORD          cn;
	LPTSTR         pString;

	if(pMedia == NULL)
		return;

	if(sCurrentMedia.vMediaId)
		dBase->SaveHistory(&sCurrentMedia);
	memcpy(&sCurrentMedia, pMedia, sizeof(sCurrentMedia));

	/* (Re)start DShow engine */
	if(SUCCEEDED(DShowInit(TRUE)))
	{
		// Clear current title
		memset(szCurrent, 0, sizeof(szCurrent));
		memset(szInfo, 0, sizeof(szInfo));

		vLen = dBase->GetStringValue(pMedia, FLD_FILE, 0, &pFile);
		if(vLen <= 1)
		{
			GlobalFree(pFile);
			return;
		}

		sFile = (LPSTR)GlobalAlloc(GPTR, ++vLen);
	#ifdef UNICODE
		StringCbPrintfA(sFile, vLen, "%ws", pFile);
		StringCbPrintfW(szFile, sizeof(szFile), L"%ws", pFile);
	#else
		StringCbPrintfA(sFile, vLen, "%hs", pFile);
		StringCbPrintfW(szFile, sizeof(szFile), L"%hs", pFile);
	#endif
		GlobalFree(pFile);

		WriteFile(logFile, "\n\r\n\r", 4, &cn, NULL);
		WriteFile(logFile, sFile, vLen, &cn, NULL);
		WriteFile(logFile, "\n\r\n\r", 4, &cn, NULL);

		// Songname
		pString = dBase->DescriptorToDoubleString(pMedia, (MediaField)(FLD_ARTIST | FLD_TITLE));
		if(pString)
		{
#ifdef UNICODE
			StringCbPrintfW(szCurrent, sizeof(szCurrent), L"%ls", pString);
#else
			StringCbPrintfW(szCurrent, sizeof(szCurrent), L"%hs", pString);
#endif //UNICODE
			GlobalFree((HGLOBAL)pString);
		}

		// Info bar
		pString = dBase->DescriptorToSingleString(pMedia, (MediaField)FLD_ALBUM);
		if(pString)
		{
#ifdef UNICODE
			StringCbPrintfW(szInfo, sizeof(szInfo), L" %ls", pString);
#else
			StringCbPrintfW(szInfo, sizeof(szInfo), L" %hs", pString);
#endif //UNICODE
			GlobalFree((HGLOBAL)pString);
		}

		// Add year to info bar
		pString = dBase->DescriptorToSingleString(pMedia, (MediaField)FLD_YEAR);
		if(pString)
		{
			LPWSTR pYear;
			size_t len;

			if(pString[0] && (pString[0] != '0'))
			{
				StringCbLengthW(szInfo, sizeof(szInfo), &len);
				pYear = &szInfo[len / sizeof(WCHAR)];
#ifdef UNICODE
				if(len)
					StringCbPrintfW(pYear, sizeof(szInfo) - len, L" - %ls", pString);
				else
					StringCbPrintfW(szInfo, sizeof(szInfo), L" %ls", pString);
#else
				if(len)
					StringCbPrintfW(pYear, sizeof(szInfo) - len, L" - %hs", pString);
				else
					StringCbPrintfW(szInfo, sizeof(szInfo), L" %hs", pString);
#endif //UNICODE
			}
			GlobalFree((HGLOBAL)pString);
		}

		if(pGraph && szFile)
		{
			vStat = pGraph->RenderFile(szFile, 0);
			if(FAILED(vStat))
			{
				StringCchPrintfA((LPSTR)szFile, 100, "Render failed with val %d", vStat);
				WriteFile(logFile, szFile, lstrlen(szFile), &cn, NULL);
				WriteFile(logFile, "\n\r\n\r\n\r\n\r", 8, &cn, NULL);
			}
		}

		if(pSeek)
		{
			pSeek->GetDuration(&tTime);
			pSeek->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
		}
		Paint();

		if(pAudio)
			pAudio->put_Volume(muted ? -2000 : 0);

		if(pControl)
			pControl->Run();

		if(pWcontrol)
		{
			hr = pWcontrol->GetNativeVideoSize(&lWidth, &lHeight, 0, 0);
			if((hr != E_NOINTERFACE) && (lWidth != 0) && (lHeight != 0))
			{
				// If this video is encoded with an unsupported codec,
				// we won't see any video, although the audio will work if it is
				// of a supported format.
				SendMessage(hVideo, WM_SHOWVIDEOWINDOW, (WPARAM)0, (LPARAM)TRUE);
				SendMessage(hPrnt, WM_SHOWVIDEOWINDOW, (WPARAM)0, (LPARAM)TRUE);
			}
		}
	}
	GlobalFree(pMedia);
}

void VideoBar::Paint()
{
	if(myGraphics == NULL)
		return;

	myGraphics->FillRectangle(bg[BG_DBLUE], 0, 0, ourRect.right, row1);
	myGraphics->FillRectangle(bg[BG_GREEN], 0, row1, ourRect.right, ourRect.bottom - row1);

	if(szCurrent[0] != TEXT('\00'))
		SetDlgItemText(hBar, IDC_TICKER, szCurrent);
	else
		SetDlgItemText(hBar, IDC_TICKER, TEXT("CARGO video player"));

	if(szInfo[0] != TEXT('\00'))
		SetDlgItemText(hBar, IDC_SUBTICKER, szInfo);
	else
		SetDlgItemText(hBar, IDC_SUBTICKER, TEXT(""));

	PaintTime();
}

void VideoBar::PaintTime()
{
	PointF point;
	REFERENCE_TIME units;
	int pos, time;
	WCHAR  vBuffer[20];
	StringFormat fmt;
	HICON ico;
	OAFilterState	state;
    Bitmap *bitmap;

	fmt.SetAlignment(StringAlignmentFar);
	fmt.SetTrimming(StringTrimmingNone);
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);

	if(myGraphics == NULL)
		return;

	if(pSeek)
		pSeek->GetDuration(&tTime);

	myGraphics->FillRectangle(bg[BG_WHITE], 2, row1 + 2, 3 * offset - 3, row2 - row1 - 5);
	myGraphics->DrawRectangle(pn, 2, row1 + 2, 3 * offset - 3, row2 - row1 - 5);

	// Print total track time
	point.X = (float)3 * offset - 4;
	point.Y = (float)(3 * row1 - 2);
	time = (int)(tTime / 10000000);	// from 100ns to 1s
	if(time > 3600)
		StringCbPrintf(vBuffer, sizeof(vBuffer), TEXT("%ld:%02ld:%02ld"), time / 3600, (time % 3600) / 60, time % 60);
	else
		StringCbPrintf(vBuffer, sizeof(vBuffer), TEXT("%ld:%02ld"), time / 60, time % 60);
	myGraphics->DrawString(vBuffer, -1, fn[1], point, &fmt, bg[BG_DBLUE]);

	// Print remaining track time
	if(pSeek)
		pSeek->GetCurrentPosition(&units); else units = 0;
	point.Y = (float)(2 * row1 + 9);
	pos = (int)(units / 10000000);	// from 100ns to 1s
	if(time > 3600)
		StringCbPrintf(vBuffer, sizeof(vBuffer), TEXT("-%ld:%02ld:%02ld"), (time - pos) / 3600, ((time - pos) % 3600) / 60, (time - pos) % 60);
	else
		StringCbPrintf(vBuffer, sizeof(vBuffer), TEXT("-%ld:%02ld"), (time - pos) / 60, (time - pos) % 60);
	myGraphics->DrawString(vBuffer, -1, fn[1], point, &fmt, bg[BG_DBLUE]);

	// Print current track position
	point.X = (float)3 * offset;
	point.Y = (float)(row1 + 2);
	if(time > 3600)
		StringCbPrintfW(vBuffer, sizeof(vBuffer), L"%ld:%02ld:%02ld", pos / 3600, (pos % 3600) / 60, pos % 60);
	else
		StringCbPrintfW(vBuffer, sizeof(vBuffer), L"%ld:%02ld", pos / 60, pos % 60);
	myGraphics->DrawString(vBuffer, -1, fn[0], point, &fmt, bg[BG_DBLUE]);

	if(tTime && time)
		SendDlgItemMessage(hBar, IDC_SEEK, TBM_SETPOS, TRUE, (pos * 10000) / time);
	else
		SendDlgItemMessage(hBar, IDC_SEEK, TBM_SETPOS, TRUE, 0);

	if(pControl)
	{
		pControl->GetState(10, &state);
		switch(state)
		{
			case State_Paused:
				ico = LoadIcon(hInst, (LPCTSTR)IDI_PAUSED);
				break;
			case State_Running:
				ico = LoadIcon(hInst, (LPCTSTR)IDI_PLAYING);
				break;
			case State_Stopped:
			default:
				ico = LoadIcon(hInst, (LPCTSTR)IDI_STOPPED);
				break;
		}
		bitmap = new Bitmap(ico);
		DestroyIcon(ico);
		myGraphics->DrawImage(bitmap, 5, row2 - offset + 5);
		delete bitmap;
	}

	// Restart refresh timer
	SetTimer(hBar, 1, 1000, NULL);
}

void VideoBar::Pause(BOOL vPause)
{
	OAFilterState state;

	if(pControl)
	{
		pControl->GetState(10, &state);
		if((state == State_Running) && (vPause == TRUE))
		{
			pControl->Pause();
			// Set the pause button activated
			SendDlgItemMessage(hCbWnd, IDC_PAUSE, BM_SETCHECK, BST_CHECKED, 0);
		}
		else if(vPause == FALSE)
		{
			pControl->Run();
			// Put the pause btton back to default state
			SendDlgItemMessage(hCbWnd, IDC_PAUSE, BM_SETCHECK, BST_UNCHECKED, 0);
		}
	}
}

void VideoBar::SetWindowLayout(HWND hWnd)
{
    RECT ourRect;
	INT  x, w;

	GetClientRect(hWnd, &ourRect);

	w = (ourRect.right - 5 * offset) / 5;
	x = 0;
	SetWindowPos(GetDlgItem(hWnd, IDC_PLAY), NULL, x, row2, w, offset, SWP_NOACTIVATE | SWP_NOZORDER);
	x += w;
	SetWindowPos(GetDlgItem(hWnd, IDC_STOP), NULL, x, row2, w, offset, SWP_NOACTIVATE | SWP_NOZORDER);
	x += w;
	SetWindowPos(GetDlgItem(hWnd, IDC_PAUSE), NULL, x, row2, w, offset, SWP_NOACTIVATE | SWP_NOZORDER);

	/* Top row, playlist add controls */
	SetWindowPos(GetDlgItem(hWnd, IDC_SEEK), NULL, 3 * offset + 2, 3 * row1 - 5, ourRect.right - 3 * offset - 4, (offset >> 1) + 2, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(GetDlgItem(hWnd, IDC_TICKER), NULL, 3 * offset + 2, row1 + 2, ourRect.right - 3 * offset - 4, row1 + 6, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(GetDlgItem(hWnd, IDC_SUBTICKER), NULL, 3 * offset + 2, 2 * row1 + 7, ourRect.right - 3 * offset - 4, (offset >> 1) - 8, SWP_NOACTIVATE | SWP_NOZORDER);


}

LRESULT CALLBACK VideoBarWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT		ps;
    RECT			ourRect;
	REFERENCE_TIME	pos;
	OAFilterState	state;
	DWORD			Position;
	LONG			curSel;
	LPWSTR			pText;
	LONG			lEvCode, lParam1, lParam2;
	INT				size;
	LRESULT id;
	PMediaDescr     pMedia;
	LPTSTR			pString;


	GetClientRect(hWnd, &ourRect);
	if(my->pControl)
		my->pControl->GetState(10, &state);

	switch(message)
	{
		case WM_CREATE:
			my->hCbWnd = hWnd;
			my->InitDC(FALSE);
			SendMessage(my->hPrnt, WM_SETREGISTERFINALWND, (WPARAM)IDW_PLR, (LPARAM)hWnd);
			break;

		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					switch(LOWORD(wParam))
					{
						case IDC_PLAY:
							SendDlgItemMessage(hWnd, IDC_SEEK, TBM_SETRANGE, TRUE, MAKELONG(0, 10000));
							if(my->pControl)
							{
								my->pControl->Run();
								SendDlgItemMessage(hWnd, IDC_PAUSE, BM_SETCHECK, BST_UNCHECKED, 0);
							}
							break;

						case IDC_STOP:
							if(my->pControl)
								my->pControl->Stop();
							pos = 0;
							if(my->pSeek)
								my->pSeek->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
							SendDlgItemMessage(hWnd, IDC_PAUSE, BM_SETCHECK, BST_UNCHECKED, 0);
							break;

						case IDC_PAUSE:
							id = SendDlgItemMessage(hWnd, IDC_PAUSE, BM_GETCHECK, 0, 0);
							my->Pause(id == BST_UNCHECKED ? TRUE : FALSE);
							break;

						default:
							if(IsChild(hWnd, (HWND)lParam))
							{
								// Pass though to parent
								PostMessage(my->hPrnt, message, wParam, lParam);
							}
							break;
					}
					break;

			}
			break;

		case WM_HSCROLL:
			if((LOWORD(wParam) == TB_THUMBTRACK) || (LOWORD(wParam) == TB_ENDTRACK ))
			{
				if((Position = HIWORD(wParam)) == 0)
					Position  = SendDlgItemMessage(hWnd, IDC_SEEK, TBM_GETPOS, 0, 0);
				pos = (my->tTime / 10000) * Position;
				if(my->pSeek)
					my->pSeek->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
			}
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			my->Paint();
			EndPaint(hWnd, &ps);
			break;

		case WM_SIZE:
			my->SetWindowLayout(hWnd);
			/* Second row, play controls */
//			w = (ourRect.right - 5 * my->offset) / 5;
//			x = 0;
//			SetWindowPos(GetDlgItem(hWnd, IDC_PLAY), NULL, x, my->row2, w, my->offset, SWP_NOACTIVATE | SWP_NOZORDER);
//			x += w;
//			SetWindowPos(GetDlgItem(hWnd, IDC_STOP), NULL, x, my->row2, w, my->offset, SWP_NOACTIVATE | SWP_NOZORDER);
//			x += w;
//			SetWindowPos(GetDlgItem(hWnd, IDC_PAUSE), NULL, x, my->row2, w, my->offset, SWP_NOACTIVATE | SWP_NOZORDER);

			/* Top row, playlist add controls */
//			SetWindowPos(GetDlgItem(hWnd, IDC_SEEK), NULL, 3 * my->offset + 2, 3 * my->row1 - 5, ourRect.right - 3 * my->offset - 4, (my->offset >> 1) + 2, SWP_NOACTIVATE | SWP_NOZORDER);
//			SetWindowPos(GetDlgItem(hWnd, IDC_TICKER), NULL, 3 * my->offset + 2, my->row1 + 2, ourRect.right - 3 * my->offset - 4, my->row1 + 6, SWP_NOACTIVATE | SWP_NOZORDER);
//			SetWindowPos(GetDlgItem(hWnd, IDC_SUBTICKER), NULL, 3 * my->offset + 2, 2 * my->row1 + 7, ourRect.right - 3 * my->offset - 4, (my->offset >> 1) - 8, SWP_NOACTIVATE | SWP_NOZORDER);

			my->InitDC(TRUE);
			my->Paint();
			break;

//		case WM_SETVIDEOWINDOW:
//			my->hVideo = (HWND)wParam;
//	//		while(my->pVmr != NULL) ;
//			if(my->pWcontrol)
//				my->InitWindowlessVMR((HWND)myPtr->hVideo, myPtr->pWcontrol);
//			my->dcVid = GetDC(my->hVideo);
//			break;

		case WM_PAINTVIDEOWINDOW:
			if(my->pWcontrol)
				my->pWcontrol->RepaintVideo(my->hVideo, my->dcVid);
			break;

		case WM_DISPLAYCHANGE:
			if(my->pWcontrol)
				my->pWcontrol->DisplayModeChanged();
			break;

		case WM_DIRECTSHOW:
			if(my->pEvent)
			{
				while(my->pEvent->GetEvent(&lEvCode, &lParam1, &lParam2, 0) == S_OK)
				{
					if(lEvCode == EC_COMPLETE)
					{
						SendMessage(my->hVideo, WM_SHOWVIDEOWINDOW, (WPARAM)0, (LPARAM)FALSE);
						my->pEvent->FreeEventParams(lEvCode, lParam1, lParam2);
						my->DShowInit(FALSE);
						break;
					}
					else
						my->pEvent->FreeEventParams(lEvCode, lParam1, lParam2);
				}
			}
			break;

		case WM_TIMER:
			my->PaintTime();
			break;

		case WM_DESTROY:
			KillTimer(hWnd, 1);
			break;

		default:
			return(DefWindowProc(hWnd, message, wParam, lParam));
   }
   return(0);
}


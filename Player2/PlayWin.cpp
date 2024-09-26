// PlayWin.cpp: implementation of the PlayWin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PlayWin.h"
#include "Ticker.h"
#include "Spectrum.h"
//#include "vmr9.h"
#include "Remove.h"
#include "Help.h"

#pragma comment(lib, "Strmiids.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

typedef struct _ComingUp
{
	LPWSTR	szFile;
} ComingUp, *pComingUp;

#define BG_WHITE		0
//#define BG_GREEN		1
#define BG_LBLUE		1
#define BG_DBLUE		2

int NowPlaying(void * cbParam, int colCount, char ** colData, char ** colNames);
LRESULT CALLBACK DShowProc(HWND hWnd2, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PlayWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static void DrawSortListBox(HWND hWnd);
static void DrawListBoxItem(LPDRAWITEMSTRUCT pItem);
static void InitListBox(BOOL loadUnload, HWND lbox);

static PlayWin		*my;

static Database		*dBase;
static WCHAR		szCurrent[66];
static WCHAR		szInfo[66];
static WCHAR		szFile[1024];
static WCHAR		szRating[15];
static MediaDescr	sCurrentMedia;

static HWND hWndRemove = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PlayWin::PlayWin(PAppInfo pAppInfo)
{
	RECT        pRect;
	HICON       ico;
	FontFamily *fm;
	INT         x, w;

	my = this;

	pAi = pAppInfo;
	hInst = pAi->hInst;
	hPrnt = pAi->hApp;
	offset = pAi->vOffs;
	dBase = pAi->hDb;
	row1 = (2 * offset) / 3;
	row2 = (5 * offset) / 2;
	myGraphics = NULL;
	autoLast = 0;
	muted = FALSE;
	saved = FALSE;
//	vVideo = FALSE;
//	dcVid = NULL;
	vNextIndex = 0;

	ZeroMemory(dShow, sizeof(dShow);
	pShow = dShow[0];
//	pGraph = NULL;
//	pConfig = NULL;
//	pControl = NULL;
//	pEvent = NULL;
//	pSeek = NULL;
//	pFilter = NULL;
//	pWcontrol = NULL;
//	pVmr = (IBaseFilter *)-1;
//	pAudio = NULL;

	hHeap = GetProcessHeap();
	MyRegisterClass(hInst);
	TickerInit(hInst);
	SpectrumInit(hInst);
	GetClientRect(hPrnt, &pRect);

	bg[BG_WHITE] = new SolidBrush(Color(255, 255, 255, 255));
	bg[BG_LBLUE] = new SolidBrush(Color(255, 201, 211, 227));
	bg[BG_DBLUE] = new SolidBrush(Color(255, 0, 0, 192));
	pn = new Pen(Color(255, 0, 0, 0), 1);

	fm = new FontFamily(L"Arial");
	fn[0] = new Font(fm, (float)row1 + 2, FontStyleRegular, UnitPixel);
	fn[1] = new Font(fm, (float)(offset >> 1) - 7, FontStyleRegular, UnitPixel);
	delete fm;

	ico = LoadIcon(hInst, (LPCTSTR)IDI_STOPPED);
	bitmap[0] = new Bitmap(ico);
	DestroyIcon(ico);
	ico = LoadIcon(hInst, (LPCTSTR)IDI_PLAYING);
	bitmap[1] = new Bitmap(ico);
	DestroyIcon(ico);
	ico = LoadIcon(hInst, (LPCTSTR)IDI_PAUSED);
	bitmap[2] = new Bitmap(ico);
	DestroyIcon(ico);

	image = new Bitmap(3 * offset, row2);
	graph = new Graphics(image);

	hWnd = CreateWindow(TEXT("CRPC_PLYR"), TEXT("CarPC play window"), WS_CHILD | WS_CLIPCHILDREN,
		0, 0, pRect.right, pRect.bottom - offset, hPrnt, NULL, hInst, NULL);

	/* Main controls on second row */
	w = (pRect.right - 5 * offset) / 5;
	x = 0;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, row2, w, offset, hWnd, (HMENU)IDC_PREV, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_PREV, IDI_PREV);
	SendDlgItemMessage(hWnd, IDC_PREV, WM_SETREPEAT, 0, (LPARAM)1500);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, row2, w, offset, hWnd, (HMENU)IDC_PLAY, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_PLAY, IDI_PLAY);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, row2, w, offset, hWnd, (HMENU)IDC_STOP, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_STOP, IDI_STOP);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, row2, w, offset, hWnd, (HMENU)IDC_PAUSE, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_PAUSE, IDI_PAUSE);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, row2, (pRect.right - 5 * offset) - x, offset, hWnd, (HMENU)IDC_NEXT, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_NEXT, IDI_NEXT);
	SendDlgItemMessage(hWnd, IDC_NEXT, WM_SETREPEAT, 0, (LPARAM)1500);

	/* Top row controls, left side */
	w = 3 * offset / 2;
	x = 0;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, 0, w, row1, hWnd, (HMENU)IDC_POINTDEC, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_POINTDEC, BM_SETCHECK, BST_INDETERMINATE, 0);
	AssignIcon(hInst, hWnd, IDC_POINTDEC, IDI_THUMBSDOWN);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, 0, (3 * offset) - x, row1, hWnd, (HMENU)IDC_POINTINC, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_POINTINC, BM_SETCHECK, BST_INDETERMINATE, 0);
	AssignIcon(hInst, hWnd, IDC_POINTINC, IDI_THUMBSUP);

	/* Top row controls, right side */
	CreateWindow(WBBUTTON, TEXT("TA"), WS_CHILD | WS_VISIBLE, pRect.right - 7 * offset, 0,
		offset, row1, hWnd, (HMENU)IDC_RDSTA, hInst, NULL);
	//AssignIcon(hInst, hWnd, IDC_RDSTA, IDI_RDS);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 6 * offset, 0,
		offset, row1, hWnd, (HMENU)IDC_EDITCURR, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_EDITCURR, BM_SETCHECK, BST_INDETERMINATE, 0);
	AssignIcon(hInst, hWnd, IDC_EDITCURR, IDI_EDIT);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 5 * offset, 0,
		offset, row1, hWnd, (HMENU)IDC_DELETE, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_DELETE, BM_SETCHECK, BST_INDETERMINATE, 0);
	AssignIcon(hInst, hWnd, IDC_DELETE, IDI_REMOVE);

	CreateWindow(WBBUTTON, TEXT("XF"), WS_CHILD | WS_VISIBLE, pRect.right - 4 * offset, 0,
		offset, row1, hWnd, (HMENU)IDC_XFADE, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_XFADE, BM_SETCHECK, BST_INDETERMINATE, 0);
	//SendDlgItemMessage(hWnd, IDC_XFADE, BM_SETCHECK, (dBase->GetBool("Crossfade") ? BST_CHECKED : BST_UNCHECKED), 0);

	CreateWindow(WBBUTTON, TEXT("SHF"), WS_CHILD | WS_VISIBLE, pRect.right - 3 * offset, 0,
		offset, row1, hWnd, (HMENU)IDC_SHUFFLE, hInst, NULL);
	if(dBase->GetConfig("Shuffle", NULL, CONFIGTYPE_BOOL) == 0)
		dBase->SetConfig("Shuffle", "1", CONFIGTYPE_BOOL);
	SendDlgItemMessage(hWnd, IDC_SHUFFLE, BM_SETCHECK, (dBase->GetBool("Shuffle") ? BST_CHECKED : BST_UNCHECKED), 0);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 2 * offset, 0,
		offset, row1, hWnd, (HMENU)IDC_REPEAT, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_REPEAT, IDI_REPEAT);
	SendDlgItemMessage(hWnd, IDC_REPEAT, BM_SETCHECK, (dBase->GetBool("Repeat") ? BST_CHECKED : BST_UNCHECKED), 0);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 1 * offset, 0,
		offset, row1, hWnd, (HMENU)IDC_INFO, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_INFO, IDI_INFO);

	/* Playlist controls on second row */
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 5 * offset, row2,
		offset, offset, hWnd, (HMENU)IDC_EDIT, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_EDIT, IDI_EDIT);
	SendDlgItemMessage(hWnd, IDC_EDIT, BM_SETCHECK, BST_INDETERMINATE, 0);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 4 * offset, row2,
		offset, offset, hWnd, (HMENU)IDC_PLAYNOW, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_PLAYNOW, IDI_PLAYNOW);
	SendDlgItemMessage(hWnd, IDC_PLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 3 * offset, row2,
		offset, offset, hWnd, (HMENU)IDC_PLAYNEXT, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_PLAYNEXT, IDI_PLAYNEXT);
	SendDlgItemMessage(hWnd, IDC_PLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 2 * offset, row2,
		offset, offset, hWnd, (HMENU)IDC_REMOVE, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_REMOVE, IDI_DEL);
	SendDlgItemMessage(hWnd, IDC_REMOVE, BM_SETCHECK, BST_INDETERMINATE, 0);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 1 * offset, row2,
		offset, offset, hWnd, (HMENU)IDC_REMOVEALL, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_REMOVEALL, IDI_DELALL);

	CreateWindow(TEXT("WB_LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS, 0, row2 + offset,
		pRect.right, pRect.bottom - 2 * offset - row2, hWnd, (HMENU)IDC_COMINGNEXT, hInst, (LPVOID)(2 * offset / 3));
	InitListBox(TRUE, GetDlgItem(hWnd, IDC_COMINGNEXT));
	SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMHEIGHT, 0, (LPARAM)24);

	CreateWindow(TEXT("WB_DESTSLIDER"), NULL, WS_CHILD | WS_VISIBLE, 3 * offset + 2, 3 * row1 - 5,
		ourRect.right - 3 * offset - 4, (offset >> 1) + 2, hWnd, (HMENU)IDC_SEEK, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_SEEK, TBM_SETRANGE, TRUE, MAKELONG(0, 10000));

	CreateWindow(TEXT("WB_TICKERBOX"), NULL, WS_CHILD | WS_VISIBLE, 3 * offset + 2, row1 + 2,
		ourRect.right - 5 * offset - 4, row1 + 6, hWnd, (HMENU)IDC_TICKER, hInst, NULL);
	CreateWindow(TEXT("WB_SPECTRUM"), NULL, WS_CHILD | WS_VISIBLE, ourRect.right - 2 * offset - 3, row1 + 2,
		2 * offset + 1, row1 + 6, hWnd, (HMENU)IDC_SPECTRUM, hInst, NULL);

	CreateWindow(TEXT("WB_TICKERBOX"), NULL, WS_CHILD | WS_VISIBLE, 3 * offset + 2, 2 * row1 + 7,
		ourRect.right - 5 * offset - 4, (offset >> 1) - 8, hWnd, (HMENU)IDC_SUBTICKER, hInst, NULL);
	CreateWindow(TEXT("WB_TICKERBOX"), NULL, WS_CHILD | WS_VISIBLE, ourRect.right - 2 * offset - 3, 2 * row1 + 7,
		2 * offset + 1, (offset >> 1) - 8, hWnd, (HMENU)IDC_RATING, hInst, NULL);

	hCallBack = CreateWindow(TEXT("CRPC_DSHW"), TEXT("DSHOW callback handler"), 0, 0, 0, 0, 0, hWnd, 0, hInst, NULL);

	if(dBase->GetConfig("Popup", NULL, CONFIGTYPE_BOOL) == 0)
		dBase->SetConfig("Popup", "1", CONFIGTYPE_BOOL);
	pPopup = new Popup(hInst, hWnd, offset, &hPopup, dBase);
	SetDlgItemText(hWnd, IDC_TICKER, TEXT("CARGO media player"));
	SetWindowText(hPopup, TEXT("CARGO media player"));

	HRESULT hr = CoInitialize(NULL);
	if(FAILED(hr))
	{
		MessageBox(NULL, TEXT("CoInit failed"), TEXT("Error"), MB_OK);
	}
	else
	{
		//logFile = CreateFile(TEXT("MediaPlayer.log"), FILE_ALL_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		logFile = NULL;
		tTime = 0;
		pSource = NULL;

		/* Retrieve the current playlist */
		dBase->GetNowPlaying(GetDlgItem(hWnd, IDC_COMINGNEXT));
	}
}

PlayWin::~PlayWin()
{
	INT i;

	DestroyWindow(hWnd);
	if(pPopup) delete pPopup;
	InitListBox(FALSE, NULL);
	for(i = 0; i < (sizeof(bg) / sizeof(bg[0])); i++) if(bg[i]) delete bg[i];
	if(pn) delete pn;
	for(i = 0; i < (sizeof(fn) / sizeof(fn[0])); i++) if(fn[i]) delete fn[i];
	for(i = 0; i < (sizeof(bitmap) / sizeof(bitmap[0])); i++) if(bitmap[i]) delete bitmap[i];
	if(graph) delete graph;
	if(image) delete image;
	DShowInit(FALSE);
	CoUninitialize();
}

//IVMRMonitorConfig

HRESULT PlayWin::DShowInit(BOOL loadUnload)
{
	IMediaEventEx * pEventEx;
    IVMRFilterConfig* pVmrConf;
	HRESULT hr;

	if(pShow->pControl)  pShow->pControl->Stop();

	if(pShow->pAudio)    pShow->pAudio->Release();	pShow->pAudio = NULL;
	if(pShow->pSource)   pShow->pSource->Release();	pShow->pSource = NULL;
	if(pShow->pWcontrol) pShow->pWcontrol->Release(); pShow->pWcontrol = NULL;
	if(pShow->pEvent)    pShow->pEvent->Release();    pShow->pEvent = NULL;
	if(pShow->pSeek)     pShow->pSeek->Release();     pShow->pSeek = NULL;
	if(pShow->pControl)  pShow->pControl->Release();  pShow->pControl = NULL;
	if(pShow->pFilter)   pShow->pFilter->Release();	pShow->pFilter = NULL;
	if(pShow->pConfig)   pShow->pConfig->Release();	pShow->pConfig = NULL;
	if(pShow->pGraph)    pShow->pGraph->Release();    pShow->pGraph = NULL;

	if(loadUnload == FALSE)
		return(0);
	else
	{
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
				IID_IGraphBuilder, (void **)&pShow->pGraph);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IGraphBuilder failed"), TEXT("Error"), MB_OK);
			return hr;
		}
		if(logFile)
			pShow->pGraph->SetLogFile((DWORD_PTR)logFile);

		hr = pShow->pGraph->QueryInterface(IID_IMediaControl, (void **)&pShow->pControl);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IMediaControl failed"), TEXT("Error"), MB_OK);
		}
		hr = pShow->pGraph->QueryInterface(IID_IMediaEvent, (void **)&pShow->pEvent);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IMediaEvent failed"), TEXT("Error"), MB_OK);
			return hr;
		}
		hr = pShow->pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pShow->pSeek);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IMediaSeeking failed"), TEXT("Error"), MB_OK);
			return hr;
		}
//		hr = pShow->pGraph->QueryInterface(IID_IGraphConfig, (void **)&pShow->pConfig);
//		if(FAILED(hr))
//		{
//			MessageBox(NULL, TEXT("IID_IGraphConfig failed"), TEXT("Error"), MB_OK);
//			return hr;
//		}
//		hr = pShow->pGraph->QueryInterface(IID_IFilterGraph, (void **)&pShow->pFilter);
//		if(FAILED(hr))
//		{
//			MessageBox(NULL, TEXT("IID_IFilterGraph failed"), TEXT("Error"), MB_OK);
//			return hr;
//		}
		hr = pShow->pGraph->QueryInterface(IID_IMediaEventEx, (void **)&pShow->pEventEx);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IMediaEventEx failed"), TEXT("Error"), MB_OK);
			return hr;
		}
		pShow->pEventEx->SetNotifyWindow((long)hCallBack, WM_DIRECTSHOW, NULL);
		pShow->pEventEx->Release();

		hr = pShow->pGraph->QueryInterface(IID_IBasicAudio, (void **)&pShow->pAudio);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IBasicAudio failed"), TEXT("Error"), MB_OK);
			return hr;
		}
		pShow->pAudio->put_Volume(0);

		// Create the VMR.
		hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void**)&pShow->pVmr);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("CLSID_VideoMixingRenderer failed"), TEXT("Error"), MB_OK);
			return hr;
		}
		if(!pShow->pVmr)
		{
			MessageBox(NULL, TEXT("CLSID_VideoMixingRenderer returned NULL pointer"), TEXT("Error"), MB_OK);
			return(E_POINTER);
		}

		// Add the VMR to the filter graph.
		hr = pShow->pGraph->AddFilter(pShow->pVmr, TEXT("Video Mixing Renderer"));
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("AddFilter on VMR failed"), TEXT("Error"), MB_OK);
			pShow->pVmr->Release();
			return hr;
		}
		// Set the rendering mode.
		hr = pShow->pVmr->QueryInterface(IID_IVMRFilterConfig, (void**)&pShow->pVmrConf);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IVMRFilterConfig failed"), TEXT("Error"), MB_OK);
			pShow->pVmr->Release();
			return(hr);
		}
		hr = pShow->pVmrConf->SetRenderingMode(VMRMode_Windowless);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("SetRenderingMode failed"), TEXT("Error"), MB_OK);
			return(hr);
		}
		pShow->pVmrConf->Release();

		hr = pVmr->QueryInterface(IID_IVMRWindowlessControl, (void**)&pShow->pWcontrol);
        if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IVMRWindowlessControl failed"), TEXT("Error"), MB_OK);
			pShow->pVmr->Release();
			return(hr);
		}
	    pShow->pVmr->Release();
		pShow->pVmr = NULL;

		// Set the target window temporary to ours
		hr = pShow->pWcontrol->SetVideoClippingWindow(hWnd);
        if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("SetVideoClippingWindow failed"), TEXT("Error"), MB_OK);
			return(hr);
		}
		hr = pShow->pWcontrol->SetAspectRatioMode(VMR_ARMODE_LETTER_BOX);
        if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("SetAspectRatioMode failed"), TEXT("Error"), MB_OK);
			return(hr);
		}
		return(hr);
	}
}


HRESULT PlayWin::InitWindowlessVMR(HWND hwndApp)
{
	RECT rcSrc, rcDest;
	long lWidth, lHeight;
	HRESULT hr;

	if(!pShow->pWcontrol)
		return(E_POINTER);

	// Set the window.
    hr = pShow->pWcontrol->SetVideoClippingWindow(hwndApp);
    if (SUCCEEDED(hr))
    {
		hr = pShow->pWcontrol->GetNativeVideoSize(&lWidth, &lHeight, NULL, NULL);
		if(SUCCEEDED(hr) && IsWindowVisible(hwndApp))
		{
			// Set the source rectangle.
			SetRect(&rcSrc, 0, 0, lWidth, lHeight);

			// Get the window client area.
			GetClientRect(hwndApp, &rcDest);

			// Resize video playback to fit screen
			if(dBase->GetBool("Widescreen"))
			{
				if((rcDest.bottom * rcSrc.right) < (rcDest.right * rcSrc.bottom))
				{
					if(rcDest.right)
					{
						lHeight = rcDest.bottom * rcSrc.right / rcDest.right;
						lHeight = rcSrc.bottom - lHeight;
						rcSrc.top += lHeight / 2;
						rcSrc.bottom -= lHeight / 2;
					}
				}
				else
				{
					if(rcDest.bottom)
					{
						lWidth = rcSrc.bottom * rcDest.right / rcDest.bottom;
						lWidth = rcSrc.right - lWidth;
						rcSrc.left += lWidth / 2;
						rcSrc.right -= lWidth / 2;
					}
				}
			}
			else // Resize destination window to fit video aspect ratio
			{
				if((rcDest.bottom * rcSrc.right) < (rcDest.right * rcSrc.bottom))
				{
					if(rcSrc.bottom)
					{
						lWidth = rcDest.bottom * rcSrc.right / rcSrc.bottom;
						lWidth = rcDest.right - lWidth;
						rcDest.left += lWidth / 2;
						rcDest.right -= lWidth / 2;
					}
				}
				else
				{
					if(rcSrc.right)
					{
						lHeight = rcSrc.bottom * rcDest.right / rcSrc.right;
						lHeight = rcDest.bottom - lHeight;
						rcDest.top += lHeight / 2;
						rcDest.bottom -= lHeight / 2;
					}
				}
			}

			// Resize video playback for 800/640 wider screen
			if(dBase->GetBool("LilliputMode"))
			{
				lHeight = rcSrc.bottom - rcSrc.top;
				lHeight = (lHeight * 640) / 800;
				lHeight = (rcSrc.bottom - rcSrc.top) - lHeight;
				rcSrc.top += lHeight / 2;
				rcSrc.bottom -= lHeight / 2;
			}

			pShow->pWcontrol->SetAspectRatioMode(VMR_ARMODE_NONE);
			// Set the video position.
			hr = pShow->pWcontrol->SetVideoPosition(&rcSrc, &rcDest);
			if(FAILED(hr))
				MessageBox(NULL, TEXT("SetVideoPosition failed"), TEXT("Error"), MB_OK);
		}
		else if(FAILED(hr))
			MessageBox(NULL, TEXT("GetNativeVideoSize failed"), TEXT("Error"), MB_OK);
	}

    hr = pShow->pWcontrol->SetBorderColor(RGB(0,0,0));
	if(FAILED(hr))
	{
		MessageBox(NULL, TEXT("SetBorderColor failed"), TEXT("Error"), MB_OK);
		return(hr);
	}
    return hr;
}

PlayWin::Show(BOOL yesNo)
{
	if(yesNo)
	{
		ShowWindow(hWnd, SW_SHOW);
		SendDlgItemMessage(hWnd, IDC_RDSTA, BM_SETCHECK, (pAi->vTA ? BST_CHECKED : BST_UNCHECKED), 0);

		if(SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0) == 0)
			SetTimer(hWnd, 3, 1000, NULL);
	}
	else
	{
		ShowWindow(hWnd, SW_HIDE);
		vHelpActive = FALSE;
		SendDlgItemMessage(hWnd, IDC_INFO, BM_SETCHECK, BST_UNCHECKED, 0);
		if(vOldCursor)
			SetCursor(vOldCursor);
		vOldCursor = NULL;
		if(hWndHelp)
			DestroyWindow(hWndHelp);
		hWndHelp = NULL;
	}
}

ATOM PlayWin::MyRegisterClass(HINSTANCE hInstance)
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
	wcex.hbrBackground	= CreateSolidBrush(RGB(200, 255, 200));
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_PLYR");
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);

	wcex.lpfnWndProc	= DShowProc;
	wcex.lpszClassName	= TEXT("CRPC_DSHW");

	return RegisterClassEx(&wcex);
}

void PlayWin::InitDC(BOOL updateDC)
{
	if(myGraphics == NULL)
	{
	   myGraphics = new Graphics(hCbWnd, FALSE);
	}
	else if(updateDC == TRUE)
	{
	   if(myGraphics) delete myGraphics;
	   myGraphics = new Graphics(hCbWnd, FALSE);
	}
	GetClientRect(hCbWnd, &ourRect);
}

void PlayWin::Paint()
{
	PointF point;
	REFERENCE_TIME units;
	int pos, time;
	WCHAR  vBuffer[20];
	StringFormat fmt;
	OAFilterState	state;

	fmt.SetAlignment(StringAlignmentFar);
	fmt.SetTrimming(StringTrimmingNone);
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);

	if(!myGraphics || !graph || !image || !fn[1] || !bg[BG_DBLUE] || !bg[BG_WHITE] || !pn)
		return;

	tTime = sCurrentMedia.vPlayTime * 10000000;
	if(pShow->pSeek && (tTime == 0))
		pShow->pSeek->GetDuration(&tTime);

	graph->FillRectangle(bg[BG_WHITE], 2, row1 + 2, 3 * offset - 3, row2 - row1 - 5);
	graph->DrawRectangle(pn, 2, row1 + 2, 3 * offset - 3, row2 - row1 - 5);

	// Print total track time
	point.X = (float)3 * offset - 4;
	point.Y = (float)(3 * row1 - 2);
	time = (int)(tTime / 10000000);	// from 100ns to 1s
	if(time > 3600)
		StringCbPrintf(vBuffer, sizeof(vBuffer), TEXT("%ld:%02ld:%02ld"), time / 3600, (time % 3600) / 60, time % 60);
	else
		StringCbPrintf(vBuffer, sizeof(vBuffer), TEXT("%ld:%02ld"), time / 60, time % 60);
	graph->DrawString(vBuffer, -1, fn[1], point, &fmt, bg[BG_DBLUE]);

	// Print remaining track time
	if(pShow->pSeek)
		pShow->pSeek->GetCurrentPosition(&units); else units = 0;
	point.Y = (float)(2 * row1 + 9);
	pos = (int)(units / 10000000);	// from 100ns to 1s
	if(time > 3600)
		StringCbPrintf(vBuffer, sizeof(vBuffer), TEXT("-%ld:%02ld:%02ld"), abs((time - pos) / 3600), abs(((time - pos) % 3600) / 60), abs((time - pos) % 60));
	else
		StringCbPrintf(vBuffer, sizeof(vBuffer), TEXT("-%ld:%02ld"), abs((time - pos) / 60), abs((time - pos) % 60));
	graph->DrawString(vBuffer, -1, fn[1], point, &fmt, bg[BG_DBLUE]);

	// Print current track position
	point.X = (float)3 * offset;
	point.Y = (float)(row1 + 2);
	if(time > 3600)
		StringCbPrintfW(vBuffer, sizeof(vBuffer), L"%ld:%02ld:%02ld", pos / 3600, (pos % 3600) / 60, pos % 60);
	else
		StringCbPrintfW(vBuffer, sizeof(vBuffer), L"%ld:%02ld", pos / 60, pos % 60);
	graph->DrawString(vBuffer, -1, fn[0], point, &fmt, bg[BG_DBLUE]);

	if(tTime && time)
		SendDlgItemMessage(hWnd, IDC_SEEK, TBM_SETPOS, TRUE, (pos * 10000) / time);
	else
		SendDlgItemMessage(hWnd, IDC_SEEK, TBM_SETPOS, TRUE, 0);

	// Pass duration and position to mainwindow (for dist. to video window)
	SendMessage(my->hPrnt, WM_DSHOWTIME, time, pos);

	if(pShow->pControl)
	{
		pShow->pControl->GetState(10, &state);
		switch(state)
		{
			case State_Paused:
				if(bitmap[2])
					graph->DrawImage(bitmap[2], 5, row2 - offset + 5);
				break;
			case State_Running:
				if(bitmap[1])
					graph->DrawImage(bitmap[1], 5, row2 - offset + 5);
				break;
			case State_Stopped:
			default:
				if(bitmap[0])
					graph->DrawImage(bitmap[0], 5, row2 - offset + 5);
				break;
		}
	}

	myGraphics->DrawImage(image, 0, 0);

	// Restart refresh timer
	SetTimer(hWnd, 1, 1000, NULL);
}

void PlayWin::Pause(BOOL vPause)
{
	OAFilterState state;

	if(pShow->pControl)
	{
		pShow->pControl->GetState(10, &state);
		if((state == State_Running) && (vPause == TRUE))
		{
			pShow->pControl->Pause();
			// Disable the skip buttons
			SendDlgItemMessage(hCbWnd, IDC_PREV, BM_SETCHECK, BST_INDETERMINATE, 0);
			SendDlgItemMessage(hCbWnd, IDC_NEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
			// Set the pause button activated
			SendDlgItemMessage(hCbWnd, IDC_PAUSE, BM_SETCHECK, BST_CHECKED, 0);
			SendDlgItemMessage(hPopup, IDC_PAUSE, BM_SETCHECK, BST_CHECKED, 0);
		}
		//else if((state == State_Paused) && (vPause == FALSE))
		else if(vPause == FALSE)
		{
			pShow->pControl->Run();
			// Reenable the skip buttons
			SendDlgItemMessage(hCbWnd, IDC_PREV, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hCbWnd, IDC_NEXT, BM_SETCHECK, BST_UNCHECKED, 0);
			// Put the pause btton back to default state
			SendDlgItemMessage(hCbWnd, IDC_PAUSE, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hPopup, IDC_PAUSE, BM_SETCHECK, BST_UNCHECKED, 0);
		}
	}
}

void PlayWin::FillPlayList()
{
	static BOOL	initDialogShown = FALSE;
	RECT		lbRect;
	INT			i, n;

	if(!dBase)
		return;

	GetWindowRect(GetDlgItem(hWnd, IDC_COMINGNEXT), &lbRect);
	lbRect.bottom -= lbRect.top;
	i = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMHEIGHT, 0, 0);
	if(i > 0)
		n = lbRect.bottom / i;
	else n = 0;

	i = 0;
	while((SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0) < n) && (i++ < (5 * n)))
	{
		if(SendDlgItemMessage(hWnd, IDC_SHUFFLE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)	// NO shuffle
			dBase->GetAutoPlay(FALSE, &autoLast, GetDlgItem(hWnd, IDC_COMINGNEXT));
		else
			dBase->GetAutoPlay(TRUE, &autoLast, GetDlgItem(hWnd, IDC_COMINGNEXT));
	}
	if((SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0) - SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETTOPINDEX, 0, 0)) < n)
		SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETTOPINDEX, (WPARAM)(SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0) - n), 0);

	if(SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0) == 0)
		SetTimer(hWnd, 3, 1000, NULL);
}

void PlayWin::Play(PMediaDescr pMedia, BOOL vStart)
{
	REFERENCE_TIME	pos = 0;
	HRESULT			hr;
	INT				vLen;
	LPSTR			sFile;
	LONG			lWidth, lHeight;
	LPTSTR			pFile = NULL;
	HRESULT			vStat;
	DWORD			cn;
	LPTSTR			pString;
	int				vBackup;

	if(!pMedia || !dBase)
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

		if(logFile)
		{
			WriteFile(logFile, "\n\r\n\r", 4, &cn, NULL);
			WriteFile(logFile, sFile, vLen, &cn, NULL);
			WriteFile(logFile, "\n\r\n\r", 4, &cn, NULL);
		}

		// Songname
		pString = dBase->DescriptorToDoubleString(pMedia, (MediaField)(FLD_ARTIST | FLD_TITLE));
		if(pString)
		{
	#ifdef UNICODE
			StringCbPrintfW(szCurrent, sizeof(szCurrent), L"%ws", pString);
	#else
			StringCbPrintfW(szCurrent, sizeof(szCurrent), L"%hs", pString);
	#endif //UNICODE
			GlobalFree((HGLOBAL)pString);
		}

		// Info bar
		vBackup = pMedia->vArtistId;
		pMedia->vArtistId = 0;
		pString = dBase->DescriptorToSingleString(pMedia, (MediaField)FLD_ALBUM);
		pMedia->vArtistId = vBackup;
		if(pString)
		{
	#ifdef UNICODE
			StringCbPrintfW(szInfo, sizeof(szInfo), L" %ws", pString);
	#else
			StringCbPrintfW(szInfo, sizeof(szInfo), L" %hs", pString);
	#endif //UNICODE
			GlobalFree((HGLOBAL)pString);
		}

		// Add year to info bar
		pString = dBase->DescriptorToSingleString(pMedia, (MediaField)FLD_YEAR);
		if(pString)
		{
			LPWSTR	pYear;
			INT		vYear;
			size_t	len;

			if(pString[0] && (pString[0] != '0'))
			{
				vYear = _ttoi(pString);
				if(vYear != 0)
				{
					StringCbLengthW(szInfo, sizeof(szInfo), &len);
					pYear = &szInfo[len / sizeof(WCHAR)];
					if(vYear >= 0)
						StringCbPrintfW(pYear, sizeof(szInfo) - len, L" - AD %d", vYear);
					else
						StringCbPrintfW(pYear, sizeof(szInfo) - len, L" - %d BC", abs(vYear));
				}
			}
			GlobalFree((HGLOBAL)pString);
		}

		if(pMedia->vRating > 0)
			StringCbPrintfW(szRating, sizeof(szRating), L"Score: +%d",  pMedia->vRating);
		else
			StringCbPrintfW(szRating, sizeof(szRating), L"Score: %d",  pMedia->vRating);

		if(pGraph && szFile)
		{
			vStat = pGraph->RenderFile(szFile, 0);
			if(FAILED(vStat))
			{
				sCurrentMedia.vInvalid = 1;
				if(logFile)
				{
					StringCchPrintfA((LPSTR)szFile, 100, "Render failed with val %d", vStat);
					WriteFile(logFile, szFile, lstrlen(szFile), &cn, NULL);
					WriteFile(logFile, "\n\r\n\r\n\r\n\r", 8, &cn, NULL);
				}
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

		if(pControl && vStart)
			pControl->Run();

		// Update display texts
		SetDlgItemText(hWnd, IDC_TICKER, szCurrent);
		SetWindowText(hPopup, szCurrent);
		SetDlgItemText(hWnd, IDC_SUBTICKER, szInfo);
		SetDlgItemText(hWnd, IDC_RATING, szRating);

		if(vStart && pPopup)
			pPopup->Show(TRUE);

		if(pWcontrol)
		{
			hr = pWcontrol->GetNativeVideoSize(&lWidth, &lHeight, 0, 0);
			if((hr != E_NOINTERFACE) && (lWidth != 0) && (lHeight != 0))
			{
				// If this video is encoded with an unsupported codec,
				// we won't see any video, although the audio will work if it is
				// of a supported format.
				vVideo = TRUE;
				SetWindowText(hVideo, szCurrent);
			}
			else if(vVideo)
			{
				vVideo = FALSE;
				SendMessage(my->hPrnt, WM_SHOWPLAYERWINDOW, (WPARAM)0, (LPARAM)0);
			}
			SendMessage(hPrnt, WM_SHOWVIDEOWINDOW, (WPARAM)0, (LPARAM)vVideo);
		}
		/* Enable score buttons */
		SendDlgItemMessage(hWnd, IDC_POINTDEC, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hWnd, IDC_POINTINC, BM_SETCHECK, BST_UNCHECKED, 0);

		/* Enable current song delete/edit controls */
		if(hWndRemove == NULL)
		{
			SendDlgItemMessage(hWnd, IDC_DELETE, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hWnd, IDC_EDITCURR, BM_SETCHECK, BST_UNCHECKED, 0);
		}
	}
	FillPlayList();
	GlobalFree(pMedia);
}

void PlayWin::SaveSettings()
{
	OAFilterState	state;
	REFERENCE_TIME	units;
	LONG			lastPos;
	PMediaDescr	   *pMedia;
	INT				i;

	if(saved)
		return;

	// Save the current playing file & position
	if(pControl)
	{
		pControl->GetState(10, &state);
		if(state == State_Running)
			pControl->Stop();
	}
	dBase->SetConfig("CurrentState", (LPVOID)&state, CONFIGTYPE_LONG);

	/* Save the current track */
	dBase->SetConfig("CurrentMedia", (LPVOID)&sCurrentMedia.vMediaId, CONFIGTYPE_INT);
	if(pSeek)
		pSeek->GetCurrentPosition(&units);
	else
		units = 0;
	units /= 10000000;
	lastPos = (LONG)units;
	dBase->SetConfig("CurrentPosition", (LPVOID)&lastPos, CONFIGTYPE_LONG);

	// Save the current playlist
	i = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0);
	pMedia = (PMediaDescr*)GlobalAlloc(GPTR, (i+1) * sizeof(PMediaDescr));
	while(i)
	{
		i--;
		pMedia[i] = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)i, 0);
		if((LRESULT)pMedia == LB_ERR)
			pMedia[i] = NULL;
	}
	if(pMedia[0])
		dBase->SaveNowPlaying(pMedia);
	GlobalFree((HGLOBAL)pMedia);
	saved = TRUE;
}

void PlayWin::RetrieveSettings()
{
	LPBOOL			lpToggle = NULL;
	OAFilterState	*state = NULL;
	LPLONG			lastPos = NULL;
	REFERENCE_TIME	units;
	PMediaDescr		pMedia;
	LPINT			pInt;
//	DWORD			vCapabilities;

	/* Retrieve current track */
	pInt = &sCurrentMedia.vMediaId;
	dBase->GetConfig("CurrentMedia", (LPVOID*)&pInt, CONFIGTYPE_INT);
	pMedia = dBase->GetMedia(sCurrentMedia.vMediaId);
	sCurrentMedia.vMediaId = 0;
	Play(pMedia, FALSE);

	/* Retrieve current track postion and state */
	units = 0;
	if(dBase->GetConfig("CurrentPosition", (LPVOID*)&lastPos, CONFIGTYPE_LONG))
	{
		units = *lastPos;
		units *= 10000000;
		dBase->Free((LPVOID)lastPos);
	}
	if(pSeek)
	{
		pSeek->GetDuration(&tTime);
	//	vCapabilities = AM_SEEKING_CanSeekAbsolute | AM_SEEKING_CanSeekForwards | AM_SEEKING_CanSeekBackwards;
	//	if(pSeek->CheckCapabilities(&vCapabilities) != E_FAIL)
		if(my->pSeek->IsFormatSupported(&TIME_FORMAT_MEDIA_TIME) == S_OK)
			pSeek->SetPositions(&units, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
	}
	if(dBase->GetConfig("CurrentState", (LPVOID*)&state, CONFIGTYPE_LONG))
	{
		if(*state != State_Stopped)
			if(pControl)
				pControl->Run();
		if(*state == State_Paused)
			PostMessage(pAi->hApp, WM_PAUSE, 0, TRUE);
		dBase->Free((LPVOID)state);
	}
	saved = FALSE;
}


LRESULT CALLBACK DShowProc(HWND hWnd2, UINT message, WPARAM wParam, LPARAM lParam)
{
	REFERENCE_TIME	pos;
	LONG			lEvCode, lParam1, lParam2;
	PMediaDescr     pMedia, pCopy;
	LPTSTR			pString;
	LRESULT			id;

	switch(message)
	{
		case WM_DIRECTSHOW:
			if(my->pEvent)
			{
				while(my->pEvent->GetEvent(&lEvCode, &lParam1, &lParam2, 0) == S_OK)
				{
					if(lEvCode == EC_COMPLETE)
					{
						// Disable rating buttons
						SendDlgItemMessage(my->hWnd, IDC_POINTDEC, BM_SETCHECK, BST_INDETERMINATE, 0);
						SendDlgItemMessage(my->hWnd, IDC_POINTINC, BM_SETCHECK, BST_INDETERMINATE, 0);
						// Disable edit and delete buttons
						if(hWndRemove == NULL)
							SendDlgItemMessage(my->hWnd, IDC_DELETE, BM_SETCHECK, BST_INDETERMINATE, 0);
						SendDlgItemMessage(my->hWnd, IDC_EDITCURR, BM_SETCHECK, BST_INDETERMINATE, 0);

						// Save actual playtime as a workaround for the sometimes wrong directX calculation
						if(my->pSeek)
							my->pSeek->GetCurrentPosition(&pos); else pos = 0;
						if(pos)
						{
							pos /= 10000000;	// from 100ns to 1s
							sCurrentMedia.vPlayTime = (INT)pos;
							dBase->ChangeTrack(&sCurrentMedia);
						}

						my->pEvent->FreeEventParams(lEvCode, lParam1, lParam2);
						my->DShowInit(FALSE);

						id = SendDlgItemMessage(my->hWnd, IDC_REPEAT, BM_GETCHECK, 0, 0);
						if(id == BST_CHECKED)
						{
							pCopy = dBase->CopyDescr(&sCurrentMedia);
							pCopy->vHistory = -1;
						}

						pMedia = (PMediaDescr)SendDlgItemMessage(my->hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)0, 0);
						if(pMedia && ((LRESULT)pMedia != LB_ERR))
						{
							my->Play(pMedia, TRUE);
							SendDlgItemMessage(my->hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)0, 0);
						}
						SendDlgItemMessage(my->hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)0, 0);

						if(id == BST_CHECKED)
						{
							// Reinsert string
							pString = dBase->DescriptorToDoubleString(pCopy, (MediaField)(FLD_ARTIST | FLD_TITLE));
							id = SendDlgItemMessage(my->hWnd, IDC_COMINGNEXT, LB_ADDSTRING, 0, (LPARAM)pString);
							GlobalFree((HGLOBAL)pString);
							SendDlgItemMessage(my->hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)id, (LPARAM)pCopy);
						}

						id = SendDlgItemMessage(my->hWnd, IDC_COMINGNEXT, LB_GETTOPINDEX, 0, 0);
						if(id--)
							SendDlgItemMessage(my->hWnd, IDC_COMINGNEXT, LB_SETTOPINDEX, id, 0);

						if(my->vNextIndex)
							my->vNextIndex--;

						my->FillPlayList();
						break;
					}
					else
						my->pEvent->FreeEventParams(lEvCode, lParam1, lParam2);
				}
			}
			break;

		default:
			return(DefWindowProc(hWnd2, message, wParam, lParam));
   }
   return(0);
}

LRESULT CALLBACK PlayWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL	initDialogShown = FALSE;
	PAINTSTRUCT		ps;
    RECT			ourRect;
	REFERENCE_TIME	pos;
	OAFilterState	state;
	DWORD			Position;
	LONG			curSel;
	LPWSTR			pText;
	INT				size;
	LRESULT			id;
	PMediaDescr     pMedia, pCopy;
	INT             x, w;
	LPTSTR			pString;
	BOOL            toggle;


	GetClientRect(hWnd, &ourRect);
	if(my->pControl)
		my->pControl->GetState(10, &state);

	switch(message)
	{
		case WM_CREATE:
			my->hCbWnd = hWnd;
			my->InitDC(FALSE);
			SendMessage(my->hPrnt, WM_SETREGISTERFINALWND, (WPARAM)IDW_PLR, (LPARAM)hWnd);
			my->vHelpActive = FALSE;
			my->vOldCursor = NULL;
			my->hWndHelp = NULL;
			RegisterHelpClass(my->hInst);
			break;

		case WM_COMMAND:
			if((LOWORD(wParam) == IDC_INFO) && (HIWORD(wParam) == BN_CLICKED))
			{
				my->vHelpActive = (SendDlgItemMessage(hWnd, IDC_INFO, BM_GETCHECK, 0, 0) == BST_UNCHECKED ? TRUE : FALSE);
				SendDlgItemMessage(hWnd, IDC_INFO, BM_SETCHECK, (my->vHelpActive ? BST_CHECKED : BST_UNCHECKED), 0);
				my->vOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

				if(my->vHelpActive)
				{
				    RECT		winRect;

					GetWindowRect(GetDlgItem(hWnd, IDC_COMINGNEXT), &winRect);
					my->hWndHelp = CreateWindow(TEXT("CRPC_HELP"), NULL, WS_POPUP | WS_DLGFRAME | WS_VISIBLE, winRect.left + 2 * my->offset, winRect.top + my->offset / 2,
									ourRect.right - 4 * my->offset, winRect.bottom - winRect.top - my->offset + 6, hWnd, 0, my->hInst, (LPVOID)my->offset);
				}
				else
				{
					KillTimer(hWnd, 2);
					if(my->vOldCursor)
						SetCursor(my->vOldCursor);
					my->vOldCursor = NULL;
					if(my->hWndHelp)
						DestroyWindow(my->hWndHelp);
					my->hWndHelp = NULL;
					return(0);
				}
			}
			if(my->vHelpActive)
			{
				SetTimer(hWnd, 2, 5000, NULL);
				// Show help window with text
				return(0);
			}
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
								if(my->hPopup)
									SendDlgItemMessage(my->hPopup, IDC_PAUSE, BM_SETCHECK, BST_UNCHECKED, 0);
								// Reenable the skip buttons
								SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, BST_UNCHECKED, 0);
								SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, BST_UNCHECKED, 0);
							}
							break;

						case IDC_STOP:
							if(my->pControl)
								my->pControl->Stop();
							pos = 0;
							if(my->pSeek)
								my->pSeek->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
							SendDlgItemMessage(hWnd, IDC_PAUSE, BM_SETCHECK, BST_UNCHECKED, 0);
							if(my->hPopup)
								SendDlgItemMessage(my->hPopup, IDC_PAUSE, BM_SETCHECK, BST_UNCHECKED, 0);
							// Reenable the skip buttons
							SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, BST_UNCHECKED, 0);
							SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, BST_UNCHECKED, 0);
							break;

						case IDC_PAUSE:
							id = SendDlgItemMessage(hWnd, IDC_PAUSE, BM_GETCHECK, 0, 0);
							SendMessage(my->hPrnt, WM_PAUSE, 0, (id == BST_UNCHECKED ? TRUE : FALSE));
							break;

						case IDC_NEXT:
							dBase->AdjustRating(&sCurrentMedia, RATE_NEXT);
							id = SendDlgItemMessage(hWnd, IDC_REPEAT, BM_GETCHECK, 0, 0);
							if(id == BST_CHECKED)
								pCopy = dBase->CopyDescr(&sCurrentMedia);

							pMedia = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)0, 0);
							if(pMedia && ((LRESULT)pMedia != LB_ERR))
							{
								my->Play(pMedia, TRUE);
								SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)0, 0);
							}
							SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)0, 0);
							if(id == BST_CHECKED)
							{
								// Reinsert string
								pString = dBase->DescriptorToDoubleString(pCopy, (MediaField)(FLD_ARTIST | FLD_TITLE));
								id = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_ADDSTRING, 0, (LPARAM)pString);
								GlobalFree((HGLOBAL)pString);
								SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)id, (LPARAM)pCopy);
							}

							id = SendDlgItemMessage(my->hWnd, IDC_COMINGNEXT, LB_GETTOPINDEX, 0, 0);
							if(id--)
								SendDlgItemMessage(my->hWnd, IDC_COMINGNEXT, LB_SETTOPINDEX, id, 0);

							if(my->vNextIndex)
								my->vNextIndex--;

							my->FillPlayList();
							break;

						case IDC_PREV:
							pCopy = dBase->CopyDescr(&sCurrentMedia);	// Save current object
							if(pCopy)
							{
								pMedia = dBase->GetHistory(&sCurrentMedia);	// Get history object
								if(pMedia)
								{
									// Reinsert current in to coming up list
									pCopy->vHistory = pMedia->vHistory + 1;
									pString = dBase->DescriptorToDoubleString(pCopy, (MediaField)(FLD_ARTIST | FLD_TITLE));
									id = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_INSERTSTRING, 0, (LPARAM)pString);
									GlobalFree((HGLOBAL)pString);
									SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)id, (LPARAM)pCopy);

									my->Play(pMedia, TRUE);
								}
							}
							break;

						// rating controls
						case IDC_POINTDEC:
							SendDlgItemMessage(hWnd, IDC_POINTDEC, BM_SETCHECK, BST_INDETERMINATE, 0);
							dBase->AdjustRating(&sCurrentMedia, RATE_DECR);
							if(sCurrentMedia.vRating > 0)
								StringCbPrintfW(szRating, sizeof(szRating), L"New score: +%d",  sCurrentMedia.vRating);
							else
								StringCbPrintfW(szRating, sizeof(szRating), L"New score: %d",  sCurrentMedia.vRating);
							SetDlgItemText(hWnd, IDC_RATING, szRating);
							break;

						case IDC_POINTINC:
							SendDlgItemMessage(hWnd, IDC_POINTINC, BM_SETCHECK, BST_INDETERMINATE, 0);
							dBase->AdjustRating(&sCurrentMedia, RATE_INCR);
							if(sCurrentMedia.vRating > 0)
								StringCbPrintfW(szRating, sizeof(szRating), L"New score: +%d",  sCurrentMedia.vRating);
							else
								StringCbPrintfW(szRating, sizeof(szRating), L"New score: %d",  sCurrentMedia.vRating);
							SetDlgItemText(hWnd, IDC_RATING, szRating);
							break;

						// special controls
						case IDC_RDSTA:
							if(!dBase->GetBool("AntennaVaux"))
							{
								my->pAi->vTA = !my->pAi->vTA;
								dBase->SetConfig("TrafficAnnouncement", (LPVOID)&my->pAi->vTA, CONFIGTYPE_BOOL);
								SendDlgItemMessage(hWnd, IDC_RDSTA, BM_SETCHECK, (my->pAi->vTA ? BST_CHECKED : BST_UNCHECKED), 0);
							}
							break;

						case IDC_EDITCURR:
							break;

						case IDC_DELETE:
							if(hWndRemove == NULL)
							{
								RemoveParm	vParm;
							    RECT		winRect;

								vParm.hDb = dBase;
								vParm.pMedia = &sCurrentMedia;
								vParm.offset = my->offset;
								GetWindowRect(hWnd, &winRect);
								RegisterRemoveClass(my->hInst);
								hWndRemove = CreateWindow(TEXT("CRPC_REMOVE"), NULL, WS_POPUP | WS_DLGFRAME | WS_VISIBLE, winRect.left + ourRect.right / 5, winRect.top + ourRect.bottom / 5,
															3 * ourRect.right / 5, 3 * ourRect.bottom / 5, hWnd, 0, my->hInst, (LPVOID)&vParm);

								SendDlgItemMessage(hWnd, IDC_DELETE, BM_SETCHECK, BST_CHECKED, 0);
								SendDlgItemMessage(hWnd, IDC_EDITCURR, BM_SETCHECK, BST_INDETERMINATE, 0);
							}
							else
							{
								PostMessage(hWndRemove, WM_CLOSE, 0, 0);
								hWndRemove = NULL;

								SendDlgItemMessage(hWnd, IDC_DELETE, BM_SETCHECK, BST_UNCHECKED, 0);
								SendDlgItemMessage(hWnd, IDC_EDITCURR, BM_SETCHECK, BST_UNCHECKED, 0);
							}
							break;

						case IDC_XFADE:
							toggle = (SendDlgItemMessage(hWnd, IDC_XFADE, BM_GETCHECK, 0, 0) == BST_UNCHECKED ? TRUE : FALSE);
							dBase->SetConfig("Crossfade", (LPVOID)&toggle, CONFIGTYPE_BOOL);
							SendDlgItemMessage(hWnd, IDC_XFADE, BM_SETCHECK, (toggle ? BST_CHECKED : BST_UNCHECKED), 0);
							break;

						case IDC_SHUFFLE:
							toggle = (SendDlgItemMessage(hWnd, IDC_SHUFFLE, BM_GETCHECK, 0, 0) == BST_UNCHECKED ? TRUE : FALSE);
							dBase->SetConfig("Shuffle", (LPVOID)&toggle, CONFIGTYPE_BOOL);
							SendDlgItemMessage(hWnd, IDC_SHUFFLE, BM_SETCHECK, (toggle ? BST_CHECKED : BST_UNCHECKED), 0);
							break;

						case IDC_REPEAT:
							toggle = (SendDlgItemMessage(hWnd, IDC_REPEAT, BM_GETCHECK, 0, 0) == BST_UNCHECKED ? TRUE : FALSE);
							dBase->SetConfig("Repeat", (LPVOID)&toggle, CONFIGTYPE_BOOL);
							SendDlgItemMessage(hWnd, IDC_REPEAT, BM_SETCHECK, (toggle ? BST_CHECKED : BST_UNCHECKED), 0);
							break;

						// Now playing controls
						case IDC_EDIT:
							break;

						case IDC_PLAYNOW:
							curSel = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCURSEL, 0, 0);
							if(curSel != LB_ERR)
							{
								id = SendDlgItemMessage(hWnd, IDC_REPEAT, BM_GETCHECK, 0, 0);
								if(id == BST_CHECKED)
									pCopy = dBase->CopyDescr(&sCurrentMedia);

								pMedia = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)curSel, 0);
								if(pMedia && ((LRESULT)pMedia != LB_ERR))
								{
									dBase->AdjustRating(pMedia, RATE_PLAYNOW);
									my->Play(pMedia, TRUE);

									if((id == BST_CHECKED) && pCopy)
									{
										// Reinsert string
										pString = dBase->DescriptorToDoubleString(pCopy, (MediaField)(FLD_ARTIST | FLD_TITLE));
										id = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_ADDSTRING, 0, (LPARAM)pString);
										GlobalFree((HGLOBAL)pString);
										SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)id, (LPARAM)pCopy);
									}
									SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)curSel, (LPARAM)NULL);
									// un-pause if paused
									id = SendDlgItemMessage(hWnd, IDC_PAUSE, BM_GETCHECK, 0, 0);
									if(id == BST_CHECKED)
										SendMessage(my->hPrnt, WM_PAUSE, 0, FALSE);
								}
								SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)curSel, 0);
								SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETCURSEL, (WPARAM)-1, 0);
								SendDlgItemMessage(hWnd, IDC_EDIT, BM_SETCHECK, BST_INDETERMINATE, 0);
								SendDlgItemMessage(hWnd, IDC_PLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
								SendDlgItemMessage(hWnd, IDC_PLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
								SendDlgItemMessage(hWnd, IDC_REMOVE, BM_SETCHECK, BST_INDETERMINATE, 0);
								my->FillPlayList();
							}
							break;

						case IDC_PLAYNEXT:
							curSel = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCURSEL, 0, 0);
							if(curSel != LB_ERR)
							{
								// Get the original text
								size = 2 * SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETTEXTLEN, (WPARAM)curSel, 0) + 2;
								pText = (LPWSTR)HeapAlloc(my->hHeap, HEAP_ZERO_MEMORY, size);
								SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETTEXT, (WPARAM)curSel, (LPARAM)pText);
								// Get the original data block
								pMedia = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)curSel, 0);
								if((LRESULT)pMedia != LB_ERR)
								{
									if(pMedia->vNoRemove == FALSE)
									{
										pMedia->vNoRemove = TRUE;
										// Remove the old line
										SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)curSel, (LPARAM)NULL);
										SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)curSel, 0);

										// Re-insert the data at top location
										SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_INSERTSTRING, (WPARAM)my->vNextIndex, (LPARAM)pText);
										SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)my->vNextIndex, (LPARAM)pMedia);
										my->vNextIndex++;
									}
									else
									{
										// Remove the old line
										SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)curSel, (LPARAM)NULL);
										SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)curSel, 0);

										// Re-insert the data at top location
										SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_INSERTSTRING, (WPARAM)0, (LPARAM)pText);
										SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)0, (LPARAM)pMedia);
									}
									if((LRESULT)pMedia != LB_ERR)
										dBase->AdjustRating(pMedia, RATE_PLAYNEXT);
								}
								// Change the buton states
								SendDlgItemMessage(hWnd, IDC_EDIT, BM_SETCHECK, BST_INDETERMINATE, 0);
								SendDlgItemMessage(hWnd, IDC_PLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
								SendDlgItemMessage(hWnd, IDC_PLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
								SendDlgItemMessage(hWnd, IDC_REMOVE, BM_SETCHECK, BST_INDETERMINATE, 0);
								my->FillPlayList();
							}
							break;
						case IDC_REMOVE:
							curSel = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCURSEL, 0, 0);
							if(curSel != LB_ERR)
							{
								pMedia = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)curSel, 0);
								if((LRESULT)pMedia != LB_ERR)
									dBase->AdjustRating(pMedia, RATE_REMOVE);
								SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)curSel, 0);
								SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETCURSEL, (WPARAM)curSel, 0);
							}
							my->FillPlayList();
							break;
						case IDC_REMOVEALL:
							curSel = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCURSEL, 0, 0);
							if(curSel != LB_ERR)
							{
								pMedia = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)curSel, 0);
								if(pMedia && ((LRESULT)pMedia != LB_ERR))
									curSel = pMedia->vMediaId;
								else
									curSel = 0;
							}
							else
								curSel = 0;

							id = 0;
							while(1)
							{
								pMedia = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)id, 0);
								if(pMedia && ((LRESULT)pMedia != LB_ERR))
								{
									if(pMedia->vMediaId == curSel)
									{
										dBase->AdjustRating(pMedia, RATE_PLAYNEXT);
										pMedia->vNoRemove = TRUE;
										SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETCURSEL, (WPARAM)id, 0);
										id++;
									}
									else if(pMedia->vNoRemove == FALSE)
									{
										dBase->AdjustRating(pMedia, RATE_REMOVEALL);
										SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)id, 0);
									}
									else
										id++;
								}
								else
									break;
							}
							my->FillPlayList();
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

				// Nowplaying listbox events
				case LBN_DBLCLK:
			/*		curSel = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCURSEL, 0, 0);
					if(curSel != LB_ERR)
					{
						szText = (LPWSTR)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)curSel, 0);
						my->PlayNow(&szText[lstrlen(szText) + 1]);
						SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)curSel, 0);
						SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETCURSEL, (WPARAM)-1, 0);
						SendDlgItemMessage(hWnd, IDC_EDIT, BM_SETCHECK, BST_INDETERMINATE, 0);
						SendDlgItemMessage(hWnd, IDC_PLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
						SendDlgItemMessage(hWnd, IDC_PLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
						SendDlgItemMessage(hWnd, IDC_REMOVE, BM_SETCHECK, BST_INDETERMINATE, 0);
						my->FillPlayList();
					}
			*/		break;
				case LBN_SELCHANGE:
					curSel = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCURSEL, 0, 0);
					if(curSel == LB_ERR)
					{
						SendDlgItemMessage(hWnd, IDC_EDIT, BM_SETCHECK, BST_INDETERMINATE, 0);
						SendDlgItemMessage(hWnd, IDC_PLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
						SendDlgItemMessage(hWnd, IDC_PLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
						SendDlgItemMessage(hWnd, IDC_REMOVE, BM_SETCHECK, BST_INDETERMINATE, 0);
					}
					else
					{
						SendDlgItemMessage(hWnd, IDC_EDIT, BM_SETCHECK, BST_UNCHECKED, 0);
						SendDlgItemMessage(hWnd, IDC_PLAYNOW, BM_SETCHECK, BST_UNCHECKED, 0);
						if(curSel != 0)
							SendDlgItemMessage(hWnd, IDC_PLAYNEXT, BM_SETCHECK, BST_UNCHECKED, 0);
						else
							SendDlgItemMessage(hWnd, IDC_PLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
						SendDlgItemMessage(hWnd, IDC_REMOVE, BM_SETCHECK, BST_UNCHECKED, 0);
					}
					break;
			}
			break;

		case WM_HSCROLL:
			if((LOWORD(wParam) == TB_THUMBTRACK) || (LOWORD(wParam) == TB_ENDTRACK ))
			{
				REFERENCE_TIME	units = 0;
			//	DWORD			vCapabilities;

				if(my->pSeek)
				{
				//	vCapabilities = AM_SEEKING_CanSeekAbsolute | AM_SEEKING_CanSeekForwards | AM_SEEKING_CanSeekBackwards;
				//	if(my->pSeek->CheckCapabilities(&vCapabilities) != E_FAIL)
					if(my->pSeek->IsFormatSupported(&TIME_FORMAT_MEDIA_TIME) == S_OK)
					{
						if((Position = HIWORD(wParam)) == 0)
							Position  = SendDlgItemMessage(hWnd, IDC_SEEK, TBM_GETPOS, 0, 0);
						pos = (my->tTime / 10000) * Position;

						my->pSeek->GetCurrentPosition(&units);
						if(pos < (50 * units))
							my->pSeek->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
					}
				}
			}
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			my->Paint();
			EndPaint(hWnd, &ps);
			break;

		case WM_SIZE:
			/* Second row, play controls */
			w = (ourRect.right - 5 * my->offset) / 5;
			x = 0;
			SetWindowPos(GetDlgItem(hWnd, IDC_PREV), NULL, x, my->row2, w, my->offset, SWP_NOACTIVATE | SWP_NOZORDER);
			x += w;
			SetWindowPos(GetDlgItem(hWnd, IDC_PLAY), NULL, x, my->row2, w, my->offset, SWP_NOACTIVATE | SWP_NOZORDER);
			x += w;
			SetWindowPos(GetDlgItem(hWnd, IDC_STOP), NULL, x, my->row2, w, my->offset, SWP_NOACTIVATE | SWP_NOZORDER);
			x += w;
			SetWindowPos(GetDlgItem(hWnd, IDC_PAUSE), NULL, x, my->row2, w, my->offset, SWP_NOACTIVATE | SWP_NOZORDER);
			x += w;
			SetWindowPos(GetDlgItem(hWnd, IDC_NEXT), NULL, x, my->row2, (ourRect.right - 5 * my->offset) - x, my->offset, SWP_NOACTIVATE | SWP_NOZORDER);

			/* Top row, playlist add controls */
			SetWindowPos(GetDlgItem(hWnd, IDC_RDSTA), NULL, ourRect.right - 7 * my->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_EDITCURR), NULL, ourRect.right - 6 * my->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_DELETE), NULL, ourRect.right - 5 * my->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_XFADE), NULL, ourRect.right - 4 * my->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_SHUFFLE), NULL, ourRect.right - 3 * my->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_REPEAT), NULL, ourRect.right - 2 * my->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_INFO), NULL, ourRect.right - 1 * my->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

			/* Second row playlist controls */
			SetWindowPos(GetDlgItem(hWnd, IDC_EDIT), NULL, ourRect.right - 5 * my->offset, my->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_PLAYNOW), NULL, ourRect.right - 4 * my->offset, my->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_PLAYNEXT), NULL, ourRect.right - 3 * my->offset, my->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_REMOVE), NULL, ourRect.right - 2 * my->offset, my->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_REMOVEALL), NULL, ourRect.right - 1 * my->offset, my->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

			SetWindowPos(GetDlgItem(hWnd, IDC_COMINGNEXT), NULL, 0, my->offset + my->row2, ourRect.right, ourRect.bottom - my->offset - my->row2, SWP_NOACTIVATE | SWP_NOZORDER);
			InitListBox(FALSE, GetDlgItem(hWnd, IDC_COMINGNEXT));
			my->FillPlayList();

			SetWindowPos(GetDlgItem(hWnd, IDC_SEEK), NULL, 3 * my->offset + 2, 3 * my->row1 - 5, ourRect.right - 3 * my->offset - 4, (my->offset >> 1) + 2, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, IDC_TICKER), NULL, 3 * my->offset + 2, my->row1 + 2, ourRect.right - 5 * my->offset - 4, my->row1 + 6, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, IDC_SPECTRUM), NULL, ourRect.right - 2 * my->offset - 3, my->row1 + 2, 2 * my->offset +1, my->row1 + 6, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, IDC_SUBTICKER), NULL, 3 * my->offset + 2, 2 * my->row1 + 7, ourRect.right - 5 * my->offset - 4, (my->offset >> 1) - 8, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, IDC_RATING), NULL, ourRect.right - 2 * my->offset - 3, 2 * my->row1 + 7, 2 * my->offset +1, (my->offset >> 1) - 8, SWP_NOACTIVATE | SWP_NOZORDER);

			my->InitDC(TRUE);
			my->Paint();
			break;

		case WM_SHOWWINDOW:
			my->vNextIndex = 0;
			break;

		case WM_DSHOWPOS:
			pos = (my->tTime / 10000) * wParam;
			if(my->pSeek)
			{
				REFERENCE_TIME	units = 0;
			//    DWORD			vCapabilities;

				//	vCapabilities = AM_SEEKING_CanSeekAbsolute | AM_SEEKING_CanSeekForwards | AM_SEEKING_CanSeekBackwards;
				//	if(my->pSeek->CheckCapabilities(&vCapabilities) != E_FAIL)
				if(my->pSeek->IsFormatSupported(&TIME_FORMAT_MEDIA_TIME) == S_OK)
				{
					my->pSeek->GetCurrentPosition(&units);
					if(pos < (50 * units))
						my->pSeek->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
				}
			}
			break;

		case WM_SETVIDEOWINDOW:
			if(my->dcVid)
			{
				ReleaseDC(my->hVideo, my->dcVid);
				my->dcVid = NULL;
			}
			my->hVideo = (HWND)wParam;
	//		while(my->pVmr != NULL) ;
			my->InitWindowlessVMR((HWND)my->hVideo);

			if(my->hVideo)
			{
				my->dcVid = GetDC(my->hVideo);
				PostMessage(my->hVideo, WM_PAINT, 0, 0);
			}
			break;

		case WM_PAINTVIDEOWINDOW:
			if(my->pWcontrol)
				my->pWcontrol->RepaintVideo(my->hVideo, my->dcVid);
			break;

		case WM_DISPLAYCHANGE:
			if(my->pWcontrol)
				my->pWcontrol->DisplayModeChanged();
			break;

		case WM_DIRECTSHOWMUTE:
			if(my->pAudio)
				my->pAudio->put_Volume((BOOL)wParam == TRUE ? -2000 : 0);
			my->muted = (BOOL)wParam;
			break;

		case WM_PAUSE:
			my->Pause(lParam ? TRUE : FALSE);
			SendDlgItemMessage(hWnd, IDC_PAUSE, BM_SETCHECK, (lParam ? BST_CHECKED : BST_UNCHECKED), 0);
			break;

		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
			break;

		case WM_LBUTTONUP:
			ReleaseCapture();
			break;

		case WM_TIMER:
			switch(wParam)
			{
				case 1:
					my->Paint();
					break;

				case 2:
					KillTimer(hWnd, 2);
					my->vHelpActive = FALSE;
					SendDlgItemMessage(hWnd, IDC_INFO, BM_SETCHECK, BST_UNCHECKED, 0);
					if(my->vOldCursor)
						SetCursor(my->vOldCursor);
					my->vOldCursor = NULL;
					if(my->hWndHelp)
						DestroyWindow(my->hWndHelp);
					my->hWndHelp = NULL;
					break;

				case 3:
					if((SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0) == 0) && !initDialogShown)
					{
						if(IsWindowVisible(hWnd))
						{
							initDialogShown = TRUE;
							MessageBox(hWnd, TEXT("Please go to the settings window to add\none or more folders containing music or video files.\n"), TEXT("Mediaplayer"), MB_OK | MB_ICONINFORMATION);
						}
					}
					else
						KillTimer(hWnd, 3);
					break;
			}
			break;

		case WM_DESTROY:
			KillTimer(hWnd, 1);
			break;

        case WM_DRAWITEM:
			if(wParam == IDC_COMINGNEXT)
				DrawListBoxItem((LPDRAWITEMSTRUCT)lParam);
			break;

		case WM_PAINTOWNERDRAWN:
			DrawSortListBox((HWND)wParam);
			break;

		case WM_DELETEITEM:
			if(wParam == IDC_COMINGNEXT)
			{
				pMedia = (PMediaDescr)((PDELETEITEMSTRUCT)lParam)->itemData;
				if(pMedia && ((LRESULT)pMedia != LB_ERR))
					GlobalFree((HGLOBAL)pMedia);
			}
			break;

		case WM_PLAYNOW:
			pMedia = (PMediaDescr)lParam;
			if(pMedia)
			{
				if(pMedia->vAux & FLD_ALBUM)
				{
					size = (INT)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0);
					pMedia->vAux &= FIELD_MASK;
					pMedia->vAux |= DISPLAY_INSERT;
					dBase->QueryToListbox(pMedia, GetDlgItem(hWnd, IDC_COMINGNEXT));
					for(id = 0; id < (SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0) - size); id++)
					{
						pMedia = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)id, 0);
						if((LRESULT)pMedia != LB_ERR)
						{
							dBase->AdjustRating(pMedia, RATE_PLAYALBUM);
							pMedia->vNoRemove = TRUE;
						}
					}
					pMedia = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)0, 0);
					pMedia->vRating += RATE_PLAYALBUM;
					SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)0, 0);
					my->Play(pMedia, TRUE);
					SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)0, 0);

				}
				else
				{
					/* Set rating and start playing */
					dBase->AdjustRating(pMedia, RATE_PLAYNOW);
					pMedia->vRating += RATE_PLAYNOW;
					my->Play(pMedia, TRUE);
				}
				id = SendDlgItemMessage(hWnd, IDC_PAUSE, BM_GETCHECK, 0, 0);
				if(id == BST_CHECKED)
					SendMessage(my->hPrnt, WM_PAUSE, 0, FALSE);
				/* Request mute tuner */
				SendMessage(my->hPrnt, WM_MUTERADIO, 0, 0);
			}
			break;

		case WM_PLAYNEXT:
			pMedia = (PMediaDescr)lParam;
			if(pMedia)
			{
				if(pMedia->vAux & FLD_ALBUM)
				{
					size = (INT)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0);
					pMedia->vAux &= FIELD_MASK;
					pMedia->vAux |= DISPLAY_INSERT;
					dBase->QueryToListbox(pMedia, GetDlgItem(hWnd, IDC_COMINGNEXT));
					for(id = 0; id < (SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0) - size); id++)
					{
						pMedia = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)id, 0);
						if((LRESULT)pMedia != LB_ERR)
						{
							dBase->AdjustRating(pMedia, RATE_PLAYALBUM);
							pMedia->vNoRemove = TRUE;
						}
					}
				}
				else
				{
					pString = dBase->DescriptorToFullString(pMedia);
					id = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_INSERTSTRING, wParam, (LPARAM)pString);
					GlobalFree((HGLOBAL)pString);
					pMedia->vNoRemove = TRUE;
					SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)id, (LPARAM)pMedia);
					dBase->AdjustRating(pMedia, RATE_PLAYNEXT);
				}
			}
			break;

		case WM_PLAYQUEUE:
			pMedia = (PMediaDescr)lParam;
			if(pMedia)
			{
				if(pMedia->vAux & FLD_ALBUM)
				{
					id = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0);
					pMedia->vAux &= FIELD_MASK;
					dBase->QueryToListbox(pMedia, GetDlgItem(hWnd, IDC_COMINGNEXT));
					while(id < SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0))
					{
						pMedia = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)id++, 0);
						if((LRESULT)pMedia != LB_ERR)
						{
							dBase->AdjustRating(pMedia, RATE_PLAYALBUM);
							pMedia->vNoRemove = TRUE;
						}
					}
				}
				else
				{
					pString = dBase->DescriptorToFullString(pMedia);
					id = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_ADDSTRING, 0, (LPARAM)pString);
					GlobalFree((HGLOBAL)pString);
					pMedia->vNoRemove = TRUE;
					SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)id, (LPARAM)pMedia);
					dBase->AdjustRating(pMedia, RATE_PLAYQUEUE);
				}
			}
			break;

		case WM_REMOVEDELETE:
			if(lParam)
			{
				switch(wParam)
				{
					case 0:	// Cancel
						SendDlgItemMessage(hWnd, IDC_DELETE, BM_SETCHECK, BST_UNCHECKED, 0);
						SendDlgItemMessage(hWnd, IDC_EDITCURR, BM_SETCHECK, BST_UNCHECKED, 0);
						break;

					case 1:	// Remove
						hWndRemove = NULL;
						pMedia = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)0, 0);
						if(pMedia && ((LRESULT)pMedia != LB_ERR))
						{
							my->Play(pMedia, TRUE);
							SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)0, 0);
						}
						SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)0, 0);
						my->FillPlayList();
						/* Mark item invalid */
						((PMediaDescr)lParam)->vInvalid = 1;
						dBase->ChangeTrack((PMediaDescr)lParam);
						break;

					case 2:	// Delete
						hWndRemove = NULL;
						pMedia = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)0, 0);
						if(pMedia && ((LRESULT)pMedia != LB_ERR))
						{
							my->Play(pMedia, TRUE);
							SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)0, 0);
						}
						SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)0, 0);
						my->FillPlayList();
						/* Destroy item and file */
						dBase->GetStringValue((PMediaDescr)lParam, FLD_FILE, 0, &pString);
						if(pString)
						{
							DeleteFile(pString);
							GlobalFree(pString);
						}
						dBase->KillTrack((PMediaDescr)lParam);
						break;
				}
				hWndRemove = NULL;
			}
			break;

		case WM_SAVEDATA:
			my->SaveSettings();
			break;

		case WM_INITDONE:
			// By now every should have been loaded...
			my->RetrieveSettings();
			my->FillPlayList();
			break;

		case WM_MMBUTTON:
			switch(GET_APPCOMMAND_LPARAM(lParam))
			{
				case APPCOMMAND_MEDIA_NEXTTRACK:
					PostMessage(hWnd, WM_COMMAND, MAKELONG(IDC_NEXT, BN_CLICKED), 0);
					return(1L);

				case APPCOMMAND_MEDIA_PREVIOUSTRACK:
					PostMessage(hWnd, WM_COMMAND, MAKELONG(IDC_PREV, BN_CLICKED), 0);
					return(1L);

				case APPCOMMAND_MEDIA_PLAY:
					PostMessage(hWnd, WM_COMMAND, MAKELONG(IDC_PLAY, BN_CLICKED), 0);
					return(1L);

				case APPCOMMAND_MEDIA_PLAY_PAUSE:
					if(my->pControl)
					{
						OAFilterState	state;
						my->pControl->GetState(10, &state);
						if((state == State_Running) || (state == State_Paused))
						{
							PostMessage(hWnd, WM_COMMAND, MAKELONG(IDC_PAUSE, BN_CLICKED), 0);
							return(1L);
						}
					}
					PostMessage(hWnd, WM_COMMAND, MAKELONG(IDC_PLAY, BN_CLICKED), 0);
					return(1L);

				case APPCOMMAND_MEDIA_PAUSE:
					PostMessage(hWnd, WM_COMMAND, MAKELONG(IDC_PAUSE, BN_CLICKED), 0);
					return(1L);

				case APPCOMMAND_MEDIA_STOP:
					PostMessage(hWnd, WM_COMMAND, MAKELONG(IDC_STOP, BN_CLICKED), 0);
					return(1L);
			}
			break;

		default:
			return(DefWindowProc(hWnd, message, wParam, lParam));
   }
   return(0);
}

static Graphics * gr;
static HDC hDc;
static Brush    * br_black;
static Brush    * br_white;
static Brush    * br_blue;
static Brush    * br_dblue;
static Brush    * br_dred;
static Font     * fn;

static void DrawSortListBox(HWND hWnd)
{
	RECT vListBox, vItem;
	INT  i, vCount;
	LONG vTop;

	GetClientRect(hWnd, &vListBox);
	vListBox.top++;
	vListBox.left++;

	vCount = SendMessage(hWnd, LB_GETCOUNT, 0, 0);
	vItem.bottom = SendMessage(hWnd, LB_GETITEMHEIGHT, 0, 0);

	if(vCount < (vListBox.bottom / vItem.bottom))
	{
		for(i = vCount; i < (vListBox.bottom / vItem.bottom); i++)
		{
			vTop = vListBox.top + i * vItem.bottom;
			gr->FillRectangle(br_blue, vListBox.left, vTop, my->offset, vItem.bottom);
			if(i % 2)
				gr->FillRectangle(br_blue, vListBox.left + my->offset, vTop, vListBox.right - my->offset, vItem.bottom);
			else
				gr->FillRectangle(br_white, vListBox.left + my->offset, vTop, vListBox.right - my->offset, vItem.bottom);
		}
	}
}

static void DrawListBoxItem(LPDRAWITEMSTRUCT pItem)
{
	RectF			rf;
	RECT			crect;
	INT				i, n, size;
	WCHAR			idTxt[12];
	StringFormat	fmt;
	LPWSTR			pText;
	LONG			top;
	PMediaDescr		pMedia;

	pItem->rcItem.right -= pItem->rcItem.left;
	pItem->rcItem.bottom -= pItem->rcItem.top;
	pMedia = (PMediaDescr)pItem->itemData;

	rf.Y = (float)pItem->rcItem.top + 3;
	rf.Height = (float)pItem->rcItem.bottom - 3;

	GetClientRect(pItem->hwndItem, &crect);
	crect.top++;
	crect.left++;

	n = SendMessage(pItem->hwndItem, LB_GETCOUNT, 0, 0);
	if((n < (crect.bottom / pItem->rcItem.bottom)) || (pItem->itemState & ODA_DRAWENTIRE))
	{
		for(i = n; i < (crect.bottom / pItem->rcItem.bottom); i++)
		{
			top = crect.top + i * pItem->rcItem.bottom;
			gr->FillRectangle(br_blue, pItem->rcItem.left, top, my->offset, pItem->rcItem.bottom);
			if(i % 2)
				gr->FillRectangle(br_blue, pItem->rcItem.left + my->offset, top, pItem->rcItem.right - my->offset, pItem->rcItem.bottom);
			else
				gr->FillRectangle(br_white, pItem->rcItem.left + my->offset, top, pItem->rcItem.right - my->offset, pItem->rcItem.bottom);
		}
	}

	size = 2 * SendMessage(pItem->hwndItem, LB_GETTEXTLEN, (WPARAM)pItem->itemID, 0) + 2;
	pText = (LPWSTR)GlobalAlloc(GPTR, size);
	if(pText != NULL)
		SendMessage(pItem->hwndItem, LB_GETTEXT, (WPARAM)pItem->itemID, (LPARAM)pText);

	fmt.SetAlignment(StringAlignmentNear);
	fmt.SetTrimming(StringTrimmingNone);
	fmt.SetLineAlignment(StringAlignmentCenter);
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);
	if(pItem->itemState & ODS_SELECTED)
	{
		gr->FillRectangle(br_blue, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.left + my->offset, pItem->rcItem.bottom);
		gr->FillRectangle(br_black, pItem->rcItem.left + my->offset, pItem->rcItem.top, pItem->rcItem.right - my->offset, pItem->rcItem.bottom);

		StringCchPrintfW(idTxt, 12, L"%u", pItem->itemID + 1);
		rf.X = (float)pItem->rcItem.left;
		rf.Width = (float)my->offset;
		if(pMedia->vHistory)
			gr->DrawString(idTxt, -1, fn, rf, &fmt, br_dblue);
		else if(pMedia->vNoRemove)
			gr->DrawString(idTxt, -1, fn, rf, &fmt, br_dred);
		else
			gr->DrawString(idTxt, -1, fn, rf, &fmt, br_black);
		rf.X += (float)my->offset;
		rf.Width = (float)pItem->rcItem.right - my->offset;
	    if(pText != NULL)
		{
			if(pMedia->vHistory)
				gr->DrawString(pText, -1, fn, rf, &fmt, br_blue);
			else
				gr->DrawString(pText, -1, fn, rf, &fmt, br_white);
		}
	}
	else if(n != 0)
	{
		if(pItem->itemID % 2)
			gr->FillRectangle(br_blue, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.right, pItem->rcItem.bottom);
		else
		{
			gr->FillRectangle(br_blue, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.left + my->offset, pItem->rcItem.bottom);
			gr->FillRectangle(br_white, pItem->rcItem.left + my->offset, pItem->rcItem.top, pItem->rcItem.right - my->offset, pItem->rcItem.bottom);
		}

		rf.X = (float)pItem->rcItem.left;
		rf.Width = (float)my->offset;
		if(pMedia->vHistory)
		{
			StringCchPrintfW(idTxt, 12, L"%u *", pItem->itemID + 1);
			gr->DrawString(idTxt, -1, fn, rf, &fmt, br_dblue);
		}
		else if(pMedia->vNoRemove)
		{
			StringCchPrintfW(idTxt, 12, L"%u", pItem->itemID + 1);
			gr->DrawString(idTxt, -1, fn, rf, &fmt, br_dred);
		}
		else
		{
			StringCchPrintfW(idTxt, 12, L"%u", pItem->itemID + 1);
			gr->DrawString(idTxt, -1, fn, rf, &fmt, br_black);
		}
		rf.X += (float)my->offset;
		rf.Width = (float)pItem->rcItem.right - my->offset;
	    if(pText != NULL)
		{
			if(pMedia->vHistory)
				gr->DrawString(pText, -1, fn, rf, &fmt, br_dblue);
			else if(pMedia->vNoRemove)
				gr->DrawString(pText, -1, fn, rf, &fmt, br_dred);
			else
				gr->DrawString(pText, -1, fn, rf, &fmt, br_black);
		}
	}
	if(pText != NULL)
    	GlobalFree((HGLOBAL)pText);
}

static void InitListBox(BOOL loadUnload, HWND lbox)
{
	if(loadUnload)
	{
		hDc = GetDC(lbox);
		gr = new Graphics(hDc, FALSE);
		br_black = new SolidBrush(Color(255, 0, 0, 0));
		br_white = new SolidBrush(Color(255, 255, 255, 255));
		br_blue = new SolidBrush(Color(255, 201, 211, 227));
		br_dblue = new SolidBrush(Color(255, 51, 61, 127));
		br_dred = new SolidBrush(Color(255, 127, 40, 40));
		fn = new Font(L"Arial", 16);
	}
	else if(lbox == NULL)
	{
		delete fn;
		delete br_blue;
		delete br_dblue;
		delete br_dred;
		delete br_white;
		delete br_black;
		delete gr;
	//	ReleaseDC(lbox, hDc);
	}
	else
	{
		delete gr;
		ReleaseDC(lbox, hDc);
		hDc = GetDC(lbox);
		gr = new Graphics(hDc, FALSE);
	}
}

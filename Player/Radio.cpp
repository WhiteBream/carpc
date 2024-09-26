// Radio.cpp: implementation of the PlayWin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Radio.h"
#include "Ticker.h"
#include "Spectrum.h"
#include <shlobj.h>

//#define SHOWINPUTDEVICEENUM
//#define SHOWINPUTDEVICE
#define DEFAULTUSBLINEINDEVICE		L"USB Audio CODEC"			// Baseboard
//#define DEFAULTAC97LINEINDEVICE		L"Realtek AC97 Audio"		// Cargo I/O board

//#define SHOWOUTPUTDEVICEENUM
//#define SHOWOUTPUTDEVICE
#define USBLINEOUTDEVICE		L"DirectSound: USB Audio CODEC"
//#define AC97LINEOUTDEVICE		L"DirectSound: Realtek AC97 Audio"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

typedef struct _ComingUp
{
	LPWSTR	szFile;
} ComingUp, *pComingUp;

#define BG_WHITE		0
#define BG_GREEN		1
#define BG_LBLUE		2
#define BG_DBLUE		3

LRESULT CALLBACK RadioProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static void DrawSortListBox(HWND hWnd);
static void DrawListBoxItem(LPDRAWITEMSTRUCT pItem);
static void InitListBox(BOOL loadUnload, HWND lbox);

static Radio * my;

static Database * dBase;
static TCHAR      szCurrent[66];
static TCHAR      szInfo[66];

const LPTSTR lpszPipeName = TEXT("\\\\.\\pipe\\whitebreamsysaccess");
#define PIPE_WAIT_TIME		100

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Radio::Radio(PAppInfo pAppInfo)
{
	RECT  pRect;
	HICON ico;
	FontFamily * fm;
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
	saved = FALSE;
	vTunerOn = FALSE;
	vRssi = 0;
	hKillThread = NULL;

	MyRegisterClass(hInst);
	TickerInit(hInst);
	SpectrumInit(hInst);
	GetClientRect(hPrnt, &pRect);

	LoadTunerDll();

	bg[BG_WHITE] = new SolidBrush(Color(255, 255, 255, 255));
	bg[BG_GREEN] = new SolidBrush(Color(255, 200, 255, 200));
	bg[BG_LBLUE] = new SolidBrush(Color(255, 201, 211, 227));
	bg[BG_DBLUE] = new SolidBrush(Color(255, 0, 0, 192));
	pn = new Pen(Color(255, 0, 0, 0), 1);

	fm = new FontFamily(L"Arial");
	fn[0] = new Font(fm, (float)row1 + 2, FontStyleRegular, UnitPixel);
	fn[1] = new Font(fm, (float)(offset >> 1) - 7, FontStyleRegular, UnitPixel);
	delete fm;

	image = new Bitmap(4 * offset, row2);
	graph = new Graphics(image);

	ico = LoadIcon(hInst, (LPCTSTR)IDI_RSSI0);
	bitmap[0] = new Bitmap(ico);
	DestroyIcon(ico);
	ico = LoadIcon(hInst, (LPCTSTR)IDI_RSSI1);
	bitmap[1] = new Bitmap(ico);
	DestroyIcon(ico);
	ico = LoadIcon(hInst, (LPCTSTR)IDI_RSSI2);
	bitmap[2] = new Bitmap(ico);
	DestroyIcon(ico);
	ico = LoadIcon(hInst, (LPCTSTR)IDI_RSSI3);
	bitmap[3] = new Bitmap(ico);
	DestroyIcon(ico);
	ico = LoadIcon(hInst, (LPCTSTR)IDI_RSSI4);
	bitmap[4] = new Bitmap(ico);
	DestroyIcon(ico);
	ico = LoadIcon(hInst, (LPCTSTR)IDI_RSSI5);
	bitmap[5] = new Bitmap(ico);
	DestroyIcon(ico);

	hWnd = CreateWindow(TEXT("CRPC_TNR"), TEXT("fm tuner"), WS_CHILD | WS_CLIPCHILDREN,
		0, 0, pRect.right, pRect.bottom - offset, hPrnt, NULL, hInst, NULL);

	/* Main controls on second row */
	w = (pRect.right - 5 * offset) / 5;
	x = 0;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, row2, w, offset, hWnd, (HMENU)IDC_SCANDOWN, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_SCANDOWN, IDI_SCANDOWN);
	SendDlgItemMessage(hWnd, IDC_SCANDOWN, WM_SETREPEAT, 0, (LPARAM)1000);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, row2, w, offset, hWnd, (HMENU)IDC_TUNEDOWN, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_TUNEDOWN, IDI_TUNEDOWN);
	SendDlgItemMessage(hWnd, IDC_TUNEDOWN, WM_SETREPEAT, 0, (LPARAM)500);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, row2, w, offset, hWnd, (HMENU)IDC_RECORD, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_RECORD, IDI_RECORD);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, row2, w, offset, hWnd, (HMENU)IDC_TUNEUP, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_TUNEUP, IDI_TUNEUP);
	SendDlgItemMessage(hWnd, IDC_TUNEUP, WM_SETREPEAT, 0, (LPARAM)500);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, row2, (pRect.right - 5 * offset) - x, offset, hWnd, (HMENU)IDC_SCANUP, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_SCANUP, IDI_SCANUP);
	SendDlgItemMessage(hWnd, IDI_SCANUP, WM_SETREPEAT, 0, (LPARAM)1000);

	// Band select buttons
	w = offset;
	x = 0;
	CreateWindow(WBBUTTON, TEXT("TMC"), WS_CHILD | WS_VISIBLE, x, 0, w, row1, hWnd, (HMENU)IDC_BANDTMC, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_BANDTMC, BM_SETCHECK, BST_INDETERMINATE, 0);
	x += w;
	CreateWindow(WBBUTTON, TEXT("AM"), WS_CHILD | WS_VISIBLE, x, 0, w, row1, hWnd, (HMENU)IDC_BANDAM, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_BANDAM, BM_SETCHECK, BST_INDETERMINATE, 0);
	x += w;
	CreateWindow(WBBUTTON, TEXT("FM"), WS_CHILD | WS_VISIBLE, x, 0, w, row1, hWnd, (HMENU)IDC_BANDFM, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_BANDFM, BM_SETCHECK, BST_INDETERMINATE, 0);

	// Generic control minibuttons
	CreateWindow(WBBUTTON, TEXT("TA"), WS_CHILD | WS_VISIBLE, pRect.right - 7 * offset, 0, 
		offset, row1, hWnd, (HMENU)IDC_RDSTA, hInst, NULL);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 6 * offset, 0, 
		offset, row1, hWnd, (HMENU)IDC_EDITCURR, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_EDITCURR, IDI_UPDATE);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 5 * offset, 0, 
		offset, row1, hWnd, (HMENU)IDC_DELETE, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_DELETE, BM_SETCHECK, BST_INDETERMINATE, 0);

	CreateWindow(WBBUTTON, TEXT("DX"), WS_CHILD | WS_VISIBLE, pRect.right - 4 * offset, 0, 
		offset, row1, hWnd, (HMENU)IDC_DX, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_DX, BM_SETCHECK, BST_INDETERMINATE, 0);

	CreateWindow(WBBUTTON, TEXT(">|<"), WS_CHILD | WS_VISIBLE, pRect.right - 3 * offset, 0, 
		offset, row1, hWnd, (HMENU)IDC_MONO, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_MONO, BM_SETCHECK, BST_INDETERMINATE, 0);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 2 * offset, 0, 
		offset, row1, hWnd, (HMENU)IDC_REPEAT, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_REPEAT, IDI_REPEAT);
	SendDlgItemMessage(hWnd, IDC_REPEAT, BM_SETCHECK, BST_INDETERMINATE, 0);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 1 * offset, 0, 
		offset, row1, hWnd, (HMENU)IDC_INFO, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_INFO, IDI_INFO);

	// Listbox controlls
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 5 * offset, row2,
		offset, offset, hWnd, (HMENU)IDC_EDIT, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_EDIT, IDI_UPDATE);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 4 * offset, row2,
		offset, offset, hWnd, (HMENU)IDC_UP, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_UP, IDI_UP);
	SendDlgItemMessage(hWnd, IDC_UP, BM_SETCHECK, BST_INDETERMINATE, 0);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 3 * offset, row2,
		offset, offset, hWnd, (HMENU)IDC_DOWN, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_DOWN, IDI_DOWN);
	SendDlgItemMessage(hWnd, IDC_DOWN, BM_SETCHECK, BST_INDETERMINATE, 0);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 2 * offset, row2,
		offset, offset, hWnd, (HMENU)IDC_REMOVE, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_REMOVE, IDI_DEL);
	SendDlgItemMessage(hWnd, IDC_REMOVE, BM_SETCHECK, BST_INDETERMINATE, 0);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 1 * offset, row2,
		offset, offset, hWnd, (HMENU)IDC_PLAYNOW, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_PLAYNOW, IDI_PLAYNOW);
	SendDlgItemMessage(hWnd, IDC_PLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);

	CreateWindow(TEXT("WB_LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS, 0,  row2 + offset,
		pRect.right, pRect.bottom - 2 * offset - row2, hWnd, (HMENU)IDC_STATIONS, hInst, (LPVOID)(2 * offset / 3));
	InitListBox(TRUE, GetDlgItem(hWnd, IDC_STATIONS));
	SendDlgItemMessage(hWnd, IDC_STATIONS, LB_SETITEMHEIGHT, 0, (LPARAM)24);

	CreateWindow(TEXT("WB_DESTSLIDER"), NULL, WS_CHILD | WS_VISIBLE, 4 * offset + 2, 3 * row1 - 5, 
		ourRect.right - 4 * offset - 4, (offset >> 1) + 2, hWnd, (HMENU)IDC_SEEK, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_SEEK, TBM_SETRANGE, TRUE, MAKELONG(0, 10000));

	CreateWindow(TEXT("WB_TICKERBOX"), NULL, WS_CHILD | WS_VISIBLE, 4 * offset + 2, row1 + 2, 
		ourRect.right - 6 * offset - 4, row1 + 6, hWnd, (HMENU)IDC_TICKER, hInst, NULL);
	CreateWindow(TEXT("WB_SPECTRUM"), NULL, WS_CHILD | WS_VISIBLE, ourRect.right - 2 * offset - 3, row1 + 2, 
		2 * offset + 1, row1 + 6, hWnd, (HMENU)IDC_SPECTRUM, hInst, NULL);

	CreateWindow(TEXT("WB_TICKERBOX"), NULL, WS_CHILD | WS_VISIBLE, 4 * offset + 2, 2 * row1 + 7, 
		ourRect.right - 6 * offset - 4, (offset >> 1) - 8, hWnd, (HMENU)IDC_SUBTICKER, hInst, NULL);
	CreateWindow(TEXT("WB_TICKERBOX"), NULL, WS_CHILD | WS_VISIBLE, ourRect.right - 2 * offset - 3, 2 * row1 + 7, 
		2 * offset + 1, (offset >> 1) - 8, hWnd, (HMENU)IDC_RATING, hInst, NULL);

	HRESULT hr = CoInitialize(NULL);
	if(FAILED(hr))
		MessageBox(NULL, TEXT("CoInit failed"), TEXT("Error"), MB_OK);
	else
		OpenTuner();

	RetrieveSettings();
}


Radio::~Radio()
{
	INT i;

	my = NULL;
	CloseTuner();

	InitListBox(FALSE, NULL);
	for(i = 0; i < (sizeof(bg) / sizeof(bg[0])); i++) delete bg[i];
	delete pn;
	for(i = 0; i < (sizeof(fn) / sizeof(fn[0])); i++) delete fn[i];
	for(i = 0; i < (sizeof(bitmap) / sizeof(bitmap[0])); i++) delete bitmap[i];
	if(graph) delete graph;
	if(image) delete image;

	if(hTuner)
		FreeLibrary(hTuner);
	hTuner = NULL;

	CoUninitialize();
	DestroyWindow(hWnd);
}


Radio::Show(BOOL yesNo)
{
	if(yesNo)
	{
		Mute(FALSE);
		ShowWindow(hWnd, SW_SHOW);
		SendDlgItemMessage(hWnd, IDC_RDSTA, BM_SETCHECK, (pAi->vTA ? BST_CHECKED : BST_UNCHECKED), 0);
	}
	else
	{
		ShowWindow(hWnd, SW_HIDE);
	}
}


typedef Tuner*(*tTunerClass)();
void Radio::LoadTunerDll(void)
{
	TCHAR		vPath[MAX_PATH];

	pTuner = NULL;
	SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, SHGFP_TYPE_CURRENT, vPath);
	PathAppend(vPath, TEXT("White Bream\\TunerLib.dll"));
	hTuner = LoadLibrary(vPath);
	if(!hTuner)
	{
		hTuner = LoadLibrary(TEXT("C:\\WhiteBream\\Projects\\C401\\Drivers\\Cargo\\TunerLib\\Release\\TunerLib.dll"));
	}
	if(hTuner)
	{
		tTunerClass NewTuner;
		NewTuner = (tTunerClass)GetProcAddress(hTuner, "NewTuner");
		if(NewTuner)
			pTuner = NewTuner();
	}
	else if(dBase->GetBool("TunerLibError") == FALSE)
	{
		MessageBox(NULL, TEXT("TunerLib.dll could not be found in \"Common Files\\White Bream\"!"), TEXT("Player error"), MB_OK | MB_ICONEXCLAMATION);
		dBase->SetConfig("TunerLibError", "1", CONFIGTYPE_BOOL);
	}
}


ATOM Radio::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= RadioProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_TNR");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

void CALLBACK RdsStation(TUNERDEV vTuner, LPCSTR pStation)
{
	if(!my) return;

	StringCbPrintfW(szCurrent, sizeof(szCurrent), L"%hs", pStation);
	SetDlgItemText(my->hWnd, IDC_TICKER, szCurrent);
}

void CALLBACK RdsText(TUNERDEV vTuner, LPCSTR pText)
{
	if(!my) return;

	StringCbPrintfW(szInfo, sizeof(szInfo), L"%hs", pText);
	SetDlgItemText(my->hWnd, IDC_SUBTICKER, szInfo);
}

void CALLBACK Rssi(TUNERDEV vTuner, BYTE vRssi)
{
	if(!my) return;

	my->vRssi = vRssi;
	my->PaintTime();
}

void CALLBACK Stereo(TUNERDEV vTuner, BOOL vStereo)
{
	if(!my) return;

	my->vStereo = vStereo;
	my->PaintTime();
}

void CALLBACK Traffic(TUNERDEV vTuner, BOOL vTa)
{
	if(!my) return;

	SendMessage(my->hPrnt, WM_RADIOTA, FALSE, vTa);
}

void Radio::OpenTuner(void)
{
	TUNERDEV vDev;

	StringCbPrintf(szCurrent, sizeof(szCurrent), TEXT("FM RDS Tuner"));

	if(!pTuner)
		return;

	vDev = pTuner->SetDevice(DEFAULT);
	if((vDev == DEFAULT) || (vDev == DEFAULT_SLAVE))
	{
		CHAR vRsp[10];
		PipeIo("SIrMI", 5, vRsp, sizeof(vRsp));
		if((vRsp[0] == '1') || (vRsp[0] == '5'))
			SendDlgItemMessage(hWnd, IDC_BANDTMC, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hWnd, IDC_BANDFM, BM_SETCHECK, BST_CHECKED, 0);
	}
 
	UINT min, max;
	pTuner->GetFrequencyRange(&min, &max);
	SendDlgItemMessage(hWnd, IDC_SEEK, TBM_SETRANGE, TRUE, MAKELONG(min / 100, max / 100));

	if(pTuner->SetBand(AM))
	{
		vHasAM = TRUE;
		SendDlgItemMessage(hWnd, IDC_BANDAM, BM_SETCHECK, BST_UNCHECKED, 0);
		pTuner->SetBand(FM);
	}
	else vHasAM = FALSE;

	vTunerOn = TRUE;

	pTuner->RssiCallback(Rssi);
	pTuner->StereoCallback(Stereo);
	pTuner->TrafficCallback(Traffic);

	pTuner->RdsStationCallback(RdsStation);
	pTuner->RdsTextCallback(RdsText);
}

INT Radio::GetFrequency(void)
{
	INT vFreq = 0;
	if(pTuner)
		vFreq = pTuner->GetFrequency() / 100;
	return(vFreq);
}

void CreateStream(void);
void CloseStream(void);
void PauseStream(BOOL vMute);


void Radio::CloseTuner(void)
{
	CloseStream();

	if(pTuner)
		delete pTuner;
	pTuner = NULL;
	vTunerOn = FALSE;
}


void Radio::ResetTexts(void)
{
	StringCbPrintf(szCurrent, sizeof(szCurrent), TEXT("FM RDS Tuner"));
	SetDlgItemText(hWnd, IDC_TICKER, szCurrent);

	StringCbPrintf(szInfo, sizeof(szInfo), TEXT(""));
	SetDlgItemText(hWnd, IDC_SUBTICKER, szInfo);

	AssignIcon(hInst, hWnd, IDC_RDSTA, 0);
}


void Radio::Mute(BOOL vMute)
{
//	if(vTunersel == '1')
//		WriteRegister(2, DMUTE, (vMute ? 0 : DMUTE));

#ifndef TDA7416SOURCE
	//activate/deactivate wavein/out streaming
	PauseStream(vMute);
#endif
}
#include "DShow.h"

IBaseFilter *pSrc = NULL, *pDest = NULL, *pSplitter = NULL;
IGraphBuilder *pGraph = NULL;
IMediaControl *pControl = NULL;
IBasicAudio	 *pAudio = NULL;

HRESULT AddFilterByCLSID(IGraphBuilder *pGraph, const GUID& clsid, LPCWSTR wszName, IBaseFilter **ppF)
{
    if (!pGraph || ! ppF) return E_POINTER;
    *ppF = 0;
    IBaseFilter *pF = 0;
    HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pF));
    if (SUCCEEDED(hr))
    {
        hr = pGraph->AddFilter(pF, wszName);
        if (SUCCEEDED(hr))
            *ppF = pF;
        else
            pF->Release();
    }
    return hr;
}

HRESULT GetUnconnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
{
    *ppPin = 0;
    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = -1;

	if(!pFilter)
		return hr;
	
	hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
        return hr;

	while (pEnum->Next(1, &pPin, NULL) == S_OK)
    {
        PIN_DIRECTION ThisPinDir;
        pPin->QueryDirection(&ThisPinDir);
        if (ThisPinDir == PinDir)
        {
            IPin *pTmp = 0;
            hr = pPin->ConnectedTo(&pTmp);
            if (SUCCEEDED(hr))  // Already connected, not the pin we want.
            {
                pTmp->Release();
            }
            else  // Unconnected, this is the pin we want.
            {
                pEnum->Release();
                *ppPin = pPin;
                return S_OK;
            }
        }
        pPin->Release();
    }
    pEnum->Release();
    // Did not find a matching pin.
    return E_FAIL;
}

HRESULT ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pSrc, IBaseFilter *pDest)
{
    IPin *pOut = 0;
    IPin *pIn = 0;

    if(!pGraph || !pSrc || !pDest)
        return E_POINTER;

    // Find an output pin on the first filter.
    HRESULT hr = GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
    if (FAILED(hr)) 
        return hr;

    hr = GetUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
    if (FAILED(hr))
        return hr;

    // Try to connect them.
    hr = pGraph->Connect(pOut, pIn);
    pIn->Release();
	pOut->Release();
    return hr;
}

static HRESULT FindRadioInputDevice(IBaseFilter ** ppSrcFilter)
{
	HRESULT hr;
	IBaseFilter * pSrc = NULL;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pClassEnum = NULL;
	IMoniker *pMoniker = NULL;
	ULONG cFetched;
	BOOL vUsbAudio = FALSE;

	if(!ppSrcFilter)
		return(E_POINTER);

	// Start with NULL to be safe
 	*ppSrcFilter = pSrc;

	// Create the system device enumerator
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
						IID_ICreateDevEnum, (void **)&pDevEnum);
	if(FAILED(hr))
		return(hr);

    // Create an enumerator for the audio output devices
    hr = pDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pClassEnum, 0);
    if (FAILED(hr))
        return(hr);

	// If there are no enumerators for the requested type, then 
	// CreateClassEnumerator will succeed, but pClassEnum will be NULL.
	if(pClassEnum == NULL)
        return(hr);

	// Check which audio device we're looking for
	if(my->pTuner)
	{
		CHAR vRsp[10];
		PipeIo("SIrMI", 5, vRsp, sizeof(vRsp));
		if((vRsp[0] == '1') || (vRsp[0] == '5'))
		{
			TUNERDEV vDev = my->pTuner->GetDevice();
			if((vDev == DEFAULT) || (vDev == DEFAULT_SLAVE))
				vUsbAudio = TRUE;
		}
	}

	// Note that if the Next() call succeeds but there are no monikers,
	// it will return S_FALSE (which is not a failure).  Therefore, we
	// check that the return code is S_OK instead of using SUCCEEDED() macro.
	while(S_OK == (pClassEnum->Next(1, &pMoniker, &cFetched)))
	{
		IPropertyBag *pPropBag;
		pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
		if(SUCCEEDED(hr))
		{
			// To retrieve the filter's friendly name, do the following:
			VARIANT varName;
			VariantInit(&varName);
			hr = pPropBag->Read(L"FriendlyName", &varName, 0);
			if(SUCCEEDED(hr))
			{
#ifdef SHOWINPUTDEVICEENUM
				MessageBox(NULL, varName.bstrVal, L"recording device", MB_OK);
#endif
				hr = S_FALSE;
				if(vUsbAudio)
				{
					if(StrNCmp(DEFAULTUSBLINEINDEVICE, varName.bstrVal, 15) == 0)
						hr = S_OK;
				}
				else
				{
					if(StrStr(varName.bstrVal, L"AC") && StrStr(varName.bstrVal, L"97"))
						hr = S_OK;
				}
				if(hr == S_OK)
				{
#ifdef SHOWINPUTDEVICE
					MessageBox(NULL, varName.bstrVal, L"Matched recording device", MB_OK);
#endif
					// Bind Moniker to a filter object
					hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSrc);
					if(FAILED(hr))
						MessageBox(NULL, TEXT("pMoniker->BindToObject failed"), TEXT("Tuner src Error"), MB_OK);
				}
			}
			VariantClear(&varName);
			pPropBag->Release();
		}
		pMoniker->Release();
    }
	pClassEnum->Release();
	pDevEnum->Release();

    // Do NOT Release() the IBaseFilter, since it will be used by the calling function.
 	*ppSrcFilter = pSrc;
	return(hr);
}

static HRESULT FindSoundDevice(IBaseFilter ** ppSrcFilter)
{
	HRESULT hr;
	CHAR vRsp[10];
	IBaseFilter * pSrc = NULL;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pClassEnum = NULL;
	IMoniker *pMoniker = NULL;
	ULONG cFetched;
	BOOL vUsbAudio = FALSE;

	if(!ppSrcFilter)
		return(E_POINTER);

	// Start with NULL to be safe
 	*ppSrcFilter = pSrc;

	// Check which audio device we're looking for, on cargo I/O version <1.2, front is the USB device
	PipeIo("SIrMI", 5, vRsp, sizeof(vRsp));
	if((vRsp[0] == '1') || (vRsp[0] == '5'))
	{
		PipeIo("SIrVI", 5, vRsp, sizeof(vRsp));
		if((vRsp[0] <= '1') && (vRsp[0] != '.') && (vRsp[2] <= '1'))
			vUsbAudio = TRUE;
	}
	// Create the system device enumerator
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
						IID_ICreateDevEnum, (void **)&pDevEnum);
	if(FAILED(hr))
		return(hr);

    // Create an enumerator for the audio output devices
    hr = pDevEnum->CreateClassEnumerator(CLSID_AudioRendererCategory, &pClassEnum, 0);
    if (FAILED(hr))
        return(hr);

	// If there are no enumerators for the requested type, then 
	// CreateClassEnumerator will succeed, but pClassEnum will be NULL.
	if(pClassEnum == NULL)
        return(hr);

	// Note that if the Next() call succeeds but there are no monikers,
	// it will return S_FALSE (which is not a failure).  Therefore, we
	// check that the return code is S_OK instead of using SUCCEEDED() macro.
	while(S_OK == (pClassEnum->Next(1, &pMoniker, &cFetched)))
	{
		IPropertyBag *pPropBag;
		pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
		if(SUCCEEDED(hr))
		{
			// To retrieve the filter's friendly name, do the following:
			VARIANT varName;
			VariantInit(&varName);
			//http://msdn2.microsoft.com/en-us/library/ms787619.aspx
			hr = pPropBag->Read(L"FriendlyName", &varName, 0);
			if(SUCCEEDED(hr))
			{
#ifdef SHOWOUTPUTDEVICEENUM
				MessageBox(NULL, varName.bstrVal, L"sounddevice", MB_OK);
#endif
				hr = S_FALSE;
				if(vUsbAudio)
				{
					// On cargo boards V1.1 and before we use the USB audio device
					if(StrNCmp(USBLINEOUTDEVICE, varName.bstrVal, 24) == 0)
						hr = S_OK;
				}
				else
				{
					// All others front channel is on AC97
					if((StrNCmp(L"DirectSound:", varName.bstrVal, 12) == 0) &&
									StrStr(varName.bstrVal, L"AC") && StrStr(varName.bstrVal, L"97"))
						hr = S_OK;
				}
				if(hr == S_OK)
				{
#ifdef SHOWOUTPUTDEVICE
					MessageBox(NULL, varName.bstrVal, L"Matched sounddevice", MB_OK);
#endif
					hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSrc);
					if(FAILED(hr))
						MessageBox(NULL, TEXT("pMoniker->BindToObject failed"), TEXT("Tuner dest Error"), MB_OK);
				}
			}
			VariantClear(&varName);
			pPropBag->Release();
		}
		pMoniker->Release();
    }
	pClassEnum->Release();
	pDevEnum->Release();

    // Do NOT Release() the IBaseFilter, since it will be used by the calling function.
 	*ppSrcFilter = pSrc;
	return(hr);
}

static HRESULT SetAudioSource(IBaseFilter *pSrc)
{
	HRESULT hr =S_OK;
//	IAMAudioInputMixer *pMixer;

	if(my->pTuner)
	{
		TUNERDEV vDev = my->pTuner->GetDevice();
	//	if((vDev == DEFAULT) || (vDev == DEFAULT_SLAVE))
	//		StrCpy(vTarget, DEFAULTUSBLINEINDEVICE);
	//	else
	//		StrCpy(vTarget, DEFAULTAC97LINEINDEVICE);
	}
	return(hr);
}

void CreateStream(void)
{
	HRESULT hr =S_OK;
	TUNERDEV vDev = (TUNERDEV)NONE;

	if(!pGraph)
	{
		if(my->pTuner)
			vDev = my->pTuner->GetDevice();

		// Create the Filter Graph Manager.
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
			IID_IGraphBuilder, (void**)&pGraph);
		if(FAILED(hr))
			MessageBox(NULL, TEXT("IID_IGraphBuilder failed"), TEXT("Tuner Error"), MB_OK);

		// Not shown: Use the System Device Enumerator to create the  audio capture filter.
		hr = FindRadioInputDevice(&pSrc);
		if(FAILED(hr))
			MessageBox(NULL, TEXT("FindRadioInputDevice(&pSrc) failed"), TEXT("Tuner Error"), MB_OK);

		SetAudioSource(pSrc);

		// Add the audio capture filter to the filter graph. 
		if(pSrc)
		{
			hr = pGraph->AddFilter(pSrc, L"Capture");
			if(FAILED(hr))
				MessageBox(NULL, TEXT("pGraph->AddFilter(pSrc, L\"Capture\") failed"), TEXT("Tuner Error"), MB_OK);
		}

		hr = AddFilterByCLSID(pGraph, CLSID_InfTee, L"Splitter", &pSplitter);

		// Find desired ouput device
		hr = FindSoundDevice(&pDest);
		if(FAILED(hr))
			MessageBox(NULL, TEXT("FindSoundDevice(&pDest) failed"), TEXT("Tuner Error"), MB_OK);

		if(pDest)
		{
			hr = pGraph->AddFilter(pDest, L"WaveOut");
			if(FAILED(hr))
				MessageBox(NULL, TEXT("pGraph->AddFilter(pDest, L\"WaveOut\") failed"), TEXT("Tuner Error"), MB_OK);
		}

		if(pSrc)
		{
			IPin *pOut = 0;
			hr = GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
			if(FAILED(hr))
				MessageBox(NULL, TEXT("GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut) failed"), TEXT("Tuner Error"), MB_OK);

			if (SUCCEEDED(hr)) 
			{
				if(FAILED(pGraph->Render(pOut)))
					MessageBox(NULL, TEXT("pGraph->Render(pOut) failed"), TEXT("Tuner Error"), MB_OK);

				pOut->Release();
				pOut = NULL;
			}
		}

		hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
		if(FAILED(hr))
			MessageBox(NULL, TEXT("IID_IMediaControl failed"), TEXT("Tuner Error"), MB_OK);

		hr = pGraph->QueryInterface(IID_IBasicAudio, (void **)&pAudio);
		if(FAILED(hr))
			MessageBox(NULL, TEXT("IID_IBasicAudio failed"), TEXT("Error"), MB_OK);

		if(pAudio)
		{
			CHAR vRsp[10];
			PipeIo("SIrMI", 5, vRsp, sizeof(vRsp));
			if(((vRsp[0] == '1') || (vRsp[0] == '5')) && ((vDev == DEFAULT) || (vDev == DEFAULT_SLAVE)))
				pAudio->put_Volume(0);	// 0dB for USB
			else
				pAudio->put_Volume(-100);	// -10dB for AC97
		}

		if(pControl)
			pControl->Run();
	}
	else
		MessageBox(NULL, TEXT("pGraph already in use"), TEXT("Tuner Error"), MB_OK);
}

void CloseStream(void)
{
	if(pGraph)
	{
		pGraph->Abort();
		if(pControl)
		{
			pControl->Stop();
			pControl->Release();
			pControl = NULL;
		}
		if(pAudio)
		{
			pAudio->Release();
			pAudio = NULL;
		}
		if(pSrc)	
		{
			pGraph->RemoveFilter(pSrc);
			pSrc->Release(); 
			pSrc = NULL;
		}
		if(pDest)
		{
			pGraph->RemoveFilter(pDest);
			pDest->Release(); 
			pDest = NULL;
		}
		pGraph->Release();
		pGraph = NULL;
	}
}

void PauseStream(BOOL vMute)
{
	if(!pGraph)
		CreateStream();

	if(pControl)
	{
		if(vMute)
			pControl->Pause();
		else
			pControl->Run();
	}
}


HRESULT FindFilterByFriendlyName(const GUID &clsid, LPWSTR pName, IBaseFilter **pFilter)
{
	HRESULT hr =S_OK;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pClassEnum = NULL;
	IMoniker *pMoniker = NULL;
	ULONG cFetched;

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,(void **)&pDevEnum);
	if(FAILED(hr) || !pDevEnum)
		return(hr);

	hr = pDevEnum->CreateClassEnumerator(clsid, &pClassEnum,0);
    if(SUCCEEDED(hr))
	{
		while(S_OK == (pClassEnum->Next(1, &pMoniker, &cFetched)))
		{
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
			if(SUCCEEDED(hr))
			{
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if(SUCCEEDED(hr))
				{
					if(StrCmp(pName, varName.bstrVal) == 0)
					{
						hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
						if(FAILED(hr))
							MessageBox(NULL, TEXT("pMoniker->BindToObject failed"), TEXT("Tuner src Error"), MB_OK);
					}
				}
				VariantClear(&varName);
			}
			pPropBag->Release();
		}
		pMoniker->Release();
    }
	pClassEnum->Release();
	pDevEnum->Release();
	return(hr);
}

IBaseFilter *pMp3Dest = NULL, *pWaveDest = NULL, *pFileDest = NULL;
IFileSinkFilter *pFileSink = NULL;

BOOL RecordStream(BOOL vStartStop)
{
	static const GUID CLSID_WavDest = {0x3c78b8e2, 0x6c4d, 0x11d1, {0xad, 0xe2, 0x0, 0x0, 0xf8, 0x75, 0x4b, 0x99}};

	IServiceProvider *pProvider = NULL;
	HRESULT hr = S_OK;

	if(pGraph)
	{
		if(vStartStop)
		{
			FindFilterByFriendlyName(CLSID_AudioCompressorCategory, L"MPEG Layer-3", &pMp3Dest);
			if(!pMp3Dest)
			{
				MessageBox(NULL, TEXT("Cannot find a suitable MP3 encoder"), TEXT("Recorder Error"), MB_OK);
				return(FALSE);
			}

			hr = pGraph->AddFilter(pMp3Dest, L"MP3");
			if(FAILED(hr))
			{
				MessageBox(NULL, TEXT("AddFilterByCLSID(pGraph, vMp3, L\"MP3\", &pMp3Dest) failed"), TEXT("Streaming Error"), MB_OK);
				return(FALSE);
			}
			hr = CoCreateInstance(CLSID_WavDest, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void **)&pWaveDest);
			if(FAILED(hr))
			{
				MessageBox(NULL, TEXT("CoCreateInstance(CLSID_WavDest, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void **)&pWavDest) failed"), TEXT("Streaming Error"), MB_OK);
				return(FALSE);
			}
			hr = pGraph->AddFilter(pWaveDest, L"WaveDest");

			hr = AddFilterByCLSID(pGraph, CLSID_FileWriter, L"File Writer", &pFileDest);
			if(FAILED(hr))
				return(FALSE);

			pFileDest->QueryInterface(IID_IFileSinkFilter, (LPVOID*)&pFileSink);
			if(pFileSink)
				pFileSink->SetFileName(L"C:\\testfile.mp3", NULL);

			// Connect the filters.
			ConnectFilters(pGraph, pSplitter, pMp3Dest);
			ConnectFilters(pGraph, pMp3Dest, pWaveDest);
			ConnectFilters(pGraph, pWaveDest, pFileDest);
		}
		else if(pMp3Dest)
		{
		//	pMp3Dest->EndWriting();
			pFileSink->Release();
			pFileDest->Release();
			pWaveDest->Release();
			pMp3Dest->Release();
			pMp3Dest = NULL;
		}
	}
	return(FALSE);
}

void Radio::UpdateTuner(void)
{
	static INT vRefresh = 0;

	if(vTunerOn)
	{
		PaintTime();
	}
	else
		OpenTuner();
}


void Radio::InitDC(BOOL updateDC)
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


void Radio::Paint()
{
	if(myGraphics == NULL)
		return;

	myGraphics->FillRectangle(bg[BG_DBLUE], 0, 0, ourRect.right, row1);
	myGraphics->FillRectangle(bg[BG_GREEN], 0, row1, ourRect.right, ourRect.bottom - row1);

	if(szCurrent[0] != TEXT('\00'))
		SetDlgItemText(hWnd, IDC_TICKER, szCurrent);

	if(szInfo[0] != TEXT('\00'))
		SetDlgItemText(hWnd, IDC_SUBTICKER, szInfo);

	PaintTime();
}


void Radio::PaintTime()
{
	PointF			point;
	WCHAR			vBuffer[20];
	StringFormat	fmt;
	INT				vRssiIcon;

	fmt.SetAlignment(StringAlignmentFar);
	fmt.SetTrimming(StringTrimmingNone);
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);

	if(!myGraphics || !graph || !image || !fn[1] || !bg[BG_DBLUE] || !bg[BG_WHITE] || !pn)
		return;

	graph->FillRectangle(bg[BG_WHITE], 2, row1 + 2, 4 * offset - 3, row2 - row1 - 5);
	graph->DrawRectangle(pn, 2, row1 + 2, 4 * offset - 3, row2 - row1 - 5);

	// Print frequency
	point.X = (float)4 * offset;
	point.Y = (float)(row1 + 2);

	vFrequency = 0;
	if(pTuner)
		vFrequency = pTuner->GetFrequency();
	StringCbPrintfW(vBuffer, sizeof(vBuffer), L"%.1f MHz", ((FLOAT)vFrequency / 1000));
	graph->DrawString(vBuffer, -1, fn[0], point, &fmt, bg[BG_DBLUE]);

	// Print stereo mode
	point.X = (float)4 * offset - 4;
	point.Y = (float)(2 * row1 + 9);
	graph->DrawString(vStereo ? L"STEREO" : L"MONO", -1, fn[1], point, &fmt, bg[BG_DBLUE]);

	// Print signal level
	point.Y = (float)(3 * row1 - 2);
	StringCbPrintfW(vBuffer, sizeof(vBuffer), L"%u dB%cV", vRssi, 181);
	graph->DrawString(vBuffer, -1, fn[1], point, &fmt, bg[BG_DBLUE]);

	SendDlgItemMessage(hWnd, IDC_SEEK, TBM_SETPOS, TRUE, (LPARAM)(vFrequency / 100));

	vRssiIcon = vRssi + 8;
	vRssiIcon /= 15;
	if((vRssiIcon < 0) || (vRssiIcon > 5))
		vRssiIcon = 0;
	if(bitmap[vRssiIcon])
		graph->DrawImage(bitmap[vRssiIcon], 10, row2 - offset + 5, 64, 32);

	myGraphics->DrawImage(image, 0, 0);
}


void Radio::SaveSettings()
{
	if(saved)
		return;

	/* Save configuration */
	INT vChannel = GetFrequency();
	dBase->SetConfig("TunerChannel", &vChannel, CONFIGTYPE_INT);

	// Save station list
	dBase->SaveStations(GetDlgItem(hWnd, IDC_STATIONS));

	saved = TRUE;
}


void Radio::RetrieveSettings()
{
	BOOL	toggle;
	LPVOID	pPtr;
	INT		vFreq;

	// Get range and mono/stero mode
	toggle = dBase->GetBool("TunerRange");
	SendDlgItemMessage(hWnd, IDC_DX, BM_SETCHECK, (toggle ? BST_CHECKED : BST_UNCHECKED), 0);
	if(pTuner)
		pTuner->SetSensitivity(toggle);

	toggle = dBase->GetBool("TunerMono");
	SendDlgItemMessage(hWnd, IDC_MONO, BM_SETCHECK, (toggle ? BST_CHECKED : BST_UNCHECKED), 0);
	SetWindowText(GetDlgItem(hWnd, IDC_MONO), (toggle ? TEXT("MON") : TEXT(">|<")));
	if(pTuner)
		pTuner->SetStereoMode(!toggle);

	pPtr = (LPVOID)&vFreq;
	dBase->GetConfig("TunerChannel", &pPtr, CONFIGTYPE_INT);
	if(pTuner)
		pTuner->SetFrequency((UINT)vFreq * 100);

	// Get station list
	dBase->GetStation(GetDlgItem(hWnd, IDC_STATIONS));

	saved = FALSE;
}

static void SetListControls(HWND hWnd)
{
	LONG	curSel;

	curSel = SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETCURSEL, 0, 0);
	if(curSel == LB_ERR)
	{
		SendDlgItemMessage(hWnd, IDC_UP, BM_SETCHECK, BST_INDETERMINATE, 0);
		SendDlgItemMessage(hWnd, IDC_DOWN, BM_SETCHECK, BST_INDETERMINATE, 0);
		SendDlgItemMessage(hWnd, IDC_REMOVE, BM_SETCHECK, BST_INDETERMINATE, 0);
		SendDlgItemMessage(hWnd, IDC_PLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
		AssignIcon(my->hInst, hWnd, IDC_EDIT, IDI_UPDATE);
	}
	else
	{
		SendDlgItemMessage(hWnd, IDC_UP, BM_SETCHECK, (curSel > 0 ? BST_UNCHECKED : BST_INDETERMINATE), 0);
		if(curSel < (SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETCOUNT, 0, 0) - 1))
			SendDlgItemMessage(hWnd, IDC_DOWN, BM_SETCHECK, BST_UNCHECKED, 0);
		else
			SendDlgItemMessage(hWnd, IDC_DOWN, BM_SETCHECK, BST_INDETERMINATE, 0);
		SendDlgItemMessage(hWnd, IDC_REMOVE, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hWnd, IDC_PLAYNOW, BM_SETCHECK, BST_UNCHECKED, 0);
		AssignIcon(my->hInst, hWnd, IDC_EDIT, IDI_EDIT);
	}							
}

LRESULT CALLBACK RadioProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT		ps;
    RECT			ourRect;
	LONG			curSel;
   	INT             x, w;
	BOOL			toggle;
	LPTSTR			pString;

	GetClientRect(hWnd, &ourRect);

	if(!my)
		return(DefWindowProc(hWnd, message, wParam, lParam));

	switch(message)
	{
		case WM_CREATE:
			my->hCbWnd = hWnd;
			my->InitDC(FALSE);
			SendMessage(my->hPrnt, WM_SETREGISTERFINALWND, (WPARAM)IDW_RAD, (LPARAM)hWnd);
			break;

		case WM_COMMAND:
			//Reset TA interrupt when something clicked
			SendMessage(my->hPrnt, WM_RADIOTA, TRUE, FALSE);
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					switch(LOWORD(wParam))
					{
						case IDC_BANDFM:
							CloseStream();
							if(my->pTuner)
							{
								if(SendDlgItemMessage(hWnd, IDC_BANDTMC, BM_GETCHECK, 0, 0) != BST_INDETERMINATE)
									SendDlgItemMessage(hWnd, IDC_BANDTMC, BM_SETCHECK, BST_UNCHECKED, 0);
								if(SendDlgItemMessage(hWnd, IDC_BANDAM, BM_GETCHECK, 0, 0) != BST_INDETERMINATE)
									SendDlgItemMessage(hWnd, IDC_BANDAM, BM_SETCHECK, BST_UNCHECKED, 0);
								SendDlgItemMessage(hWnd, IDC_BANDFM, BM_SETCHECK, BST_CHECKED, 0);
								my->pTuner->SetDevice(DEFAULT);
								my->pTuner->SetBand(FM);
								CreateStream();
							}
							break;
						case IDC_BANDAM:
							CloseStream();
							if(my->pTuner)
							{
								if(SendDlgItemMessage(hWnd, IDC_BANDTMC, BM_GETCHECK, 0, 0) != BST_INDETERMINATE)
									SendDlgItemMessage(hWnd, IDC_BANDTMC, BM_SETCHECK, BST_UNCHECKED, 0);
								SendDlgItemMessage(hWnd, IDC_BANDAM, BM_SETCHECK, BST_CHECKED, 0);
								if(SendDlgItemMessage(hWnd, IDC_BANDFM, BM_GETCHECK, 0, 0) != BST_INDETERMINATE)
									SendDlgItemMessage(hWnd, IDC_BANDFM, BM_SETCHECK, BST_UNCHECKED, 0);
								my->pTuner->SetDevice(DEFAULT);
								my->pTuner->SetBand(AM);
								CreateStream();
							}
							break;
						case IDC_BANDTMC:
							CloseStream();
							if(my->pTuner)
							{
								SendDlgItemMessage(hWnd, IDC_BANDTMC, BM_SETCHECK, BST_CHECKED, 0);
								if(SendDlgItemMessage(hWnd, IDC_BANDAM, BM_GETCHECK, 0, 0) != BST_INDETERMINATE)
									SendDlgItemMessage(hWnd, IDC_BANDAM, BM_SETCHECK, BST_UNCHECKED, 0);
								if(SendDlgItemMessage(hWnd, IDC_BANDFM, BM_GETCHECK, 0, 0) != BST_INDETERMINATE)
									SendDlgItemMessage(hWnd, IDC_BANDFM, BM_SETCHECK, BST_UNCHECKED, 0);
								my->pTuner->SetDevice(TMCDEVICE);
								my->pTuner->SetBand(FM);
								CreateStream();
							}
							break;

						case IDC_SCANUP:
							if(my->pTuner)
								my->pTuner->SetSeek(TRUE);
							else
								SendDlgItemMessage(hWnd, LOWORD(wParam), BM_SETCHECK, BST_INDETERMINATE, 0);
							break;
						case IDC_SCANDOWN:
							if(my->pTuner)
								my->pTuner->SetSeek(FALSE);
							else
								SendDlgItemMessage(hWnd, LOWORD(wParam), BM_SETCHECK, BST_INDETERMINATE, 0);
							break;
						case IDC_TUNEUP:
							if(my->pTuner)
								my->pTuner->SetTune(TRUE);
							else
								SendDlgItemMessage(hWnd, LOWORD(wParam), BM_SETCHECK, BST_INDETERMINATE, 0);
							break;
						case IDC_TUNEDOWN:
							if(my->pTuner)
								my->pTuner->SetTune(FALSE);
							else
								SendDlgItemMessage(hWnd, LOWORD(wParam), BM_SETCHECK, BST_INDETERMINATE, 0);
							break;

						case IDC_RECORD:
							toggle = (SendDlgItemMessage(hWnd, IDC_RECORD, BM_GETCHECK, 0, 0) == BST_CHECKED ? FALSE : TRUE);
							if(!RecordStream(toggle))
								toggle = FALSE;
							SendDlgItemMessage(hWnd, IDC_RECORD, BM_SETCHECK, (toggle ? BST_CHECKED : BST_UNCHECKED), 0);
							break;

						case IDC_RDSTA:
							my->pAi->vTA = !my->pAi->vTA;
							dBase->SetConfig("TrafficAnnouncement", (LPVOID)&my->pAi->vTA, CONFIGTYPE_BOOL);
							SendDlgItemMessage(hWnd, IDC_RDSTA, BM_SETCHECK, (my->pAi->vTA ? BST_CHECKED : BST_UNCHECKED), 0);
							break;
						case IDC_DX:
							toggle = (SendDlgItemMessage(hWnd, IDC_DX, BM_GETCHECK, 0, 0) == BST_UNCHECKED ? TRUE : FALSE);
							dBase->SetConfig("TunerRange", (LPVOID)&toggle, CONFIGTYPE_BOOL);
							SendDlgItemMessage(hWnd, IDC_DX, BM_SETCHECK, (toggle ? BST_CHECKED : BST_UNCHECKED), 0);
							if(my->pTuner)
								my->pTuner->SetSensitivity(toggle);
							break;
						case IDC_MONO:
							toggle = (SendDlgItemMessage(hWnd, IDC_MONO, BM_GETCHECK, 0, 0) == BST_UNCHECKED ? TRUE : FALSE);
							dBase->SetConfig("TunerMono", (LPVOID)&toggle, CONFIGTYPE_BOOL);
							SendDlgItemMessage(hWnd, IDC_MONO, BM_SETCHECK, (toggle ? BST_CHECKED : BST_UNCHECKED), 0);
							SetWindowText(GetDlgItem(hWnd, IDC_MONO), (toggle ? TEXT("MON") : TEXT(">|<")));
							if(my->pTuner)
								my->pTuner->SetStereoMode(!toggle);
							break;
						case IDC_REPEAT:
							if(SendDlgItemMessage(hWnd, IDC_REPEAT, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
								SendDlgItemMessage(hWnd, IDC_REPEAT, BM_SETCHECK, BST_CHECKED, 0);
							else
								SendDlgItemMessage(hWnd, IDC_REPEAT, BM_SETCHECK, BST_UNCHECKED, 0);
							break;

						case IDC_EDITCURR:
							x = 60 * sizeof(TCHAR);
							pString = (LPTSTR)GlobalAlloc(GPTR, x);
							if(pString)
							{
								if(_tcscmp(szCurrent, TEXT("FM RDS Tuner")) != 0)
								{
									TCHAR vStation[sizeof(szCurrent) / sizeof(TCHAR)];
									StringCbCopy(vStation, sizeof(vStation), szCurrent);
									Strip(vStation);
									StringCbPrintf(pString, x, TEXT("%ls (%.1f MHz)"), vStation, (FLOAT)my->GetFrequency() / 10);
								}
								else
									StringCbPrintf(pString, x, TEXT("%.1f MHz"), (FLOAT)my->GetFrequency() / 10);

								x = SendDlgItemMessage(hWnd, IDC_STATIONS, LB_ADDSTRING, 0, (LPARAM)pString);
								SendDlgItemMessage(hWnd, IDC_STATIONS, LB_SETITEMDATA, (WPARAM)x, (LPARAM)my->GetFrequency());
								GlobalFree(pString);
							}
							break;

						// Now playing controls
						case IDC_EDIT:
							curSel = SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETCURSEL, 0, 0);
							if(curSel == LB_ERR)
							{
								x = 60 * sizeof(TCHAR);
								pString = (LPTSTR)GlobalAlloc(GPTR, x);
								if(pString)
								{
									if(_tcscmp(szCurrent, TEXT("FM RDS Tuner")) != 0)
									{
										TCHAR vStation[sizeof(szCurrent) / sizeof(TCHAR)];
										StringCbCopy(vStation, sizeof(vStation), szCurrent);
										Strip(vStation);
										StringCbPrintf(pString, x, TEXT("%ls (%.1f MHz)"), vStation, (FLOAT)my->GetFrequency() / 10);
									}
									else
										StringCbPrintf(pString, x, TEXT("%.1f MHz"), (FLOAT)my->GetFrequency() / 10);

									SendDlgItemMessage(hWnd, IDC_STATIONS, LB_INSERTSTRING, 0, (LPARAM)pString);
									SendDlgItemMessage(hWnd, IDC_STATIONS, LB_SETITEMDATA, 0, (LPARAM)my->GetFrequency());
									GlobalFree(pString);
								}
							}
							else
							{
								//EDIT
							}
							break;

						case IDC_UP:
							curSel = SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETCURSEL, 0, 0);
							if((curSel != LB_ERR) && (curSel != 0))
							{
								x = sizeof(TCHAR) * SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETTEXTLEN, (WPARAM)curSel, 0) + 2;
								pString = (LPTSTR)GlobalAlloc(GPTR, x);
								if(pString)
								{
									x = SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETITEMDATA, curSel, 0);
									SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETTEXT, (WPARAM)curSel, (LPARAM)pString);
									SendDlgItemMessage(hWnd, IDC_STATIONS, LB_DELETESTRING, (WPARAM)curSel, 0);
									SendDlgItemMessage(hWnd, IDC_STATIONS, LB_INSERTSTRING, (WPARAM)--curSel, (LPARAM)pString);
									SendDlgItemMessage(hWnd, IDC_STATIONS, LB_SETCURSEL, (WPARAM)curSel, 0);
									SendDlgItemMessage(hWnd, IDC_STATIONS, LB_SETITEMDATA, curSel, (LPARAM)x);
									GlobalFree(pString);
								}
							}
							SetListControls(hWnd);
							break;
						case IDC_DOWN:
							curSel = SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETCURSEL, 0, 0);
							if(curSel != LB_ERR)
							{
								x = sizeof(TCHAR) * SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETTEXTLEN, (WPARAM)curSel, 0) + 2;
								pString = (LPTSTR)GlobalAlloc(GPTR, x);
								if(pString)
								{
									x = SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETITEMDATA, curSel, 0);
									SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETTEXT, (WPARAM)curSel, (LPARAM)pString);
									SendDlgItemMessage(hWnd, IDC_STATIONS, LB_DELETESTRING, (WPARAM)curSel, 0);
									SendDlgItemMessage(hWnd, IDC_STATIONS, LB_INSERTSTRING, (WPARAM)++curSel, (LPARAM)pString);
									SendDlgItemMessage(hWnd, IDC_STATIONS, LB_SETCURSEL, (WPARAM)curSel, 0);
									SendDlgItemMessage(hWnd, IDC_STATIONS, LB_SETITEMDATA, curSel, (LPARAM)x);
									GlobalFree(pString);
								}
							}
							SetListControls(hWnd);
							break;

						case IDC_REMOVE:
							curSel = SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETCURSEL, 0, 0);
							if(curSel != LB_ERR)
							{
								SendDlgItemMessage(hWnd, IDC_STATIONS, LB_DELETESTRING, (WPARAM)curSel, 0);
								//SendDlgItemMessage(hWnd, IDC_STATIONS, LB_SETCURSEL, (WPARAM)curSel, 0);
							}
							SetListControls(hWnd);
							break;

						case IDC_PLAYNOW:
							curSel = SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETCURSEL, 0, 0);
							if(curSel != LB_ERR)
							{
								x = SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETITEMDATA, (WPARAM)curSel, 0);
								my->pTuner->SetFrequency(x * 100);
							}
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
			/*		curSel = SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETCURSEL, 0, 0);
					if(curSel != LB_ERR)
					{
						szText = (LPWSTR)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)curSel, 0);
						my->PlayNow(&szText[lstrlen(szText) + 1]);
						SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)curSel, 0);
						SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETCURSEL, (WPARAM)-1, 0);
						SendDlgItemMessage(hWnd, IDC_PLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
						SendDlgItemMessage(hWnd, IDC_PLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
						SendDlgItemMessage(hWnd, IDC_REMOVE, BM_SETCHECK, BST_INDETERMINATE, 0);
						myPtr->FillPlayList();
					}
			*/		break;
				case LBN_SELCHANGE:
					SetListControls(hWnd);
					break;
			}
			break;

		case WM_HSCROLL:
			if((LOWORD(wParam) == TB_THUMBTRACK) || (LOWORD(wParam) == TB_ENDTRACK ))
			{
				x = SendDlgItemMessage(hWnd, IDC_SEEK, TBM_GETPOS, 0, 0);
				my->pTuner->SetFrequency(x * 100);
				my->PaintTime();
			}
			break;

		case WM_PAINT:
			GetClientRect(my->hPrnt, &ourRect);
			ourRect.bottom -= my->offset;
    		SetWindowPos(hWnd, 0, 0, 0, ourRect.right, ourRect.bottom, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
            BeginPaint(hWnd, &ps);
			my->Paint();
			EndPaint(hWnd, &ps);
			break;

		case WM_SIZE:
			w = (ourRect.right - 5 * my->offset) / 5;
			x = 0;
			SetWindowPos(GetDlgItem(hWnd, IDC_SCANDOWN), NULL, x, my->row2, w, my->offset, SWP_NOACTIVATE | SWP_NOZORDER);
			x += w;
			SetWindowPos(GetDlgItem(hWnd, IDC_TUNEDOWN), NULL, x, my->row2, w, my->offset, SWP_NOACTIVATE | SWP_NOZORDER);
			x += w;
			SetWindowPos(GetDlgItem(hWnd, IDC_RECORD), NULL, x, my->row2, w, my->offset, SWP_NOACTIVATE | SWP_NOZORDER);
			x += w;
			SetWindowPos(GetDlgItem(hWnd, IDC_TUNEUP), NULL, x, my->row2, w, my->offset, SWP_NOACTIVATE | SWP_NOZORDER);
			x += w;
			SetWindowPos(GetDlgItem(hWnd, IDC_SCANUP), NULL, x, my->row2, (ourRect.right - 5 * my->offset) - x, my->offset, SWP_NOACTIVATE | SWP_NOZORDER);

			SetWindowPos(GetDlgItem(hWnd, IDC_RDSTA), NULL, ourRect.right - 7 * my->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_EDITCURR), NULL, ourRect.right - 6 * my->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_DELETE), NULL, ourRect.right - 5 * my->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_DX), NULL, ourRect.right - 4 * my->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_MONO), NULL, ourRect.right - 3 * my->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_REPEAT), NULL, ourRect.right - 2 * my->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_INFO), NULL, ourRect.right - 1 * my->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

			SetWindowPos(GetDlgItem(hWnd, IDC_EDIT), 0, ourRect.right - 5 * my->offset, my->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_UP), 0, ourRect.right - 4 * my->offset, my->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_DOWN), 0, ourRect.right - 3 * my->offset, my->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_REMOVE), 0, ourRect.right - 2 * my->offset, my->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_PLAYNOW), 0, ourRect.right - 1 * my->offset, my->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

			SetWindowPos(GetDlgItem(hWnd, IDC_STATIONS), 0, 0, my->offset + my->row2, ourRect.right, ourRect.bottom - my->offset - my->row2, SWP_NOACTIVATE | SWP_NOZORDER);
			InitListBox(FALSE, GetDlgItem(hWnd, IDC_STATIONS));
  
			SetWindowPos(GetDlgItem(hWnd, IDC_SEEK), 0, 4 * my->offset + 2, 3 * my->row1 - 5, ourRect.right - 4 * my->offset - 4, (my->offset >> 1) + 2, SWP_NOACTIVATE | SWP_NOZORDER); 
			SetWindowPos(GetDlgItem(hWnd, IDC_TICKER), NULL, 4 * my->offset + 2, my->row1 + 2, ourRect.right - 6 * my->offset - 4, my->row1 + 6, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, IDC_SPECTRUM), NULL, ourRect.right - 2 * my->offset - 3, my->row1 + 2, 2 * my->offset +1, my->row1 + 6, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, IDC_SUBTICKER), NULL, 4 * my->offset + 2, 2 * my->row1 + 7, ourRect.right - 6 * my->offset - 4, (my->offset >> 1) - 8, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, IDC_RATING), NULL, ourRect.right - 2 * my->offset - 3, 2 * my->row1 + 7, 2 * my->offset +1, (my->offset >> 1) - 8, SWP_NOACTIVATE | SWP_NOZORDER);

			my->InitDC(TRUE);
			my->Paint();
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
					my->UpdateTuner();
					break;
			}
			break;

		case WM_DESTROY:
			KillTimer(hWnd, 1);
			break;

        case WM_DRAWITEM:
			DrawListBoxItem((LPDRAWITEMSTRUCT)lParam);
			break;

		case WM_PAINTOWNERDRAWN:
			DrawSortListBox((HWND)wParam);
			break;

		case WM_SAVEDATA:
			my->SaveSettings();
			break;

		case WM_MMBUTTON:
			switch(GET_APPCOMMAND_LPARAM(lParam))
			{
				case APPCOMMAND_MEDIA_CHANNEL_DOWN:
					PostMessage(hWnd, WM_COMMAND, MAKELONG(IDC_SCANDOWN, BN_CLICKED), 0);
					return(1L);

				case APPCOMMAND_MEDIA_CHANNEL_UP:
					PostMessage(hWnd, WM_COMMAND, MAKELONG(IDC_SCANUP, BN_CLICKED), 0);
					return(1L);

				case APPCOMMAND_MEDIA_RECORD:
					PostMessage(hWnd, WM_COMMAND, MAKELONG(IDC_RECORD, BN_CLICKED), 0);
					return(1L);

				case APPCOMMAND_MEDIA_STOP:
					if(SendDlgItemMessage(hWnd, IDC_RECORD, BM_GETCHECK, 0, 0) == BST_CHECKED)
						PostMessage(hWnd, WM_COMMAND, MAKELONG(IDC_RECORD, BN_CLICKED), 0);
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
static Font     * fn;

static void DrawSortListBox(HWND hWnd)
{
	RECT vListBox, vItem;
	INT  i, vCount;
	LONG vTop;

	if(!my) return;

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
	RECT		    crect;
	INT				i, n, size;
	WCHAR			idTxt[12];
	StringFormat	fmt;
	LPWSTR			pText;
	LONG            top;

	if(!my) return;

	int offset = my->offset;

	pItem->rcItem.right -= pItem->rcItem.left;
	pItem->rcItem.bottom -= pItem->rcItem.top;

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
			gr->FillRectangle(br_blue, pItem->rcItem.left, top, offset, pItem->rcItem.bottom);
			if(i % 2)
				gr->FillRectangle(br_blue, pItem->rcItem.left + offset, top, pItem->rcItem.right - offset, pItem->rcItem.bottom);
			else
				gr->FillRectangle(br_white, pItem->rcItem.left + offset, top, pItem->rcItem.right - offset, pItem->rcItem.bottom);
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
		gr->FillRectangle(br_blue, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.left + offset, pItem->rcItem.bottom);
		gr->FillRectangle(br_black, pItem->rcItem.left + offset, pItem->rcItem.top, pItem->rcItem.right - offset, pItem->rcItem.bottom);

		StringCchPrintfW(idTxt, 12, L"%u", pItem->itemID + 1);
		rf.X = (float)pItem->rcItem.left;
		rf.Width = (float)offset;
		gr->DrawString(idTxt, -1, fn, rf, &fmt, br_black);
		rf.X += (float)offset;
		rf.Width = (float)pItem->rcItem.right - offset;
	    if(pText != NULL)
			gr->DrawString(pText, -1, fn, rf, &fmt, br_white);
	}
	else if(n != 0)
	{
		if(pItem->itemID % 2)
			gr->FillRectangle(br_blue, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.right, pItem->rcItem.bottom);
		else
		{
			gr->FillRectangle(br_blue, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.left + offset, pItem->rcItem.bottom);
			gr->FillRectangle(br_white, pItem->rcItem.left + offset, pItem->rcItem.top, pItem->rcItem.right - offset, pItem->rcItem.bottom);
		}

		StringCchPrintfW(idTxt, 12, L"%u", pItem->itemID + 1);
		rf.X = (float)pItem->rcItem.left;
		rf.Width = (float)offset;
		gr->DrawString(idTxt, -1, fn, rf, &fmt, br_black);
		rf.X += (float)offset;
		rf.Width = (float)pItem->rcItem.right - offset;
	    if(pText != NULL)
			gr->DrawString(pText, -1, fn, rf, &fmt, br_black);
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
		fn = new Font(L"Arial", 16);
	}
	else if(lbox == NULL)
	{
		delete fn;
		delete br_blue;
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

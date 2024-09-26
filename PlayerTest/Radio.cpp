// Radio.cpp: implementation of the PlayWin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Radio.h"
#include "Ticker.h"
#include "Spectrum.h"

//#define TMRSYSMM		//Multimedia Timer	- heavy CPU use
#define TMRSYSWQ		//TimerWaitQueue
//#define TMRSYSST		//SetTimer			- inaccurate
//#define TMRSYSTH		//Wait Thread		- realtime prio level needed

#pragma warning(disable : 4995)

#ifdef TMRSYSMM
#pragma comment(lib, "Winmm.lib")
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

// REG2
#define ENABLE		(1 << 0)
#define DISABLE		(1 << 6)
#define SEEKON		(1 << 8)
#define SEEKUP		(1 << 9)
#define SEEKMODE	(1 << 10)
#define RDSMODE		(1 << 11)
#define MONO		(1 << 13)
#define DMUTE		(1 << 14)
#define DSMUTE		(1 << 15)
// REG3
#define TUNEON		(1 << 15)
// REG4 bits
#define RDS			(1 << 12)
#define DE			(1 << 11)
// REG5 bits
#define SEEKLVL(x)	(x << 8)
#define BAND(x)		(x << 6)
#define SPACE(x)	(x << 4)
#define VOLUME(x)	(x << 0)
// REG6
#define SEEKSNR(x)	(x << 4)
#define SEEKCNT(x)	(x << 0)
// REG10
#define RDSR		(1 << 15)
#define STC			(1 << 14)
#define RDSS		(1 << 11)
#define ST			(1 << 8)

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
	vChannel = 0;
	vUpdateFreq = TRUE;
	memset(vStereo, 0, sizeof(vStereo));
	hThread = NULL;
	hKillThread = NULL;
	vTunersel = '1';
	vAntenna = FALSE;

	MyRegisterClass(hInst);
	TickerInit(hInst);
	SpectrumInit(hInst);
	GetClientRect(hPrnt, &pRect);

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
	SendDlgItemMessage(hWnd, IDC_RECORD, BM_SETCHECK, BST_INDETERMINATE, 0);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, row2, w, offset, hWnd, (HMENU)IDC_TUNEUP, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_TUNEUP, IDI_TUNEUP);
	SendDlgItemMessage(hWnd, IDC_TUNEUP, WM_SETREPEAT, 0, (LPARAM)500);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, row2, (pRect.right - 5 * offset) - x, offset, hWnd, (HMENU)IDC_SCANUP, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_SCANUP, IDI_SCANUP);
	SendDlgItemMessage(hWnd, IDI_SCANUP, WM_SETREPEAT, 0, (LPARAM)1000);

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

	RetrieveSettings();

#ifdef TMRSYSMM
	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(TIMECAPS));
	vTimerRes = min(max(tc.wPeriodMin, 5), tc.wPeriodMax);
	timeBeginPeriod(vTimerRes);     
#endif

	HRESULT hr = CoInitialize(NULL);
	if(FAILED(hr))
		MessageBox(NULL, TEXT("CoInit failed"), TEXT("Error"), MB_OK);
	else
		OpenTuner();
}

Radio::~Radio()
{
	INT i;

#ifdef TMRSYSMM
	timeEndPeriod(vTimerRes);
#endif

	WriteRegister(2, 0xFFFF, DISABLE);
	InitListBox(FALSE, NULL);
	for(i = 0; i < (sizeof(bg) / sizeof(bg[0])); i++) delete bg[i];
	delete pn;
	for(i = 0; i < (sizeof(fn) / sizeof(fn[0])); i++) delete fn[i];
	for(i = 0; i < (sizeof(bitmap) / sizeof(bitmap[0])); i++) delete bitmap[i];
	if(graph) delete graph;
	if(image) delete image;

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
		if(vTunerOn)
			SetAntenna(TRUE);
	}
	else
	{
		if(SendDlgItemMessage(hWnd, IDC_RDSTA, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
			SetAntenna(FALSE);

		ShowWindow(hWnd, SW_HIDE);
	}
}

void Radio::SetAntenna(BOOL vOnOff)
{
	if(!dBase->GetBool("AntennaVaux"))
		return;

	if(vOnOff && !vAntenna)
	{
		PipeIo("IOw61", 6, NULL, 0);
		vAntenna = TRUE;
	}
	else if(!vOnOff && vAntenna)
	{
		PipeIo("IOw60", 6, NULL, 0);
		vAntenna = FALSE;
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


USHORT Radio::ReadRegister(INT vRegister)
{
	CHAR	vCmdTxt[5];
	BYTE	vInpTxt[5];
#ifdef LOCALPIPEIO
	DWORD	bytesRead;
	BOOL	vResult;
	CHAR	vRetry = 0;
#endif

	vCmdTxt[0] = 'T';
	vCmdTxt[1] = vTunersel;
	vCmdTxt[2] = 'r';
	vCmdTxt[3] = vRegister + '1';
	vCmdTxt[4] = '\0';

#ifdef LOCALPIPEIO
	do
	{
		vResult = CallNamedPipe(lpszPipeName, vCmdTxt, sizeof(vCmdTxt), vInpTxt, sizeof(vInpTxt), &bytesRead, PIPE_WAIT_TIME);
		if(!vResult)
		{
			if(GetLastError() == ERROR_TOO_MANY_OPEN_FILES)
			{
				HANDLE hWaitEvent;
				hWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
				if(hWaitEvent)
				{
					WaitForSingleObject(hWaitEvent, 1);
					CloseHandle(hWaitEvent);
				}
			}
			if(vRetry++ == 25)
			{
				vTunerOn = FALSE;
				return(vRadioRegisters[vRegister]);
			}
		}
		else if((vInpTxt[0] == 'E') && (vInpTxt[1] == 'R') && (vInpTxt[2] == 'R'))
		{
			vResult = FALSE;
			if(vRetry++ == 5)
			{
				vTunerOn = FALSE;
				return(vRadioRegisters[vRegister]);
			}
		}
	} while(!vResult);
#else
	PipeIo(vCmdTxt, sizeof(vCmdTxt), vInpTxt, sizeof(vInpTxt));
#endif

	if(vInpTxt[2] & 1) vInpTxt[0] = 0;
	if(vInpTxt[2] & 2) vInpTxt[1] = 0;

	vRadioRegisters[vRegister] = (((USHORT)vInpTxt[0]) << 8) | vInpTxt[1];
	return(vRadioRegisters[vRegister]);
}


void Radio::WriteRegister(INT vRegister, USHORT vMask, USHORT vData)
{
	USHORT	vCommand;
	CHAR	vCmdTxt[8];
	DWORD	byteCnt;
#ifdef LOCALPIPEIO
	BYTE	vInpTxt[5];
	BOOL	vResult;
	CHAR	vRetry = 0;
#endif

	vCommand = vRadioRegisters[vRegister];
	vCommand &= ~vMask;
	vCommand |= vData;
	vRadioRegisters[vRegister] = vCommand;

	byteCnt = 7;
	vCmdTxt[0] = 'T';
	vCmdTxt[1] = vTunersel;
	vCmdTxt[2] = 'w';
	vCmdTxt[3] = vRegister + '1';
	vCmdTxt[4] = vCommand >> 8;
	vCmdTxt[5] = vCommand & 0xFF;
	vCmdTxt[6] = '\0';
	vCmdTxt[7] = '\0';

	if(!vCmdTxt[4]) { vCmdTxt[4] = (CHAR)0xFF; vCmdTxt[6] |= 1; vCmdTxt[7] = 0; }
	if(!vCmdTxt[5]) { vCmdTxt[5] = (CHAR)0xFF; vCmdTxt[6] |= 2; vCmdTxt[7] = 0; }
	if(vCmdTxt[6]) byteCnt++;

#ifdef LOCALPIPEIO
	do
	{
		vResult = CallNamedPipe(lpszPipeName, vCmdTxt, byteCnt, vInpTxt, sizeof(vInpTxt), &byteCnt, PIPE_WAIT_TIME);
		if(!vResult)
		{
			if(GetLastError() == ERROR_TOO_MANY_OPEN_FILES)
			{
				HANDLE hWaitEvent;
				hWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
				if(hWaitEvent)
				{
					WaitForSingleObject(hWaitEvent, 1);
					CloseHandle(hWaitEvent);
				}
			}
			if(vRetry++ == 25)
			{
				vTunerOn = FALSE;
				return;
			}
		}
		else if((vInpTxt[0] == 'E') && (vInpTxt[1] == 'R') && (vInpTxt[2] == 'R'))
		{
			vResult = FALSE;
			if(vRetry++ == 5)
			{
				vTunerOn = FALSE;
				return;
			}
		}
	} while(!vResult);
#else
	PipeIo(vCmdTxt, byteCnt, NULL, 0);
#endif
}

CHAR vOldStationName[10] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0, 0};
CHAR vNewStationName[10] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0, 0};
CHAR vProgramName[65] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
						0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
						0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
						0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0};

#ifdef TMRSYSTH
DWORD WINAPI StatusThread(LPVOID lpParam) 
{
	HANDLE hEvents[2];
	LARGE_INTEGER liDueTime;

	liDueTime.QuadPart=0;

	hEvents[0] = CreateWaitableTimer(NULL, FALSE, NULL);
	hEvents[1] = (HANDLE)lpParam;
	if(hEvents[0] && hEvents[1])
	{
		SetWaitableTimer(hEvents[0], &liDueTime, 30, NULL, NULL, FALSE);

		while(WaitForMultipleObjects(2, hEvents, FALSE, INFINITE) == WAIT_OBJECT_0)
		{
			if(my)
			//	my->GetStatus();
				my->GetStatusFromThread();
			else
				break;
		}
		CloseHandle(hEvents[0]);
		CloseHandle(hEvents[1]);
	}
	ExitThread(0);
	return(0);
}
#endif

#ifdef TMRSYSMM
void CALLBACK StatusTimer(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2) 
{ 
	if(my)
		my->GetStatusFromThread();
} 
#endif

#ifdef TMRSYSWQ
void CALLBACK StatusTimerQueueCb(void* lpParam, BOOLEAN TimerOrWaitFired) 
{
	static BOOL vBusy = FALSE;
	if(vBusy)
		return;
	vBusy = TRUE;
	if(my)
		my->GetStatusFromThread();
	vBusy = FALSE;
} 
#endif


void Radio::OpenTuner(void)
{
	CHAR	vInpTxt[10];
	DWORD	bytesRead;
	INT		x;

	StringCbPrintf(szCurrent, sizeof(szCurrent), TEXT("FM RDS Tuner"));
	if(!CallNamedPipe(lpszPipeName, "SIrMI", 5, vInpTxt, sizeof(vInpTxt), &bytesRead, PIPE_WAIT_TIME))
	{
		StringCbPrintf(szInfo, sizeof(szInfo), TEXT(" CARGO System Access Service not running"));
		SendDlgItemMessage(hWnd, IDC_SCANDOWN, BM_SETCHECK, BST_INDETERMINATE, 0);
		SendDlgItemMessage(hWnd, IDC_TUNEDOWN, BM_SETCHECK, BST_INDETERMINATE, 0);
		SendDlgItemMessage(hWnd, IDC_RECORD, BM_SETCHECK, BST_INDETERMINATE, 0);
		SendDlgItemMessage(hWnd, IDC_TUNEUP, BM_SETCHECK, BST_INDETERMINATE, 0);
		SendDlgItemMessage(hWnd, IDC_SCANUP, BM_SETCHECK, BST_INDETERMINATE, 0);
		SendDlgItemMessage(hWnd, IDC_EDIT, BM_SETCHECK, BST_INDETERMINATE, 0);
		SendDlgItemMessage(hWnd, IDC_EDITCURR, BM_SETCHECK, BST_INDETERMINATE, 0);
		SendDlgItemMessage(hWnd, IDC_SEEK, BM_SETCHECK, BST_INDETERMINATE, 0);
		vTunerOn = FALSE;
		SetTimer(hWnd, 1, 2000, NULL);
	}
	else
	{
		if((vInpTxt[0] == 'E') && (vInpTxt[1] == 'R') && (vInpTxt[2] == 'R'))
		{
			StringCbPrintf(szInfo, sizeof(szInfo), TEXT(" Communication error with CARGO System Access Controller"));
			SendDlgItemMessage(hWnd, IDC_SCANDOWN, BM_SETCHECK, BST_INDETERMINATE, 0);
			SendDlgItemMessage(hWnd, IDC_TUNEDOWN, BM_SETCHECK, BST_INDETERMINATE, 0);
			SendDlgItemMessage(hWnd, IDC_RECORD, BM_SETCHECK, BST_INDETERMINATE, 0);
			SendDlgItemMessage(hWnd, IDC_TUNEUP, BM_SETCHECK, BST_INDETERMINATE, 0);
			SendDlgItemMessage(hWnd, IDC_SCANUP, BM_SETCHECK, BST_INDETERMINATE, 0);
			SendDlgItemMessage(hWnd, IDC_EDIT, BM_SETCHECK, BST_INDETERMINATE, 0);
			SendDlgItemMessage(hWnd, IDC_EDITCURR, BM_SETCHECK, BST_INDETERMINATE, 0);
			SendDlgItemMessage(hWnd, IDC_SEEK, BM_SETCHECK, BST_INDETERMINATE, 0);
			vTunerOn = FALSE;
			SetTimer(hWnd, 1, 2000, NULL);
		}
		else
		{
			if(vInpTxt[0] == 0x01)
			{
				// CARGO system
				vTunersel = '2';
				// init tuner
				CallNamedPipe(lpszPipeName, "T2s0", 5, vInpTxt, sizeof(vInpTxt), &bytesRead, PIPE_WAIT_TIME);
				CallNamedPipe(lpszPipeName, "T2s1", 5, vInpTxt, sizeof(vInpTxt), &bytesRead, PIPE_WAIT_TIME);
			}
		//	else
			{
				// MERGO or UNIGO
				vTunersel = '1';
			}
			memset(vRadioRegisters, 0, sizeof(vRadioRegisters));
			WriteRegister(2, 0xFFFF, DISABLE);
			WriteRegister(2, 0xFFFF, DSMUTE | DMUTE | ENABLE);
			WriteRegister(3, 0xFFFF, 0);
			WriteRegister(4, 0xFFFF, RDS | DE);
			WriteRegister(5, 0xFFFF, SEEKLVL(20) | VOLUME(15));
			SetParams(EUROPE, SP100);
			Sensitivity(TRUE);

			for(x = 0; x < 16; x++)
				ReadRegister(x);

			ScanTimeout();
			if(vTunersel == '1')
				for(x = 1; x < 16; x++)
					ScanTimeout();
			vTunerOn = TRUE;
			SetAntenna(TRUE);

			// By now every should have been loaded...
			//RetrieveSettings();
			LPVOID pPtr = (LPVOID)&vChannel;
			dBase->GetConfig("TunerChannel", &pPtr, CONFIGTYPE_INT);
			SetFrequency(vChannel);

			SendDlgItemMessage(hWnd, IDC_SCANDOWN, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hWnd, IDC_TUNEDOWN, BM_SETCHECK, BST_UNCHECKED, 0);
			//SendDlgItemMessage(hWnd, IDC_RECORD, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hWnd, IDC_TUNEUP, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hWnd, IDC_SCANUP, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hWnd, IDC_EDIT, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hWnd, IDC_EDITCURR, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hWnd, IDC_SEEK, BM_SETCHECK, BST_UNCHECKED, 0);

			SendDlgItemMessage(hWnd, IDC_RDSTA, BM_SETCHECK, (pAi->vTA ? BST_CHECKED : BST_UNCHECKED), 0);

#ifdef TMRSYSST
			SetTimer(hWnd, 1, 30, NULL);
#else
			SetTimer(hWnd, 1, 500, NULL);
#endif

#ifdef TMRSYSMM
			if(timeSetEvent(30, 5, StatusTimer, NULL, TIME_PERIODIC) == NULL)
				MessageBoxA(NULL, "timeSetEvent failed", "", MB_OK);
#endif

#ifdef TMRSYSWQ
			if(!hThread)
				CreateTimerQueueTimer(&hThread, NULL, StatusTimerQueueCb, NULL, 30, 30, WT_EXECUTEINIOTHREAD);
#endif

#ifdef TMRSYSTH
			if(!hThread)
			{
				hKillThread = CreateEvent(NULL, TRUE, FALSE, NULL);
				hThread = CreateThread(NULL, 0, StatusThread, hKillThread, 0, NULL);
				if(hThread)
				{
					SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);
					SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
				}
			}
#endif
		}
	}
}

void Radio::CloseTuner(void)
{
	SetAntenna(FALSE);

#ifdef TMRSYSWQ
	if(hThread)
	{
		DeleteTimerQueueTimer(NULL, hThread, INVALID_HANDLE_VALUE);
		hThread = NULL;
	}
#endif

#ifdef TMRSYSTH
	if(hKillThread)
		SetEvent(hKillThread);
	if(hThread)
	{
		WaitForSingleObject(hThread, 1000);
		CloseHandle(hThread);
	}
	hKillThread = NULL;
	hThread = NULL;
#endif
}

void Radio::ResetTexts(void)
{
	StringCbPrintf(szCurrent, sizeof(szCurrent), TEXT("FM RDS Tuner"));
	SetDlgItemText(hWnd, IDC_TICKER, szCurrent);
	memset(vOldStationName, 0, sizeof(vOldStationName));
	memset(vNewStationName, 0, sizeof(vNewStationName));

	StringCbPrintf(szInfo, sizeof(szInfo), TEXT(""));
	SetDlgItemText(hWnd, IDC_SUBTICKER, szInfo);
	memset(vProgramName, 0, sizeof(vProgramName));

	AssignIcon(hInst, hWnd, IDC_RDSTA, 0);
}

//Unused
void Radio::GetStatus(void)
{
	static	vResetRds = 0;
	INT		vLevel;
	UINT	vRdsData[4];
	INT		i;

	if(vTunerOn)
	{
		if(ReadRegister(10) & RDSR)
		{
			vResetRds = 0;
			vRdsData[0] = ReadRegister(12);
			vRdsData[1] = ReadRegister(13);
			vRdsData[2] = ReadRegister(14);
			vRdsData[3] = ReadRegister(15);

			switch(vRdsData[1] & 0xF800)
			{
				case 0x0000:	// Type 0 groups : Basic tuning and switching information
					if((vTP != 0x0410) && ((vRdsData[1] & 0x0410) == 0x0410))	// TA activated
					{
						if(vTAFilter++ == 8)
						{
							vTAFilter = 0;
							vTP = vRdsData[1] & 0x0410;
							SendMessage(hPrnt, WM_RADIOTA, FALSE, TRUE);
						}
					}
					else if((vTP == 0x0410) && ((vRdsData[1] & 0x0410) == 0x0400))	// TA deactivated
					{
						if(vTAFilter++ == 4)
						{
							vTAFilter = 0;
							vTP = vRdsData[1] & 0x0410;
							SendMessage(hPrnt, WM_RADIOTA, FALSE, FALSE);
						}
					}

					if(vTP == 0x0410)
						AssignIcon(hInst, hWnd, IDC_RDSTA, IDI_TAON);
					else
						AssignIcon(hInst, hWnd, IDC_RDSTA, 0);

					i = 2 * (vRdsData[1] & 0x3);
					vNewStationName[i++] = vRdsData[3] >> 8;
					vNewStationName[i++] = vRdsData[3] & 0xFF;
					vNewStationName[8] = '\0';
					if(i == 8)
					{
						if(strcmp(vOldStationName, vNewStationName) == 0)
						{
							StringCbPrintf(szCurrent, sizeof(szCurrent), TEXT("%hs"), vOldStationName);
							SetDlgItemText(hWnd, IDC_TICKER, szCurrent);
						}
						strcpy(vOldStationName, vNewStationName);
					}

					if(!(vRdsData[1] & 0x0800))	// Type 0A groups : Check Alternative Frequencies table
					{
					}
					break;

				case 0x1000:	// Type 1 groups: Programme Item Number and slow labelling codes (for TMC)
					break;

				case 0x2000:	// Type 2 groups: RadioText
				case 0x2800:
					if((vRdsData[1] & 0x0010) != vRefresh)
					{
						memset(vProgramName, 0, sizeof(vProgramName));
						vRefresh = vRdsData[1] & 0x0010;
					}
					i = vRdsData[1] & 0xF;
					vProgramName[4*i] = vRdsData[2] >> 8;
					if(i)
						if(vProgramName[4*i-1] == ' ' && vProgramName[4*i] == ' ') vProgramName[4*i] = '\0';
					vProgramName[4*i+1] = vRdsData[2] & 0xFF;
					if(vProgramName[4*i] == ' ' && vProgramName[4*i+1] == ' ') vProgramName[4*i+1] = '\0';
					vProgramName[4*i+2] = vRdsData[3] >> 8;
					if(vProgramName[4*i+1] == ' ' && vProgramName[4*i+2] == ' ') vProgramName[4*i+2] = '\0';
					vProgramName[4*i+3] = vRdsData[3] & 0xFF;
					if(vProgramName[4*i+2] == ' ' && vProgramName[4*i+3] == ' ') vProgramName[4*i+3] = '\0';

					StringCbPrintf(szInfo, sizeof(szInfo), TEXT(" %hs"), vProgramName);
					SetDlgItemText(hWnd, IDC_SUBTICKER, szInfo);
					break;

				case 0x3000:	// Type 3A groups: Application identification for Open data
					break;

				case 0x4000:	// Type 4A groups : Clock-time and date
					break;

				case 0x8000:	// Type 8A groups: Traffic Message Channel or ODA
					break;
			}
		}
		else if(vResetRds++ > 66)
		{
			ResetTexts();
			vResetRds = 0;
		}

		if(vChannel != (ReadRegister(11) & 0x3FF))
		{
			vChannel = vRadioRegisters[11] & 0x3FF;
			ResetTexts();
			vUpdateFreq = TRUE;
			vTAFilter = 0;
		}

		vLevel = vRadioRegisters[10] & 0xFF;
		if(vLevel != vRssi)
		{
			vRssi = vLevel;
			vUpdateFreq = TRUE;
		}

		if((vRadioRegisters[10] & ST) != vStereoMode)
		{
			vStereoMode = vRadioRegisters[10] & ST;
			vUpdateFreq = TRUE;
			StringCbPrintf(vStereo, sizeof(vStereo), (vRadioRegisters[10] & ST) ? L"STEREO" : L"MONO");
		}
	}
}

void Radio::GetStatusFromThread(void)
{
	static	vResetRds = 0;
	INT		vLevel;

	if(vTunerOn)
	{
		if(ReadRegister(10) & RDSR)
		{
			WPARAM wParam = MAKELONG(ReadRegister(13), ReadRegister(12));
			LPARAM lParam = MAKELONG(ReadRegister(15), ReadRegister(14));
			PostMessage(hWnd, WM_UPDATERDS, wParam, lParam);
			vResetRds = 0;
		}
		else if(vResetRds++ > 66)
		{
			PostMessage(hWnd, WM_RESETRDS, 0, 0);
			vResetRds = 0;
		}

		if(vChannel != (ReadRegister(11) & 0x3FF))
		{
			vChannel = vRadioRegisters[11] & 0x3FF;
			PostMessage(hWnd, WM_RESETRDS, 0, 0);
			vUpdateFreq = TRUE;
			vTAFilter = 0;
		}

		vLevel = vRadioRegisters[10] & 0xFF;
		if(vLevel != vRssi)
		{
			vRssi = vLevel;
			vUpdateFreq = TRUE;
		}

		if((vRadioRegisters[10] & ST) != vStereoMode)
		{
			vStereoMode = vRadioRegisters[10] & ST;
			vUpdateFreq = TRUE;
			StringCbPrintf(vStereo, sizeof(vStereo), (vRadioRegisters[10] & ST) ? L"STEREO" : L"MONO");
		}
	}
}

void Radio::UpdateRds(WPARAM wParam, LPARAM lParam)
{
	UINT	vRdsData[4];
	INT		i;

	if(vTunerOn)
	{
		vRdsData[0] = HIWORD(wParam);
		vRdsData[1] = LOWORD(wParam);
		vRdsData[2] = HIWORD(lParam);
		vRdsData[3] = LOWORD(lParam);

		switch(vRdsData[1] & 0xF800)
		{
			case 0x0000:	// Type 0 groups : Basic tuning and switching information
				if((vTP != 0x0410) && ((vRdsData[1] & 0x0410) == 0x0410))	// TA activated
				{
					if(vTAFilter++ == 8)
					{
						vTAFilter = 0;
						vTP = vRdsData[1] & 0x0410;
						SendMessage(hPrnt, WM_RADIOTA, FALSE, TRUE);
					}
				}
				else if((vTP == 0x0410) && ((vRdsData[1] & 0x0410) == 0x0400))	// TA deactivated
				{
					if(vTAFilter++ == 4)
					{
						vTAFilter = 0;
						vTP = vRdsData[1] & 0x0410;
						SendMessage(hPrnt, WM_RADIOTA, FALSE, FALSE);
					}
				}

				if(vTP == 0x0410)
					AssignIcon(hInst, hWnd, IDC_RDSTA, IDI_TAON);
				else
					AssignIcon(hInst, hWnd, IDC_RDSTA, 0);

				i = 2 * (vRdsData[1] & 0x3);
				vNewStationName[i++] = vRdsData[3] >> 8;
				vNewStationName[i++] = vRdsData[3] & 0xFF;
				vNewStationName[i] = '\0';
				if(i == 8)
				{
					if(strcmp(vOldStationName, vNewStationName) == 0)
					{
						StringCbPrintf(szCurrent, sizeof(szCurrent), TEXT("%hs"), vOldStationName);
						SetDlgItemText(hWnd, IDC_TICKER, szCurrent);
					}
					strcpy(vOldStationName, vNewStationName);
					memset(vNewStationName, 0, sizeof(vNewStationName));
				}

				if(!(vRdsData[1] & 0x0800))	// Type 0A groups : Check Alternative Frequencies table
				{
				}
				break;

			case 0x1000:	// Type 1 groups: Programme Item Number and slow labelling codes (for TMC)
				break;

			case 0x2000:	// Type 2 groups: RadioText
			case 0x2800:
				if((vRdsData[1] & 0x0010) != vRefresh)
				{
					memset(vProgramName, 0, sizeof(vProgramName));
					vRefresh = vRdsData[1] & 0x0010;
				}
				i = vRdsData[1] & 0xF;
				vProgramName[4*i] = vRdsData[2] >> 8;
				if(i)
					if(vProgramName[4*i-1] == ' ' && vProgramName[4*i] == ' ') vProgramName[4*i] = '\0';
				vProgramName[4*i+1] = vRdsData[2] & 0xFF;
				if(vProgramName[4*i] == ' ' && vProgramName[4*i+1] == ' ') vProgramName[4*i+1] = '\0';
				vProgramName[4*i+2] = vRdsData[3] >> 8;
				if(vProgramName[4*i+1] == ' ' && vProgramName[4*i+2] == ' ') vProgramName[4*i+2] = '\0';
				vProgramName[4*i+3] = vRdsData[3] & 0xFF;
				if(vProgramName[4*i+2] == ' ' && vProgramName[4*i+3] == ' ') vProgramName[4*i+3] = '\0';

				StringCbPrintf(szInfo, sizeof(szInfo), TEXT(" %hs"), vProgramName);
				SetDlgItemText(hWnd, IDC_SUBTICKER, szInfo);
				break;

			case 0x3000:	// Type 3A groups: Application identification for Open data
				break;

			case 0x4000:	// Type 4A groups : Clock-time and date
				break;

			case 0x8000:	// Type 8A groups: Traffic Message Channel or ODA
				break;
		}
	}
}

BOOL Radio::ScanTimeout(void)
{
	INT vTimeout = 0;
	while(ReadRegister(10) & STC) if(vTimeout++ > 100) return(TRUE);
	return(FALSE);
}

void Radio::SetParams(REGION vReg, SPACING vSpace)
{
	vSpacing = vSpace;
	vRegion = vReg;

	WriteRegister(5, (UINT)(REGMASK | SPMASK), (UINT)(vReg | vSpace));

	switch(vReg)
	{
		case EUROPE:	vChannelMax = 1080-875; break;	// 108 - 87.5 * 10
		case JAPANWB:	vChannelMax = 1080-760; break;	// 108 - 76 * 10
		case JAPAN:		vChannelMax = 900-760; break;	// 90 - 76 * 10
		default:		vChannelMax = 10; break;
	}
	if(vSpace == SP50)	vChannelMax *= 2;
	if(vSpace == SP200)	vChannelMax /= 2;

	SendDlgItemMessage(hWnd, IDC_SEEK, TBM_SETRANGE, TRUE, MAKELONG(0, vChannelMax));
}

void Radio::Scan(BOOL vDirection)
{
	USHORT	vCommand;

	//Stop pending seek
	WriteRegister(2, SEEKON, 0);
	WriteRegister(3, TUNEON, 0);
	if(!ScanTimeout())
	{
		vCommand = vDirection ? SEEKUP : 0;	// up/down
		vCommand |= SEEKON | SEEKMODE;	// start seek

		WriteRegister(2, SEEKON | SEEKUP | SEEKMODE, vCommand);
	}
}

void Radio::Tune(BOOL vDirection)
{
	INT	vChan;

	//Stop pending tune
	WriteRegister(2, SEEKON, 0);
	WriteRegister(3, TUNEON, 0);
	if(!ScanTimeout())
	{
		vChan = vRadioRegisters[11];
		vChan &= 0x3FF;

		if(vDirection)
			vChan++;
		else
			vChan--;
		vChan &= 0x3FF;
		if(vChan > vChannelMax)
			vChan = 0;

		WriteRegister(3, 0xFFFF, TUNEON | vChan);
	}
}

void Radio::SetFrequency(INT vChan)
{
	//Stop pending tune
	WriteRegister(2, SEEKON, 0);
	WriteRegister(3, TUNEON, 0);
	if(!ScanTimeout())
	{
		vChan &= 0x3FF;
		if(vChan > vChannelMax)
			vChan = vChannelMax;

		WriteRegister(3, 0xFFFF, TUNEON | vChan);
	}
}

void Radio::MonoStereo(BOOL vMono)
{
	WriteRegister(2, MONO, (vMono ? MONO : 0));
}

void Radio::Sensitivity(BOOL vHigh)
{
	if(vHigh)
	{
		WriteRegister(5, 0xFF00, SEEKLVL(25));
		WriteRegister(6, 0x00FF, SEEKSNR(4) | SEEKCNT(8));
	}
	else
	{
		WriteRegister(5, 0xFF00, SEEKLVL(0));
		WriteRegister(6, 0x00FF, SEEKSNR(4) | SEEKCNT(15));
	}
}

void Radio::Mute(BOOL vMute)
{
	CHAR	vInpTxt[10];
//	WriteRegister(2, DMUTE, (vMute ? 0 : DMUTE));

	if(!vMute)
		PipeIo("MXwS3", 6, vInpTxt, sizeof(vInpTxt));
	else
		PipeIo("MXwS1", 6, vInpTxt, sizeof(vInpTxt));
}

void Radio::UpdateTuner(void)
{
	static INT vRefresh = 0;

	if(vTunerOn)
	{
#ifdef TMRSYSST
		GetStatus();
		if(vUpdateFreq && (vRefresh++ % 33 == 0))
#else
		if(vUpdateFreq)
#endif
		{
			PaintTime();
			vUpdateFreq = FALSE;
		}
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
	switch(vSpacing)
	{
		case 0x00:	vFrequency = vChannel * 0.2;		break;	// 200kHz
		case 0x10:	vFrequency = vChannel * 0.1;		break;	// 100kHz
		case 0x20:	vFrequency = vChannel * 0.05;	break;	// 50kHz
		default:	vFrequency = 0;	break;	//error
	}
	switch(vRegion)
	{
		case 0x00:	vFrequency += 87.5;		break;	// Europe/US
		case 0x40:	vFrequency += 76.0;		break;	// Japan WB
		case 0x80:	vFrequency += 76.0;		break;	// Japan
		default:	vFrequency = 0;	break;	//error
	}
	StringCbPrintfW(vBuffer, sizeof(vBuffer), L"%.1f MHz", vFrequency);
	graph->DrawString(vBuffer, -1, fn[0], point, &fmt, bg[BG_DBLUE]);

	// Print RDS error rate
	point.X = (float)4 * offset - 4;
	point.Y = (float)(2 * row1 + 9);
	graph->DrawString(vStereo, -1, fn[1], point, &fmt, bg[BG_DBLUE]);

	// Print signal level
	point.Y = (float)(3 * row1 - 2);
	StringCbPrintfW(vBuffer, sizeof(vBuffer), L"%u dB%cV", vRssi, 181);
	graph->DrawString(vBuffer, -1, fn[1], point, &fmt, bg[BG_DBLUE]);

	SendDlgItemMessage(hWnd, IDC_SEEK, TBM_SETPOS, TRUE, vChannel);

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
	dBase->SetConfig("TunerChannel", &vChannel, CONFIGTYPE_INT);

	// Save station list
	dBase->SaveStations(GetDlgItem(hWnd, IDC_STATIONS));

	saved = TRUE;
}

void Radio::RetrieveSettings()
{
	BOOL	toggle;
	//LPVOID	pPtr;

	// Get range and mono/stero mode
	toggle = dBase->GetBool("TunerRange");
	SendDlgItemMessage(hWnd, IDC_DX, BM_SETCHECK, (toggle ? BST_CHECKED : BST_UNCHECKED), 0);
	MonoStereo(toggle);

	toggle = dBase->GetBool("TunerMono");
	SendDlgItemMessage(hWnd, IDC_MONO, BM_SETCHECK, (toggle ? BST_CHECKED : BST_UNCHECKED), 0);
	SetWindowText(GetDlgItem(hWnd, IDC_MONO), (toggle ? TEXT("MON") : TEXT(">|<")));
	Sensitivity(toggle);

	//pPtr = (LPVOID)&vChannel;
	//dBase->GetConfig("TunerChannel", &pPtr, CONFIGTYPE_INT);
	//SetFrequency(vChannel);

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
						case IDC_SCANUP:
							my->Scan(TRUE);
							break;
						case IDC_SCANDOWN:
							my->Scan(FALSE);
							break;
						case IDC_TUNEUP:
							my->Tune(TRUE);
							break;
						case IDC_TUNEDOWN:
							my->Tune(FALSE);
							break;

						case IDC_RECORD:
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
							my->Sensitivity(toggle);
							break;
						case IDC_MONO:
							toggle = (SendDlgItemMessage(hWnd, IDC_MONO, BM_GETCHECK, 0, 0) == BST_UNCHECKED ? TRUE : FALSE);
							dBase->SetConfig("TunerMono", (LPVOID)&toggle, CONFIGTYPE_BOOL);
							SendDlgItemMessage(hWnd, IDC_MONO, BM_SETCHECK, (toggle ? BST_CHECKED : BST_UNCHECKED), 0);
							SetWindowText(GetDlgItem(hWnd, IDC_MONO), (toggle ? TEXT("MON") : TEXT(">|<")));
							my->MonoStereo(toggle);
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
									StringCbPrintf(pString, x, TEXT("%ls (%.1f MHz)"), vStation, my->vFrequency);
								}
								else
									StringCbPrintf(pString, x, TEXT("%.1f MHz"), my->vFrequency);

								x = SendDlgItemMessage(hWnd, IDC_STATIONS, LB_ADDSTRING, 0, (LPARAM)pString);
								SendDlgItemMessage(hWnd, IDC_STATIONS, LB_SETITEMDATA, (WPARAM)x, (LPARAM)my->vChannel);
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
										StringCbPrintf(pString, x, TEXT("%ls (%.1f MHz)"), vStation, my->vFrequency);
									}
									else
										StringCbPrintf(pString, x, TEXT("%.1f MHz"), my->vFrequency);

									SendDlgItemMessage(hWnd, IDC_STATIONS, LB_INSERTSTRING, 0, (LPARAM)pString);
									SendDlgItemMessage(hWnd, IDC_STATIONS, LB_SETITEMDATA, 0, (LPARAM)my->vChannel);
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
								SendDlgItemMessage(hWnd, IDC_STATIONS, LB_SETCURSEL, (WPARAM)curSel, 0);
							}
							SetListControls(hWnd);
							break;

						case IDC_PLAYNOW:
							curSel = SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETCURSEL, 0, 0);
							if(curSel != LB_ERR)
							{
								x = SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETITEMDATA, (WPARAM)curSel, 0);
								my->SetFrequency(x);
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
				my->SetFrequency(x);
				my->vChannel = x;
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
			my->UpdateTuner();
			break;

		case WM_UPDATERDS:
			my->UpdateRds(wParam, lParam);
			break;

		case WM_RESETRDS:
			my->ResetTexts();
			break;

		case WM_DESTROY:
			my->CloseTuner();
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

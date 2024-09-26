// Call.cpp: implementation of the Call class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Call.h"
#include "Ticker.h"

#include "commctrl.h"

#pragma comment(lib, "Strmiids.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

#define BG_WHITE		0
#define BG_GREEN		1
#define BG_LBLUE		2
#define BG_DBLUE		3

LRESULT CALLBACK CallProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static Call * myPtr;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Call::Call(HINSTANCE hInstance, HWND hParent, int bottomOffset)
{
	RECT  pRect;
	FontFamily * fm;

	myPtr = this;

	hInst = hInstance;
	hPrnt = hParent;
	offset = bottomOffset;
	myGraphics = NULL;
	muted = FALSE;

	pGraph = NULL;
	pConfig = NULL;
	pAudio = NULL;

	hHeap = GetProcessHeap();
	MyRegisterClass(hInstance);
	TickerInit(hInstance);
	GetClientRect(hParent, &pRect);

	bg[BG_WHITE] = new SolidBrush(Color(255, 255, 255, 255));
	bg[BG_GREEN] = new SolidBrush(Color(255, 200, 255, 200));
	bg[BG_LBLUE] = new SolidBrush(Color(255, 201, 211, 227));
	bg[BG_DBLUE] = new SolidBrush(Color(255, 0, 0, 192));
	pn = new Pen(Color(255, 0, 0, 0), 1);

	fm = new FontFamily(TEXT("Arial"));
	fn[0] = new Font(fm, (float)bottomOffset * 2 / 3, FontStyleRegular, UnitPixel);
	fn[1] = new Font(fm, (float)(bottomOffset >> 1) - 7, FontStyleRegular, UnitPixel);
	delete fm;

	hCall = CreateWindow(TEXT("CRPC_CRKT"), TEXT("CarPC call window"), WS_CHILD | WS_CLIPCHILDREN,
		0, 0, pRect.right, pRect.bottom - bottomOffset, hParent, NULL, hInstance, NULL);

	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("1"), WS_CHILD | WS_VISIBLE, 0, 0,
		bottomOffset, bottomOffset, hCall, (HMENU)IDC_ONE, hInstance, NULL);
	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("2"), WS_CHILD | WS_VISIBLE, 0, 0,
		bottomOffset, bottomOffset, hCall, (HMENU)IDC_TWO, hInstance, NULL);
	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("3"), WS_CHILD | WS_VISIBLE, 0, 0,
		bottomOffset, bottomOffset, hCall, (HMENU)IDC_THREE, hInstance, NULL);
	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("4"), WS_CHILD | WS_VISIBLE, 0, 0,
		bottomOffset, bottomOffset, hCall, (HMENU)IDC_FOUR, hInstance, NULL);
	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("5"), WS_CHILD | WS_VISIBLE, 0, 0,
		bottomOffset, bottomOffset, hCall, (HMENU)IDC_FIVE, hInstance, NULL);
	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("6"), WS_CHILD | WS_VISIBLE, 0, 0,
		bottomOffset, bottomOffset, hCall, (HMENU)IDC_SIX, hInstance, NULL);
	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("7"), WS_CHILD | WS_VISIBLE, 0, 0,
		bottomOffset, bottomOffset, hCall, (HMENU)IDC_SEVEN, hInstance, NULL);
	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("8"), WS_CHILD | WS_VISIBLE, 0, 0,
		bottomOffset, bottomOffset, hCall, (HMENU)IDC_EIGHT, hInstance, NULL);
	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("9"), WS_CHILD | WS_VISIBLE, 0, 0,
		bottomOffset, bottomOffset, hCall, (HMENU)IDC_NINE, hInstance, NULL);
	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("*"), WS_CHILD | WS_VISIBLE, 0, 0,
		bottomOffset, bottomOffset, hCall, (HMENU)IDC_STAR, hInstance, NULL);
	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("0"), WS_CHILD | WS_VISIBLE, 0, 0,
		bottomOffset, bottomOffset, hCall, (HMENU)IDC_ZERO, hInstance, NULL);
	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("#"), WS_CHILD | WS_VISIBLE, 0, 0,
		bottomOffset, bottomOffset, hCall, (HMENU)IDC_SHARP, hInstance, NULL);

	SetKeypadPositions();

	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("YES"), WS_CHILD | WS_VISIBLE, 1 * bottomOffset, 2 * bottomOffset,
		(3 * bottomOffset) >> 1, bottomOffset, hCall, (HMENU)IDC_CYES, hInstance, NULL);
//	ico = LoadIcon(hInstance, (LPCTSTR)IDI_CYES);
//	SendDlgItemMessage(hCall, IDC_YES, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
//	DestroyIcon(ico);

	CreateWindow(TEXT("WB_DESTBUTTON"), TEXT("NO"), WS_CHILD | WS_VISIBLE, (7 * bottomOffset) >> 1, 2 * bottomOffset,
		(3 * bottomOffset) >> 1, bottomOffset, hCall, (HMENU)IDC_CNO, hInstance, NULL);
//	ico = LoadIcon(hInstance, (LPCTSTR)IDI_CNO);
//	SendDlgItemMessage(hCall, IDC_NO, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
//	DestroyIcon(ico);

	CreateWindow(TEXT("WB_TICKERBOX"), TEXT("test"), WS_CHILD | WS_VISIBLE, bottomOffset, bottomOffset >> 1, 
		ourRect.right - 2 * bottomOffset, bottomOffset, hCall, (HMENU)IDC_DISPLAY, hInstance, NULL);

	HRESULT hr = CoInitialize(NULL);
	if(FAILED(hr))
	{
		MessageBox(NULL, TEXT("CoInit failed"), TEXT("Error"), MB_OK);
	}
	else
	{
		DShowInit(TRUE);
	}
}

Call::~Call()
{
	delete bg[BG_GREEN];
	delete bg[BG_DBLUE];
	delete bg[BG_LBLUE];
	delete bg[BG_WHITE];
	delete pn;
	delete fn[0];
	delete fn[1];
	DShowInit(FALSE);
	CoUninitialize();
	DestroyWindow(hCall);
}

HRESULT Call::DShowInit(BOOL loadUnload)
{
	HRESULT hr;

	HANDLE logFile;

	logFile = CreateFile(TEXT("DSHOW.LOG"), FILE_ALL_ACCESS, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(loadUnload)
	{
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
				IID_IGraphBuilder, (void **)&pGraph);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IGraphBuilder failed"), TEXT("Error"), MB_OK);
			return hr;
		}
		pGraph->SetLogFile((DWORD_PTR)logFile);

		hr = pGraph->QueryInterface(IID_IBasicAudio, (void **)&pAudio);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("IID_IBasicAudio failed"), TEXT("Error"), MB_OK);
			return hr;
		}
		pAudio->put_Volume(0);
		return(hr);
	}
	else
	{
		if(pAudio)    pAudio->Release();	pAudio = NULL;
		return(0);
	}
}

void Call::SetKeypadPositions(void)
{
    RECT	ourRect;
	int		x[3], y;

	GetClientRect(hCall, &ourRect);
	x[0] = ourRect.right / 2;
	x[1] = ourRect.right / 2 + 1 * offset;
	x[2] = ourRect.right / 2 + 2 * offset;
	y = 2 * offset;

	SetWindowPos(GetDlgItem(hCall, IDC_ONE), 0, x[0], y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	SetWindowPos(GetDlgItem(hCall, IDC_TWO), 0, x[1], y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	SetWindowPos(GetDlgItem(hCall, IDC_THREE), 0, x[2], y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	y += offset;
	SetWindowPos(GetDlgItem(hCall, IDC_FOUR), 0, x[0], y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	SetWindowPos(GetDlgItem(hCall, IDC_FIVE), 0, x[1], y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	SetWindowPos(GetDlgItem(hCall, IDC_SIX), 0, x[2], y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	y += offset;
	SetWindowPos(GetDlgItem(hCall, IDC_SEVEN), 0, x[0], y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	SetWindowPos(GetDlgItem(hCall, IDC_EIGHT), 0, x[1], y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	SetWindowPos(GetDlgItem(hCall, IDC_NINE), 0, x[2], y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	y += offset;
	SetWindowPos(GetDlgItem(hCall, IDC_STAR), 0, x[0], y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	SetWindowPos(GetDlgItem(hCall, IDC_ZERO), 0, x[1], y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	SetWindowPos(GetDlgItem(hCall, IDC_SHARP), 0, x[2], y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

	SetWindowPos(GetDlgItem(hCall, IDC_DISPLAY), 0, offset, offset / 2, ourRect.right - 2 * offset, offset, SWP_NOACTIVATE | SWP_NOZORDER);

}


Call::Show(BOOL yesNo)
{
	if(yesNo)
	{
		ShowWindow(hCall, SW_SHOW);
		UpdateWindow(hCall);
		PostMessage(hCall, WM_PAINT, 0, 0);
	}
	else
	{
		ShowWindow(hCall, SW_HIDE);
	}
}

ATOM Call::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= CallProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_CRKT");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

void Call::InitDC(BOOL updateDC)
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

void Call::Paint()
{
//	RECT  lbRect;

	if(myGraphics == NULL)
		return;

	myGraphics->FillRectangle(bg[BG_GREEN], 0, 0, ourRect.right, ourRect.bottom);

	PaintTime();
}

void Call::PaintTime()
{
	PointF point;
	StringFormat fmt;

	fmt.SetAlignment(StringAlignmentFar);
	fmt.SetTrimming(StringTrimmingNone);
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);

	if(myGraphics == NULL)
		return;

	SetTimer(hCall, 1, 1000, NULL);
}




LRESULT CALLBACK CallProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT		ps;
    RECT			ourRect;
//	OAFilterState	state;
//	HRESULT			hr;
    
	GetClientRect(hWnd, &ourRect);

	switch(message)
	{
		case WM_CREATE:
			myPtr->hCbWnd = hWnd;
			myPtr->InitDC(FALSE);
			SendMessage(myPtr->hPrnt, WM_SETREGISTERFINALWND, (WPARAM)0, (LPARAM)hWnd);
			break;

		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					switch(LOWORD(wParam))
					{
						case IDC_CYES:
							break;
					}
					break;
			}
			break;

		case WM_PAINT:
			GetClientRect(myPtr->hPrnt, &ourRect);
			ourRect.bottom -= myPtr->offset;
    		SetWindowPos(hWnd, 0, 0, 0, ourRect.right, ourRect.bottom, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
            BeginPaint(hWnd, &ps);
			myPtr->Paint();
			EndPaint(hWnd, &ps);
			break;

		case WM_SIZE:
			myPtr->SetKeypadPositions();

			myPtr->InitDC(TRUE);
			myPtr->Paint();
			break;

		case WM_DIRECTSHOWMUTE:
			if((BOOL)wParam == TRUE)
				myPtr->pAudio->put_Volume(-2000);
			else
				myPtr->pAudio->put_Volume(0);
			myPtr->muted = (BOOL)wParam;
			break;

		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
			break;

		case WM_LBUTTONUP:
			ReleaseCapture();
			break;

		case WM_TIMER:
			myPtr->PaintTime();
			break;

		case WM_DESTROY:
			KillTimer(hWnd, 1);
			break;

		default:
			return(DefWindowProc(hWnd, message, wParam, lParam));
   }
   return(0);
}


// Video.cpp: implementation of the Video class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Video.h"
#include "Ticker.h"
#include "Spectrum.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define VIDEOENABLESIGNAL	INP_BREAK
#define VIDEOENABLELEVEL	TRUE

#define VIDEOCLASS			TEXT("CRPCPLR_VID")
#define VIDEOCONTROLCLASS	TEXT("CRPCPLR_VIDCNT")

LRESULT CALLBACK VideoWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK VideoControlWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static Video * my;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Video::Video(PAppInfo pAppInfo)
{
	RECT pRect;

	my = this;
	pAi = pAppInfo;

	myGraphics = NULL;
	vVideoActive = FALSE;
	vCursor = FALSE;
	vBlockCheckNeeded = TRUE;

	barGraphics = NULL;
	barGraph = NULL;
	barImage = NULL;

	MyRegisterClass();
	GetClientRect(pAi->hApp, &pRect);

	if(FindWindow(TEXT("CRPC_STRTBTTN"), NULL))
		vBlockCheckNeeded = TRUE;

	GraphicsPath path;
	Rect pathBound;
	pathBound.X = pathBound.Y = 0;
	pathBound.Width = pRect.right;
	pathBound.Height = pRect.bottom - pAi->vOffs;
	path.AddRectangle(pathBound);

	bkGnd = new SolidBrush(Color(255, 0, 0, 0));
	textClr = new SolidBrush(Color(255, 0, 255, 0));
	font = new Font(L"Arial", 16);

	hVideo = CreateWindow(VIDEOCLASS, TEXT("video player"), WS_CHILD | WS_CLIPCHILDREN ,
		0, 0, pRect.right, pRect.bottom - pAi->vOffs, pAi->hApp, NULL, pAi->hInst, NULL);

	hService = OpenPsuEvent(hVideo, VIDEOENABLESIGNAL);
	if(!hService)
	{
		if(pAppInfo->hDb->GetBool("PsuEventsError") == FALSE)
		{
			MessageBox(NULL, TEXT("PsuEvents.dll could not be found in \"Common Files\\White Bream\"!"), TEXT("Player error"), MB_OK | MB_ICONEXCLAMATION);
			pAppInfo->hDb->SetConfig("PsuEventsError", "1", CONFIGTYPE_BOOL);
		}
	}
}

Video::~Video()
{
	if(font) delete font;
	if(bkGnd) delete bkGnd;
	if(textClr) delete textClr;
	DestroyWindow(hVideo);
	if(hService) ClosePsuEvent(hService, NULL);
}

Video::Show(BOOL yesNo)
{
	WINDOWPLACEMENT sPlacem;
	sPlacem.length = sizeof(WINDOWPLACEMENT);

	if(yesNo)
	{
		if(!vVideoActive && (GetParent(hVideo) == NULL))
		{
			SetWindowLong(hVideo, GWL_STYLE, (GetWindowLong(hVideo, GWL_STYLE) | WS_CHILD));
			SetWindowLong(hVideo, GWL_EXSTYLE, NULL);
			SetParent(hVideo, pAi->hApp);
			GetWindowPlacement(hVideo, &sPlacem);
			sPlacem.showCmd = SW_SHOWNORMAL;
			SetWindowPlacement(hVideo, &sPlacem);
		}
		ShowWindow(hVideo, SW_SHOW);
		UpdateWindow(hVideo);
		Paint();
		SendMessage(pAi->hApp, WM_SETVIDEOWINDOW, (WPARAM)hCbWnd, 0);
	}
	else
	{
		KillTimer(hVideo, 1);
		if(vCursor)
			ShowCursor(TRUE);
		vCursor = FALSE;
		ShowWindow(hVideo, SW_HIDE);
	}
}

ATOM Video::MyRegisterClass(void)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= VideoControlWinProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= pAi->hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= CreateSolidBrush(RGB(200, 255, 200));
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= VIDEOCONTROLCLASS;
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);

	wcex.lpfnWndProc	= VideoWinProc;
	wcex.hbrBackground	= CreateSolidBrush(RGB(0, 0, 0));
	wcex.lpszClassName	= VIDEOCLASS;

	return RegisterClassEx(&wcex);
}

void Video::InitDC(BOOL updateDC)
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

void Video::Paint()
{
	RectF vRect;
	StringFormat textFmt;
	LPWSTR defText = NULL;

	if(myGraphics == NULL)
		return;

	if(!vVideoActive || vBlockNeeded)	// Video disabled
	{
		myGraphics->FillRectangle(bkGnd, 0, 0, ourRect.right, ourRect.bottom);
		if(!vVideoActive)
		{
			defText = L"No video";
			vRect.X = 1;
			vRect.Y = 1;
			vRect.Width = (float)ourRect.right - 2;
			vRect.Height = (float)ourRect.bottom - 2;
		}
		else
		{
			defText = L"Video blocked while driving\nplease apply parking brake to enable video";
			vRect.X = (float)pAi->vOffs * 2;
			vRect.Y = (float)3 * pAi->vOffs / 2;
			vRect.Width = (float)ourRect.right - pAi->vOffs * 4;
			vRect.Height = (float)ourRect.bottom - pAi->vOffs * 3;
		}

		textFmt.SetAlignment(StringAlignmentCenter);
		textFmt.SetTrimming(StringTrimmingNone);
		textFmt.SetLineAlignment(StringAlignmentCenter);

		myGraphics->DrawString(defText, -1, font, vRect, &textFmt, textClr);
	}

}

LRESULT CALLBACK VideoWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT     ps;
	WINDOWPLACEMENT sPlacem;
    RECT            pRect;
	BOOL            toggle;

	GetClientRect(my->pAi->hApp, &pRect);
	pRect.bottom -= my->pAi->vOffs;
	sPlacem.length = sizeof(WINDOWPLACEMENT);

	switch(message)
	{
		case WM_CREATE:
			my->hCbWnd = hWnd;
			my->InitDC(FALSE);
			SendMessage(my->pAi->hApp, WM_SETREGISTERFINALWND, (WPARAM)IDW_VID, (LPARAM)hWnd);
			SendMessage(my->pAi->hApp, WM_SETVIDEOWINDOW, (WPARAM)hWnd, 0);

			my->hBar = CreateWindow(VIDEOCONTROLCLASS, NULL, WS_CHILD | WS_CLIPCHILDREN, 0, pRect.bottom - my->pAi->vOffs, 
								pRect.right, my->pAi->vOffs, hWnd, NULL, my->pAi->hInst, NULL);
			SendMessage(my->hBar, BM_SETCHECK, BST_INDETERMINATE, 0);
			my->vBlockNeeded = FALSE;
			break;

		case WM_PAINT:
			GetWindowPlacement(hWnd, &sPlacem);
			if(sPlacem.showCmd != SW_SHOWMAXIMIZED)
	    		SetWindowPos(hWnd, 0, 0, 0, pRect.right, pRect.bottom, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
			BeginPaint(hWnd, &ps);
			my->Paint();
			EndPaint(hWnd, &ps);
			if(!my->vBlockNeeded)
				SendMessage(my->pAi->hApp, WM_PAINTVIDEOWINDOW, 0, 0);
			break;

		case WM_SIZE:
			GetWindowPlacement(hWnd, &sPlacem);
			if(lParam)
			{
				if(sPlacem.showCmd != SW_SHOWMAXIMIZED)
					SetWindowPos(my->hBar, NULL, 0, pRect.bottom - my->pAi->vOffs, pRect.right, my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);

				my->InitDC(TRUE);
				my->Paint();
				SendMessage(my->pAi->hApp, WM_SETVIDEOWINDOW, (WPARAM)hWnd, 0);
			}
			else
			{
				if(my->vCursor)
					ShowCursor(TRUE);
				my->vCursor = FALSE;
			}
			break;

		case WM_LBUTTONDBLCLK:
			KillTimer(hWnd, 2);
			my->vDblClickEnable = 0;
			GetWindowPlacement(hWnd, &sPlacem);
			if(sPlacem.showCmd == SW_SHOWMAXIMIZED)
			{
				SetParent(my->pAi->hApp, NULL);
				SetWindowLong(my->pAi->hApp, GWL_STYLE, (GetWindowLong(my->pAi->hApp, GWL_STYLE) & ~WS_CHILD));
				SetWindowLong(hWnd, GWL_STYLE, (GetWindowLong(hWnd, GWL_STYLE) | WS_CHILD));
				SetWindowLong(hWnd, GWL_EXSTYLE, NULL);
				SetParent(hWnd, my->pAi->hApp);
				ShowWindow(my->pAi->hApp, SW_RESTORE);
				UpdateWindow(my->pAi->hApp);
				SetFocus(my->pAi->hApp);
				SetWindowPos(my->hBar, NULL, 0, pRect.bottom - my->pAi->vOffs, pRect.right, my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);
				toggle = FALSE;
				my->pAi->hDb->SetConfig("Fullscreen", (LPVOID)&toggle, CONFIGTYPE_BOOL);
			}
			else if(my->vVideoActive)
			{
				SetParent(hWnd, NULL);
				SetWindowLong(hWnd, GWL_STYLE, (GetWindowLong(hWnd, GWL_STYLE) & ~WS_CHILD));
				SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
				SetWindowLong(my->pAi->hApp, GWL_STYLE, (GetWindowLong(my->pAi->hApp, GWL_STYLE) | WS_CHILD));
				SetParent(my->pAi->hApp, hWnd);
				ShowWindow(my->pAi->hApp, SW_HIDE);
				ShowWindow(my->hBar, SW_HIDE);
				ShowWindow(hWnd, SW_SHOWMAXIMIZED);

				/* Get the desired size of the buttons */
				HWND hStart;
				RECT startRect = {0, GetSystemMetrics(SM_CYSCREEN) - my->pAi->vOffs, 0, 0};
				hStart = FindWindow(TEXT("CRPC_STRTBTTN"), NULL);
				if(hStart)
					GetWindowRect(hStart, &startRect);
				SetWindowPos(my->hBar, NULL, startRect.right, startRect.top, GetSystemMetrics(SM_CXSCREEN) - startRect.right, my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);
				ShowWindow(my->hBar, SW_SHOW);
				toggle = TRUE;
				my->pAi->hDb->SetConfig("Fullscreen", (LPVOID)&toggle, CONFIGTYPE_BOOL);
			}
			my->InitDC(TRUE);
			my->Paint();
			SendMessage(my->pAi->hApp, WM_SETVIDEOWINDOW, (WPARAM)hWnd, 0);
			break;

		case WM_SHOWVIDEOWINDOW:
			my->vVideoActive = lParam ? TRUE : FALSE;
			GetWindowPlacement(hWnd, &sPlacem);
			if(my->vVideoActive)
			{
				toggle = my->pAi->hDb->GetBool("Fullscreen");
				if(toggle)
					SetTimer(hWnd, 3, 10, NULL);
				else
				{
					if(sPlacem.showCmd == SW_SHOWMAXIMIZED)
					{
						SetWindowLong(hWnd, GWL_STYLE, (GetWindowLong(hWnd, GWL_STYLE) | WS_CHILD));
						SetWindowLong(hWnd, GWL_EXSTYLE, NULL);
						SetParent(hWnd, my->pAi->hApp);
					}
					SendMessage(my->pAi->hApp, WM_SETVIDEOWINDOW, (WPARAM)hWnd, 0);
					ShowWindow(hWnd, SW_SHOWNORMAL);
					ShowWindow(my->hBar, SW_SHOW);
					UpdateWindow(my->pAi->hApp);
					SetFocus(my->pAi->hApp);
				}
				my->Paint();
				SetTimer(hWnd, 1, 5000, NULL);
				SendMessage(my->hBar, BM_SETCHECK, BST_UNCHECKED, 0);
				my->vBlockCheckNeeded = TRUE;
			}
			else if(sPlacem.showCmd == SW_SHOWMAXIMIZED)
			{
				SetParent(my->pAi->hApp, NULL);
				SetWindowLong(my->pAi->hApp, GWL_STYLE, (GetWindowLong(my->pAi->hApp, GWL_STYLE) & ~WS_CHILD));
				SetWindowLong(hWnd, GWL_STYLE, (GetWindowLong(hWnd, GWL_STYLE) | WS_CHILD));
				SetWindowLong(hWnd, GWL_EXSTYLE, NULL);
				SetParent(hWnd, my->pAi->hApp);
				ShowWindow(my->pAi->hApp, SW_RESTORE);
				UpdateWindow(my->pAi->hApp);
				SetFocus(my->pAi->hApp);
				SetWindowPos(my->hBar, NULL, 0, pRect.bottom - my->pAi->vOffs, pRect.right, my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);
			}
			if(!my->vVideoActive)
			{
				my->vBlockCheckNeeded = FALSE;
				SendMessage(my->hBar, WM_DSHOWTIME, 0, 0);
				SendMessage(my->hBar, BM_SETCHECK, BST_INDETERMINATE, 0);
				if(my->vCursor)
					ShowCursor(TRUE);
				my->vCursor = FALSE;
				SetWindowText(my->hBar, TEXT(""));
				ShowWindow(my->hBar, SW_SHOW);
				my->Paint();
			}
			break;

		case WB_PSUEVENT:
			if(wParam == VIDEOENABLESIGNAL)
			{
				my->vBlockNeeded = lParam ? !VIDEOENABLELEVEL : VIDEOENABLELEVEL;
				if(my->vBlockNeeded)
				{
					my->Paint();
					SendMessage(my->pAi->hApp, WM_SETVIDEOWINDOW, (WPARAM)NULL, 0);
				}
				else if(my->vVideoActive)
				{
					SendMessage(my->pAi->hApp, WM_SETVIDEOWINDOW, (WPARAM)my->hCbWnd, 0);
					SendMessage(my->pAi->hApp, WM_SHOWVIDEOWINDOW, 0, my->vVideoActive);
				}
			}
			break;

		case WM_DSHOWTIME:
			if(!my->vVideoActive)
			{
				my->vBlockNeeded = FALSE;
				break;
			}
			if(my->vBlockCheckNeeded && my->vBlockNeeded)
				my->Paint();

			SendMessage(my->hBar, message, wParam, lParam);
			break;

		case WM_DISPLAYCHANGE:
			SendMessage(my->pAi->hApp, message, wParam, lParam);
			break;

		case WM_PAUSE:
		case WM_SETTEXT:
			SendMessage(my->hBar, message, wParam, lParam);
			break;

		case WM_SHOWWINDOW:
			if(wParam)
			{
				ShowWindow(my->hBar, SW_SHOW);
				SendMessage(my->hBar, WM_RESETTIMER, 0, 0);
			}

			if(wParam && my->vVideoActive)
				SetTimer(hWnd, 1, 5000, NULL);

			my->vDblClickEnable = 0;
			break;

		case WM_TIMER:
			switch(wParam)
			{
				case 1:	// Deactivate cursor
					if(my->vVideoActive && IsWindowVisible(hWnd))
					{
						if(!my->vCursor)
							ShowCursor(FALSE);
						my->vCursor = TRUE;
					}
					KillTimer(hWnd, wParam);
					break;

				case 2:	// Doubletap timer
					KillTimer(hWnd, wParam);
					my->vDblClickEnable = 0;
					break;

				case 3:	// Fullscreen delay
					KillTimer(hWnd, wParam);
					if(GetParent(hWnd) != NULL)
					{
						SetParent(hWnd, NULL);
						SetWindowLong(hWnd, GWL_STYLE, (GetWindowLong(hWnd, GWL_STYLE) & ~WS_CHILD));
						SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
						SetWindowLong(my->pAi->hApp, GWL_STYLE, (GetWindowLong(my->pAi->hApp, GWL_STYLE) | WS_CHILD));
						SetParent(my->pAi->hApp, hWnd);
						ShowWindow(my->pAi->hApp, SW_HIDE);
						ShowWindow(my->hBar, SW_HIDE);
						ShowWindow(hWnd, SW_SHOWMAXIMIZED);
						/* Get the desired size of the buttons */
						HWND hStart;
						RECT startRect = {0, GetSystemMetrics(SM_CYSCREEN) - my->pAi->vOffs, 0, 0};
						hStart = FindWindow(TEXT("CRPC_STRTBTTN"), NULL);
						if(hStart)
							GetWindowRect(hStart, &startRect);
						SetWindowPos(my->hBar, NULL, startRect.right, startRect.top, GetSystemMetrics(SM_CXSCREEN) - startRect.right, my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);
						SetForegroundWindow(hWnd);
						SetFocus(hWnd);
					}
					else
						SendMessage(my->pAi->hApp, WM_SETVIDEOWINDOW, (WPARAM)my->hCbWnd, 0);
					ShowWindow(my->hBar, SW_SHOW);
					break;
			}
			break;

		case WM_LBUTTONDOWN:
			if(ShowWindow(my->hBar, SW_SHOW))
				SendMessage(my->hBar, WM_SHOWWINDOW, TRUE, 0);
			if(my->vDblClickEnable == 0)
				my->vDblClickEnable = 1;
			break;

		case WM_LBUTTONUP:
			if(my->vDblClickEnable == 1)
			{
				my->vDblClickEnable++;
				SetTimer(hWnd, 2, 1000, NULL);
			}
			else if(my->vDblClickEnable == 2)
				SendMessage(hWnd, WM_LBUTTONDBLCLK, 0, 0);
			break;

		case WM_MOUSEMOVE:
			if(my->vMousemove == lParam)
				break;
			my->vMousemove = lParam;

			if(my->vCursor)
				ShowCursor(TRUE);
			my->vCursor = FALSE;

			if(my->vVideoActive)
			{
				TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, hWnd, 0};
				TrackMouseEvent(&tme);
				SetTimer(hWnd, 1, 2500, NULL);
			}
			else my->vMousemove = ~lParam;
			break;

		case WM_MOUSELEAVE:
			KillTimer(hWnd, 1);
			if(my->vCursor)
				ShowCursor(TRUE);
			my->vCursor = FALSE;
			return(0);

		case WM_SYSKEYDOWN:
			if(wParam == VK_F4)
			{
				SetParent(my->pAi->hApp, NULL);
				SetWindowLong(my->pAi->hApp, GWL_STYLE, (GetWindowLong(my->pAi->hApp, GWL_STYLE) & ~WS_CHILD));
				SetWindowLong(hWnd, GWL_STYLE, (GetWindowLong(hWnd, GWL_STYLE) | WS_CHILD));
				SetWindowLong(hWnd, GWL_EXSTYLE, NULL);
				SetParent(hWnd, my->pAi->hApp);
				SendMessage(my->pAi->hApp, WM_DESTROY, 0, 0);
			}
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}

void Video::InitBarDC(BOOL updateDC, HWND hWnd)
{
	if(barGraphics == NULL)
	{
	   barGraphics = new Graphics(hWnd, FALSE);
	}
	else if(updateDC == TRUE)
	{
	   delete barGraphics;
	   barGraphics = new Graphics(hWnd, FALSE);
	}
}

void Video::BarGdi(BOOL vStart)
{
	FontFamily	*fm;

	if(vStart)
	{
		barImage = new Bitmap(3 * pAi->vOffs - 4, pAi->vOffs - 4);
		barGraph = new Graphics(barImage);
		fm = new FontFamily(L"Arial");
		barFont = new Font(fm, (float)2 * pAi->vOffs / 3 + 2, FontStyleRegular, UnitPixel);
		delete fm;
		barBg = new SolidBrush(Color(255, 255, 255, 255));
		barText = new SolidBrush(Color(255, 0, 0, 192));
		barPen = new Pen(Color(255, 0, 0, 0), 1);
	}
	else
	{
		if(barImage) delete barImage; barImage = NULL;
		if(barGraph) delete barGraph; barGraph = NULL;
		if(barFont) delete barFont; barFont = NULL;
		if(barBg) delete barBg; barBg = NULL;
		if(barText) delete barText; barText = NULL;
		if(barPen) delete barPen; barPen = NULL;
	}
}

void Video::PaintTime()
{
	PointF point;
	WCHAR  vBuffer[20];
	StringFormat fmt;

	fmt.SetAlignment(StringAlignmentFar);
	fmt.SetTrimming(StringTrimmingNone);
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);

	if(!barGraphics || !barGraph || !barImage || !barFont || !barBg || !barText || !barPen)
		return;

	barGraph->DrawRectangle(barPen, 0, 0, 3 * pAi->vOffs - 5, pAi->vOffs - 5);
	barGraph->FillRectangle(barBg, 1, 1, 3 * pAi->vOffs - 6, pAi->vOffs - 6);

	// Print current track position
	point.X = (float)3 * pAi->vOffs - 4;
	point.Y = (float)2;
	StringCbPrintfW(vBuffer, sizeof(vBuffer), L"%ld:%02ld:%02ld", vDshowPos / 3600, (vDshowPos % 3600) / 60, vDshowPos % 60);
	barGraph->DrawString(vBuffer, -1, barFont, point, &fmt, barText);

	barGraphics->DrawImage(barImage, 2, 2);
}

LRESULT CALLBACK VideoControlWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT     ps;
    RECT            pRect;

	GetClientRect(hWnd, &pRect);
	pRect.bottom -= my->pAi->vOffs;

	switch(message)
	{
		case WM_CREATE:
			TickerInit(my->pAi->hInst);
			SpectrumInit(my->pAi->hInst);

			my->BarGdi(TRUE);

			my->vDshowPos = 0;
			my->vDshowTime = 0;

			CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, 3 * my->pAi->vOffs, 0, my->pAi->vOffs, my->pAi->vOffs, hWnd, (HMENU)IDC_PAUSE, my->pAi->hInst, NULL);
			AssignIcon(my->pAi->hInst, hWnd, IDC_PAUSE, IDI_PAUSE);

			CreateWindow(TEXT("WB_TICKERBOX"), NULL, WS_CHILD | WS_VISIBLE, 4 * my->pAi->vOffs + 2, 2, 
				pRect.right - 6 * my->pAi->vOffs - 6, (my->pAi->vOffs >> 1) - 4, hWnd, (HMENU)IDC_TICKER, my->pAi->hInst, NULL);

			CreateWindow(TEXT("WB_DESTSLIDER"), NULL, WS_CHILD | WS_VISIBLE, 4 * my->pAi->vOffs + 2, my->pAi->vOffs >> 1, 
				pRect.right - 6 * my->pAi->vOffs - 6, (my->pAi->vOffs >> 1) - 2, hWnd, (HMENU)IDC_SEEK, my->pAi->hInst, NULL);
			SendDlgItemMessage(hWnd, IDC_SEEK, TBM_SETRANGE, TRUE, MAKELONG(0, 10000));

			CreateWindow(TEXT("WB_SPECTRUM"), NULL, WS_CHILD | WS_VISIBLE, pRect.right - 2 * my->pAi->vOffs - 3, 2, 
						2 * my->pAi->vOffs + 1, my->pAi->vOffs - 4, hWnd, (HMENU)IDC_SPECTRUM, my->pAi->hInst, NULL);

			my->InitBarDC(FALSE, hWnd);
			break;

		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					LRESULT id;
					switch(LOWORD(wParam))
					{
						case IDC_PAUSE:
							id = SendDlgItemMessage(hWnd, IDC_PAUSE, BM_GETCHECK, 0, 0);
							SendMessage(my->pAi->hApp, WM_PAUSE, 0, (id == BST_UNCHECKED ? TRUE : FALSE));
							if(id != BST_UNCHECKED)
								SetTimer(hWnd, 1, 5000, NULL);
							else
								KillTimer(hWnd, 1);
							break;
					}
			}
			break;

		case WM_HSCROLL:
			if((LOWORD(wParam) == TB_THUMBTRACK) || (LOWORD(wParam) == TB_ENDTRACK ))
			{
				INT vPos;
				if((vPos = HIWORD(wParam)) == 0)
					vPos = SendDlgItemMessage(hWnd, IDC_SEEK, TBM_GETPOS, 0, 0);

				if(my->vVideoActive)
				{
					SendMessage(my->pAi->hApp, WM_DSHOWPOS, (WPARAM)vPos, 0);
					SetTimer(hWnd, 1, 5000, NULL);
				}
			}
			break;

		case WM_PAUSE:
			if(SendDlgItemMessage(hWnd, IDC_PAUSE, BM_GETCHECK, 0, 0) != BST_INDETERMINATE)
				SendDlgItemMessage(hWnd, IDC_PAUSE, BM_SETCHECK, (lParam ? BST_CHECKED : BST_UNCHECKED), 0);
			if(!lParam)
			{
				KillTimer(hWnd, 1);
				ShowWindow(hWnd, SW_SHOW);
			}
			break;

		case WM_SETTEXT:
			SendDlgItemMessage(hWnd, IDC_TICKER, message, wParam, lParam);
			break;

		case WM_DSHOWTIME:
			if(wParam)
				SendDlgItemMessage(hWnd, IDC_SEEK, TBM_SETPOS, TRUE, ((int)lParam * 10000) / (int)wParam);
			else
				SendDlgItemMessage(hWnd, IDC_SEEK, TBM_SETPOS, TRUE, 0);
			my->vDshowTime = (INT)wParam;
			my->vDshowPos = (INT)lParam;
			my->PaintTime();
			break;

		case BM_SETCHECK:
			SendDlgItemMessage(hWnd, IDC_SEEK, message, wParam, lParam);
			SendDlgItemMessage(hWnd, IDC_PAUSE, message, wParam, lParam);
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			my->PaintTime();
			EndPaint(hWnd, &ps);
			break;

		case WM_SIZE:
			SetWindowPos(GetDlgItem(hWnd, IDC_TICKER), NULL, 0, 0, pRect.right - 6 * my->pAi->vOffs - 6, (my->pAi->vOffs >> 1) - 4, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, IDC_SEEK), NULL, 0, 0, pRect.right - 6 * my->pAi->vOffs - 6, (my->pAi->vOffs >> 1) - 2, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER); 
			SetWindowPos(GetDlgItem(hWnd, IDC_SPECTRUM), NULL, pRect.right - 2 * my->pAi->vOffs - 3, 2, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER); 

			my->InitBarDC(TRUE, hWnd);
			my->PaintTime();
			break;

		case WM_RESETTIMER:
			SetTimer(hWnd, 1, 5000, NULL);
			break;

		case WM_SHOWWINDOW:
			if(wParam && (SendDlgItemMessage(hWnd, IDC_PAUSE, BM_GETCHECK, 0, 0) != BST_CHECKED))

				SetTimer(hWnd, 1, 5000, NULL);
			break;

		case WM_PARENTNOTIFY:
			if(LOWORD(wParam) != WM_DESTROY)
				SetTimer(hWnd, 1, 5000, NULL);
			break;

		case WM_TIMER:
			switch(wParam)
			{
				case 1:
					KillTimer(hWnd, 1);
					ShowWindow(hWnd, SW_HIDE);
					break;
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			SetTimer(hWnd, 1, 5000, NULL);
			break;

		case WM_DESTROY:
			my->BarGdi(FALSE);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}


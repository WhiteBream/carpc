// Popup.cpp: implementation of the Popup class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Popup.h"
#include "Ticker.h"

#include "commctrl.h"

#pragma comment(lib, "Strmiids.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define BG_WHITE		0
#define BG_GREEN		1
#define BG_LBLUE		2
#define BG_DBLUE		3

#define PROGRESS_DELAY	200

LRESULT CALLBACK PopupProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static Popup * my;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Popup::Popup(HINSTANCE hInstance, HWND hParent, int bottomOffset, HWND * pWnd, Database * dBase)
{
	RECT  pRect;
	FontFamily * fm;

	my = this;

	hInst = hInstance;
	hPrnt = hParent;
	offset = bottomOffset;
	myGraphics = NULL;
	dB = dBase;

	MyRegisterClass(hInstance);
	TickerInit(hInstance);
	GetClientRect(hParent, &pRect);

	bg[BG_WHITE] = new SolidBrush(Color(255, 255, 255, 255));
	bg[BG_GREEN] = new SolidBrush(Color(255, 200, 255, 200));
	bg[BG_LBLUE] = new SolidBrush(Color(255, 201, 211, 227));
	bg[BG_DBLUE] = new SolidBrush(Color(255, 0, 0, 192));
	pn = new Pen(Color(255, 0, 0, 0), 1);

	fm = new FontFamily(L"Arial");
	fn[0] = new Font(fm, (float)(bottomOffset >> 1) - 7, FontStyleRegular, UnitPixel);
	delete fm;

	vPos.x = (GetSystemMetrics(SM_CXSCREEN) - 6 * bottomOffset) / 2;
	vPos.y = (GetSystemMetrics(SM_CYSCREEN) - 5 * bottomOffset / 2) / 2;

	hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, TEXT("CRPC_POPUP"), NULL, WS_DLGFRAME | WS_POPUP,
		vPos.x, vPos.y, 6 * bottomOffset + 6, 5 * bottomOffset / 2 + 6, NULL, NULL, hInstance, NULL);

	// Song title
	CreateWindow(TEXT("WB_TICKERBOX"), TEXT("TICK"), WS_CHILD | WS_VISIBLE, 3, 4,
		5 * bottomOffset - 6, 2 * bottomOffset / 3 + 7, hWnd, (HMENU)IDC_TICKER, hInstance, NULL);

	// The NEXT button
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, 5 * bottomOffset, 0,
		bottomOffset, bottomOffset, hWnd, (HMENU)IDC_NEXT, hInstance, NULL);
	AssignIcon(hInstance, hWnd, IDC_NEXT, IDI_NEXT);

	// Bring up Player application
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, 0 * bottomOffset, bottomOffset,
		bottomOffset, bottomOffset, hWnd, (HMENU)IDC_PLR, hInstance, NULL);
	AssignIcon(hInstance, hWnd, IDC_PLR, IDI_PLR);

	// Enable popup button
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, 1 * bottomOffset, bottomOffset,
		bottomOffset, bottomOffset, hWnd, (HMENU)IDC_ENABLE, hInstance, NULL);

	// Pause playback
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, 2 * bottomOffset, bottomOffset,
		bottomOffset, bottomOffset, hWnd, (HMENU)IDC_PAUSE, hInstance, NULL);
	AssignIcon(hInstance, hWnd, IDC_PAUSE, IDI_PAUSE);

	// Close popup window
	vChanging = TRUE;
	CreateWindow(WBBUTTON, TEXT("HIDE"), WS_CHILD | WS_VISIBLE, 3 * bottomOffset, bottomOffset,
		3 * bottomOffset, bottomOffset, hWnd, (HMENU)IDC_HIDE, hInstance, NULL);

	*pWnd = hWnd;
}

Popup::~Popup()
{
	delete bg[BG_GREEN];
	delete bg[BG_DBLUE];
	delete bg[BG_LBLUE];
	delete bg[BG_WHITE];
	delete pn;
	delete fn[0];
	vChanging = TRUE;
	DestroyWindow(hWnd);
}

ATOM Popup::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= PopupProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_POPUP");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

void 
Popup::Show(BOOL yesNo)
{
	RECT	vRect;
	POINT	pt;
	DWORD	vProcessId;

	vChanging = TRUE;
	if(yesNo)
	{
		yesNo = FALSE;
		if(dB)
			yesNo = dB->GetBool("Popup");

		AssignIcon(hInst, hWnd, IDC_ENABLE, yesNo ? IDI_ENABLE : IDI_DISABLE);
		SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, yesNo ? BST_CHECKED : BST_UNCHECKED, 0);
		if(!yesNo)
			return;

		vProcessId = GetWindowThreadProcessId(hWnd, NULL);
		GetWindowRect(hPrnt, &vRect);
		vRect.left += 5;
		vRect.right -= 5;
		vRect.top += 40;
		vRect.bottom -= 5;

		pt.x = vRect.left;
		pt.y = vRect.bottom;
		if(GetWindowThreadProcessId(WindowFromPoint(pt), NULL) == vProcessId)
			return;
		pt.x = vRect.left;
		pt.y = vRect.top;
		if(GetWindowThreadProcessId(WindowFromPoint(pt), NULL) == vProcessId)
			return;
		pt.x = vRect.right;
		pt.y = vRect.bottom;
		if(GetWindowThreadProcessId(WindowFromPoint(pt), NULL) == vProcessId)
			return;
		pt.x = vRect.right;
		pt.y = vRect.top;
		if(GetWindowThreadProcessId(WindowFromPoint(pt), NULL) == vProcessId)
			return;

		// Only display the popup when player is not visible
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		vProgress = 0;
		SetTimer(hWnd, 1, PROGRESS_DELAY, NULL);
		SetFocus(hWnd);
	}
	else
	{
		ShowWindow(hWnd, SW_HIDE);
		KillTimer(hWnd, 1);
	}
}

void Popup::InitDC(BOOL updateDC)
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

void Popup::Paint()
{
	if(myGraphics == NULL)
		return;

	// Background
	myGraphics->FillRectangle(bg[BG_GREEN], 0, 0, ourRect.right, ourRect.bottom);
	// Timedelay bar
	myGraphics->DrawRectangle(pn, 3, 2 * offset + 3, 6 * offset - 9, offset / 2 - 9);
	myGraphics->FillRectangle(bg[BG_WHITE], 4, 2 * offset + 4, 6 * offset - 10, offset / 2 - 10);
	myGraphics->FillRectangle(bg[BG_LBLUE], 6, 2 * offset + 6, 6 * offset - 14, offset / 2 - 14);
}

BOOL Popup::Progress()
{
	long vWidth;

	if(myGraphics == NULL)
		return(FALSE);	// Stop immediately

	if(SendDlgItemMessage(hWnd, IDC_PAUSE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
	{
		vProgress += 2;
		vWidth = vProgress * (6 * offset - 14) / 100;

		myGraphics->FillRectangle(bg[BG_DBLUE], 6, 2 * offset + 6, (int)vWidth, offset / 2 - 14);
		myGraphics->FillRectangle(bg[BG_LBLUE], 6 + (int)vWidth, 2 * offset + 6, 6 * offset - 14 - (int)vWidth, offset / 2 - 14);
	}

	if(vProgress >= 100)
		return(FALSE);
	return(TRUE);
}

LRESULT CALLBACK PopupProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT	ps;
    RECT		ourRect;
	BOOL		toggle;

	GetClientRect(hWnd, &ourRect);

	switch(message)
	{
		case WM_CREATE:
			my->hCbWnd = hWnd;
			my->InitDC(FALSE);
			my->vMoving = FALSE;
			break;

		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					switch(LOWORD(wParam))
					{
						case IDC_PLR:
							my->Show(FALSE);
							SetForegroundWindow(GetAncestor(my->hPrnt, GA_PARENT));
							break;

						case IDC_HIDE:
							my->Show(FALSE);
							break;

						case IDC_PAUSE:
							break;

						case IDC_NEXT:
							my->vProgress = 0;
							UpdateWindow(hWnd);
							break;

						case IDC_ENABLE:
							toggle = (SendDlgItemMessage(hWnd, IDC_ENABLE, BM_GETCHECK, 0, 0) == BST_CHECKED ? FALSE : TRUE);
							SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, toggle ? BST_CHECKED : BST_UNCHECKED, 0);
							my->dB->SetConfig("Popup", (LPVOID)&toggle, CONFIGTYPE_BOOL);
							AssignIcon(my->hInst, hWnd, IDC_ENABLE, toggle ? IDI_ENABLE : IDI_DISABLE);
							return(0);

						default:
							break;
					}
					// Pass though to parent
					if(IsChild(hWnd, (HWND)lParam))
						PostMessage(my->hPrnt, message, wParam, lParam);
					break;

				default:
					break;
			}
			break;

	//	case WM_LBUTTONDOWN:
	//		if(IsWindowVisible(hWnd))
	//		{
	//			my->vMoving = TRUE;
	//			my->vMoveStart = MAKEPOINTS(lParam);
	//			KillTimer(hWnd, 1);
	//		}
	//		break;

	//	case WM_MOUSEMOVE:
	//		if((wParam & MK_LBUTTON) && my->vMoving)
	//		{
	//			my->vPos.x += LOWORD(lParam) - my->vMoveStart.x;
	//			my->vPos.y += HIWORD(lParam) - my->vMoveStart.y;
	//			SetWindowPos(hWnd, 0, my->vPos.x, my->vPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	//		}
	//		break;

	//	case WM_LBUTTONUP:
	//		if(my->vMoving)
	//		{
	//			my->vMoving = FALSE;
	//			SetTimer(hWnd, 1, PROGRESS_DELAY, NULL);
	//		}
	//		break;

		case WM_SETTEXT:
			SetDlgItemText(hWnd, IDC_TICKER, (LPWSTR)lParam);
			break;

		case WM_MOUSEWHEEL:
			GetWindowRect(hWnd, &ourRect);
			if((LOWORD(lParam) < ourRect.left) || (LOWORD(lParam) > ourRect.right) ||
				(HIWORD(lParam) < ourRect.top) || (HIWORD(lParam) > ourRect.bottom))
				my->Show(FALSE);
			break;

		case WM_KEYDOWN:
			my->Show(FALSE);
			break;

		case WM_SHOWWINDOW:
			return(DefWindowProc(hWnd, message, wParam, lParam));

		case WM_PAINT:
			BeginPaint(hWnd, &ps);
			my->Paint();
			EndPaint(hWnd, &ps);
			break;

		case WM_NCACTIVATE:
			if(wParam == FALSE)
				my->Show(FALSE);
			return(DefWindowProc(hWnd, message, wParam, lParam));

		case WM_TIMER:
			if(my->Progress() == FALSE)
				my->Show(FALSE);
			else
				SetTimer(hWnd, 1, PROGRESS_DELAY, NULL);
			break;

		case WM_DESTROY:
			KillTimer(hWnd, 1);
			break;

		default:
			return(DefWindowProc(hWnd, message, wParam, lParam));
   }
   return(0);
}

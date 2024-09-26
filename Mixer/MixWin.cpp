// MixWin.cpp: implementation of the MixWin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainWindow.h"
#include "MixWin.h"
#include "Spectrum.h"
#include <shlobj.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

#define WINCLASS	(LPTSTR)TEXT("CRGMXWN")

LRESULT CALLBACK MixWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void SetControlPosition(HWND hWnd, INT vControl, INT vValue);

static Bitmap *myBackground;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MixWin::MixWin(PAppInfo pAppInfo) : Client(pAppInfo, WINCLASS, (WNDPROC)MixWinProc, NULL)
{
	RECT	vRect;
	INT		X, Y, H, W, i;
	INT		textSize;
	CHAR	vInpTxt[10];

//	myBackground = new Bitmap(sAi.hInst, MAKEINTRESOURCE(IDB_BACKGROUND));

	GetClientRect(hWnd, &vRect);

	X = vRect.right / 30;
	Y = pAi->vOffs / 2;
	H = vRect.bottom - 5 * pAi->vOffs / 2;
	W = pAi->vOffs;
	CreateWindow(WBFADER, NULL, WS_CHILD | WS_VISIBLE | TBS_VERT, X, Y, W, H, hWnd, (HMENU)IDC_VOLUME, pAi->hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETRANGE, TRUE, MAKELONG(25, 95));
	SendDlgItemMessage(hWnd, IDC_VOLUME, WB_SETTICKCOUNT, 12, 0);
	SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETPOS, TRUE, 75);
	textSize = SendDlgItemMessage(hWnd, IDC_VOLUME, WB_GETTEXTSIZE, 0, 0);
	SendDlgItemMessage(hWnd, IDC_VOLUME, WB_SETALPHA, 0, ALPHA);

	Y += H + pAi->vOffs / 2;
	H = pAi->vOffs;
	W = (3 * vRect.right / 10) - vRect.right / 15;
	CreateWindow(WBFADER, NULL, WS_CHILD | WS_VISIBLE, X, Y, W, H, hWnd, (HMENU)IDC_BALANCE, pAi->hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_BALANCE, TBM_SETRANGE, TRUE, MAKELONG(0, 60));
	SendDlgItemMessage(hWnd, IDC_BALANCE, WB_SETTICKCOUNT, 7, 0);
	SendDlgItemMessage(hWnd, IDC_BALANCE, WB_SETTEXTSIZE, 0, (LPARAM)textSize);
	SendDlgItemMessage(hWnd, IDC_BALANCE, WB_SETALPHA, 0, ALPHA);

	X += 3 * vRect.right / 20;
	Y = pAi->vOffs / 2;
	H -= 3 * pAi->vOffs / 2;
	W = pAi->vOffs;
	CreateWindow(WBFADER, NULL, WS_CHILD | WS_VISIBLE | TBS_VERT, X, Y, W, H, hWnd, (HMENU)IDC_FADER, pAi->hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_FADER, TBM_SETRANGE, TRUE, MAKELONG(0, 50));
	SendDlgItemMessage(hWnd, IDC_FADER, WB_SETTICKCOUNT, 7, 0);
	SendDlgItemMessage(hWnd, IDC_FADER, WB_SETALPHA, 0, ALPHA);

	Y += H + 2 * pAi->vOffs / 3;
	H = 2 * pAi->vOffs / 3;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, X, Y, W, H, hWnd, (HMENU)IDC_RESET, pAi->hInst, NULL);

	CreateWindow(TEXT("WB_SPECTRUM"), NULL, WS_CHILD | WS_VISIBLE, vRect.right - 7 *pAi->vOffs - 3, 2, 
				7 * pAi->vOffs, 3 * pAi->vOffs - 4, hWnd, (HMENU)IDC_SPECTRUM, pAi->hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_SPECTRUM, WB_SETALPHA, 0, ALPHA);

	W = pAi->vOffs - 4;
	X = 4 + (7 * vRect.right) / 20 - (W / 2);
	Y = 7 * pAi->vOffs /2;
	H = vRect.bottom - Y;
	CreateWindow(WBFADER, NULL, WS_CHILD | WS_VISIBLE | TBS_VERT, X, Y, W, H, hWnd, (HMENU)IDC_EQ1, pAi->hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_EQ1, TBM_SETRANGE, TRUE, MAKELONG(0, 30));
	SendDlgItemMessage(hWnd, IDC_EQ1, WB_SETTICKCOUNT, 11, 0);
	X += pAi->vOffs;
	CreateWindow(WBFADER, NULL, WS_CHILD | WS_VISIBLE | TBS_VERT, X, Y, W, H, hWnd, (HMENU)IDC_EQ2, pAi->hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_EQ2, TBM_SETRANGE, TRUE, MAKELONG(0, 30));
	SendDlgItemMessage(hWnd, IDC_EQ2, TBM_SETTICFREQ, 3, 0);
	X += pAi->vOffs;
	CreateWindow(WBFADER, NULL, WS_CHILD | WS_VISIBLE | TBS_VERT, X, Y, W, H, hWnd, (HMENU)IDC_EQ3, pAi->hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_EQ3, TBM_SETRANGE, TRUE, MAKELONG(0, 30));
	SendDlgItemMessage(hWnd, IDC_EQ3, TBM_SETTICFREQ, 3, 0);
	X += pAi->vOffs;
	CreateWindow(WBFADER, NULL, WS_CHILD | WS_VISIBLE | TBS_VERT, X, Y, W, H, hWnd, (HMENU)IDC_EQ4, pAi->hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_EQ4, TBM_SETRANGE, TRUE, MAKELONG(0, 30));
	SendDlgItemMessage(hWnd, IDC_EQ4, TBM_SETTICFREQ, 3, 0);
	X += pAi->vOffs;
	CreateWindow(WBFADER, NULL, WS_CHILD | WS_VISIBLE | TBS_VERT, X, Y, W, H, hWnd, (HMENU)IDC_EQ5, pAi->hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_EQ5, TBM_SETRANGE, TRUE, MAKELONG(0, 30));
	SendDlgItemMessage(hWnd, IDC_EQ5, TBM_SETTICFREQ, 3, 0);
	X += pAi->vOffs;
	CreateWindow(WBFADER, NULL, WS_CHILD | WS_VISIBLE | TBS_VERT, X, Y, W, H, hWnd, (HMENU)IDC_EQ6, pAi->hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_EQ6, TBM_SETRANGE, TRUE, MAKELONG(0, 30));
	SendDlgItemMessage(hWnd, IDC_EQ6, TBM_SETTICFREQ, 3, 0);
	X += pAi->vOffs;
	CreateWindow(WBFADER, NULL, WS_CHILD | WS_VISIBLE | TBS_VERT, X, Y, W, H, hWnd, (HMENU)IDC_EQ7, pAi->hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_EQ7, TBM_SETRANGE, TRUE, MAKELONG(0, 30));
	SendDlgItemMessage(hWnd, IDC_EQ7, TBM_SETTICFREQ, 3, 0);

	SendDlgItemMessage(hWnd, IDC_EQ1, WB_SETALPHA, 0, ALPHA);
	SendDlgItemMessage(hWnd, IDC_EQ2, WB_SETALPHA, 0, ALPHA);
	SendDlgItemMessage(hWnd, IDC_EQ3, WB_SETALPHA, 0, ALPHA);
	SendDlgItemMessage(hWnd, IDC_EQ4, WB_SETALPHA, 0, ALPHA);
	SendDlgItemMessage(hWnd, IDC_EQ5, WB_SETALPHA, 0, ALPHA);
	SendDlgItemMessage(hWnd, IDC_EQ6, WB_SETALPHA, 0, ALPHA);
	SendDlgItemMessage(hWnd, IDC_EQ7, WB_SETALPHA, 0, ALPHA);

	// Request current settings from controller
	PipeIo("MXrV", 5, vInpTxt, sizeof(vInpTxt));
	SetControlPosition(hWnd, IDC_VOLUME, vInpTxt[0]);

	PipeIo("MXrB", 5, vInpTxt, sizeof(vInpTxt));
	SetControlPosition(hWnd, IDC_BALANCE, vInpTxt[0]);

	PipeIo("MXrF", 5, vInpTxt, sizeof(vInpTxt));
	SetControlPosition(hWnd, IDC_FADER, vInpTxt[0]);

	PipeIo("MXrR", 5, vInpTxt, sizeof(vInpTxt));
	SetControlPosition(hWnd, IDC_RESET, vInpTxt[0]);

	if(!PipeIo("MXrQA", 6, vInpTxt, sizeof(vInpTxt)))
		memset(vInpTxt, 15, sizeof(vInpTxt));
	restore_data(vInpTxt, 7);
	for(i = 0; i < 7; i++)
	{
		vInpTxt[i] &= 0x1F;
		if(vInpTxt[i] <= 15) vInpTxt[i] -= 15;
		else vInpTxt[i] = 31 - vInpTxt[i];
	}
	SetControlPosition(hWnd, IDC_EQ1, vInpTxt[0]);
	SetControlPosition(hWnd, IDC_EQ2, vInpTxt[1]);
	SetControlPosition(hWnd, IDC_EQ3, vInpTxt[2]);
	SetControlPosition(hWnd, IDC_EQ4, vInpTxt[3]);
	SetControlPosition(hWnd, IDC_EQ5, vInpTxt[4]);
	SetControlPosition(hWnd, IDC_EQ6, vInpTxt[5]);
	SetControlPosition(hWnd, IDC_EQ7, vInpTxt[6]);

	SendMessage(hWnd, WM_SIZE, 0, 0);
}

MixWin::~MixWin()
{
//	delete myBackground;
//	DestroyWindow(hWnd);
}

void MixWin::Paint(HDC hDc)
{
	Graphics * ourGraphics;
	Font           *defFont;
	StringFormat	*fontFormat;
    RECT	vRect;

	GetClientRect(hWnd, &vRect);

	ourGraphics = new Graphics(hDc);

//	ourGraphics->FillRectangle(ourBgnd, 0, 0, vRect.right, vRect.bottom);

	Brush *fontColor = new SolidBrush(Color(255, 0, 0, 192));
	defFont = new Font(L"Arial", (float)10);

	fontFormat = new StringFormat();
	fontFormat->SetAlignment(StringAlignmentCenter);
	fontFormat->SetTrimming(StringTrimmingNone);
	fontFormat->SetLineAlignment(StringAlignmentCenter);
	fontFormat->SetFormatFlags(StringFormatFlagsNoWrap);

	RectF fontRect = RectF((REAL)vRect.right / 20, 0, (REAL)pAi->vOffs, (REAL)pAi->vOffs / 2);

	ourGraphics->DrawString(L"Volume", -1, defFont, fontRect, fontFormat, fontColor);
	fontRect.X = (REAL)(3 * vRect.right / 10) - vRect.right / 20 - 3 * pAi->vOffs / 4;
	ourGraphics->DrawString(L"Fader", -1, defFont, fontRect, fontFormat, fontColor);

	fontRect.Y = (REAL)vRect.bottom + 3 - 7 * pAi->vOffs / 2;
	fontRect.Height += 5;
	ourGraphics->DrawString(L"Reset\nlevel", -1, defFont, fontRect, fontFormat, fontColor);

	fontRect.X = (REAL)vRect.right / 20;
	fontRect.Y = (REAL)vRect.bottom - 2 * pAi->vOffs;
	fontRect.Width = (REAL)pAi->vOffs;
	fontRect.Height = (REAL)pAi->vOffs / 2;

	ourGraphics->DrawString(L"Balance", -1, defFont, fontRect, fontFormat, fontColor);

	fontRect.Width = (REAL)pAi->vOffs - 4;
	fontRect.X = 4 + (7 * (REAL)vRect.right) / 20 - (fontRect.Width / 2);
	fontRect.Y = (REAL)vRect.bottom / 2 - pAi->vOffs / 2 + 4;
	fontRect.Height = (REAL)pAi->vOffs / 2;

	ourGraphics->DrawString(L"60Hz", -1, defFont, fontRect, fontFormat, fontColor);
	fontRect.X += vRect.right / 10;
	ourGraphics->DrawString(L"160Hz", -1, defFont, fontRect, fontFormat, fontColor);
	fontRect.X += vRect.right / 10;
	ourGraphics->DrawString(L"400Hz", -1, defFont, fontRect, fontFormat, fontColor);
	fontRect.X += vRect.right / 10;
	ourGraphics->DrawString(L"1kHz", -1, defFont, fontRect, fontFormat, fontColor);
	fontRect.X += vRect.right / 10;
	ourGraphics->DrawString(L"2.5kHz", -1, defFont, fontRect, fontFormat, fontColor);
	fontRect.X += vRect.right / 10;
	ourGraphics->DrawString(L"6.4kHz", -1, defFont, fontRect, fontFormat, fontColor);
	fontRect.X += vRect.right / 10;
	ourGraphics->DrawString(L"16kHz", -1, defFont, fontRect, fontFormat, fontColor);

	delete fontColor;
	delete fontFormat;
	delete defFont;
	delete ourGraphics;
}

void MixWin::PaintBkgnd(HDC hDc)
{
	Brush    * ourBgnd;
	Graphics * ourGraphics;
    RECT	vRect;

	GetClientRect(hWnd, &vRect);

	ourGraphics = new Graphics(hDc);
	ourBgnd = new SolidBrush(Color(ALPHA, 200, 255, 200));

	Rect rect = Rect(0, 0, vRect.right, vRect.bottom + pAi->vOffs);
//	ourGraphics->DrawImage(myBackground, rect, 0, 0, myBackground->GetWidth(), myBackground->GetHeight(), UnitPixel);
	ourGraphics->FillRectangle(ourBgnd, 0, 0, vRect.right, vRect.bottom);

	delete ourGraphics;
	delete ourBgnd;
}

static HWND hMoving = NULL;

LRESULT CALLBACK MixWinProc(HWND hWnd, UINT vMsg, WPARAM wParam, LPARAM lParam)
{
	MixWin	*my;
	PAINTSTRUCT ps;
	HDC		hDc;
    RECT	ourRect;
	INT		X, Y, W, H;
	CHAR	vOutTxt[10];
	CHAR	vInpTxt[10];
	BOOL	vToggle;

	my = (MixWin*)GetWindowLong(hWnd, GWL_USERDATA);
	if((my == NULL) && (vMsg != WM_NCCREATE))
		return DefWindowProc(hWnd, vMsg, wParam, lParam);

	GetClientRect(hWnd, &ourRect);

	switch(vMsg)
	{
		case WM_NCCREATE:
			my = (MixWin*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLong(hWnd, GWL_USERDATA, (LONG)my);
			return(my ? DefWindowProc(hWnd, vMsg, wParam, lParam) : FALSE);

		case WM_CREATE:
			my->hCbWnd = hWnd;
//			my->InitDC(FALSE);
			SendMessage(my->pAi->hApp, WM_SETREGISTERFINALWND, (WPARAM)IDW_PLR, (LPARAM)hWnd);
			SpectrumInit(my->pAi->hInst);
			break;

		case WM_ERASEBKGND:
			my->PaintBkgnd((HDC)wParam);
			return(TRUE);

		case WM_PRINTCLIENT:
			my->PaintBkgnd((HDC)wParam);
			my->Paint((HDC)wParam);
			break;

		case WM_PAINT:
			hDc = BeginPaint(hWnd, &ps);
			my->Paint(hDc);
			EndPaint(hWnd, &ps);
			break;

		case WM_SIZE:
			X = ourRect.right / 20;
			Y = my->pAi->vOffs / 2;
			H = ourRect.bottom - 5 * my->pAi->vOffs / 2;
			W = my->pAi->vOffs;
			SetWindowPos(GetDlgItem(hWnd, IDC_VOLUME), 0, X, Y, W, H, SWP_NOACTIVATE | SWP_NOZORDER);
			X = ourRect.right / 30;
			Y += H + my->pAi->vOffs / 2;
			H = my->pAi->vOffs;
			W = (3 * ourRect.right / 10) - ourRect.right / 15;
			SetWindowPos(GetDlgItem(hWnd, IDC_BALANCE), 0, X, Y, W, H, SWP_NOACTIVATE | SWP_NOZORDER);
			X = (3 * ourRect.right / 10) - ourRect.right / 20 - 3 * my->pAi->vOffs / 4;
			Y = my->pAi->vOffs / 2;
			H = ourRect.bottom - 5 * my->pAi->vOffs / 2;
			H -= 3 * my->pAi->vOffs / 2;
			W = my->pAi->vOffs;
			SetWindowPos(GetDlgItem(hWnd, IDC_FADER), 0, X, Y, W, H, SWP_NOACTIVATE | SWP_NOZORDER);
			Y += H + 2 * my->pAi->vOffs / 3;
			SetWindowPos(GetDlgItem(hWnd, IDC_RESET), 0, X, Y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);

			X = (3 * ourRect.right / 10);
			Y = 0;
			W = ourRect.right - X;
			H = (ourRect.bottom / 2) - my->pAi->vOffs / 2;
			SetWindowPos(GetDlgItem(hWnd, IDC_SPECTRUM), 0, X + 3, Y + 3, W - 6, H - 6, SWP_NOACTIVATE | SWP_NOZORDER);

			W = my->pAi->vOffs - 4;
			X = 4 + (7 * ourRect.right) / 20 - (W / 2);
			Y = ourRect.bottom /2;
			H = ourRect.bottom - Y;
			SetWindowPos(GetDlgItem(hWnd, IDC_EQ1), 0, X, Y, W, H, SWP_NOACTIVATE | SWP_NOZORDER);
			X += ourRect.right / 10;
			SetWindowPos(GetDlgItem(hWnd, IDC_EQ2), 0, X, Y, W, H, SWP_NOACTIVATE | SWP_NOZORDER);
			X += ourRect.right / 10;
			SetWindowPos(GetDlgItem(hWnd, IDC_EQ3), 0, X, Y, W, H, SWP_NOACTIVATE | SWP_NOZORDER);
			X += ourRect.right / 10;
			SetWindowPos(GetDlgItem(hWnd, IDC_EQ4), 0, X, Y, W, H, SWP_NOACTIVATE | SWP_NOZORDER);
			X += ourRect.right / 10;
			SetWindowPos(GetDlgItem(hWnd, IDC_EQ5), 0, X, Y, W, H, SWP_NOACTIVATE | SWP_NOZORDER);
			X += ourRect.right / 10;
			SetWindowPos(GetDlgItem(hWnd, IDC_EQ6), 0, X, Y, W, H, SWP_NOACTIVATE | SWP_NOZORDER);
			X += ourRect.right / 10;
			SetWindowPos(GetDlgItem(hWnd, IDC_EQ7), 0, X, Y, W, H, SWP_NOACTIVATE | SWP_NOZORDER);
			break;

		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					switch(LOWORD(wParam))
					{
						case IDC_RESET:
							vToggle = SendDlgItemMessage(hWnd, LOWORD(wParam), BM_GETCHECK, 0, 0) == BST_CHECKED ? TRUE : FALSE;
							StringCbPrintfA(vOutTxt, sizeof(vOutTxt), "MXwR%c", vToggle ? '0' : '1');
							if(my->pAi->hSetValue) my->pAi->hSetValue(vOutTxt, (INT)strlen(vOutTxt) + 1);
							else PipeIo(vOutTxt, (DWORD)strlen(vOutTxt) + 1, vInpTxt, sizeof(vInpTxt));
							break;
					}
					break;
			}
			break;

		case WM_HSCROLL:
		case WM_VSCROLL:
			if((LOWORD(wParam) == TB_THUMBTRACK) || (LOWORD(wParam) == TB_ENDTRACK ))
			{
				CHAR vPos;
				if(LOWORD(wParam) == TB_THUMBTRACK)
					vPos = HIWORD(wParam);
				else
					vPos = (CHAR)SendMessage((HWND)lParam, TBM_GETPOS, 0, 0) & 0xFF;

				hMoving = (HWND)lParam;
				if(LOWORD(wParam) == TB_ENDTRACK)
					hMoving = NULL;

				vOutTxt[0] = 0;
				if((HWND)lParam == GetDlgItem(hWnd, IDC_VOLUME))
				{
					vPos = 40 - vPos;
					StringCbPrintfA(vOutTxt, sizeof(vOutTxt), "MXwV%c", (CHAR)(vPos & 0xFF));
				}
				else if((HWND)lParam == GetDlgItem(hWnd, IDC_BALANCE))
				{
					vPos -= 30;
					StringCbPrintfA(vOutTxt, sizeof(vOutTxt), "MXwB%c", (CHAR)(vPos & 0xFF));
				}
				else if((HWND)lParam == GetDlgItem(hWnd, IDC_FADER))
				{
					vPos -= 25;
					vPos = -vPos;
					StringCbPrintfA(vOutTxt, sizeof(vOutTxt), "MXwF%c", (CHAR)(vPos & 0xFF));
				}
				else if((HWND)lParam == GetDlgItem(hWnd, IDC_EQ1))
				{
					vPos -= 15;
					vPos = -vPos;
					StringCbPrintfA(vOutTxt, sizeof(vOutTxt), "MXwQ1%c", (CHAR)(vPos & 0xFF));
				}
				else if((HWND)lParam == GetDlgItem(hWnd, IDC_EQ2))
				{
					vPos -= 15;
					vPos = -vPos;
					StringCbPrintfA(vOutTxt, sizeof(vOutTxt), "MXwQ2%c", (CHAR)(vPos & 0xFF));
				}
				else if((HWND)lParam == GetDlgItem(hWnd, IDC_EQ3))
				{
					vPos -= 15;
					vPos = -vPos;
					StringCbPrintfA(vOutTxt, sizeof(vOutTxt), "MXwQ3%c", (CHAR)(vPos & 0xFF));
				}
				else if((HWND)lParam == GetDlgItem(hWnd, IDC_EQ4))
				{
					vPos -= 15;
					vPos = -vPos;
					StringCbPrintfA(vOutTxt, sizeof(vOutTxt), "MXwQ4%c", (CHAR)(vPos & 0xFF));
				}
				else if((HWND)lParam == GetDlgItem(hWnd, IDC_EQ5))
				{
					vPos -= 15;
					vPos = -vPos;
					StringCbPrintfA(vOutTxt, sizeof(vOutTxt), "MXwQ5%c", (CHAR)(vPos & 0xFF));
				}
				else if((HWND)lParam == GetDlgItem(hWnd, IDC_EQ6))
				{
					vPos -= 15;
					vPos = -vPos;
					StringCbPrintfA(vOutTxt, sizeof(vOutTxt), "MXwQ6%c", (CHAR)(vPos & 0xFF));
				}
				else if((HWND)lParam == GetDlgItem(hWnd, IDC_EQ7))
				{
					vPos -= 15;
					vPos = -vPos;
					StringCbPrintfA(vOutTxt, sizeof(vOutTxt), "MXwQ7%c", (CHAR)(vPos & 0xFF));
				}
				if(vOutTxt[0])
				{
					if(my->pAi->hSetValue) my->pAi->hSetValue(vOutTxt, (INT)strlen(vOutTxt) + 1);
					else PipeIo(vOutTxt, (DWORD)strlen(vOutTxt) + 1, vInpTxt, sizeof(vInpTxt));
				}
			}
			break;

		case WM_MIXERCHANGED:
			switch(wParam)
			{
				case 'V':	SetControlPosition(hWnd, IDC_VOLUME, LOWORD(lParam));	break;
				case 'B':	SetControlPosition(hWnd, IDC_BALANCE, LOWORD(lParam));	break;
				case 'F':	SetControlPosition(hWnd, IDC_FADER, LOWORD(lParam));	break;
				case 'R':	SetControlPosition(hWnd, IDC_RESET, LOWORD(lParam));	break;

				case 'Q':
					switch(LOWORD(lParam))
					{
						case '1':	SetControlPosition(hWnd, IDC_EQ1, HIWORD(lParam));	break;
						case '2':	SetControlPosition(hWnd, IDC_EQ2, HIWORD(lParam));	break;
						case '3':	SetControlPosition(hWnd, IDC_EQ3, HIWORD(lParam));	break;
						case '4':	SetControlPosition(hWnd, IDC_EQ4, HIWORD(lParam));	break;
						case '5':	SetControlPosition(hWnd, IDC_EQ5, HIWORD(lParam));	break;
						case '6':	SetControlPosition(hWnd, IDC_EQ6, HIWORD(lParam));	break;
						case '7':	SetControlPosition(hWnd, IDC_EQ7, HIWORD(lParam));	break;
					}
					break;

				default:
					break;
			}
			break;

		default:
			return DefWindowProc(hWnd, vMsg, wParam, lParam);
   }
   return(0);
}

void
SetControlPosition(HWND hWnd, INT vControl, INT vValue)
{
	TCHAR	vText[10];
	CHAR	vInpTxt[10];

	if(!hWnd) return;

	switch(vControl)
	{
		case IDC_VOLUME:
			if(GetDlgItem(hWnd, vControl) != hMoving)
				SendDlgItemMessage(hWnd, vControl, TBM_SETPOS, TRUE, 40 - vValue);
			if(vValue >= 0)
				StringCbPrintf(vText, sizeof(vText), TEXT("+%ddB"), vValue);
			else
				StringCbPrintf(vText, sizeof(vText), TEXT("%ddB"), vValue);
			SetWindowText(GetDlgItem(hWnd, vControl), vText);
			break;

		case IDC_BALANCE:
			if(GetDlgItem(hWnd, vControl) != hMoving)
				SendDlgItemMessage(hWnd, IDC_BALANCE, TBM_SETPOS, TRUE, (LPARAM)vValue + 30);
			StringCbPrintf(vText, sizeof(vText), TEXT("%d"), -abs(vValue));
			SetWindowText(GetDlgItem(hWnd, IDC_BALANCE), vText);
			break;

		case IDC_FADER:
			if(GetDlgItem(hWnd, vControl) != hMoving)
				SendDlgItemMessage(hWnd, IDC_FADER, TBM_SETPOS, TRUE, (LPARAM)(-vValue) +25);
			StringCbPrintf(vText, sizeof(vText), TEXT("%ddB"), -abs(vValue));
			SetWindowText(GetDlgItem(hWnd, IDC_FADER), vText);
			break;

		case IDC_RESET:
			PipeIo("MXrR", 5, vInpTxt, sizeof(vInpTxt));
			if(vInpTxt[0] > 15)
				StringCbPrintf(vText, sizeof(vText), TEXT("RST"));
			else if(vInpTxt[0] >= 0)
				StringCbPrintf(vText, sizeof(vText), TEXT("+%ddB"), vInpTxt[0]);
			else
				StringCbPrintf(vText, sizeof(vText), TEXT("%ddB"), vInpTxt[0]);
			SetWindowText(GetDlgItem(hWnd, IDC_RESET), vText);
			SendDlgItemMessage(hWnd, IDC_RESET, BM_SETCHECK, (vInpTxt[0] > 15 ? BST_UNCHECKED : BST_CHECKED), 0);
			break;

		case IDC_EQ1:
		case IDC_EQ2:
		case IDC_EQ3:
		case IDC_EQ4:
		case IDC_EQ5:
		case IDC_EQ6:
		case IDC_EQ7:
			if(GetDlgItem(hWnd, vControl) != hMoving)
				SendDlgItemMessage(hWnd, vControl, TBM_SETPOS, TRUE, (LPARAM)(-vValue) + 15);
			if(vValue > 0)
				StringCbPrintf(vText, sizeof(vText), TEXT("+%ddB"), vValue);
			else
				StringCbPrintf(vText, sizeof(vText), TEXT("%ddB"), vValue);
			SetWindowText(GetDlgItem(hWnd, vControl), vText);
			break;
	}
	hMoving = NULL;
}


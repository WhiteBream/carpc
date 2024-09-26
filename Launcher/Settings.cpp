// Settings.cpp: implementation of the StartButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Settings.h"
#include "Config.h"
#include "Shutdown.h"
#include "About.h"
#include <stdio.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#pragma warning(disable : 4995)

extern void SetScreenSize(void);

LRESULT CALLBACK SettingsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static LONG		addCount = 0, updCount = 0;
static LPTSTR	lpCurrentDir = NULL;

static Settings * my;

//typedef enum _ConfigType
//{
//	CONFIGTYPE_NONE,
//	CONFIGTYPE_BOOL = 1,
//	CONFIGTYPE_INT,
//	CONFIGTYPE_LONG,
//	CONFIGTYPE_STRING,
//	CONFIGTYPE_UNICODE
//} tConfigType;

void StartTouchConfig(LPVOID pConfig, BOOL vValue)
{
	DWORD	vSize;
	HKEY	hReg;
	TCHAR	vApp[MAX_PATH], vPath[MAX_PATH];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    memset(&pi, 0, sizeof(pi));

	if(RegCreateKey(HKEY_LOCAL_MACHINE, TEXT("Software\\TouchKit"), &hReg) == ERROR_SUCCESS)
	{
		vSize = sizeof(vPath);
		RegQueryValueEx(hReg, TEXT("InstallPath"), 0, NULL, (PBYTE)vPath, &vSize); 
		RegCloseKey(hReg);
		_tcscpy(vApp, vPath);
		_tcscat(vApp, TEXT("\\TouchKit.exe"));

		CreateProcess(vApp, NULL, NULL, NULL, FALSE, 0, NULL, vPath, &si, &pi);
		if(pi.hProcess) CloseHandle(pi.hProcess);
		if(pi.hThread) CloseHandle(pi.hThread);
	}
	else
		MessageBox(NULL, TEXT("Unable to find TouchKit utility"), TEXT("TouchKit"), MB_OK | MB_ICONEXCLAMATION);
}

void StartTouchCalibration(LPVOID pConfig, BOOL vValue)
{
	DWORD	vSize;
	HKEY	hReg;
	TCHAR	vApp[MAX_PATH], vPath[MAX_PATH];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    memset(&pi, 0, sizeof(pi));

	if(RegCreateKey(HKEY_LOCAL_MACHINE, TEXT("Software\\TouchKit"), &hReg) == ERROR_SUCCESS)
	{
		vSize = sizeof(vPath);
		RegQueryValueEx(hReg, TEXT("InstallPath"), 0, NULL, (PBYTE)vPath, &vSize); 
		RegCloseKey(hReg);
		_tcscpy(vApp, vPath);
		_tcscat(vApp, TEXT("\\xAuto4PtsCal.exe"));

		CreateProcess(vApp, NULL, NULL, NULL, FALSE, 0, NULL, vPath, &si, &pi);
		if(pi.hProcess) CloseHandle(pi.hProcess);
		if(pi.hThread) CloseHandle(pi.hThread);
	}
	else
		MessageBox(NULL, TEXT("Unable to find TouchKit calibation utility\n(xAuto4PtsCal.exe)"), TEXT("TouchKit"), MB_OK | MB_ICONEXCLAMATION);
}

typedef enum _ConfigStyle
{
	CT_YESNO = 1,
	CT_EXECUTE = 2,
	CT_INCR = 4,
	CT_FUNCTION = 0x100,
	CT_DBASE = 0x200,
	CT_INF = 0x400,
	CT_LOCKED = 0x1000,
	CT_CONFIRM = 0x2000
} ConfigStyle;

typedef struct _ConfigParam
{
	INT			vStyle;
	tConfigType	vType;
	LPTSTR		pTexts[10];
	LPWSTR		pText;
	union
	{
		LPSTR	pDbaseEntry;
		void	(*pCallback)(LPVOID pConfig, BOOL vValue);
		tInfVal	vInfTag;
	} pType;
} ConfigParam;

static const ConfigParam sConfig[] = 
{
	{	
		CT_YESNO | CT_INF, CONFIGTYPE_BOOL, 
		{TEXT("Resize display resolution on startup"),
		TEXT("Maintain default resolution")}, 
		L"Resize or maintain window size when running Cargo as shell.", 
		(LPSTR)SETSCREENSIZE
	},
	{	
		CT_YESNO | CT_INF, CONFIGTYPE_BOOL, 
		{TEXT("Resize display to 800 x 600"),
		TEXT("Resize display to 640 x 480")},
		L"Select resolution to use when resizing is enabled.", 
		(LPSTR)SCREENSIZE
	},
	{	
		CT_YESNO | CT_DBASE, CONFIGTYPE_BOOL, 
		{TEXT("Video is adjusted for 7\" widescreen monitor"),
		TEXT("Video playback on standard monitor")}, 
		L"Video and images can be scaled to compensate for widescreen scaling of small in-car displays.", 
		"LilliputMode"
	},
	{	
		CT_EXECUTE | CT_FUNCTION, CONFIGTYPE_NONE, 
		{TEXT("Touchscreen configuration"),
		TEXT("Touchscreen configuration")}, 
		L"Start TouchKit configuration utitily (if installed).", 
		(LPSTR)StartTouchConfig
	},
	{	
		CT_EXECUTE | CT_FUNCTION, CONFIGTYPE_NONE, 
		{TEXT("Touchscreen calibration"),
		TEXT("Touchscreen calibration")}, 
		L"Start TouchKit calibration utitily (if installed).", 
		(LPSTR)StartTouchCalibration
	},
	{0, CONFIGTYPE_NONE, {NULL}, NULL, NULL}
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Settings::Settings(HINSTANCE hInstance, HWND hParent)
{
	RECT	pRect;
	INT		y;
	LONG	newStyle;
	HWND	vOldWin;

	ConfigGetInt(BUTTONHEIGHT, &offset);
	instance = hInstance;
	parent = hParent;

	myGraphics = NULL;
	fn = NULL;
	br_black = NULL;
	br_white = NULL;
	pn = NULL;
	bkGnd = NULL;
	myGraphics = NULL;
	dBase = NULL;

	vOldWin = FindWindow(TEXT("CRMN_STTNGS"), NULL);
	if(vOldWin)
	{
		ShowWindow(vOldWin, SW_SHOW);
		BringWindowToTop(vOldWin);
		UpdateWindow(vOldWin);
		delete this;
	}
	else
	{
		my = this;

		statusFormat = new StringFormat();
		statusFormat->SetAlignment(StringAlignmentNear);
		statusFormat->SetTrimming(StringTrimmingNone);

		MyRegisterClass();
		//GetClientRect(parent, &pRect);
		pRect.left = pRect.top = 0;
		pRect.right = GetSystemMetrics(SM_CXSCREEN);
		pRect.bottom = GetSystemMetrics(SM_CYSCREEN);

		pRect.bottom -= offset;
		statusRect = new RectF(1, (float)pRect.bottom - offset + 1, (float)pRect.right - offset - 2, (float)offset - 2);
		vHelpText = NULL;
		vHelpTextRect = new RectF((float)vHelpRect.X + 1, (float)vHelpRect.Y + 1, (float)vHelpRect.Width - 2, (float)vHelpRect.Height - 2);
		vPsuStatus[0] = 0;
		vPsuStatusRect = new RectF((float)vHelpRect.X + 1, (float)vHelpRect.Y + 200, (float)vHelpRect.Width - 2, (float)vHelpRect.Height - 2);

		bkGnd = new SolidBrush(Color(255, 200, 255, 200));
		br_black = new SolidBrush(Color(255, 0, 0, 0));
		br_white = new SolidBrush(Color(255, 255, 255, 255));
		pn = new Pen(Color(255, 0, 0, 0), 1);
		fn = new Font(L"Arial", (float)offset / 4);

		/* Start database */
		dBase = new ConfigDb();

		hSett = CreateWindow(TEXT("CRMN_STTNGS"), TEXT("CARGO Menu Settings"), WS_BORDER | WS_CLIPCHILDREN,
			0, 0, pRect.right, pRect.bottom, NULL, NULL, instance, NULL);
		newStyle = GetWindowLong(hSett, GWL_STYLE);
		newStyle &= ~(WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_OVERLAPPED | WS_TILED | WS_DLGFRAME );
		SetWindowLong(hSett, GWL_STYLE, newStyle);
		SetWindowPos(hSett, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		CreateWindow(TEXT("WB_LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_HASSTRINGS, 0, 0,
			pRect.right - 3 * offset, pRect.bottom / 2 - offset, hSett, (HMENU)IDC_SETTINGS, instance, (LPVOID)(2 * offset / 3));
		SendDlgItemMessage(hSett, IDC_SETTINGS, LB_SETITEMHEIGHT, 0, (LPARAM)24);

		y = 24 * ((pRect.bottom / 2 - offset - 2) / 24) + 2;

		vHelpRect.X = 0;
		vHelpRect.Y = y - 1;
		vHelpRect.Width = pRect.right - 6 * offset - 1;
		vHelpRect.Height = offset;
		vHelpText = NULL;
		vHelpTextRect->X = (float)vHelpRect.X + 1;
		vHelpTextRect->Y = (float)vHelpRect.Y + 1;
		vHelpTextRect->Width = (float)vHelpRect.Width - 2;
		vHelpTextRect->Height = (float)vHelpRect.Height - 2;
		vPsuStatusRect->X = (float)vHelpRect.X + 1;
		vPsuStatusRect->Y = (float)vHelpRect.Y + 200;
		vPsuStatusRect->Width = (float)vHelpRect.Width - 2;
		vPsuStatusRect->Height = (float)vHelpRect.Height - 2;

		CreateWindow(WBBUTTON, TEXT("ON"), WS_CHILD | WS_VISIBLE, pRect.right - 6 * offset, y, 
			3 * offset / 2, offset, hSett, (HMENU)IDC_ENABLE, instance, NULL);
		SendDlgItemMessage(hSett, IDC_ENABLE, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

		CreateWindow(WBBUTTON, TEXT("OFF"), WS_CHILD | WS_VISIBLE, pRect.right - 9 * offset / 2, y, 
			3 * offset / 2, offset, hSett, (HMENU)IDC_DISABLE, instance, NULL);
		SendDlgItemMessage(hSett, IDC_DISABLE, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

		CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 5 * offset / 2, offset / 2,
			offset * 2, offset, hSett, (HMENU)IDC_CFGEXIT, instance, NULL);
		AssignIcon(instance, hSett, IDC_CFGEXIT, IDI_EXIT);

		CreateWindow(TEXT("STATIC"), NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY, pRect.right - 5 * offset / 2 - 7, 2 * offset,
			122, 24, hSett, (HMENU)IDC_CARGOLOGO, instance, NULL);
		hBmp = LoadBitmap(instance, MAKEINTRESOURCE(IDB_CARGO));
		SendDlgItemMessage(hSett, IDC_CARGOLOGO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);

		CreateWindow(WBBUTTON, TEXT("Explorer"), WS_CHILD | WS_VISIBLE, pRect.right - 5 * offset / 2, pRect.bottom - 3 * offset,
			offset * 2, offset, hSett, (HMENU)IDC_EXPLORER, instance, NULL);

		CreateWindow(WBBUTTON, TEXT("OK"), WS_CHILD | WS_VISIBLE, pRect.right - 5 * offset / 2, pRect.bottom - 3 * offset / 2,
			offset * 2, offset, hSett, (HMENU)IDC_OK, instance, NULL);
	//	AssignIcon(instance, hSett, IDC_OK, IDI_DEL);

		hDc = GetDC(hSett);
		myGraphics = new Graphics(hDc);
		Show(TRUE);
	}
}

Settings::~Settings()
{
	if(dBase) delete dBase;
	if(fn) delete fn;
	if(br_black) delete br_black;
	if(br_white) delete br_white;
	if(pn) delete pn;
	if(bkGnd) delete bkGnd;
	if(myGraphics) delete myGraphics;
	ReleaseDC(hSett, hDc);
	DeleteObject(hBmp);
	DestroyWindow(hSett);
}

Settings::Show(BOOL yesNo)
{
	LRESULT     id, last;
	INT         i;

	if(yesNo)
	{
		last = SendDlgItemMessage(hSett, IDC_SETTINGS, LB_GETCURSEL, 0, 0);
		SendDlgItemMessage(hSett, IDC_SETTINGS, LB_RESETCONTENT, 0, 0);
		for(i = 0; sConfig[i].vStyle; i++)
		{
			INT	vInt = 0;
			if(sConfig[i].vStyle & CT_INF)
			{
				if(sConfig[i].vStyle & CT_YESNO)
				{
					ConfigGetInt(sConfig[i].pType.vInfTag, &vInt);
				}
				else if(sConfig[i].vStyle & CT_INCR)
				{
					vInt = FALSE;
				}
				else if(sConfig[i].vStyle & CT_EXECUTE)
					MessageBox(hSett, TEXT("Exec parameter not possible with inf-file entry"), TEXT("Config error"), MB_OK | MB_ICONEXCLAMATION);
			}
			else if(sConfig[i].vStyle & CT_DBASE)
			{
				if(sConfig[i].vStyle & CT_YESNO)
				{
					vInt = dBase->GetBool(sConfig[i].pType.pDbaseEntry);
				}
				else if(sConfig[i].vStyle & CT_EXECUTE)
					MessageBox(hSett, TEXT("Exec parameter not possible with database entry"), TEXT("Config error"), MB_OK | MB_ICONEXCLAMATION);
			}
			else if(sConfig[i].vStyle & CT_FUNCTION)
			{
				if(sConfig[i].vStyle & CT_YESNO)
					MessageBox(hSett, TEXT("Yes/no parameter not possible with function entry"), TEXT("Config error"), MB_OK | MB_ICONEXCLAMATION);
			}
			id = SendDlgItemMessage(hSett, IDC_SETTINGS, LB_ADDSTRING, 0, (LPARAM)(vInt ? sConfig[i].pTexts[0] : sConfig[i].pTexts[1]));
			SendDlgItemMessage(hSett, IDC_SETTINGS, LB_SETITEMDATA, (WPARAM)id, (LPARAM)&sConfig[i]);
		}
		SendDlgItemMessage(hSett, IDC_SETTINGS, LB_SETCURSEL, (WPARAM)last, 0);
		ShowWindow(hSett, SW_SHOW);
	}
	else
		ShowWindow(hSett, SW_HIDE);
}

ATOM Settings::MyRegisterClass(void)
{
	WNDCLASSEX wcex;

	wcex.cbSize         = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= SettingsWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= instance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= CreateSolidBrush(RGB(200, 255, 200));
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRMN_STTNGS");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

void Settings::Infobox(void)
{
	if(myGraphics == NULL)
		return;

	myGraphics->FillRectangle(br_white, vHelpRect);
	myGraphics->DrawRectangle(pn, vHelpRect);
	if(vHelpText)
		myGraphics->DrawString(vHelpText, -1, fn, *vHelpTextRect, statusFormat, br_black);

	myGraphics->FillRectangle(bkGnd, *vPsuStatusRect);
	if(vPsuStatus)
		myGraphics->DrawString(vPsuStatus, -1, fn, *vPsuStatusRect, statusFormat, br_black);
}

void Settings::InitDC(BOOL updateDC)
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
	statusRect->Y = (float)ourRect.bottom - offset + 1;
	statusRect->Width = (float)ourRect.right - 2;
	statusRect->Height = (float)offset - 2;
}

LRESULT CALLBACK SettingsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT			ps;
    RECT				ourRect;
	LRESULT				i;
	BOOL				toggle;
	INT					y;
	ConfigParam			*pConfig;
    STARTUPINFOA		si;
    PROCESS_INFORMATION	pi;
	CHAR vStatus[20] = {0};
	CHAR vInput[10] = {0};

	GetClientRect(hWnd, &ourRect);

	switch (message)
	{
		case WM_CREATE:
			my->hCbWnd = hWnd;
			my->InitDC(FALSE);
			SetTimer(hWnd, 1, 1000, NULL);
			break;

		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					switch(LOWORD(wParam))
					{
				//		case IDCANCEL:
						case IDC_OK:
							PostMessage(hWnd, WM_CLOSE, 0, 0);
							break;

						case IDC_EXPLORER:
							//Terminate cargo
							PostQuitMessage(0);
							//Start Windows Explorer
							memset(&si, 0, sizeof(si));
							si.cb = sizeof(si);
							si.dwFlags = STARTF_USESHOWWINDOW;
							si.wShowWindow = SW_SHOWDEFAULT;  
							memset(&pi, 0, sizeof(pi));
							CreateProcessA(NULL, "explorer.exe", NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
							if(pi.hProcess) CloseHandle(pi.hProcess);
							if(pi.hThread) CloseHandle(pi.hThread);
							break;

						case IDC_ENABLE:
							i = SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_GETCURSEL, 0, 0);
							pConfig = (ConfigParam*)SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_GETITEMDATA, (WPARAM)i, 0);
							if((LRESULT)pConfig != LB_ERR)
							{
								if((pConfig->vStyle & CT_INF) && (pConfig->vStyle & CT_YESNO))
								{
									ConfigSetInt(pConfig->pType.vInfTag, 1);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_DELETESTRING, i, 0);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_INSERTSTRING, i, (LPARAM)pConfig->pTexts[0]);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_SETITEMDATA, (WPARAM)i, (LPARAM)pConfig);
									SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
									SendDlgItemMessage(hWnd, IDC_DISABLE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_SETCURSEL, i, 0);
								}
								if((pConfig->vStyle & CT_DBASE) && (pConfig->vStyle & CT_YESNO))
								{
									toggle = TRUE;
									my->dBase->SetConfig(pConfig->pType.pDbaseEntry, (LPVOID)&toggle, CONFIGTYPE_BOOL);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_DELETESTRING, i, 0);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_INSERTSTRING, i, (LPARAM)pConfig->pTexts[0]);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_SETITEMDATA, (WPARAM)i, (LPARAM)pConfig);
									SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
									SendDlgItemMessage(hWnd, IDC_DISABLE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_SETCURSEL, i, 0);
								}
								else if((pConfig->vStyle & CT_FUNCTION) && (pConfig->vStyle & CT_EXECUTE))
								{
									if(pConfig->vStyle & CT_CONFIRM)
										if(MessageBox(hWnd, pConfig->pTexts[0], TEXT("Are you really sure?"), MB_YESNO | MB_ICONEXCLAMATION | MB_APPLMODAL) == IDNO)
											return(0);
									pConfig->pType.pCallback((LPVOID)pConfig, TRUE);
									SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
								}
								if((pConfig->pType.vInfTag == SETSCREENSIZE) || (pConfig->pType.vInfTag == SCREENSIZE))
								{
									SetScreenSize();
									PostMessage(hWnd, WM_CLOSE, 0, 0);
								}
							}
							break;

						case IDC_DISABLE:
							i = SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_GETCURSEL, 0, 0);
							pConfig = (ConfigParam*)SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_GETITEMDATA, (WPARAM)i, 0);
							if((LRESULT)pConfig != LB_ERR)
							{
								if((pConfig->vStyle & CT_INF) && (pConfig->vStyle & CT_YESNO))
								{
									ConfigSetInt(pConfig->pType.vInfTag, 0);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_DELETESTRING, i, 0);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_INSERTSTRING, i, (LPARAM)pConfig->pTexts[1]);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_SETITEMDATA, (WPARAM)i, (LPARAM)pConfig);
									SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
									SendDlgItemMessage(hWnd, IDC_DISABLE, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_SETCURSEL, i, 0);
								}
								else if((pConfig->vStyle & CT_DBASE) && (pConfig->vStyle & CT_YESNO))
								{
									toggle = FALSE;
									my->dBase->SetConfig(pConfig->pType.pDbaseEntry, (LPVOID)&toggle, CONFIGTYPE_BOOL);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_DELETESTRING, i, 0);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_INSERTSTRING, i, (LPARAM)pConfig->pTexts[1]);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_SETITEMDATA, (WPARAM)i, (LPARAM)pConfig);
									SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
									SendDlgItemMessage(hWnd, IDC_DISABLE, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_SETCURSEL, i, 0);
								}
								if((pConfig->pType.vInfTag == SETSCREENSIZE) || (pConfig->pType.vInfTag == SCREENSIZE))
								{
									SetScreenSize();
									PostMessage(hWnd, WM_CLOSE, 0, 0);
								}
							}
							break;

						case IDC_CARGOLOGO:
							RegisterAboutClass(my->instance);
							GetWindowRect(hWnd, &ourRect);
							CreateWindow(TEXT("CRPC_ABOUT"), NULL, WS_POPUP | WS_DLGFRAME | WS_VISIBLE, ourRect.left + 3 * my->offset / 2, ourRect.top + 3 * my->offset / 2, 
										 ourRect.right - ourRect.left - 3 * my->offset, ourRect.bottom - ourRect.top - 2 * my->offset, hWnd, 0, my->instance, (LPVOID)my->offset);
							break;

						case IDC_CFGEXIT:
							ShutdownRequest(my->parent);
							break;
					}
					break;

				case LBN_SELCHANGE:
					switch(LOWORD(wParam))
					{
						case IDC_SETTINGS:
							i = SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_GETCURSEL, 0, 0);
							pConfig = (ConfigParam*)SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_GETITEMDATA, (WPARAM)i, 0);
							if((LRESULT)pConfig != LB_ERR)
							{
								INT value = 0;

								my->vHelpText = pConfig->pText;
								SendMessage(hWnd, WM_PAINT, 0, 0);
								toggle = FALSE;
								if(pConfig->vStyle & CT_INF)
								{
									if(pConfig->vStyle & CT_YESNO)
										ConfigGetInt(pConfig->pType.vInfTag, &value);
									else if(pConfig->vStyle & CT_INCR)
										ConfigGetInt(pConfig->pType.vInfTag, &value);
								}
								else if(pConfig->vStyle & CT_DBASE)
								{
									if(pConfig->vStyle & CT_YESNO)
										value = my->dBase->GetBool(pConfig->pType.pDbaseEntry);
									else if(pConfig->vStyle & CT_INCR)
										value = my->dBase->GetInt(pConfig->pType.pDbaseEntry);
								}
								if(pConfig->vStyle & CT_YESNO)
								{
									SetWindowText(GetDlgItem(hWnd, IDC_ENABLE), TEXT("ON"));
									SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, (WPARAM)(value ? BST_INDETERMINATE : BST_UNCHECKED), 0);
									SetWindowText(GetDlgItem(hWnd, IDC_DISABLE), TEXT("OFF"));
									SendDlgItemMessage(hWnd, IDC_DISABLE, BM_SETCHECK, (WPARAM)(value ? BST_UNCHECKED : BST_INDETERMINATE), 0);
								}
								else if(pConfig->vStyle & CT_EXECUTE)
								{
									SetWindowText(GetDlgItem(hWnd, IDC_ENABLE), TEXT("OK"));
									SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, BST_UNCHECKED, 0);
									SetWindowText(GetDlgItem(hWnd, IDC_DISABLE), TEXT(""));
									SendDlgItemMessage(hWnd, IDC_DISABLE, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
								}
								else if(pConfig->vStyle & CT_INCR)
								{
									SetWindowText(GetDlgItem(hWnd, IDC_ENABLE), TEXT("-"));
									SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, BST_UNCHECKED, 0);
									SetWindowText(GetDlgItem(hWnd, IDC_DISABLE), TEXT("+"));
									SendDlgItemMessage(hWnd, IDC_DISABLE, BM_SETCHECK, BST_UNCHECKED, 0);
								}
								if(pConfig->vStyle & CT_LOCKED)
								{
									SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
									SendDlgItemMessage(hWnd, IDC_DISABLE, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
								}
							}
							break;
					}
					break;
			}
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			my->Infobox();
			EndPaint(hWnd, &ps);
			break;

		case WM_SIZE:
			ourRect.right = GetSystemMetrics(SM_CXSCREEN);
			ourRect.bottom = GetSystemMetrics(SM_CYSCREEN);
			ourRect.bottom -= my->offset;

			SetWindowPos(GetDlgItem(my->hSett, IDC_SETTINGS), NULL, 0, 0, ourRect.right - 3 * my->offset, ourRect.bottom / 2 - my->offset, SWP_NOZORDER | SWP_NOMOVE);
			y = 24 * ((ourRect.bottom / 2 - my->offset - 2) / 24) + 2;
			SetWindowPos(GetDlgItem(my->hSett, IDC_ENABLE), NULL, ourRect.right - 6 * my->offset, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(my->hSett, IDC_DISABLE), NULL, ourRect.right - 9 * my->offset / 2, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

			my->vHelpRect.Y = y - 1;
			my->vHelpRect.Width = ourRect.right - 6 *  my->offset - 1;
			my->vHelpTextRect->Y = (float)my->vHelpRect.Y + 1;
			my->vHelpTextRect->Width = (float)my->vHelpRect.Width - 2;

			my->vPsuStatusRect->Y = (float)my->vHelpRect.Y + 200;
			my->vPsuStatusRect->Width = (float)my->vHelpRect.Width - 2;

			SetWindowPos(GetDlgItem(my->hSett, IDC_CFGEXIT), NULL, ourRect.right - 5 * my->offset / 2, my->offset / 2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(my->hSett, IDC_CARGOLOGO), NULL, ourRect.right - 5 * my->offset / 2 - 7, 2 * my->offset, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

			SetWindowPos(GetDlgItem(my->hSett, IDC_EXPLORER), NULL, ourRect.right - 5 * my->offset / 2, ourRect.bottom - 3 * my->offset, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(my->hSett, IDC_OK), NULL, ourRect.right - 5 * my->offset / 2, ourRect.bottom - 3 * my->offset / 2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

			my->InitDC(TRUE);
			break;

		case WM_TIMER:
			/* Get PSU status */
			if(PipeIo("ZU", 3, vStatus, sizeof(vStatus)))
			{
				if(strncmp(vStatus, "comm.", 5) != 0)
				{
					if(PipeIo("ZI", 3, vInput, sizeof(vInput)))
						StringCbPrintfW(my->vPsuStatus, sizeof(my->vPsuStatus), L"PSU Status: %hs, Vin=%hs", vStatus, vInput);
					else
						StringCbPrintfW(my->vPsuStatus, sizeof(my->vPsuStatus), L"PSU Status: %hs, Vin=???", vStatus);
				}
				else
					StringCbPrintfW(my->vPsuStatus, sizeof(my->vPsuStatus), L"PSU Status: %hs", vStatus);
			}
			else
				StringCbPrintfW(my->vPsuStatus, sizeof(my->vPsuStatus), L"PSU Status: SysAccess not available?");
			my->Infobox();
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}


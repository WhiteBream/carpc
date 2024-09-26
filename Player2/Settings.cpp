// Settings.cpp: implementation of the StartButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Settings.h"
#include "Seldir.h"
#include "About.h"
#include <stdio.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define MAX_RECURSE_DEPTH       25
#define EXTENSION_SEPARATOR     L'.'
#define SEARCH_STRING           TEXT("*")

LRESULT CALLBACK SettingsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static LONG		addCount = 0, updCount = 0;
static LPTSTR	lpCurrentDir = NULL;
static TCHAR	vLastPath[MAX_PATH];

static Settings * my;

typedef enum _ConfigStyle
{
	CT_YESNO = 1,
	CT_EXECUTE = 2,
	CT_INCR = 4,
	CT_FUNCTION = 0x100,
	CT_DBASE = 0x200,
	CT_LOCKED = 0x1000,
	CT_CONFIRM = 0x2000
} ConfigStyle;

typedef struct _ConfigParam
{
	INT			vStyle;
	tConfigType	vType;
	LPTSTR		pTextOn;
	LPTSTR		pTextOff;
	LPWSTR		pText;
	union
	{
		LPSTR	pDbaseEntry;
		void	(*pCallback)(LPVOID pConfig, BOOL vValue);
	} pType;
} ConfigParam;

void DeleteDatabase(LPVOID pConfig, BOOL vValue)
{
	my->pAi->hDb->Recreate();
}
void DeleteConfiguration(LPVOID pConfig, BOOL vValue)
{
	my->pAi->hDb->RecreateCfg();
}
void GarbageCollection(LPVOID pConfig, BOOL vValue)
{
	my->cSearch->GarbageCollection();
}

static const ConfigParam sConfig[] = 
{
	{	
		CT_YESNO | CT_DBASE, CONFIGTYPE_BOOL, 
		TEXT("Show a popup window for next song"),
		TEXT("No popup window for next song"), 
		L"Enable or disable the popup window that can be show when the player is not visible.", 
		"Popup"
	},
	{	
		CT_YESNO | CT_DBASE, CONFIGTYPE_BOOL, 
		TEXT("Scale video to fill screen (widescreen)"),
		TEXT("Standard video playback"),
		L"Scale video playback for normal view (with black bars) or widescreen mode (screen filled).", 
		"Widescreen"
	},
	{	
		CT_YESNO | CT_DBASE, CONFIGTYPE_BOOL, 
		TEXT("On screen keyboard is enabled"),
		TEXT("On screen keyboard is disabled"), 
		L"Set the default behavior of the on screen keyboard.", 
		"OnScreenKeyboard"
	},
	{	
		CT_YESNO | CT_DBASE, CONFIGTYPE_BOOL, 
		TEXT("Searching starts at 3 characters"),
		TEXT("Searching is not restricted"), 
		L"Searching is quite time-consuming on slow systems. Restriction skips some searches.",
		"SearchLimit"
	},
	{	
		CT_YESNO | CT_DBASE | CT_LOCKED, CONFIGTYPE_BOOL, 
		TEXT("Spectrum analyzer enabled"),
		TEXT("Spectrum analyzer disabled"), 
		L"The spectrum analyzer works only on an original CARGO car entertainment system.", 
		"Spectrum"
	},
	{	
		CT_YESNO | CT_DBASE, CONFIGTYPE_BOOL, 
		TEXT("Autoplay can select video tracks"),
		TEXT("Autoplay does not select video tracks"), 
		L"Allow the autoplay selection to add videos (which may get blocked while driving).", 
		"AutoVideo"
	},
	{	
		CT_YESNO | CT_DBASE, CONFIGTYPE_BOOL, 
		TEXT("Activate Vant ouput when tuner active"),
		TEXT("No electric antenna installed"), 
		L"Electric antennas or antennaamplifiers can be powered from the Vant output on White Bream mobile computers.", 
		"AntennaVaux"
	},
	{	
		CT_EXECUTE | CT_FUNCTION, CONFIGTYPE_NONE, 
		TEXT("Clean database"),
		NULL, 
		L"Remove unused entries from database.", 
		(LPSTR)GarbageCollection
	},
	{	
		CT_EXECUTE | CT_FUNCTION | CT_CONFIRM, CONFIGTYPE_NONE, 
		TEXT("Delete database"),
		NULL, 
		L"Delete the entire music database.\nDO NOT USE FOR FUN!", 
		(LPSTR)DeleteDatabase
	},
	{	
		CT_EXECUTE | CT_FUNCTION | CT_CONFIRM, CONFIGTYPE_NONE, 
		TEXT("Delete player configuration"),
		NULL, 
		L"Delete the configuration settings.\nDO NOT USE FOR FUN!", 
		(LPSTR)DeleteConfiguration
	},
	{0, CONFIGTYPE_NONE, NULL, NULL, NULL, NULL}
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Settings::Settings(PAppInfo pAppInfo)
{
	RECT        pRect;
	PMediaDescr pSelect;
	INT         y;

	my = this;
	pAi = pAppInfo;

	myGraphics = NULL;

	statusFormat = new StringFormat();
	statusFormat->SetAlignment(StringAlignmentNear);
	statusFormat->SetTrimming(StringTrimmingNone);

	lpCurrentDir = (LPWSTR)GlobalAlloc(GPTR, 2);
	MyRegisterClass();
	GetClientRect(pAi->hApp, &pRect);

	pRect.bottom -= pAi->vOffs;
	statusRect = new RectF(1, (float)pRect.bottom - pAi->vOffs + 1, (float)pRect.right - pAi->vOffs - 2, (float)pAi->vOffs - 2);
	vHelpText = NULL;
	vHelpTextRect = new RectF((float)vHelpRect.X + 1, (float)vHelpRect.Y + 1, (float)vHelpRect.Width - 2, (float)vHelpRect.Height - 2);

	bkGnd = new SolidBrush(Color(255, 200, 255, 200));
	br_black = new SolidBrush(Color(255, 0, 0, 0));
	br_white = new SolidBrush(Color(255, 255, 255, 255));
	pn = new Pen(Color(255, 0, 0, 0), 1);
	fn = new Font(L"Arial", (float)pAi->vOffs / 4);

	hSett = CreateWindow(TEXT("CRPC_STTNGS"), TEXT("settings"), WS_CHILD | WS_CLIPCHILDREN,
		0, 0, pRect.right, pRect.bottom, pAi->hApp, NULL, pAi->hInst, NULL);

	cSearch = new Search(hSett, pAi->hDb);

	CreateWindow(TEXT("WB_LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_HASSTRINGS, 0, 0,
		pRect.right - 3 * pAi->vOffs, pRect.bottom / 2 - pAi->vOffs, hSett, (HMENU)IDC_SETTINGS, pAi->hInst, (LPVOID)(2 * pAi->vOffs / 3));
	SendDlgItemMessage(hSett, IDC_SETTINGS, LB_SETITEMHEIGHT, 0, (LPARAM)24);

	y = 24 * ((pRect.bottom / 2 - pAi->vOffs - 2) / 24) + 2;

	vHelpRect.X = 0;
	vHelpRect.Y = y - 1;
	vHelpRect.Width = pRect.right - 6 * pAi->vOffs - 1;
	vHelpRect.Height = pAi->vOffs;
	vHelpText = NULL;
	vHelpTextRect->X = (float)vHelpRect.X + 1;
	vHelpTextRect->Y = (float)vHelpRect.Y + 1;
	vHelpTextRect->Width = (float)vHelpRect.Width - 2;
	vHelpTextRect->Height = (float)vHelpRect.Height - 2;

	CreateWindow(WBBUTTON, TEXT("ON"), WS_CHILD | WS_VISIBLE, pRect.right - 6 * pAi->vOffs, y, 
		3 * pAi->vOffs / 2, pAi->vOffs, hSett, (HMENU)IDC_ENABLE, pAi->hInst, NULL);
	SendDlgItemMessage(hSett, IDC_ENABLE, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

	CreateWindow(WBBUTTON, TEXT("OFF"), WS_CHILD | WS_VISIBLE, pRect.right - 9 * pAi->vOffs / 2, y, 
		3 * pAi->vOffs / 2, pAi->vOffs, hSett, (HMENU)IDC_DISABLE, pAi->hInst, NULL);
	SendDlgItemMessage(hSett, IDC_DISABLE, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 5 * pAi->vOffs / 2, pAi->vOffs / 2,
		pAi->vOffs * 2, pAi->vOffs, hSett, (HMENU)IDC_CFGEXIT, pAi->hInst, NULL);
	AssignIcon(pAi->hInst, hSett, IDC_CFGEXIT, IDI_EXIT);

	CreateWindow(TEXT("STATIC"), NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY, pRect.right - 5 * pAi->vOffs / 2 - 7, 2 * pAi->vOffs,
		122, 24, hSett, (HMENU)IDC_CARGOLOGO, pAi->hInst, NULL);
	hBmp = LoadBitmap(pAi->hInst, MAKEINTRESOURCE(IDB_CARGO));
	SendDlgItemMessage(hSett, IDC_CARGOLOGO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);

	/* Folder add region */
	CreateWindow(TEXT("WB_LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | LBS_SORT | LBS_NOTIFY | LBS_HASSTRINGS,
		pAi->vOffs, pRect.bottom / 2, pRect.right - pAi->vOffs, pRect.bottom / 2 - pAi->vOffs, hSett, (HMENU)IDC_DIRLIST, pAi->hInst, (LPVOID)(2 * pAi->vOffs / 3));
	SendDlgItemMessage(hSett, IDC_DIRLIST, LB_SETITEMHEIGHT, 0, (LPARAM)24);
	pSelect = (PMediaDescr)GlobalAlloc(GPTR, sizeof(MediaDescr));
	pSelect->vRootId = 1;
	pAi->hDb->TableToListbox(pSelect, GetDlgItem(hSett, IDC_DIRLIST));
	GlobalFree((HGLOBAL)pSelect);

	y = (24 * ((pRect.bottom / 2 - pAi->vOffs - 2) / 24) + 3) / 3;

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, 0, pRect.bottom / 2,
		pAi->vOffs, y, hSett, (HMENU)IDC_ADDFOLDER, pAi->hInst, NULL);
	AssignIcon(pAi->hInst, hSett, IDC_ADDFOLDER, IDI_ADDFOLDER);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, 0, pRect.bottom / 2 + y,
		pAi->vOffs, y, hSett, (HMENU)IDC_DELFOLDER, pAi->hInst, NULL);
	AssignIcon(pAi->hInst, hSett, IDC_DELFOLDER, IDI_REMOVE);
	SendDlgItemMessage(hSett, IDC_DELFOLDER, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, 0, pRect.bottom / 2 + 2 * y,
		pAi->vOffs, y, hSett, (HMENU)IDC_UPDATE, pAi->hInst, NULL);
	AssignIcon(pAi->hInst, hSett, IDC_UPDATE, IDI_UPDATE);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - pAi->vOffs, pRect.bottom - pAi->vOffs,
		pAi->vOffs, pAi->vOffs, hSett, (HMENU)IDC_CFGABORT, pAi->hInst, NULL);
	AssignIcon(pAi->hInst, hSett, IDC_CFGABORT, IDI_DEL);
	SendDlgItemMessage(hSett, IDC_CFGABORT, BM_SETCHECK, BST_INDETERMINATE, 0);

	hDc = GetDC(hSett);
	myGraphics = new Graphics(hDc);
}

Settings::~Settings()
{
	delete fn;
	delete br_black;
	delete br_white;
	delete pn;
	delete bkGnd;
	delete myGraphics;
	delete cSearch;
	ReleaseDC(hSett, hDc);
	DeleteObject(hBmp);
	DestroyWindow(hSett);
//	GlobalFree((HGLOBAL)lpCurrentDir);
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
			BOOL vToggle = TRUE;
			if(sConfig[i].vStyle & CT_DBASE)
			{
				if(sConfig[i].vStyle & CT_YESNO)
					vToggle = pAi->hDb->GetBool(sConfig[i].pType.pDbaseEntry);
				else if(sConfig[i].vStyle & CT_EXECUTE)
					MessageBox(hSett, TEXT("Exec parameter not possible with database entry"), TEXT("Config error"), MB_OK | MB_ICONEXCLAMATION);
			}
			else if(sConfig[i].vStyle & CT_FUNCTION)
			{
				if(sConfig[i].vStyle & CT_YESNO)
					MessageBox(hSett, TEXT("Yes/no parameter not possible with function entry"), TEXT("Config error"), MB_OK | MB_ICONEXCLAMATION);
			}
			id = SendDlgItemMessage(hSett, IDC_SETTINGS, LB_ADDSTRING, 0, (LPARAM)(vToggle ? sConfig[i].pTextOn : sConfig[i].pTextOff));
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
	wcex.hInstance		= pAi->hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= CreateSolidBrush(RGB(200, 255, 200));
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_STTNGS");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
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
	statusRect->Y = (float)ourRect.bottom - pAi->vOffs + 1;
	statusRect->Width = (float)ourRect.right - 2;
	statusRect->Height = (float)pAi->vOffs - 2;
}

void Settings::UpdateFindCounter(void)
{
	WCHAR  count[] = L"%lu files added, %lu updated\n\r%ls";
	size_t lenText;
	LPWSTR cntText;
	
	if(myGraphics == NULL)
		return;

	myGraphics->FillRectangle(br_white, vHelpRect);
	myGraphics->DrawRectangle(pn, vHelpRect);
	if(vHelpText)
		myGraphics->DrawString(vHelpText, -1, fn, *vHelpTextRect, statusFormat, br_black);

	lenText = lstrlen(count) + lstrlen(lpCurrentDir) + 20;
	cntText = (LPWSTR)GlobalAlloc(GPTR, 2 * lenText);
	if(cntText)
	{
		if(lpCurrentDir == NULL)
			StringCchPrintf(cntText, lenText, count, addCount, updCount, L"");
		else
			StringCchPrintf(cntText, lenText, count, addCount, updCount, lpCurrentDir);
	}

	myGraphics->FillRectangle(br_white, 0, ourRect.bottom - pAi->vOffs, ourRect.right - pAi->vOffs, pAi->vOffs);
	myGraphics->DrawRectangle(pn, 0, ourRect.bottom - pAi->vOffs, ourRect.right - pAi->vOffs - 1, pAi->vOffs - 1);
	if(cntText)
		myGraphics->DrawString(cntText, -1, fn, *statusRect, statusFormat, br_black);

	GlobalFree((HGLOBAL)cntText);
}

void Settings::UpdateCurrentDirectory(LPCTSTR lpDir)
{
    DWORD strSize;

    SetCurrentDirectory(lpDir);
    strSize = GetCurrentDirectory(0, NULL);
    lpCurrentDir = (LPTSTR)GlobalReAlloc(lpCurrentDir, 2 * strSize, GPTR); 
	GetCurrentDirectory(strSize, lpCurrentDir);
}

LRESULT CALLBACK SettingsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
    RECT        ourRect;
	LPTSTR      pTmp;
	size_t      vLen;
	LRESULT     i;
	BOOL        toggle;
	INT         y;
	ConfigParam *pConfig;

	GetClientRect(hWnd, &ourRect);

	switch (message)
	{
		case WM_CREATE:
			my->hCbWnd = hWnd;
			my->InitDC(FALSE);
			SendMessage(my->pAi->hApp, WM_SETREGISTERFINALWND, (WPARAM)IDW_MXR, (LPARAM)hWnd);
			RegisterSelDirClass(my->pAi->hInst);
			memset(vLastPath, 0, sizeof(vLastPath));
			break;

		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					switch(LOWORD(wParam))
					{
						case IDC_CFGABORT:
							break;

						case IDC_ENABLE:
							i = SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_GETCURSEL, 0, 0);
							pConfig = (ConfigParam*)SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_GETITEMDATA, (WPARAM)i, 0);
							if((LRESULT)pConfig != LB_ERR)
							{
								if((pConfig->vStyle & CT_DBASE) && (pConfig->vStyle & CT_YESNO))
								{
									toggle = TRUE;
									my->pAi->hDb->SetConfig(pConfig->pType.pDbaseEntry, (LPVOID)&toggle, CONFIGTYPE_BOOL);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_DELETESTRING, i, 0);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_INSERTSTRING, i, (LPARAM)pConfig->pTextOn);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_SETITEMDATA, (WPARAM)i, (LPARAM)pConfig);
									SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
									SendDlgItemMessage(hWnd, IDC_DISABLE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_SETCURSEL, i, 0);
								}
								else if((pConfig->vStyle & CT_FUNCTION) && (pConfig->vStyle & CT_EXECUTE))
								{
									if(pConfig->vStyle & CT_CONFIRM)
										if(MessageBox(hWnd, pConfig->pTextOn, TEXT("Are you really sure?"), MB_YESNO | MB_ICONEXCLAMATION | MB_APPLMODAL) == IDNO)
											return(0);
									pConfig->pType.pCallback((LPVOID)pConfig, TRUE);
									SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
								}
							}
							break;

						case IDC_DISABLE:
							i = SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_GETCURSEL, 0, 0);
							pConfig = (ConfigParam*)SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_GETITEMDATA, (WPARAM)i, 0);
							if((LRESULT)pConfig != LB_ERR)
							{
								if((pConfig->vStyle & CT_DBASE) && (pConfig->vStyle & CT_YESNO))
								{
									toggle = FALSE;
									my->pAi->hDb->SetConfig(pConfig->pType.pDbaseEntry, (LPVOID)&toggle, CONFIGTYPE_BOOL);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_DELETESTRING, i, 0);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_INSERTSTRING, i, (LPARAM)pConfig->pTextOff);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_SETITEMDATA, (WPARAM)i, (LPARAM)pConfig);
									SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
									SendDlgItemMessage(hWnd, IDC_DISABLE, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
									SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_SETCURSEL, i, 0);
								}
							}
							break;

						case IDC_ADDFOLDER:
							GetWindowRect(hWnd, &ourRect);
							i = (LRESULT)CreateWindow(TEXT("CRPC_SELDIR"), NULL, WS_POPUP | WS_DLGFRAME | WS_VISIBLE, ourRect.left + 3 * my->pAi->vOffs / 2, ourRect.top + 3 * my->pAi->vOffs / 2, 
										 ourRect.right - ourRect.left - 3 * my->pAi->vOffs, ourRect.bottom - ourRect.top - 2 * my->pAi->vOffs, hWnd, 0, my->pAi->hInst, (LPVOID)my->pAi->vOffs);
							SendMessage((HWND)i, WM_SETPATH, 0, (LPARAM)vLastPath);
							break;

						case IDC_DELFOLDER:
							i = SendDlgItemMessage(hWnd, IDC_DIRLIST, LB_GETCURSEL, 0, 0);
							if(i != LB_ERR)
							{
								vLen = SendDlgItemMessage(hWnd, IDC_DIRLIST, LB_GETTEXTLEN, (WPARAM)i, 0);
								if(vLen)
								{
									vLen = (vLen + 1) * sizeof(TCHAR);
									pTmp = (LPTSTR)GlobalAlloc(GPTR, vLen);
									if(pTmp)
									{
										SendDlgItemMessage(hWnd, IDC_DIRLIST, LB_GETTEXT, (WPARAM)i, (LPARAM)pTmp);
										my->pAi->hDb->AddRemoveRootEntry(FALSE, pTmp);
										GlobalFree((HGLOBAL)pTmp);
									}
								}
								SendDlgItemMessage(hWnd, IDC_DIRLIST, LB_DELETESTRING, (WPARAM)i, 0);
							}
							SendDlgItemMessage(hWnd, IDC_DELFOLDER, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
							break;

						case IDC_UPDATE:
							i = SendDlgItemMessage(hWnd, IDC_DIRLIST, LB_GETCOUNT, 0, 0);
							if(i && (i != LB_ERR))
							{
								INT idx = i;
								for(i = 0; i < idx; i++)
								{
									vLen = SendDlgItemMessage(hWnd, IDC_DIRLIST, LB_GETTEXTLEN, (WPARAM)i, 0);
									if(vLen)
									{
										vLen = (vLen + 1) * sizeof(TCHAR);
										pTmp = (LPTSTR)GlobalAlloc(GPTR, vLen);
										if(pTmp)
										{
											SendDlgItemMessage(hWnd, IDC_DIRLIST, LB_GETTEXT, (WPARAM)i, (LPARAM)pTmp);
											my->cSearch->AddFolder(FALSE, pTmp);
											GlobalFree((HGLOBAL)pTmp);
										}
									}
								}
							}
							break;

						case IDC_CFGEXIT:
							PostMessage(my->pAi->hApp, WM_CLOSE, 0, 0);
							break;

						case IDC_CARGOLOGO:
							RegisterAboutClass(my->pAi->hInst);
							GetWindowRect(hWnd, &ourRect);
							CreateWindow(TEXT("CRPC_ABOUT"), NULL, WS_POPUP | WS_DLGFRAME | WS_VISIBLE, ourRect.left + 3 * my->pAi->vOffs / 2, ourRect.top + 3 * my->pAi->vOffs / 2, 
										 ourRect.right - ourRect.left - 3 * my->pAi->vOffs, ourRect.bottom - ourRect.top - 2 * my->pAi->vOffs, hWnd, 0, my->pAi->hInst, (LPVOID)my->pAi->vOffs);
							break;
					}
					break;

				case LBN_SELCHANGE:
					switch(LOWORD(wParam))
					{
						case IDC_DIRLIST:
							SendDlgItemMessage(hWnd, IDC_DELFOLDER, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							break;

						case IDC_SETTINGS:
							i = SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_GETCURSEL, 0, 0);
							pConfig = (ConfigParam*)SendDlgItemMessage(hWnd, IDC_SETTINGS, LB_GETITEMDATA, (WPARAM)i, 0);
							if((LRESULT)pConfig != LB_ERR)
							{
								INT value = 0;

								my->vHelpText = pConfig->pText;
								SendMessage(hWnd, WM_PAINT, 0, 0);
								toggle = FALSE;
								if(pConfig->vStyle & CT_DBASE)
								{
									if(pConfig->vStyle & CT_YESNO)
									{
										toggle = my->pAi->hDb->GetBool(pConfig->pType.pDbaseEntry);
									}
									else if(pConfig->vStyle & CT_INCR)
									{
										value = my->pAi->hDb->GetInt(pConfig->pType.pDbaseEntry);
									}
								}
								if(pConfig->vStyle & CT_YESNO)
								{
									SetWindowText(GetDlgItem(hWnd, IDC_ENABLE), TEXT("ON"));
									SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, (WPARAM)(toggle ? BST_INDETERMINATE : BST_UNCHECKED), 0);
									SetWindowText(GetDlgItem(hWnd, IDC_DISABLE), TEXT("OFF"));
									SendDlgItemMessage(hWnd, IDC_DISABLE, BM_SETCHECK, (WPARAM)(toggle ? BST_UNCHECKED : BST_INDETERMINATE), 0);
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

		case WM_RECEIVESTRING:
			if(lParam)
			{
				if(my->pAi->hDb->AddRemoveRootEntry(TRUE, (LPTSTR)lParam) == TRUE)
				{
					StringCbCopy(vLastPath, sizeof(vLastPath), (LPTSTR)lParam);
					SendDlgItemMessage(hWnd, IDC_DIRLIST, LB_ADDSTRING, 0, lParam);
					my->cSearch->AddFolder(TRUE, (LPTSTR)lParam);
				}
				GlobalFree((HGLOBAL)lParam);
			}
			break;

		case WM_SEARCHCOUNT:
			addCount = (LONG)wParam;
			updCount = (LONG)lParam;
			my->UpdateFindCounter();
			break;

		case WM_SEARCHPATH:
			lpCurrentDir = (LPTSTR)wParam;
			my->UpdateFindCounter();
			break;

		case WM_SAVESEARCHQUEUE:
		//	my->pAi->hDb->SetConfig("SearchQueue", (LPVOID)&toggle, CONFIGTYPE_BOOL);
			break;

		case WM_SAVESEARCHPATH:
		//	my->pAi->hDb->SetConfig("SearchQueue", (LPVOID)&toggle, CONFIGTYPE_BOOL);
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			my->UpdateFindCounter();
			EndPaint(hWnd, &ps);
			break;

		case WM_SIZE:
			SetWindowPos(GetDlgItem(my->hSett, IDC_SETTINGS), NULL, 0, 0, ourRect.right - 3 * my->pAi->vOffs, ourRect.bottom / 2 - my->pAi->vOffs, SWP_NOZORDER | SWP_NOMOVE);
			y = 24 * ((ourRect.bottom / 2 - my->pAi->vOffs - 2) / 24) + 2;
			SetWindowPos(GetDlgItem(my->hSett, IDC_SETTDESCR), NULL, 0, y, ourRect.right - 6 * my->pAi->vOffs, my->pAi->vOffs, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(my->hSett, IDC_ENABLE), NULL, ourRect.right - 6 * my->pAi->vOffs, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(my->hSett, IDC_DISABLE), NULL, ourRect.right - 9 * my->pAi->vOffs / 2, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

			my->vHelpRect.Y = y - 1;
			my->vHelpRect.Width = ourRect.right - 6 *  my->pAi->vOffs - 1;
			my->vHelpTextRect->Y = (float)my->vHelpRect.Y + 1;
			my->vHelpTextRect->Width = (float)my->vHelpRect.Width - 2;

			SetWindowPos(GetDlgItem(my->hSett, IDC_CFGEXIT), NULL, ourRect.right - 5 * my->pAi->vOffs / 2, my->pAi->vOffs / 2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(my->hSett, IDC_CARGOLOGO), NULL, ourRect.right - 5 * my->pAi->vOffs / 2 - 7, 2 * my->pAi->vOffs, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

			SetWindowPos(GetDlgItem(my->hSett, IDC_DIRLIST), NULL, my->pAi->vOffs, ourRect.bottom / 2, ourRect.right - my->pAi->vOffs, ourRect.bottom / 2 - my->pAi->vOffs, SWP_NOZORDER);
			y = (24 * ((ourRect.bottom / 2 - my->pAi->vOffs - 2) / 24) + 3) / 3;
			SetWindowPos(GetDlgItem(my->hSett, IDC_ADDFOLDER), NULL, 0, ourRect.bottom / 2, my->pAi->vOffs, y, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(my->hSett, IDC_DELFOLDER), NULL, 0, ourRect.bottom / 2 + y, my->pAi->vOffs, y, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(my->hSett, IDC_UPDATE), NULL, 0, ourRect.bottom / 2 + 2 * y, my->pAi->vOffs, y, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(my->hSett, IDC_CFGABORT), NULL, ourRect.right - my->pAi->vOffs, ourRect.bottom - my->pAi->vOffs, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

			my->InitDC(TRUE);
			my->UpdateFindCounter();
			break;

		case WM_DELETEITEM:
			switch(((LPDRAWITEMSTRUCT)lParam)->CtlID)
			{
				case IDC_DIRLIST:
					if(((PDELETEITEMSTRUCT)lParam)->itemData)
						GlobalFree((HGLOBAL)((PDELETEITEMSTRUCT)lParam)->itemData);
					break;
			}
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}


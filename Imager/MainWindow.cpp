// MainWindow.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MainWindow.h"
#include "Seldir.h"
#include <Commctrl.h>

#define DEFAULTSIZE		48

enum _Timers
{
	TMR_START = 1,
	TMR_PAINT,
	TMR_PLAY
} Timers;

struct _draw
{
	Graphics	*gr;
	HDC			hDc;
	Brush		*br_black;
	Brush		*br_white;
	Brush		*br_blue;
	Font		*fn;
} draw;

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void DrawListBox(HWND hWnd);
static void DrawListBoxItem(LPDRAWITEMSTRUCT pItem);
static void InitListBox(BOOL loadUnload, HWND lbox);

static MainWindow * my;


MainWindow::MainWindow(HINSTANCE hInstance, HWND hParent)
{
	GdiplusStartupInput	gdiplusStartupInput;
	HWND				hStart;
	Rect				btnPlace;
	STARTUPINFO			supInfo;
	INT					winH, winW, winX, winY;

	MyRegisterClass(hInstance);
	hInst = hInstance;
	my = this;
	vJustStarted = FALSE;

	hStart = FindWindow(TEXT("CRPC_STRTBTTN"), NULL);
	if(hStart)
		GetClientRect(hStart, &startRect);
	else
		startRect.bottom = startRect.left = startRect.right = startRect.top = 0;
	if(!startRect.bottom)
		startRect.bottom = DEFAULTSIZE;
	offs = startRect.bottom;

	memset(&supInfo, 0, sizeof(supInfo));
	GetStartupInfo(&supInfo);
	if(supInfo.dwFlags & STARTF_USESIZE)
	{
		winH = supInfo.dwYSize;
		winW = supInfo.dwXSize;
	}
	else
	{
		winH = 8 * startRect.bottom;
		winW = (4 * winH) / 3;
	}
	if(supInfo.dwFlags & STARTF_USEPOSITION)
	{
		winX = supInfo.dwX;
		winY = supInfo.dwY;
	}
	else
	{
		winX = CW_USEDEFAULT;
		winY = CW_USEDEFAULT;
	}

	// Initialize GDI+.
	gdiplusStartupInput.DebugEventCallback = NULL;
	gdiplusStartupInput.GdiplusVersion = 1;
	gdiplusStartupInput.SuppressBackgroundThread = FALSE;
	gdiplusStartupInput.SuppressExternalCodecs = FALSE;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	myGraphics = NULL;
	bkGnd1 = new SolidBrush(Color(255, 0, 0, 192));
	bkGnd2 = new SolidBrush(Color(255, 200, 255, 200));
	font = new Font(TEXT("Arial"), 16);
	//fontColor = new SolidBrush(Color(50, 200, 255, 200));
	fontFormat = new StringFormat();
	fontFormat->SetAlignment(StringAlignmentFar);
	fontFormat->SetTrimming(StringTrimmingNone);
	fontFormat->SetLineAlignment(StringAlignmentFar);
	fontFormat->SetFormatFlags(StringFormatFlagsNoWrap);

	DestinatorInit(hInstance);

	/* Start database */
	dBase = new ConfigDb();

	hWnd = CreateWindow(TEXT("CRGIMG_MN"), TEXT("Picture viewer"), WS_TILEDWINDOW | WS_CLIPCHILDREN,
						winX, winY, winW, winH, NULL, NULL, hInstance, NULL);

	GetClientRect(hWnd, &ourRect);
	btnPlace.X = startRect.right;
	btnPlace.Y = ourRect.bottom - startRect.bottom;
	btnPlace.Width = startRect.bottom;
	btnPlace.Height = startRect.bottom;

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y,
				btnPlace.Width, btnPlace.Height, hWnd, (HMENU)IDC_FOLDER, hInstance, NULL);
	AssignIcon(hInstance, hWnd, IDC_FOLDER, IDI_FOLDER);

//	btnPlace.X += startRect.bottom;
//	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y,
//				btnPlace.Width, btnPlace.Height, hWnd, (HMENU)IDC_ZOOMOUT, hInstance, NULL);
//	AssignIcon(hInstance, hWnd, IDC_ZOOMOUT, IDI_ZOOMOUT);
//	SendDlgItemMessage(hWnd, IDC_ZOOMOUT, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

//	btnPlace.X += startRect.bottom;
//	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y,
//				btnPlace.Width, btnPlace.Height, hWnd, (HMENU)IDC_ZOOMIN, hInstance, NULL);
//	AssignIcon(hInstance, hWnd, IDC_ZOOMIN, IDI_ZOOMIN);
//	SendDlgItemMessage(hWnd, IDC_ZOOMIN, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

	btnPlace.X += startRect.bottom;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y,
					btnPlace.Width, btnPlace.Height, hWnd, (HMENU)IDC_PREV, hInstance, NULL);
	AssignIcon(hInstance, hWnd, IDC_PREV, IDI_PREV);
	SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

	btnPlace.X += startRect.bottom;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y,
				btnPlace.Width, btnPlace.Height, hWnd, (HMENU)IDC_NEXT, hInstance, NULL);
	AssignIcon(hInstance, hWnd, IDC_NEXT, IDI_NEXT);
	SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

	btnPlace.X += startRect.bottom;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y,
				btnPlace.Width, btnPlace.Height, hWnd, (HMENU)IDC_PLAY, hInstance, NULL);
	AssignIcon(hInstance, hWnd, IDC_PLAY, IDI_PLAY);
	SendDlgItemMessage(hWnd, IDC_PLAY, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

	btnPlace.X += startRect.bottom;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y,
				btnPlace.Width, btnPlace.Height, hWnd, (HMENU)IDC_EXIT, hInstance, NULL);
	AssignIcon(hInstance, hWnd, IDC_EXIT, IDI_EXIT);

	CreateWindow(WBVSCROLL, NULL, WS_CHILD, ourRect.right - 2 * offs / 3, 0,
				2 * offs / 3, btnPlace.Y, hWnd, (HMENU)IDC_VSCROLL, hInstance, NULL);
	SendDlgItemMessage(hWnd, IDC_VSCROLL, TBM_SETRANGE, TRUE, MAKELONG(0, 0));

	CreateWindow(TEXT("LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | LBS_SORT | LBS_HASSTRINGS, ourRect.right - offs, btnPlace.Y,
				 0, 0, hWnd, (HMENU)IDC_FILES, hInstance, NULL);

	ShowWindow(hWnd, SW_SHOW);
	SetTimer(hWnd, TMR_START, 200, NULL);
}

MainWindow::~MainWindow()
{
	if(dBase) delete dBase;
	DestroyWindow(hWnd);
	if(myGraphics) delete myGraphics;
	if(bkGnd1) delete bkGnd1;
	if(bkGnd2) delete bkGnd2;
	if(fontFormat) delete fontFormat;
	//if(fontColor) delete fontColor;
	if(font) delete font;
	GdiplusShutdown(gdiplusToken);
}

ATOM MainWindow::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= MainWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIEWER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRGIMG_MN");
	wcex.hIconSm		= (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_VIEWER), IMAGE_ICON, 16, 16, 0);

	return RegisterClassEx(&wcex);
}

void MainWindow::InitDC(BOOL updateDC)
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

BOOL IsFileImage(LPCTSTR lpFileName)
{
    LPCTSTR extImage[] = {TEXT("JPG"), TEXT("JPEG"), TEXT("BMP"), TEXT("GIF"), TEXT("PNG"), NULL};
	LPTSTR *extType;
    LPTSTR  extStart;
	TCHAR   extName[10];
    INT     i;

	extType = (LPTSTR*)extImage;

    extStart = _tcsrchr(lpFileName, TEXT('.'));
	if(extStart == NULL)	// No extension at all
	{
		return(FALSE);
	}
    extStart++;
	StringCbCopy(extName, sizeof(extName), extStart);
	CharUpper(extName);

    i = 0;
    while(extType[i])
    {
        if(_tcscmp(extName, extType[i]) == 0)
            return(TRUE);
        i++;
    }
    return(FALSE);
}

typedef enum _DisplayMode
{
	ZOOM1X,
} DisplayMode;

DisplayMode vDisplayMode = ZOOM1X;

TCHAR	vCurrentFile[MAX_PATH] = {0};

void MainWindow::Paint()
{
	if(myGraphics == NULL)
		return;

	Image			*pImg = new Bitmap(ourRect.right, ourRect.bottom);
	Graphics		*pGraph = new Graphics(pImg);
	Bitmap			*pBmp = NULL;
	RectF			vDest, vSrc;
	REAL			lWidth, lHeight;

	pGraph->FillRectangle(bkGnd2, 0, 0, ourRect.right, ourRect.bottom - offs);
	pGraph->FillRectangle(bkGnd1, 0, ourRect.bottom - offs, ourRect.right, offs);

	switch(vDisplayMode)
	{
		case ZOOM1X:
			pBmp = new Bitmap(vCurrentFile);
			break;
	}

	if(pBmp)
	{
		if(pBmp->GetHeight() && pBmp->GetWidth())
		{
			vDest.X = 0;
			vDest.Y = 0;
			vDest.Width = (REAL)ourRect.right;
			vDest.Height = (REAL)ourRect.bottom - offs;

			vSrc.X = 0;
			vSrc.Y = 0;
			vSrc.Width = (REAL)pBmp->GetWidth();
			vSrc.Height = (REAL)pBmp->GetHeight();

			if((vDest.Height * vSrc.Width) < (vDest.Width * vSrc.Height))
			{
				lWidth = vDest.Height * vSrc.Width / vSrc.Height;
				lWidth = vDest.Width - lWidth;
				vDest.X += lWidth / 2;
				vDest.Width -= lWidth;
			}
			else
			{
				lHeight = vSrc.Height * vDest.Width / vSrc.Width;
				lHeight = vDest.Height - lHeight;
				vDest.Y += lHeight / 2;
				vDest.Height -= (REAL)lHeight;
			}
			// Resize image for 800/640 wider screen
			if(dBase->GetBool("LilliputMode"))
			{
				lHeight = (vSrc.Height * 640) / 800;
				lHeight = vSrc.Height - lHeight;
				vSrc.Y += lHeight / 2;
				vSrc.Height -= lHeight;
			}
			pGraph->DrawImage(pBmp, vDest, vSrc.X, vSrc.Y, vSrc.Width, vSrc.Height, UnitPixel);

			ImageAttributes *attrib = new ImageAttributes();
			attrib->SetGamma(0.25);
			vSrc.X = 0;
			vSrc.Y = 0;
			vSrc.Width = (REAL)ourRect.right;
			vSrc.Height = (REAL)ourRect.bottom - offs;
			fontColor = new TextureBrush(pImg, vSrc, attrib);
			delete attrib;

			pGraph->DrawString(vCurrentFile, -1, font, vDest, fontFormat, fontColor);
			if(fontColor) delete fontColor;
		}
	}
	myGraphics->DrawImage(pImg, 0, 0);

	if(pBmp) delete pBmp;
	if(pGraph) delete pGraph;
	if(pImg) delete pImg;
}


LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT		ps;
	int				h, w;
	RECT			ourRect, wr;
	LRESULT			id;
	HANDLE			hSearch;
	WIN32_FIND_DATA	findData;
	LPTSTR			pPath;

	h = my->startRect.bottom ? my->startRect.bottom : DEFAULTSIZE;
	w = my->startRect.right;

	switch (message)
	{
		case WM_CREATE:
			my->hCbWnd = hWnd;
			my->InitDC(FALSE);
			RegisterSelDirClass(my->hInst);
			break;

		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				switch(LOWORD(wParam))
				{
					case IDC_FOLDER:
						GetWindowRect(hWnd, &ourRect);
						id = (LRESULT)CreateWindow(TEXT("CRPC_SELDIR"), NULL, WS_POPUP | WS_DLGFRAME | WS_VISIBLE,
										ourRect.left + 3 * my->offs / 2, ourRect.top + 3 * my->offs / 2,
										 ourRect.right - ourRect.left - 3 * my->offs, ourRect.bottom - ourRect.top - 3 * my->offs, hWnd, 0, my->hInst, (LPVOID)my->offs);

						pPath = NULL;
						if(my->dBase->GetConfig("ImagerLastDir", (LPVOID*)&pPath, CONFIGTYPE_UNICODE))
						{
							my->dBase->Free(pPath);
							pPath = (LPTSTR)GlobalAlloc(GPTR, MAX_PATH * sizeof(TCHAR));
							GetCurrentDirectory(MAX_PATH, pPath);
							SendMessage((HWND)id, WM_SETPATH, 0, (LPARAM)pPath);
							GlobalFree(pPath);
						}
						break;

					case IDC_ZOOMIN:
						break;

					case IDC_ZOOMOUT:
						break;

					case IDC_PREV:
						id = SendDlgItemMessage(hWnd, IDC_FILES, LB_GETCURSEL, 0, 0);
						if(SendDlgItemMessage(hWnd, IDC_FILES, LB_SETCURSEL, --id, 0) != LB_ERR)
						{
							SendDlgItemMessage(hWnd, IDC_FILES, LB_GETTEXT, id, (LPARAM)vCurrentFile);
							my->Paint();
							my->dBase->SetConfig("ImagerLastIndex", (LPVOID)&id, CONFIGTYPE_INT);
							if(id == 0)
								SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
							SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							SendDlgItemMessage(hWnd, IDC_PLAY, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							AssignIcon(my->hInst, hWnd, IDC_PLAY, IDI_PLAY);
						}
						else
							SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
						break;

					case IDC_NEXT:
						if(SendDlgItemMessage(hWnd, IDC_PLAY, BM_GETCHECK, 0, 0) != BST_CHECKED)
						{
							id = SendDlgItemMessage(hWnd, IDC_FILES, LB_GETCURSEL, 0, 0);
							if(++id >= SendDlgItemMessage(hWnd, IDC_FILES, LB_GETCOUNT, 0, 0))
							{
								SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
								break;
							}
							if(SendDlgItemMessage(hWnd, IDC_FILES, LB_SETCURSEL, id, 0) != LB_ERR)
							{
								SendDlgItemMessage(hWnd, IDC_FILES, LB_GETTEXT, id, (LPARAM)vCurrentFile);
								my->Paint();
								my->dBase->SetConfig("ImagerLastIndex", (LPVOID)&id, CONFIGTYPE_INT);
								if(++id >= SendDlgItemMessage(hWnd, IDC_FILES, LB_GETCOUNT, 0, 0))
									SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
								SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							}
							else
								SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
						}
						else
						{
							if(SendDlgItemMessage(hWnd, IDC_NEXT, BM_GETCHECK, 0, 0) == BST_CHECKED)
							{
								id = FALSE;
								SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							}
							else
							{
								id = TRUE;
								SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
							}
							my->dBase->SetConfig("ImagerPlayRepeat", (LPVOID)&id, CONFIGTYPE_BOOL);
						}
						break;

					case IDC_PLAY:
						if(SendDlgItemMessage(hWnd, IDC_PLAY, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
						{
							SendDlgItemMessage(hWnd, IDC_PLAY, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
							AssignIcon(my->hInst, hWnd, IDC_PLAY, IDI_STOP);
							SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
							AssignIcon(my->hInst, hWnd, IDC_NEXT, IDI_REPEAT);
							if(my->dBase->GetBool("ImagerPlayRepeat"))
								SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
							SetTimer(hWnd, TMR_PLAY, 3000, NULL);
						}
						else
						{
							SendDlgItemMessage(hWnd, IDC_PLAY, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							AssignIcon(my->hInst, hWnd, IDC_PLAY, IDI_PLAY);
							AssignIcon(my->hInst, hWnd, IDC_NEXT, IDI_NEXT);

							id = SendDlgItemMessage(hWnd, IDC_FILES, LB_GETCURSEL, 0, 0);
							if(id)
								SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							if(++id < SendDlgItemMessage(hWnd, IDC_FILES, LB_GETCOUNT, 0, 0))
								SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							else
								SendDlgItemMessage(hWnd, IDC_PLAY, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
							KillTimer(hWnd, TMR_PLAY);
						}
						break;

					case IDC_EXIT:
						PostQuitMessage(0);
						break;
				}
			}
			break;

		case WM_RECEIVESTRING:
			if(lParam)
			{
				SendDlgItemMessage(hWnd, IDC_FILES, LB_RESETCONTENT, 0, 0);
				SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
				SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
				AssignIcon(my->hInst, hWnd, IDC_NEXT, IDI_NEXT);
				SendDlgItemMessage(hWnd, IDC_PLAY, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
				AssignIcon(my->hInst, hWnd, IDC_PLAY, IDI_PLAY);
				SetCurrentDirectory((LPCTSTR)lParam);

				my->dBase->SetConfig("ImagerLastDir", (LPVOID)lParam, CONFIGTYPE_UNICODE);

				hSearch = FindFirstFile(TEXT("*"), &findData); 
				if(hSearch != INVALID_HANDLE_VALUE)
					PostMessage(hWnd, WM_FINDNEXTFILE, (WPARAM)hSearch, 0);

				GlobalFree((HGLOBAL)lParam);
			}
			break;

		case WM_FINDNEXTFILE:
			hSearch = (HANDLE)wParam;
			if(hSearch)
			{
				if(FindNextFile(hSearch, &findData))
				{
					if(!(findData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_OFFLINE)))
					{
						if(IsFileImage(findData.cFileName))
						{
							id = SendDlgItemMessage(hWnd, IDC_FILES, LB_ADDSTRING, 0, (LPARAM)findData.cFileName);
							switch(SendDlgItemMessage(hWnd, IDC_FILES, LB_GETCOUNT, 0, 0))
							{
								case 0:
									break;
								case 1:
									if(my->vJustStarted == FALSE)
									{
										StringCbCopy(vCurrentFile, sizeof(vCurrentFile), findData.cFileName);
										my->Paint();
										SendDlgItemMessage(hWnd, IDC_FILES, LB_SETCURSEL, 0, 0);
									}
									break;
								case 2:
									SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
									SendDlgItemMessage(hWnd, IDC_PLAY, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
									break;
							}
							if(my->vJustStarted && !my->dBase->GetInt("ImagerPlayIndex"))
							{
								INT vLastIndex;
								LPINT pIdx = &vLastIndex;
								if(my->dBase->GetConfig("ImagerLastIndex", (LPVOID*)&pIdx, CONFIGTYPE_INT))
									if(vLastIndex == id)
									{
										if(id > 0)
											SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

										StringCbCopy(vCurrentFile, sizeof(vCurrentFile), findData.cFileName);
										my->Paint();
										SendDlgItemMessage(hWnd, IDC_FILES, LB_SETCURSEL, id, 0);
										my->vJustStarted = FALSE;
									}
							}
						}
					}
					PostMessage(hWnd, WM_FINDNEXTFILE, wParam, 0);
				}
				else
				{
					if((id = my->dBase->GetInt("ImagerPlayIndex")))
					{
						if(SendDlgItemMessage(hWnd, IDC_FILES, LB_SETCURSEL, --id, 0) != LB_ERR)
						{
							SendDlgItemMessage(hWnd, IDC_FILES, LB_GETTEXT, id, (LPARAM)vCurrentFile);
							SendDlgItemMessage(hWnd, IDC_PLAY, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
							AssignIcon(my->hInst, hWnd, IDC_PLAY, IDI_STOP);
							SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
							AssignIcon(my->hInst, hWnd, IDC_NEXT, IDI_REPEAT);
							if(my->dBase->GetBool("ImagerPlayRepeat"))
								SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
							SetTimer(hWnd, TMR_PLAY, 3000, NULL);
						}
						else
						{
							if(SendDlgItemMessage(hWnd, IDC_FILES, LB_SETCURSEL, 0, 0) != LB_ERR)
								SendDlgItemMessage(hWnd, IDC_FILES, LB_GETTEXT, 0, (LPARAM)vCurrentFile);
						}
						my->Paint();
						id = LB_ERR;
						my->dBase->SetConfig("ImagerPlayIndex", (LPVOID)&id, CONFIGTYPE_INT);
					}
					FindClose(hSearch);
				}
			}
			break;

		case WM_SIZE:
			GetClientRect(hWnd, &ourRect);
			if((ourRect.right < (w + (8 * h))) || (ourRect.bottom < (3 * (w + (8 * h)) / 4)))
			{
				GetWindowRect(hWnd, &wr);
				wr.right -= wr.left;
				wr.right -= ourRect.right; // result is width overhead
				wr.bottom -= wr.top;
				wr.bottom -= ourRect.bottom; // result is height overhead

				if(ourRect.right < (w + (8 * h))) ourRect.right = w + (8 * h);
				if(ourRect.bottom < (3 * (w + (8 * h)) / 4)) ourRect.bottom = 3 * (w + (8 * h)) / 4;

				SetWindowPos(hWnd, 0, 0, 0, ourRect.right + wr.right, ourRect.bottom + wr.bottom,
										SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
				GetClientRect(hWnd, &ourRect);
			}
			id = 0;
			SetWindowPos(GetDlgItem(hWnd, IDC_FOLDER), 0, w + id++ * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
		//	SetWindowPos(GetDlgItem(hWnd, IDC_ZOOMOUT), 0, w + id++ * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
		//	SetWindowPos(GetDlgItem(hWnd, IDC_ZOOMIN), 0, w + id++ * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_PREV), 0, w + id++ * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_NEXT), 0, w + id++ * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_PLAY), 0, w + id++ * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_EXIT), 0, w + id++ * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

			SetWindowPos(GetDlgItem(hWnd, IDC_VSCROLL), 0, ourRect.right - 2 * my->offs / 3, 0, 2 * my->offs / 3, ourRect.bottom - my->offs, SWP_NOACTIVATE | SWP_NOZORDER);
			//SetWindowPos(GetDlgItem(hWnd, IDC_HSCROLL), 0, 0, ourRect.bottom - 2 * my->offs / 3, ourRect.right - 2 * my->offs / 3, 2 * my->offs / 3, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, IDC_FILES), 0, ourRect.right - my->offs, ourRect.bottom - my->offs, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

			my->InitDC(TRUE);
			my->Paint();
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			SetTimer(hWnd, TMR_PAINT, 10, NULL);
			EndPaint(hWnd, &ps);
			break;

		case WM_TIMER:
			if(wParam == TMR_START)
			{
				KillTimer(hWnd, wParam);
				// Open database and retrieve current path
				pPath = NULL;
				if(my->dBase->GetConfig("ImagerLastDir", (LPVOID*)&pPath, CONFIGTYPE_UNICODE))
				{
					my->vJustStarted = TRUE;
					SetCurrentDirectory(pPath);
					hSearch = FindFirstFile(TEXT("*"), &findData); 
					if(hSearch != INVALID_HANDLE_VALUE)
						PostMessage(hWnd, WM_FINDNEXTFILE, (WPARAM)hSearch, 0);
					my->dBase->Free(pPath);
				}
			}
			else if(wParam == TMR_PAINT)
			{
				KillTimer(hWnd, wParam);
				my->Paint();
			}
			else if(wParam == TMR_PLAY)
			{
				POINT	testPoint;
				HWND	hTest;

				testPoint.x = GetSystemMetrics(SM_CXSCREEN) / 2;
				testPoint.y = GetSystemMetrics(SM_CYSCREEN) / 2;
				hTest = WindowFromPoint(testPoint);
				if((hTest == hWnd) || IsChild(hWnd, hTest))
				{
					SetTimer(hWnd, TMR_PLAY, 3000, NULL);
					id = SendDlgItemMessage(hWnd, IDC_FILES, LB_GETCURSEL, 0, 0);
					if(++id >= SendDlgItemMessage(hWnd, IDC_FILES, LB_GETCOUNT, 0, 0))
					{
						if(SendDlgItemMessage(hWnd, IDC_NEXT, BM_GETCHECK, 0, 0) != BST_CHECKED)
						{
							SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							SendDlgItemMessage(hWnd, IDC_PLAY, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
							AssignIcon(my->hInst, hWnd, IDC_NEXT, IDI_NEXT);
							KillTimer(hWnd, wParam);
						}
						else id = 0;
					}
					if(SendDlgItemMessage(hWnd, IDC_FILES, LB_SETCURSEL, id, 0) != LB_ERR)
					{
						SendDlgItemMessage(hWnd, IDC_FILES, LB_GETTEXT, id, (LPARAM)vCurrentFile);
						my->Paint();
					}
				}
				else
					// Window not visible, postpone autoplay with 1 second
					SetTimer(hWnd, TMR_PLAY, 1000, NULL);
			}
			break;

		case WM_MOUSEWHEEL:
			my->curtop += -GET_WHEEL_DELTA_WPARAM(wParam) / 40;
			if(my->curtop < 0) my->curtop = 0;
			if(my->curtop > (my->range - my->visibleCount)) my->curtop = my->range - my->visibleCount;
			SendDlgItemMessage(hWnd, IDC_FILES, LB_SETTOPINDEX, (LPARAM)my->curtop, 0);
			SendDlgItemMessage(hWnd, IDC_VSCROLL, TBM_SETPOS, TRUE, my->curtop);
			return(0);

		case WM_DESTROY:
			id = 0;
			if(SendDlgItemMessage(hWnd, IDC_PLAY, BM_GETCHECK, 0, 0) == BST_CHECKED)
				id = SendDlgItemMessage(hWnd, IDC_FILES, LB_GETCURSEL, 0, 0) + 1;
			else
				id = LB_ERR;
			my->dBase->SetConfig("ImagerPlayIndex", (LPVOID)&id, CONFIGTYPE_INT);
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}


static void DrawListBox(HWND hWnd)
{
	RECT vListBox, vItem;
	INT  i, vCount;
	LONG vTop;

	GetClientRect(hWnd, &vListBox);
	vCount = SendMessage(hWnd, LB_GETCOUNT, 0, 0);
	vItem.bottom = SendMessage(hWnd, LB_GETITEMHEIGHT, 0, 0);
if(vItem.bottom==0)
return;
	if(vCount < (vListBox.bottom / vItem.bottom))
	{
		for(i = vCount; i < (vListBox.bottom / vItem.bottom); i++)
		{
			vTop = vListBox.top + i * vItem.bottom;
			if(i % 2)
				draw.gr->FillRectangle(draw.br_blue, vListBox.left, vTop, vListBox.right, vItem.bottom);
			else
				draw.gr->FillRectangle(draw.br_white, vListBox.left + vItem.bottom, vTop, vListBox.right - vItem.bottom, vItem.bottom);
		}
	}
}

static void DrawListBoxItem(LPDRAWITEMSTRUCT pItem)
{
	RectF        rf;
	RECT         crect;
	INT          i, n, size;
	StringFormat fmt;
	LPWSTR       pText;
	LONG         top;

	pItem->rcItem.right -= pItem->rcItem.left;
	pItem->rcItem.bottom -= pItem->rcItem.top;

	rf.X = (float)pItem->rcItem.left;
	rf.Y = (float)pItem->rcItem.top + 3;
	rf.Width = (float)pItem->rcItem.right;
	rf.Height = (float)pItem->rcItem.bottom - 3;

	GetClientRect(pItem->hwndItem, &crect);

	n = SendMessage(pItem->hwndItem, LB_GETCOUNT, 0, 0);
	if((n < (crect.bottom / pItem->rcItem.bottom)) || (pItem->itemState & ODA_DRAWENTIRE))
	{
		for(i = n; i < (crect.bottom / pItem->rcItem.bottom); i++)
		{
			top = crect.top + i * pItem->rcItem.bottom;
			if(i % 2)
				draw.gr->FillRectangle(draw.br_blue, pItem->rcItem.left, top, pItem->rcItem.right, pItem->rcItem.bottom);
			else
				draw.gr->FillRectangle(draw.br_white, pItem->rcItem.left, top, pItem->rcItem.right, pItem->rcItem.bottom);
		}
	}

	size = 2 * SendMessage(pItem->hwndItem, LB_GETTEXTLEN, (WPARAM)pItem->itemID, 0) + 2;
	pText = (LPWSTR)GlobalAlloc(GPTR, size);
	if(pText)
		SendMessage(pItem->hwndItem, LB_GETTEXT, (WPARAM)pItem->itemID, (LPARAM)pText);

	fmt.SetAlignment(StringAlignmentNear);
	fmt.SetTrimming(StringTrimmingNone);
	fmt.SetLineAlignment(StringAlignmentCenter);
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);
	if(pItem->itemState & ODS_SELECTED)
	{
   		draw.gr->FillRectangle(draw.br_black, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.right, pItem->rcItem.bottom);
		if(pText)
			draw.gr->DrawString(pText, -1, draw.fn, rf, &fmt, draw.br_white);
	}
	else if(n != 0)
	{
		if(pItem->itemID % 2)
			draw.gr->FillRectangle(draw.br_blue, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.right, pItem->rcItem.bottom);
		else
			draw.gr->FillRectangle(draw.br_white, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.right, pItem->rcItem.bottom);

		// Item text
		if(pText)
			draw.gr->DrawString(pText, -1, draw.fn, rf, &fmt, draw.br_black);
	}
	if(pText)
		GlobalFree(pText);
}

static void InitListBox(BOOL loadUnload, HWND lbox)
{
	if(loadUnload)
	{
		draw.hDc = GetDC(lbox);
		draw.gr = new Graphics(draw.hDc, FALSE);
		if(!draw.br_black) draw.br_black = new SolidBrush(Color(255, 0, 0, 0));
		if(!draw.br_white) draw.br_white = new SolidBrush(Color(255, 255, 255, 255));
		if(!draw.br_blue) draw.br_blue = new SolidBrush(Color(255, 201, 211, 227));
		if(!draw.fn) draw.fn = new Font(L"Arial", 16);
	}
	else if(lbox == NULL)
	{
		delete draw.fn;
		delete draw.br_blue;
		delete draw.br_white;
		delete draw.br_black;
		delete draw.gr;
	//	ReleaseDC(lbox, draw.hDc);
	}
	else
	{
		delete draw.gr;
		ReleaseDC(lbox, draw.hDc);
		draw.hDc = GetDC(lbox);
		draw.gr = new Graphics(draw.hDc, FALSE);
	}
}


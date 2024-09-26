// Sorted.cpp: implementation of the Sorted class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Sorted.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

LRESULT CALLBACK SortedWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void DrawSortListBox(HWND hWnd);
static void DrawSortListBoxItem(LPDRAWITEMSTRUCT pItem);
static void InitSortListBox(BOOL loadUnload, HWND lbox);

static Sorted * my;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Sorted::Sorted(PAppInfo pAppInfo)
{
	RECT  pRect;
	INT   x, w;

	my = this;
	pAi = pAppInfo;
	myGraphics = NULL;
	MyRegisterClass();

	GetClientRect(pAi->hApp, &pRect);
	bkGnd = new SolidBrush(Color(255, 200, 255, 200));
	pn = new Pen(Color(255, 0, 0, 0), 1);
	font = new Font(L"Arial", 16);

	hList = CreateWindow(TEXT("CRPC_SRTD"), TEXT("sorted lists"), WS_CHILD | WS_CLIPCHILDREN,
		0, 0, pRect.right, pRect.bottom - pAi->vOffs, pAi->hApp, NULL, pAi->hInst, NULL);

	w = (pRect.right >> 1) / 3;
	x = 0;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, 0, w, pAi->vOffs, hList, (HMENU)IDC_SORTARTIST, pAi->hInst, NULL);
	AssignIcon(pAi->hInst, hList, IDC_SORTARTIST, IDI_ARTIST);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, 0, w, pAi->vOffs, hList, (HMENU)IDC_SORTALBUM, pAi->hInst, NULL);
	AssignIcon(pAi->hInst, hList, IDC_SORTALBUM, IDI_LST);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, 0, (pRect.right >> 1) - x, pAi->vOffs, hList, (HMENU)IDC_SORTTITLE, pAi->hInst, NULL);
	AssignIcon(pAi->hInst, hList, IDC_SORTTITLE, IDI_RAD);

	CreateWindow(TEXT("WB_LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_SORT | LBS_HASSTRINGS, 0, pAi->vOffs,
		pRect.right >> 1, pRect.bottom - 2 * pAi->vOffs, hList, (HMENU)IDC_SORTLIST, pAi->hInst,  (LPVOID)(2 * pAi->vOffs / 3));
	SendDlgItemMessage(hList, IDC_SORTLIST, LB_SETITEMHEIGHT, 0, (LPARAM)24);

	w = (pRect.right >> 1) / 4;
	x = pRect.right >> 1;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, 0, w, pAi->vOffs, hList, (HMENU)IDC_SORTSHOW, pAi->hInst, NULL);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, 0, w, pAi->vOffs, hList, (HMENU)IDC_SORTPLAYNOW, pAi->hInst, NULL);
	AssignIcon(pAi->hInst, hList, IDC_SORTPLAYNOW, IDI_PLAYNOW);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, 0, w, pAi->vOffs, hList, (HMENU)IDC_SORTPLAYNEXT, pAi->hInst, NULL);
	AssignIcon(pAi->hInst, hList, IDC_SORTPLAYNEXT, IDI_PLAYNEXT);
	x += w;
	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, x, 0, pRect.right - x, pAi->vOffs, hList, (HMENU)IDC_SORTPLAYQUEUE, pAi->hInst, NULL);
	AssignIcon(pAi->hInst, hList, IDC_SORTPLAYQUEUE, IDI_PLAYQUEUE);

	CreateWindow(TEXT("WB_LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS, pRect.right >> 1, pAi->vOffs,
		pRect.right >> 1, pRect.bottom - 2 * pAi->vOffs, hList, (HMENU)IDC_SORTRESULT, pAi->hInst, (LPVOID)(2 * pAi->vOffs / 3));
	InitSortListBox(TRUE, GetDlgItem(hList, IDC_SORTRESULT));
	SendDlgItemMessage(hList, IDC_SORTRESULT, LB_SETITEMHEIGHT, 0, (LPARAM)24);
}

Sorted::~Sorted()
{
	InitSortListBox(FALSE, NULL);
	delete font;
	delete pn;
	delete bkGnd;
	DestroyWindow(hList);
}

Sorted::Show(BOOL yesNo)
{
	if(yesNo)
	{
		ShowWindow(hList, SW_SHOW);
		if((SendDlgItemMessage(hList, IDC_SORTARTIST, BM_GETCHECK, 0, 0) != BST_CHECKED) &&
				(SendDlgItemMessage(hList, IDC_SORTALBUM, BM_GETCHECK, 0, 0) != BST_CHECKED) &&
				(SendDlgItemMessage(hList, IDC_SORTTITLE, BM_GETCHECK, 0, 0) != BST_CHECKED))
		{
			SendMessage(hList, WM_COMMAND, MAKEWPARAM(IDC_SORTARTIST, BN_CLICKED), 0);
			SendDlgItemMessage(hList, IDC_SORTPLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
			SendDlgItemMessage(hList, IDC_SORTPLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
			SendDlgItemMessage(hList, IDC_SORTPLAYQUEUE, BM_SETCHECK, BST_INDETERMINATE, 0);
		}
		vNextIndex = 0;
	}
	else
		ShowWindow(hList, SW_HIDE);
}

ATOM Sorted::MyRegisterClass()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= SortedWinProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= pAi->hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_SRTD");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

void Sorted::InitDC(BOOL updateDC)
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

void Sorted::Paint()
{
	if(myGraphics == NULL)
		return;

	myGraphics->FillRectangle(bkGnd, 0, 0, ourRect.right, ourRect.bottom);
}

LRESULT CALLBACK SortedWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT	ps;
    RECT		ourRect;
	LONG		idCurrent;
	LPWSTR		pText;
	PMediaDescr pSelect, pMedia;
	INT         x, w;
	LPWSTR      szText;
	INT         vLen;
	HICON       ico;

	GetClientRect(hWnd, &ourRect);

	switch (message)
	{
		case WM_CREATE:
			my->hCbWnd = hWnd;
			my->InitDC(FALSE);
			SendMessage(my->pAi->hApp, WM_SETREGISTERFINALWND, (WPARAM)IDW_SRT, (LPARAM)hWnd);
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			my->Paint();
			EndPaint(hWnd, &ps);
			break;

		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					switch(LOWORD(wParam))
					{
						case IDC_SORTARTIST:
							if(SendDlgItemMessage(hWnd, IDC_SORTARTIST, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
							{
								SendDlgItemMessage(hWnd, IDC_SORTALBUM, BM_SETCHECK, BST_UNCHECKED, 0);
								SendDlgItemMessage(hWnd, IDC_SORTTITLE, BM_SETCHECK, BST_UNCHECKED, 0);
								SendDlgItemMessage(hWnd, IDC_SORTARTIST, BM_SETCHECK, BST_CHECKED, 0);
								// Clear existing data in listBox & Add current list
								SendDlgItemMessage(hWnd, IDC_SORTLIST, LB_RESETCONTENT, NULL, NULL);
								pSelect = (PMediaDescr)GlobalAlloc(GPTR, sizeof(MediaDescr));
								if(pSelect)
								{
									HCURSOR vOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));  
									pSelect->vAux = FLD_ARTIST;
									my->pAi->hDb->TableToListbox(pSelect, GetDlgItem(hWnd, IDC_SORTLIST));
									GlobalFree((HGLOBAL)pSelect);
									SetCursor(vOldCur);  
								}
								/* Set first line of listbox active and simulate click */
								SendDlgItemMessage(hWnd, IDC_SORTLIST, LB_SETCURSEL, 0, 0);
								SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_SORTLIST, LBN_SELCHANGE), 0);
							}
							break;
						case IDC_SORTALBUM:
							if(SendDlgItemMessage(hWnd, IDC_SORTALBUM, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
							{
								SendDlgItemMessage(hWnd, IDC_SORTARTIST, BM_SETCHECK, BST_UNCHECKED, 0);
								SendDlgItemMessage(hWnd, IDC_SORTTITLE, BM_SETCHECK, BST_UNCHECKED, 0);
								SendDlgItemMessage(hWnd, IDC_SORTALBUM, BM_SETCHECK, BST_CHECKED, 0);
								// Clear existing data in listBox & Add current list
								SendDlgItemMessage(hWnd, IDC_SORTLIST, LB_RESETCONTENT, NULL, NULL);
								pSelect = (PMediaDescr)GlobalAlloc(GPTR, sizeof(MediaDescr));
								if(pSelect)
								{
									HCURSOR vOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));  
									pSelect->vAux = FLD_ALBUM;
									pSelect->vArtistId = 0;
									my->pAi->hDb->TableToListbox(pSelect, GetDlgItem(hWnd, IDC_SORTLIST));
									GlobalFree((HGLOBAL)pSelect);
									SetCursor(vOldCur);  
								}
								/* Set first line of listbox active and simulate click */
								SendDlgItemMessage(hWnd, IDC_SORTLIST, LB_SETCURSEL, 0, 0);
								SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_SORTLIST, LBN_SELCHANGE), 0);
							}
							break;
						case IDC_SORTTITLE:
							if(SendDlgItemMessage(hWnd, IDC_SORTTITLE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
							{
								SendDlgItemMessage(hWnd, IDC_SORTARTIST, BM_SETCHECK, BST_UNCHECKED, 0);
								SendDlgItemMessage(hWnd, IDC_SORTALBUM, BM_SETCHECK, BST_UNCHECKED, 0);
								SendDlgItemMessage(hWnd, IDC_SORTTITLE, BM_SETCHECK, BST_CHECKED, 0);
								// Clear existing data in listBox & Add current list
								SendDlgItemMessage(hWnd, IDC_SORTLIST, LB_RESETCONTENT, NULL, NULL);
								pSelect = (PMediaDescr)GlobalAlloc(GPTR, sizeof(MediaDescr));
								if(pSelect)
								{
									HCURSOR vOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));  
									pSelect->vAux = FLD_TITLE;
									my->pAi->hDb->TableToListbox(pSelect, GetDlgItem(hWnd, IDC_SORTLIST));
									GlobalFree((HGLOBAL)pSelect);
									SetCursor(vOldCur);  
								}
								/* Set first line of listbox active and simulate click */
								SendDlgItemMessage(hWnd, IDC_SORTLIST, LB_SETCURSEL, 0, 0);
								SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_SORTLIST, LBN_SELCHANGE), 0);
							}
							break;

						case IDC_SORTPLAYNOW:
							idCurrent = SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETCURSEL, 0, 0);
							if(idCurrent != LB_ERR)
							{
								pSelect = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETITEMDATA, (WPARAM)idCurrent, 0);
								if(pSelect && ((LRESULT)pSelect != LB_ERR))
								{
									pMedia = my->pAi->hDb->CopyDescr(pSelect);
									if(pMedia)
									{
										SendMessage(my->pAi->hApp, WM_PLAYNOW, 0, (LPARAM)pMedia);
										SendDlgItemMessage(hWnd, IDC_SORTPLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
									}
								}
							}
							break;
						case IDC_SORTPLAYNEXT:
							idCurrent = SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETCURSEL, 0, 0);
							if(idCurrent != LB_ERR)
							{
								pSelect = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETITEMDATA, (WPARAM)idCurrent, 0);
								if(pSelect && ((LRESULT)pSelect != LB_ERR))
								{
									pMedia = my->pAi->hDb->CopyDescr(pSelect);
									if(pMedia)
									{
										SendMessage(my->pAi->hApp, WM_PLAYNEXT, (WPARAM)my->vNextIndex++, (LPARAM)pMedia);
										SendDlgItemMessage(hWnd, IDC_SORTPLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
									}
								}
							}
							break;
						case IDC_SORTPLAYQUEUE:
							idCurrent = SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETCURSEL, 0, 0);
							if(idCurrent != LB_ERR)
							{
								pSelect = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETITEMDATA, (WPARAM)idCurrent, 0);
								if(pSelect && ((LRESULT)pSelect != LB_ERR))
								{
									pMedia = my->pAi->hDb->CopyDescr(pSelect);
									if(pMedia)
									{
										SendMessage(my->pAi->hApp, WM_PLAYQUEUE, 0, (LPARAM)pMedia);
										SendDlgItemMessage(hWnd, IDC_SORTPLAYQUEUE, BM_SETCHECK, BST_INDETERMINATE, 0);
									}
								}
							}
							break;
						case IDC_SORTSHOW:
							idCurrent = SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETCURSEL, 0, 0);
							if(idCurrent != LB_ERR)
							{
								/* Get the current item text, so we can find this item back later */
								pText = (LPWSTR)GlobalAlloc(GPTR, 2 * SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETTEXTLEN, (WPARAM)idCurrent, 0) + 2);
								SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETTEXT, (WPARAM)idCurrent, (LPARAM)pText);

								pSelect = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETITEMDATA, (WPARAM)idCurrent, 0);
								if(pSelect && ((LRESULT)pSelect != LB_ERR))
								{
									pMedia = (PMediaDescr)GlobalAlloc(GPTR, sizeof(MediaDescr));
									if(pMedia)
									{
										HCURSOR vOldCur;  

										pMedia->vAlbumId = pSelect->vAlbumId;
										pMedia->vArtistId = pSelect->vArtistId;
										SendDlgItemMessage(hWnd, IDC_SORTARTIST, BM_SETCHECK, BST_UNCHECKED, 0);
										SendDlgItemMessage(hWnd, IDC_SORTTITLE, BM_SETCHECK, BST_UNCHECKED, 0);
										SendDlgItemMessage(hWnd, IDC_SORTALBUM, BM_SETCHECK, BST_UNCHECKED, 0);
										SendDlgItemMessage(hWnd, IDC_SORTLIST, LB_RESETCONTENT, 0, 0);
										switch(pSelect->vAux & FIELD_MASK)
										{
											case FLD_ARTIST:
												SendDlgItemMessage(hWnd, IDC_SORTARTIST, BM_SETCHECK, BST_CHECKED, 0);
												vOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));  
												pMedia->vAux = FLD_ARTIST;
												my->pAi->hDb->TableToListbox(pMedia, GetDlgItem(hWnd, IDC_SORTLIST));
												SetCursor(vOldCur);  
												break;

											case FLD_ALBUM:
												SendDlgItemMessage(hWnd, IDC_SORTALBUM, BM_SETCHECK, BST_CHECKED, 0);
												vOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));  
												pMedia->vAux = FLD_ALBUM;
												my->pAi->hDb->TableToListbox(pMedia, GetDlgItem(hWnd, IDC_SORTLIST));
												SetCursor(vOldCur);  
												break;

											default:
												break;
										}
										GlobalFree((HGLOBAL)pMedia);
									}
								}
								/* Now set selection to selected item using the saved string */
								idCurrent = SendDlgItemMessage(hWnd, IDC_SORTLIST, LB_FINDSTRING, (WPARAM)-1, (LPARAM)pText);
								if(idCurrent != LB_ERR)
								{
									SendDlgItemMessage(hWnd, IDC_SORTLIST, LB_SETCURSEL, (WPARAM)idCurrent, 0);
									SendDlgItemMessage(hWnd, IDC_SORTLIST, LB_SETTOPINDEX, (WPARAM)idCurrent, 0);
									PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_SORTLIST, LBN_SELCHANGE), (LPARAM)idCurrent);
								}
								GlobalFree((HGLOBAL)pText);
							}
							break;
					}
					break;

				/* Listbox events */
				case LBN_SELCHANGE:
					switch(LOWORD(wParam))
					{
						case IDC_SORTLIST:
							SendDlgItemMessage(hWnd, IDC_SORTPLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
							SendDlgItemMessage(hWnd, IDC_SORTPLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
							SendDlgItemMessage(hWnd, IDC_SORTPLAYQUEUE, BM_SETCHECK, BST_INDETERMINATE, 0);
							SendDlgItemMessage(hWnd, IDC_SORTSHOW, BM_SETCHECK, BST_INDETERMINATE, 0);
							SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_RESETCONTENT, NULL, NULL);

							idCurrent = SendDlgItemMessage(hWnd, IDC_SORTLIST, LB_GETCURSEL, 0, 0);
							if(idCurrent == LB_ERR)
								break;
							pSelect = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_SORTLIST, LB_GETITEMDATA, (WPARAM)idCurrent, 0);
							if(pSelect && ((LRESULT)pSelect != LB_ERR))
							{
								pMedia = my->pAi->hDb->CopyDescr(pSelect);
								if(pMedia)
								{
									HCURSOR vOldCur;

									/* Request album list first ... */
									switch(pMedia->vAux & FIELD_MASK)
									{
										case FLD_ARTIST:
											vOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));  
											pMedia->vAux = DISPLAY_ITEM | FLD_ALBUM;
											my->pAi->hDb->TableToListbox(pMedia, GetDlgItem(hWnd, IDC_SORTRESULT));
											pMedia->vAux = DISPLAY_ITEM | FLD_TITLE;
											pMedia->vInvalid = 0;
											my->pAi->hDb->QueryToListbox(pMedia, GetDlgItem(hWnd, IDC_SORTRESULT));
											SetCursor(vOldCur);  
											break;

										case FLD_ALBUM:
											vOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));  
											pMedia->vAux = DISPLAY_ITEM | FLD_ARTIST;
											my->pAi->hDb->TableToListbox(pMedia, GetDlgItem(hWnd, IDC_SORTRESULT));
											pMedia->vAux = DISPLAY_EXCL | FLD_TITLE | FLD_ARTIST;
											my->pAi->hDb->QueryToListbox(pMedia, GetDlgItem(hWnd, IDC_SORTRESULT));
											SetCursor(vOldCur);  
											break;

										case FLD_TITLE:
											vOldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));  
											pMedia->vAux = DISPLAY_ITEM | FLD_ARTIST;
											my->pAi->hDb->TableToListbox(pMedia, GetDlgItem(hWnd, IDC_SORTRESULT));
											pMedia->vAux = DISPLAY_ITEM | FLD_ALBUM;
											my->pAi->hDb->TableToListbox(pMedia, GetDlgItem(hWnd, IDC_SORTRESULT));
											pMedia->vAux = DISPLAY_ITEM | FLD_FILE;
											my->pAi->hDb->QueryToListbox(pMedia, GetDlgItem(hWnd, IDC_SORTRESULT));
											SetCursor(vOldCur);  
											break;

										default: break;
									}
									GlobalFree((HGLOBAL)pMedia);
								}
							}
							break;
						case IDC_SORTRESULT:
							/* Change the play buttons */
							idCurrent = SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETCURSEL, 0, 0);
							if(idCurrent != LB_ERR)
							{
								pSelect = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETITEMDATA, (WPARAM)idCurrent, 0);
								if(pSelect && ((LRESULT)pSelect != LB_ERR))
								{
									/* Set appropriate icons for jump selection button */
									if(pSelect->vAux & FLD_TITLE)
									{
										switch(pSelect->vTypeId)
										{
											default:		ico = LoadIcon(my->pAi->hInst, (LPCTSTR)IDI_INFO);		break;
											case FT_MP3:	ico = LoadIcon(my->pAi->hInst, (LPCTSTR)IDI_RAD);		break;
											case FT_VIDEO:	ico = LoadIcon(my->pAi->hInst, (LPCTSTR)IDI_VID);		break;
										}
										SendDlgItemMessage(hWnd, IDC_SORTSHOW, BM_SETCHECK, BST_INDETERMINATE, 0);
										SendDlgItemMessage(hWnd, IDC_SORTSHOW, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
										DestroyIcon(ico);
									}
									else
									{
										switch(pSelect->vAux & FIELD_MASK)
										{
											case FLD_ALBUM:
												ico = LoadIcon(my->pAi->hInst, (LPCTSTR)IDI_LST);
												SendDlgItemMessage(hWnd, IDC_SORTSHOW, BM_SETCHECK, BST_UNCHECKED, 0);
												SendDlgItemMessage(hWnd, IDC_SORTSHOW, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
												DestroyIcon(ico);
												break;

											case FLD_ARTIST:
												ico = LoadIcon(my->pAi->hInst, (LPCTSTR)IDI_ARTIST);
												SendDlgItemMessage(hWnd, IDC_SORTSHOW, BM_SETCHECK, BST_UNCHECKED, 0);
												SendDlgItemMessage(hWnd, IDC_SORTSHOW, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
												DestroyIcon(ico);
												break;

											case FLD_FILE:
												ico = LoadIcon(my->pAi->hInst, (LPCTSTR)IDI_FILE);
												SendDlgItemMessage(hWnd, IDC_SORTSHOW, BM_SETCHECK, BST_INDETERMINATE, 0);
												SendDlgItemMessage(hWnd, IDC_SORTSHOW, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
												DestroyIcon(ico);
												break;

											default:
												SendDlgItemMessage(hWnd, IDC_SORTSHOW, BM_SETCHECK, BST_INDETERMINATE, 0);
												SendDlgItemMessage(hWnd, IDC_SORTSHOW, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)NULL);
												break;
										}
									}
									/* Activate appropriate buttons depending on current selection */
									if((pSelect->vAux & FLD_TITLE) || (pSelect->vAux & FLD_ALBUM))
									{
										// Enable the show and play buttons only when an track entry is selected
										SendDlgItemMessage(hWnd, IDC_SORTPLAYNOW, BM_SETCHECK, BST_UNCHECKED, 0);
										SendDlgItemMessage(hWnd, IDC_SORTPLAYNEXT, BM_SETCHECK, BST_UNCHECKED, 0);
										SendDlgItemMessage(hWnd, IDC_SORTPLAYQUEUE, BM_SETCHECK, BST_UNCHECKED, 0);
									}
									else
									{
										SendDlgItemMessage(hWnd, IDC_SORTPLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
										SendDlgItemMessage(hWnd, IDC_SORTPLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
										SendDlgItemMessage(hWnd, IDC_SORTPLAYQUEUE, BM_SETCHECK, BST_INDETERMINATE, 0);
									}
								}
							}
							else
							{
								SendDlgItemMessage(hWnd, IDC_SORTPLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
								SendDlgItemMessage(hWnd, IDC_SORTPLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
								SendDlgItemMessage(hWnd, IDC_SORTPLAYQUEUE, BM_SETCHECK, BST_INDETERMINATE, 0);
							}
							break;
					}
					break;
				case LBN_DBLCLK:
					switch(LOWORD(wParam))
					{
						case IDC_SORTRESULT:
							/*Play the selected file */
							idCurrent = SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETCURSEL, 0, 0);
							if(idCurrent != LB_ERR)
							{
								vLen = (INT)SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETTEXTLEN, (WPARAM)idCurrent, 0);
								szText = (LPTSTR)GlobalAlloc(GPTR, ++vLen * sizeof(TCHAR));
								if(szText)
								{
									SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETTEXT, (WPARAM)idCurrent, (LPARAM)szText);
									pSelect = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_SORTRESULT, LB_GETITEMDATA, (WPARAM)idCurrent, 0);
								//	pMedia = my->pAi->hDb->CopyDescr(pSelect);
								//	SendMessage(my->pAi->hApp, WM_PLAYNOW, (WPARAM)szText, (LPARAM)pMedia);
								}
								GlobalFree(szText);
								SendDlgItemMessage(hWnd, IDC_SORTPLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
							}
							break;
					}
					break;
			}
			break;

		case WM_SIZE:
			w = (ourRect.right >> 1) / 3;
			x = 0;
			SetWindowPos(GetDlgItem(hWnd, IDC_SORTARTIST), 0, x, 0, w, my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);
			x += w;
			SetWindowPos(GetDlgItem(hWnd, IDC_SORTALBUM), 0, x, 0, w, my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);
			x += w;
			SetWindowPos(GetDlgItem(hWnd, IDC_SORTTITLE), 0, x, 0, (ourRect.right >> 1) - x, my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);

			SetWindowPos(GetDlgItem(hWnd, IDC_SORTLIST), NULL, 0, my->pAi->vOffs, ourRect.right >> 1, ourRect.bottom - my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);

			w = (ourRect.right >> 1) / 4;
			x = ourRect.right >> 1;
			SetWindowPos(GetDlgItem(hWnd, IDC_SORTSHOW), 0, x, 0, w, my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);
			x += w;
			SetWindowPos(GetDlgItem(hWnd, IDC_SORTPLAYNOW), 0, x, 0, w, my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);
			x += w;
			SetWindowPos(GetDlgItem(hWnd, IDC_SORTPLAYNEXT), 0, x, 0, w, my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);
			x += w;
			SetWindowPos(GetDlgItem(hWnd, IDC_SORTPLAYQUEUE), 0, x, 0, ourRect.right - x, my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);

			SetWindowPos(GetDlgItem(hWnd, IDC_SORTRESULT), NULL, ourRect.right >> 1, my->pAi->vOffs, ourRect.right >> 1, ourRect.bottom - my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);
			InitSortListBox(FALSE, GetDlgItem(hWnd, IDC_SORTRESULT));

			my->InitDC(TRUE);
			my->Paint();
			break;

		case WM_VSCROLL:
			return(0);

		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
			break;

		case WM_LBUTTONUP:
			ReleaseCapture();
			break;

        case WM_DRAWITEM:
			switch(((LPDRAWITEMSTRUCT)lParam)->CtlID)
			{
				case IDC_SORTRESULT:
					DrawSortListBoxItem((LPDRAWITEMSTRUCT)lParam);
					break;
			}
			break;

		case WM_PAINTOWNERDRAWN:
			DrawSortListBox((HWND)wParam);
			break;

		case WM_DELETEITEM:
			switch(((PDELETEITEMSTRUCT)lParam)->CtlID)
			{
				case IDC_SORTLIST:
				case IDC_SORTRESULT:
					pSelect = (PMediaDescr)((PDELETEITEMSTRUCT)lParam)->itemData;
					if(pSelect && ((LRESULT)pSelect != LB_ERR))
						GlobalFree((HGLOBAL)pSelect);
					return(TRUE);
			}
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}


static Graphics * gr;
static HDC hDc;
static Brush    * br_black = NULL;
static Brush    * br_white = NULL;
static Brush    * br_blue = NULL;
static Font     * fn = NULL;

void DrawSortListBox(HWND hWnd)
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
			if(i % 2)
				gr->FillRectangle(br_blue, vListBox.left, vTop, vListBox.right, vItem.bottom);
			else
			{
				gr->FillRectangle(br_blue, vListBox.left, vTop, vItem.bottom, vItem.bottom);
				gr->FillRectangle(br_white, vListBox.left + vItem.bottom, vTop, vListBox.right - vItem.bottom, vItem.bottom);
			}
		}
	}
}

void DrawSortListBoxItem(LPDRAWITEMSTRUCT pItem)
{
	RectF        rf;
	RECT         crect;
	INT          i, n, size;
	StringFormat fmt;
	HICON        hIco;
	HDC          hDC;
	LPWSTR       pText, pString;
	LONG         top;

	PMediaDescr  pMedia = (PMediaDescr)pItem->itemData;
	int          offset = my->pAi->vOffs;

	pItem->rcItem.right -= pItem->rcItem.left;
	pItem->rcItem.bottom -= pItem->rcItem.top;

	rf.X = (float)pItem->rcItem.left;
	rf.Y = (float)pItem->rcItem.top + 3;
	rf.Width = (float)pItem->rcItem.right;
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
			if(i % 2)
				gr->FillRectangle(br_blue, pItem->rcItem.left, top, pItem->rcItem.right, pItem->rcItem.bottom);
			else
			{
				gr->FillRectangle(br_blue, pItem->rcItem.left, top, pItem->rcItem.bottom, pItem->rcItem.bottom);
				gr->FillRectangle(br_white, pItem->rcItem.left + pItem->rcItem.bottom, top, pItem->rcItem.right - pItem->rcItem.bottom, pItem->rcItem.bottom);
			}
		}
	}

	size = 2 * SendMessage(pItem->hwndItem, LB_GETTEXTLEN, (WPARAM)pItem->itemID, 0) + 2;
	pText = (LPWSTR)GlobalAlloc(GPTR, size);
	SendMessage(pItem->hwndItem, LB_GETTEXT, (WPARAM)pItem->itemID, (LPARAM)pText);

	// Strip directory from filename string
	pString = pText;
	if(pMedia != NULL)
	{
		if((pMedia->vAux & (FIELD_MASK)) == FLD_FILE)
		{
			pString = wcsrchr(pText, '\\');
			if(pString)
				pString++;
			else
				pString = pText;
		}
	}

	fmt.SetAlignment(StringAlignmentNear);
	fmt.SetTrimming(StringTrimmingNone);
	fmt.SetLineAlignment(StringAlignmentCenter);
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);
	if(pItem->itemState & ODS_SELECTED)
	{
   		gr->FillRectangle(br_black, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.right, pItem->rcItem.bottom);
		rf.X += pItem->rcItem.bottom;
		rf.Width -= pItem->rcItem.bottom;
		gr->DrawString(pString, -1, fn, rf, &fmt, br_white);
	}
	else if(n != 0)
	{
		if(pItem->itemID % 2)
			gr->FillRectangle(br_blue, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.right, pItem->rcItem.bottom);
		else
		{
			gr->FillRectangle(br_blue, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.bottom, pItem->rcItem.bottom);
			gr->FillRectangle(br_white, pItem->rcItem.left + pItem->rcItem.bottom, pItem->rcItem.top, pItem->rcItem.right - pItem->rcItem.bottom, pItem->rcItem.bottom);
		}

		// Item text
		rf.X += pItem->rcItem.bottom;
		rf.Width -= pItem->rcItem.bottom;
		gr->DrawString(pString, -1, fn, rf, &fmt, br_black);
	}
	// Item icon
	if((n != -1) && (pItem->itemData != NULL))
	{
		hIco = NULL;
		if(pMedia->vAux & FLD_TITLE)
		{
			switch(((PMediaDescr)pItem->itemData)->vTypeId)
			{
				default:		hIco = LoadIcon(my->pAi->hInst, (LPCTSTR)IDI_INFO);		break;
				case FT_MP3:	hIco = LoadIcon(my->pAi->hInst, (LPCTSTR)IDI_RAD);		break;
				case FT_VIDEO:	hIco = LoadIcon(my->pAi->hInst, (LPCTSTR)IDI_VID);		break;
			}
		}
		else
		{
			switch(pMedia->vAux & (FIELD_MASK))
			{
				case FLD_ALBUM:		hIco = LoadIcon(my->pAi->hInst, (LPCTSTR)IDI_LST);		break;
				case FLD_ARTIST:	hIco = LoadIcon(my->pAi->hInst, (LPCTSTR)IDI_ARTIST);	break;
				case FLD_FILE:		hIco = LoadIcon(my->pAi->hInst, (LPCTSTR)IDI_FILE);		break;
				default:	break;
			}
		}
		if(hIco != NULL)
		{
			hDC = GetDC(pItem->hwndItem);
			DrawIconEx(hDC, pItem->rcItem.left + 4, pItem->rcItem.top + 4, hIco, 16, 16, NULL, NULL, DI_NORMAL);
			DestroyIcon(hIco);
			ReleaseDC(pItem->hwndItem, hDC);
		}
		else if(pMedia->vAlbumId && (pMedia->vTrack != -1))
		{
			/* Draw track number */
		}
	}
	GlobalFree((HGLOBAL)pText);
}

void InitSortListBox(BOOL loadUnload, HWND lbox)
{
	if(loadUnload)
	{
		hDc = GetDC(lbox);
		gr = new Graphics(hDc, FALSE);
		if(!br_black) br_black = new SolidBrush(Color(255, 0, 0, 0));
		if(!br_white) br_white = new SolidBrush(Color(255, 255, 255, 255));
		if(!br_blue) br_blue = new SolidBrush(Color(255, 201, 211, 227));
		if(!fn) fn = new Font(L"Arial", 16);
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

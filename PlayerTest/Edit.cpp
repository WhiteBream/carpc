// Edit.cpp: implementation of the MediaDB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Edit.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

int MediaListDBif(void * cbParam, int colCount, char ** colData, char ** colNames);
LRESULT CALLBACK EditWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static void DrawSortListBox(HWND hWnd);
static void DrawListBoxItem(LPDRAWITEMSTRUCT pItem);
static void InitListBox(BOOL loadUnload, HWND lbox);

static Edit * my;
static HWND hOsk;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Edit::Edit(PAppInfo pAppInfo)
{
	RECT pRect;
	HICON ico;
	HFONT hArial;
	LOGFONT lf;

	my = this;
	pAi = pAppInfo;
	myGraphics = NULL;

	MyRegisterClass();
	GetClientRect(pAi->hApp, &pRect);

	bkGnd = new SolidBrush(Color(255, 200, 255, 200));
	font = new Font(L"Arial", 16);
	pn = new Pen(Color(255, 0, 0, 0), 1);

	/* Top window for media database */
	hEdit = CreateWindow(TEXT("CRPC_EDIT"), NULL, WS_CHILD | WS_CLIPCHILDREN, 0, 0, pRect.right, pRect.bottom - pAi->vOffs, pAi->hApp, NULL, pAi->hInst, NULL);

	CreateWindow(TEXT("EDIT"), NULL, WS_CHILD | WS_VISIBLE, pAi->vOffs + 1, 1, pRect.right - 5 * pAi->vOffs - 2, pAi->vOffs - 2, hEdit, (HMENU)IDC_OSK, pAi->hInst, NULL);
	/* Set edit font */
	memset(&lf, 0, sizeof(lf));
	lf.lfHeight = 24;
	lf.lfWeight = 500;
	StringCbCopyW(lf.lfFaceName, sizeof(lf.lfFaceName), (LPCWSTR)L"Arial");
	hArial = CreateFontIndirect(&lf);
	SendMessage(GetDlgItem(hEdit, IDC_OSK), WM_SETFONT, (WPARAM)hArial, (LPARAM)TRUE);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, 0, 0, pAi->vOffs, pAi->vOffs, hEdit, (HMENU)IDC_MDBOSK, pAi->hInst, NULL);
	ico = LoadIcon(pAi->hInst, (LPCTSTR)IDI_OSK);
	SendDlgItemMessage(hEdit, IDC_MDBOSK, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
	DestroyIcon(ico);
	vOsk = pAi->hDb->GetBool("OnScreenKeyboard");
	SendDlgItemMessage(hEdit, IDC_MDBOSK, BM_SETCHECK, (vOsk ? BST_CHECKED : BST_UNCHECKED), 0);

	CreateWindow(WBBUTTON, TEXT("A"), WS_CHILD | WS_VISIBLE, pRect.right - 4 * pAi->vOffs, 0, pAi->vOffs, pAi->vOffs, hEdit, (HMENU)IDC_SORTSELALL, pAi->hInst, NULL);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 3 * pAi->vOffs, 0, pAi->vOffs, pAi->vOffs, hEdit, (HMENU)IDC_SORTPLAYNOW, pAi->hInst, NULL);
	ico = LoadIcon(pAi->hInst, (LPCTSTR)IDI_PLAYNOW);
	SendDlgItemMessage(hEdit, IDC_SORTPLAYNOW, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
	DestroyIcon(ico);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 2 * pAi->vOffs, 0, pAi->vOffs, pAi->vOffs, hEdit, (HMENU)IDC_SORTPLAYNEXT, pAi->hInst, NULL);
	ico = LoadIcon(pAi->hInst, (LPCTSTR)IDI_PLAYNEXT);
	SendDlgItemMessage(hEdit, IDC_SORTPLAYNEXT, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
	DestroyIcon(ico);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 1 * pAi->vOffs, 0, pAi->vOffs, pAi->vOffs, hEdit, (HMENU)IDC_SORTPLAYQUEUE, pAi->hInst, NULL);
	ico = LoadIcon(pAi->hInst, (LPCTSTR)IDI_PLAYQUEUE);
	SendDlgItemMessage(hEdit, IDC_SORTPLAYQUEUE, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
	DestroyIcon(ico);

	if(vOsk)
		CreateWindow(TEXT("WB_LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | LBS_SORT | LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS,
					0, pAi->vOffs, pRect.right, pRect.bottom - 5 * pAi->vOffs, hEdit, (HMENU)IDC_MDBLIST, pAi->hInst, (LPVOID)(2 * pAi->vOffs / 3));
	else
		CreateWindow(TEXT("WB_LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | LBS_SORT | LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS,
					0, pAi->vOffs, pRect.right, pRect.bottom - 2 * pAi->vOffs, hEdit, (HMENU)IDC_MDBLIST, pAi->hInst, (LPVOID)(2 * pAi->vOffs / 3));
	InitListBox(TRUE, GetDlgItem(hEdit, IDC_MDBLIST));
	SendDlgItemMessage(hEdit, IDC_MDBLIST, LB_SETITEMHEIGHT, 0, (LPARAM)24);

	SendDlgItemMessage(hEdit, IDC_SORTSELALL, BM_SETCHECK, BST_INDETERMINATE, 0);
	SendDlgItemMessage(hEdit, IDC_SORTPLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
	SendDlgItemMessage(hEdit, IDC_SORTPLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
	SendDlgItemMessage(hEdit, IDC_SORTPLAYQUEUE, BM_SETCHECK, BST_INDETERMINATE, 0);

	hOsk = CreateWindow(TEXT("WB_KEYBOARD"), NULL, WS_CHILD, 0, pRect.bottom - 4 * pAi->vOffs,	pRect.right, 3 * pAi->vOffs, hEdit, (HMENU)0, pAi->hInst, NULL);
	SendMessage(hOsk, WM_SETOSKTARGET, 0, (LPARAM)GetDlgItem(hEdit, IDC_OSK));
	if(vOsk)
		ShowWindow(hOsk, SW_SHOW);
}

Edit::~Edit()
{
	if(pn) delete pn;
	if(font) delete font;
	if(bkGnd) delete bkGnd;
	if(myGraphics) delete myGraphics;
	DestroyWindow(GetDlgItem(hEdit, IDC_OSK));
	DestroyWindow(hOsk);
	DestroyWindow(hEdit);
}

Edit::Show(BOOL yesNo)
{
	TCHAR bText[31];

	if(yesNo)
	{
		ShowWindow(hEdit, SW_SHOW);
		SendDlgItemMessage(hEdit, IDC_MDBLIST, WM_ACTIVATE, WA_CLICKACTIVE, (LPARAM)hEdit);
		SetActiveWindow(GetDlgItem(hEdit, IDC_OSK));

		if(SendMessage(GetDlgItem(hEdit, IDC_MDBLIST), LB_GETCOUNT, 0, 0) == 0)
		{
			memset(bText, 0, sizeof(bText));
			GetWindowText(GetDlgItem(hEdit, IDC_OSK), bText, (sizeof(bText) - 1) / sizeof(TCHAR));
			pAi->hDb->MediaDbQuery(bText, GetDlgItem(hEdit, IDC_MDBLIST));
		}
	}
	else
		ShowWindow(hEdit, SW_HIDE);
}

ATOM Edit::MyRegisterClass(void)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= EditWinProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= pAi->hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_EDIT");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

void Edit::InitDC(BOOL updateDC)
{
	if(updateDC)
	   delete myGraphics;

	if(updateDC || (myGraphics == NULL))
	   myGraphics = new Graphics(hCbWnd, FALSE);

	GetClientRect(hCbWnd, &ourRect);
}

void Edit::Paint()
{
	if(myGraphics == NULL)
		return;

	myGraphics->FillRectangle(bkGnd, 0, 0, ourRect.right, ourRect.bottom);

	// Draw the rectangle around the edit box
	myGraphics->DrawRectangle(pn, pAi->vOffs, 0, ourRect.right - 5 * pAi->vOffs - 1, pAi->vOffs - 1);
}

LRESULT CALLBACK EditWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
    RECT        ourRect;
	WCHAR       bText[31];
	LONG        idCnt, idCurrent;
	LPWSTR      szText;
	PMediaDescr pQuery, pMedia;
	INT         vLen;

	GetClientRect(hWnd, &ourRect);

	switch (message)
	{
		case WM_CREATE:
			my->hCbWnd = hWnd;
			my->InitDC(FALSE);
			SendMessage(my->pAi->hApp, WM_SETREGISTERFINALWND, (WPARAM)IDW_MDB, (LPARAM)hWnd);
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			my->Paint();
			EndPaint(hWnd, &ps);
			SetFocus(GetDlgItem(hWnd, IDC_OSK));
			break;

		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					switch(LOWORD(wParam))
					{
						case IDC_SORTSELALL:
							idCnt = SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETCOUNT, 0, 0);
							if(idCnt == LB_ERR)
								break;
							for(idCurrent = 0; idCurrent < idCnt; idCurrent++)
							{
								vLen = (INT)SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETTEXTLEN, (WPARAM)idCurrent, 0);
								szText = (LPTSTR)GlobalAlloc(GPTR, ++vLen * sizeof(TCHAR));
								if(szText)
								{
									SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETTEXT, (WPARAM)idCurrent, (LPARAM)szText);
									pQuery = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETITEMDATA, (WPARAM)idCurrent, 0);
									pMedia = my->pAi->hDb->CopyDescr(pQuery);
									SendMessage(my->pAi->hApp, WM_PLAYQUEUE, (WPARAM)szText, (LPARAM)pMedia);
								}
								SendDlgItemMessage(hWnd, IDC_SORTSELALL, BM_SETCHECK, BST_INDETERMINATE, 0);
								SendDlgItemMessage(hWnd, IDC_SORTPLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
								SendDlgItemMessage(hWnd, IDC_SORTPLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
								SendDlgItemMessage(hWnd, IDC_SORTPLAYQUEUE, BM_SETCHECK, BST_INDETERMINATE, 0);
							}
							break;

						case IDC_SORTPLAYNOW:
							idCurrent = SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETCURSEL, 0, 0);
							if(idCurrent != LB_ERR)
							{
								vLen = (INT)SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETTEXTLEN, (WPARAM)idCurrent, 0);
								szText = (LPTSTR)GlobalAlloc(GPTR, ++vLen * sizeof(TCHAR));
								if(szText)
								{
									SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETTEXT, (WPARAM)idCurrent, (LPARAM)szText);
									pQuery = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETITEMDATA, (WPARAM)idCurrent, 0);
									pMedia = my->pAi->hDb->CopyDescr(pQuery);
									SendMessage(my->pAi->hApp, WM_PLAYNOW, (WPARAM)szText, (LPARAM)pMedia);
								}
								GlobalFree(szText);
								SendDlgItemMessage(hWnd, IDC_SORTPLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
							}
							break;
						case IDC_SORTPLAYNEXT:
							idCurrent = SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETCURSEL, 0, 0);
							if(idCurrent != LB_ERR)
							{
								vLen = (INT)SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETTEXTLEN, (WPARAM)idCurrent, 0);
								szText = (LPTSTR)GlobalAlloc(GPTR, ++vLen * sizeof(TCHAR));
								if(szText)
								{
									SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETTEXT, (WPARAM)idCurrent, (LPARAM)szText);
									pQuery = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETITEMDATA, (WPARAM)idCurrent, 0);
									pMedia = my->pAi->hDb->CopyDescr(pQuery);
									SendMessage(my->pAi->hApp, WM_PLAYNEXT, (WPARAM)szText, (LPARAM)pMedia);
								}
								GlobalFree(szText);
								SendDlgItemMessage(hWnd, IDC_SORTPLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
							}
							break;
						case IDC_SORTPLAYQUEUE:
							idCurrent = SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETCURSEL, 0, 0);
							if(idCurrent != LB_ERR)
							{
								vLen = (INT)SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETTEXTLEN, (WPARAM)idCurrent, 0);
								szText = (LPTSTR)GlobalAlloc(GPTR, ++vLen * sizeof(TCHAR));
								if(szText)
								{
									SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETTEXT, (WPARAM)idCurrent, (LPARAM)szText);
									pQuery = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETITEMDATA, (WPARAM)idCurrent, 0);
									pMedia = my->pAi->hDb->CopyDescr(pQuery);
									SendMessage(my->pAi->hApp, WM_PLAYQUEUE, (WPARAM)szText, (LPARAM)pMedia);
								}
								GlobalFree(szText);
								SendDlgItemMessage(hWnd, IDC_SORTPLAYQUEUE, BM_SETCHECK, BST_INDETERMINATE, 0);
							}
							break;

						case IDC_MDBOSK:
							if(SendDlgItemMessage(hWnd, IDC_MDBOSK, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
								my->vOsk = TRUE;
							else
								my->vOsk = FALSE;
							my->pAi->hDb->SetConfig("OnScreenKeyboard", (LPVOID)&my->vOsk, CONFIGTYPE_BOOL);
							SendDlgItemMessage(hWnd, IDC_MDBOSK, BM_SETCHECK, (my->vOsk ? BST_CHECKED : BST_UNCHECKED), 0);
							if(my->vOsk)
							{
								SetWindowPos(GetDlgItem(hWnd, IDC_MDBLIST), NULL, 0, my->pAi->vOffs, ourRect.right, ourRect.bottom - (4 * my->pAi->vOffs), SWP_NOACTIVATE | SWP_NOZORDER);
								ShowWindow(hOsk, SW_SHOW);
								SetFocus(hOsk);
							}
							else
							{
								ShowWindow(hOsk, SW_HIDE);
								SetWindowPos(GetDlgItem(hWnd, IDC_MDBLIST), NULL, 0, my->pAi->vOffs, ourRect.right, ourRect.bottom - my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);
							}
							InitListBox(FALSE, GetDlgItem(hWnd, IDC_MDBLIST));
							break;
					}
					break;
				case EN_CHANGE:
					/* From editbox */
					SendDlgItemMessage(hWnd, IDC_SORTPLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
					SendDlgItemMessage(hWnd, IDC_SORTPLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
					SendDlgItemMessage(hWnd, IDC_SORTPLAYQUEUE, BM_SETCHECK, BST_INDETERMINATE, 0);

					SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_RESETCONTENT, NULL, NULL);
					memset(bText, 0, sizeof(bText));
					GetWindowText(GetDlgItem(hWnd, IDC_OSK), bText, (sizeof(bText) - 1) / sizeof(TCHAR));
					my->pAi->hDb->MediaDbQuery(bText, GetDlgItem(hWnd, IDC_MDBLIST));
					SendDlgItemMessage(hWnd, IDC_SORTSELALL, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hWnd, IDC_MDBLIST, WM_PAINT, 0, 0);
					break;

				case LBN_SELCHANGE:
					/* From listbox */
					SendDlgItemMessage(hWnd, IDC_SORTPLAYNOW, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hWnd, IDC_SORTPLAYNEXT, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hWnd, IDC_SORTPLAYQUEUE, BM_SETCHECK, BST_UNCHECKED, 0);
					break;

				case LBN_DBLCLK:
					switch(LOWORD(wParam)) // Get the listbox ID
					{
						case IDC_MDBLIST:
							idCurrent = SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETCURSEL, 0, 0);
							if(idCurrent != LB_ERR)
							{
								/* Put selected item next in now playing list */
					//			vLen = (INT)SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETTEXTLEN, (WPARAM)idCurrent, 0);
					//			szText = (LPTSTR)GlobalAlloc(GPTR, ++vLen * sizeof(TCHAR));
					//			SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETTEXT, (WPARAM)idCurrent, (LPARAM)szText);
					//			pQuery = (PMediaDescr)SendDlgItemMessage(hWnd, IDC_MDBLIST, LB_GETITEMDATA, (WPARAM)idCurrent, 0);
					//			SendMessage(my->hPrnt, WM_PLAYNEXT, (WPARAM)szText, (LPARAM)pQuery);
					//			SendDlgItemMessage(hWnd, IDC_SORTPLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
							}
							break;
					}
					break;
			}
			break;

		case WM_SIZE:
			SetWindowPos(GetDlgItem(hWnd, IDC_MDBOSK), NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_OSK), NULL, my->pAi->vOffs + 1, 1, ourRect.right - 5 * my->pAi->vOffs - 2, my->pAi->vOffs - 2, SWP_NOACTIVATE | SWP_NOZORDER);

			SetWindowPos(GetDlgItem(hWnd, IDC_SORTSELALL), NULL, ourRect.right - 4 * my->pAi->vOffs, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_SORTPLAYNOW), NULL, ourRect.right - 3 * my->pAi->vOffs, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_SORTPLAYNEXT), NULL, ourRect.right - 2 * my->pAi->vOffs, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_SORTPLAYQUEUE), NULL, ourRect.right - 1 * my->pAi->vOffs, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

			if(my->vOsk)
			{
				SetWindowPos(GetDlgItem(hWnd, IDC_MDBLIST), NULL, 0, my->pAi->vOffs, ourRect.right, ourRect.bottom - (4 * my->pAi->vOffs), SWP_NOACTIVATE | SWP_NOZORDER);
				SetWindowPos(hOsk, NULL, 0, ourRect.bottom - 3 * my->pAi->vOffs, ourRect.right, 3 * my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);
			}
			else
				SetWindowPos(GetDlgItem(hWnd, IDC_MDBLIST), NULL, 0, my->pAi->vOffs, ourRect.right, ourRect.bottom - my->pAi->vOffs, SWP_NOACTIVATE | SWP_NOZORDER);
			InitListBox(FALSE, GetDlgItem(hWnd, IDC_MDBLIST));

			my->InitDC(TRUE);
			my->Paint();
			break;

		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
			break;

		case WM_LBUTTONUP:
			ReleaseCapture();
			break;

        case WM_DRAWITEM:
			switch(((LPDRAWITEMSTRUCT)lParam)->CtlID)
			{
				case IDC_MDBLIST:
					DrawListBoxItem((LPDRAWITEMSTRUCT)lParam);
					break;
				case IDC_OSK:
				//	MessageBox(NULL, TEXT("Text control"), TEXT(""), MB_OK);
					break;
			}
			break;

		case WM_PAINTOWNERDRAWN:
			DrawSortListBox((HWND)wParam);
			break;

		case WM_DELETEITEM:
			pQuery = (PMediaDescr)((PDELETEITEMSTRUCT)lParam)->itemData;
			if(pQuery && ((LRESULT)pQuery != LB_ERR))
				GlobalFree((LPVOID)pQuery);
			return(TRUE);

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}


static Graphics *gr;
static Brush    *br_black;
static Brush    *br_white;
static Brush    *br_blue;
static Font     *fn;

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
			gr->FillRectangle(br_blue, vListBox.left, vTop, my->pAi->vOffs, vItem.bottom);
			if(i % 2)
				gr->FillRectangle(br_blue, vListBox.left + my->pAi->vOffs, vTop, vListBox.right - my->pAi->vOffs, vItem.bottom);
			else
				gr->FillRectangle(br_white, vListBox.left + my->pAi->vOffs, vTop, vListBox.right - my->pAi->vOffs, vItem.bottom);
		}
	}
}

static void DrawListBoxItem(LPDRAWITEMSTRUCT pItem)
{
	RectF         rf;
	RECT          crect;
	int           i, n;
	WCHAR         idTxt[12];
	StringFormat  fmt;
	LPWSTR        pText;
	size_t        len;
	Brush        *color = NULL;
	LONG          top;

	int offset = my->pAi->vOffs;
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

	fmt.SetAlignment(StringAlignmentNear);
	fmt.SetTrimming(StringTrimmingNone);
	fmt.SetLineAlignment(StringAlignmentCenter);
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);
	if(pItem->itemState & ODS_SELECTED)
	{
		gr->FillRectangle(br_blue, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.left + offset, pItem->rcItem.bottom);
		gr->FillRectangle(br_black, pItem->rcItem.left + offset, pItem->rcItem.top, pItem->rcItem.right - offset, pItem->rcItem.bottom);
		color = br_white;
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
		color = br_black;
	}
	if(color != NULL)
	{
		len = SendMessage(pItem->hwndItem, LB_GETTEXTLEN, (WPARAM)pItem->itemID, 0)+1;
		pText = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, len * sizeof(TCHAR));
		if(pText)
		{
			SendMessage(pItem->hwndItem, LB_GETTEXT, (WPARAM)pItem->itemID, (LPARAM)pText);
			StringCbPrintfW(idTxt, sizeof(idTxt), L"%u", pItem->itemID + 1);
			rf.X = (float)pItem->rcItem.left;
			rf.Width = (float)offset;
			gr->DrawString(idTxt, -1, fn, rf, &fmt, br_black);
			rf.X += (float)offset;
			rf.Width = (float)pItem->rcItem.right - offset;
			gr->DrawString(pText, len-1, fn, rf, &fmt, color);
			HeapFree(GetProcessHeap(), 0, pText);
		}
	}
}

static void InitListBox(BOOL loadUnload, HWND lbox)
{
	if(loadUnload)
	{
		gr = new Graphics(GetDC(lbox), FALSE);
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
	}
	else
	{
		delete gr;
		gr = new Graphics(GetDC(lbox), FALSE);
	}
}
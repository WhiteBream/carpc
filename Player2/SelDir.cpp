// SelDir.cpp: implementation of the SelDir class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SelDir.h"
#include <stdio.h>
#include <shlobj.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

typedef enum _FOLDERTYPES
{
	NORMALFOLDER,
	MYCOMPFOLDER,
	MYDOCSFODLER,
	DISKDRIVE,
	PARENTFOLDER
} FOLDERTYPES;

typedef struct _SelDirParms
{
	Graphics   * gr;
	HINSTANCE    hInst;
	HWND         hParent;
	RECT         ourRect;
	Brush      * br_black;
	Brush      * br_white;
	Pen        * pn;
	Font       * fn;
	INT          offset;
    TCHAR        szCurDir[MAX_PATH+1];
	HANDLE		 hSearch;
} SelDirParms;

LRESULT CALLBACK SelDirWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void DrawSelDirListBoxItem(LPDRAWITEMSTRUCT pItem);
static void InitSelDirListBox(BOOL loadUnload, HWND lbox);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ATOM RegisterSelDirClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize         = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= SelDirWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(LONG_PTR);
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= CreateSolidBrush(RGB(200, 255, 200));
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_SELDIR");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}


LRESULT CALLBACK SelDirWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPCREATESTRUCT  cs;
	PAINTSTRUCT     ps;
	SelDirParms   * dw;
	size_t          len;
	RECT            pRect;
	HICON           ico;
	WIN32_FIND_DATA findData;
	TCHAR           szTemp[MAX_PATH];
	LPTSTR          extStart;
	LRESULT			id;

	dw = (SelDirParms *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if((dw == NULL) && (message != WM_CREATE))
		return(DefWindowProc(hWnd, message, wParam, lParam));

	GetClientRect(hWnd, &pRect);

	switch (message)
	{
		case WM_CREATE:
			dw = (SelDirParms *)GlobalAlloc(GPTR, sizeof(SelDirParms));
			if(dw == NULL)
				return(1L);	// Error
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)dw);

			cs = (LPCREATESTRUCT)lParam;
			dw->hParent = cs->hwndParent;
			dw->offset = (INT)cs->lpCreateParams;
			
			dw->br_black = new SolidBrush(Color(255, 0, 0, 0));
			dw->br_white = new SolidBrush(Color(255, 255, 255, 255));
			dw->pn = new Pen(Color(255, 0, 0, 0), 1);
			dw->fn = new Font(L"Arial", (float)dw->offset / 4);
			dw->gr = new Graphics(hWnd, FALSE);

			/* Window elements */
			len = 1 + dw->offset / 3;
			CreateWindow(TEXT("WB_LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_HASSTRINGS,
				len, len, pRect.right - 2 * len, pRect.bottom - dw->offset - 2 * len, hWnd, (HMENU)IDC_SEL, cs->hInstance, (LPVOID)(2 * dw->offset / 3));
			InitSelDirListBox(TRUE, GetDlgItem(hWnd, IDC_SEL));
			SendDlgItemMessage(hWnd, IDC_SEL, LB_SETITEMHEIGHT, 0, (LPARAM)24);

			CreateWindow(TEXT("STATIC"), NULL, WS_CHILD, 1, 1, 0, 0, hWnd, (HMENU)IDC_CURDIR, 0, NULL);

			CreateWindow(TEXT("EDIT"), NULL, WS_CHILD | WS_VISIBLE, 1, pRect.bottom - dw->offset + 1, 
				pRect.right - 3 * dw->offset - 2, dw->offset - 2, hWnd, (HMENU)IDC_DIR, cs->hInstance, NULL);

			CreateWindow(WBBUTTON, TEXT("GO"), WS_CHILD | WS_VISIBLE, pRect.right - 3 * dw->offset, pRect.bottom - dw->offset,
				dw->offset, dw->offset, hWnd, (HMENU)IDC_GO, cs->hInstance, NULL);
			SendDlgItemMessage(hWnd, IDC_GO, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

			CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - 2 * dw->offset, pRect.bottom - dw->offset,
				dw->offset, dw->offset, hWnd, (HMENU)IDC_OK, cs->hInstance, NULL);
			ico = LoadIcon(cs->hInstance, (LPCTSTR)IDI_FILE);
			SendDlgItemMessage(hWnd, IDC_OK, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
			DestroyIcon(ico);
			SendDlgItemMessage(hWnd, IDC_OK, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);

			CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, pRect.right - dw->offset, pRect.bottom - dw->offset,
				dw->offset, dw->offset, hWnd, (HMENU)IDC_CANCEL, cs->hInstance, NULL);
			ico = LoadIcon(cs->hInstance, (LPCTSTR)IDI_DEL);
			SendDlgItemMessage(hWnd, IDC_CANCEL, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
			DestroyIcon(ico);

			/* Fill directory list */ 
			if(FAILED(SHGetFolderPath(NULL, CSIDL_MYMUSIC, NULL, SHGFP_TYPE_CURRENT, dw->szCurDir)))
				if(FAILED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, dw->szCurDir)))
					StringCchPrintf(dw->szCurDir, MAX_PATH, TEXT("C:"));
			SetFocus(GetDlgItem(hWnd, IDC_DIR));
			PostMessage(hWnd, WM_FINDFIRSTFILE,	(WPARAM)0, 0);
			break;

		case WM_SETPATH:
			if(lParam)
				if(((LPTSTR)lParam)[0])
				{
					StringCbCopy(dw->szCurDir, sizeof(dw->szCurDir), (LPTSTR)lParam);
					extStart = _tcsrchr(dw->szCurDir, TEXT('\\'));
					if(extStart)
						*extStart = TEXT('\0');
				}
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			len = pRect.bottom;
			/* Border around edit control */
			GetWindowRect(GetDlgItem(hWnd, IDC_DIR), &pRect);
			pRect.right -= pRect.left;
			pRect.bottom -= pRect.top;
			dw->gr->DrawRectangle(dw->pn, 0, len - dw->offset, pRect.right + 1, pRect.bottom + 1);
			EndPaint(hWnd, &ps);
			break;

		case WM_NCDESTROY:
			delete dw->gr;
			delete dw->fn;
			delete dw->br_black;
			delete dw->br_white;
			delete dw->pn;
			GlobalFree(dw);
			InitSelDirListBox(FALSE, NULL);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
			break;

		case WM_FINDFIRSTFILE:
			StringCchLength(dw->szCurDir, MAX_PATH, &len);
			if(dw->szCurDir[0] == TEXT('M'))	// 'M'y Computer, list drives
			{
				if(GetLogicalDriveStrings(sizeof(dw->szCurDir), dw->szCurDir))
				{
					LPTSTR pList;
					for(pList = dw->szCurDir; *pList != 0; pList++)
					{
						id = SendDlgItemMessage(hWnd, IDC_SEL, LB_ADDSTRING, 0, (LPARAM)pList);
						SendDlgItemMessage(hWnd, IDC_SEL, LB_SETITEMDATA, id, (LPARAM)DISKDRIVE);
						do
						{
							pList++;
						} while(*pList);
					}
				}
			}
			else
			{
				if((dw->szCurDir[len-1] == TEXT('\\')) && (dw->szCurDir[len-2] != TEXT(':')))
					dw->szCurDir[--len] = TEXT('\0');
				if(dw->szCurDir[len-1] == TEXT(':'))
				{
					dw->szCurDir[len++] = TEXT('\\');
					dw->szCurDir[len] = TEXT('\0');
				}
				SetWindowText(GetDlgItem(hWnd, IDC_CURDIR), dw->szCurDir);
				SetWindowText(GetDlgItem(hWnd, IDC_DIR), dw->szCurDir);
				SendDlgItemMessage(hWnd, IDC_DIR, EM_SETSEL, (WPARAM)0, (LPARAM)-1);

				if(dw->szCurDir[len - 1] == TEXT('\\'))
					StringCchPrintf(&dw->szCurDir[len], MAX_PATH - len, TEXT("*"));
				else
					StringCchPrintf(&dw->szCurDir[len], MAX_PATH - len, TEXT("\\*"));

				if(dw->szCurDir[len - 2] == TEXT(':'))	// Root as in C:
				{
					id = SendDlgItemMessage(hWnd, IDC_SEL, LB_ADDSTRING, 0, (LPARAM)TEXT("My Computer"));
					SendDlgItemMessage(hWnd, IDC_SEL, LB_SETITEMDATA, id, (LPARAM)MYCOMPFOLDER);
				}

				dw->hSearch = FindFirstFile(dw->szCurDir, &findData); 
				if(dw->hSearch != INVALID_HANDLE_VALUE)
				{
					if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
					{
						if(StrCmp(findData.cFileName, TEXT("..")) == 0)
						{
							id = SendDlgItemMessage(hWnd, IDC_SEL, LB_ADDSTRING, 0, (LPARAM)TEXT("Parent folder"));
							SendDlgItemMessage(hWnd, IDC_SEL, LB_SETITEMDATA, id, (LPARAM)PARENTFOLDER);
						}
						else if(StrCmp(findData.cFileName, TEXT(".")) != 0)
						{
							if(!(findData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_OFFLINE)))
							{
								StringCchPrintf(dw->szCurDir, MAX_PATH, TEXT("  %s"), findData.cFileName);
								SendDlgItemMessage(hWnd, IDC_SEL, LB_ADDSTRING, 0, (LPARAM)dw->szCurDir);
								SendDlgItemMessage(hWnd, IDC_SEL, LB_SETITEMDATA, id, (LPARAM)NORMALFOLDER);
							}
						}
					}
					PostMessage(hWnd, WM_FINDNEXTFILE, 0, 0);
				}
			}
			break;

		case WM_FINDNEXTFILE:
			if(FindNextFile(dw->hSearch, &findData))
			{
				if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
				{
					if(StrCmp(findData.cFileName, TEXT("..")) == 0)
					{
						id = SendDlgItemMessage(hWnd, IDC_SEL, LB_ADDSTRING, 0, (LPARAM)TEXT("Parent folder"));
						SendDlgItemMessage(hWnd, IDC_SEL, LB_SETITEMDATA, id, (LPARAM)PARENTFOLDER);
					}
					else if(StrCmp(findData.cFileName, TEXT(".")) != 0)
					{
						if(!(findData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_OFFLINE)))
						{
							StringCchPrintf(dw->szCurDir, MAX_PATH, TEXT("  %s"), findData.cFileName);
							id = SendDlgItemMessage(hWnd, IDC_SEL, LB_ADDSTRING, 0, (LPARAM)dw->szCurDir);
							SendDlgItemMessage(hWnd, IDC_SEL, LB_SETITEMDATA, id, (LPARAM)NORMALFOLDER);
						}
					}
				}
				PostMessage(hWnd, WM_FINDNEXTFILE, 0, 0);
			}
			else
				FindClose(dw->hSearch);
			break;

		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					switch(LOWORD(wParam))
					{
						case IDC_GO:
							len = SendDlgItemMessage(hWnd, IDC_SEL, LB_GETCURSEL, 0, 0);
							id = 0;
							if(len != LB_ERR)
								id = SendDlgItemMessage(hWnd, IDC_SEL, LB_GETITEMDATA, len, 0);

							GetWindowText(GetDlgItem(hWnd, IDC_DIR), dw->szCurDir, MAX_PATH);
							SendDlgItemMessage(hWnd, IDC_SEL, LB_RESETCONTENT, 0, 0);
							PostMessage(hWnd, WM_FINDFIRSTFILE,	(WPARAM)0, id);
				 			SetFocus(GetDlgItem(hWnd, IDC_DIR));
							break;

						case IDC_OK:
							/* Pass directory to callee and close */
							len = SendDlgItemMessage(hWnd, IDC_DIR, WM_GETTEXTLENGTH, 0, 0) + 1;
							extStart = (LPTSTR)GlobalAlloc(GPTR, sizeof(TCHAR) * len);
							if(extStart)
							{
								GetWindowText(GetDlgItem(hWnd, IDC_DIR), extStart, len);
								PostMessage(dw->hParent, WM_RECEIVESTRING, (WPARAM)len, (LPARAM)extStart);
								PostMessage(hWnd, WM_CLOSE, 0, 0);
							}
							break;

						case IDC_CANCEL:
							/* DestroyWindow gives troubles! */
							PostMessage(hWnd, WM_CLOSE, 0, 0);
							break;
					}
					break;

				case LBN_SELCHANGE:
					if(LOWORD(wParam) == IDC_SEL)
					{
						SendDlgItemMessage(hWnd, IDC_GO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
						GetWindowText(GetDlgItem(hWnd, IDC_CURDIR), dw->szCurDir, MAX_PATH);
						len = SendDlgItemMessage(hWnd, IDC_SEL, LB_GETCURSEL, 0, 0);
						if(len != LB_ERR)
						{
							SendDlgItemMessage(hWnd, IDC_SEL, LB_GETTEXT, (WPARAM)len, (LPARAM)szTemp);
							id = SendDlgItemMessage(hWnd, IDC_SEL, LB_GETITEMDATA, len, 0);

				//			if(szTemp[0] == TEXT('M')) // 'M'y Computer
							if(id == MYCOMPFOLDER) // 'M'y Computer
							{
								StringCbCopy(dw->szCurDir, sizeof(dw->szCurDir), szTemp);
								SendDlgItemMessage(hWnd, IDC_OK, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
							}
				//			else if(szTemp[0] == TEXT('P')) // 'P'arent folder
							else if(id == PARENTFOLDER) // 'P'arent folder
							{
								extStart = _tcsrchr(dw->szCurDir, TEXT('\\'));
								if(extStart != NULL)
								{
									if(*(extStart - 1) != TEXT(':'))
										*extStart = TEXT('\0');
									else
										extStart[1] = TEXT('\0');
								}
								SendDlgItemMessage(hWnd, IDC_OK, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							}
				//			else if(szTemp[0] != TEXT(' ') && szTemp[1] == TEXT(':'))
							else if(id == DISKDRIVE)
							{
								StringCbCopy(dw->szCurDir, sizeof(dw->szCurDir), szTemp);
								SendDlgItemMessage(hWnd, IDC_OK, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							}
							else
							{
								StringCchLength(dw->szCurDir, MAX_PATH, &len);
								if(dw->szCurDir[len - 1] == TEXT('\\'))
									StringCchPrintf(&dw->szCurDir[len], MAX_PATH - len, TEXT("%s"), &szTemp[2]);
								else
									StringCchPrintf(&dw->szCurDir[len], MAX_PATH - len, TEXT("\\%s"), &szTemp[2]);
								SendDlgItemMessage(hWnd, IDC_OK, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							}
						}
						SetWindowText(GetDlgItem(hWnd, IDC_DIR), dw->szCurDir);
					}
					break;

				case LBN_DBLCLK:
					if(LOWORD(wParam) == IDC_SEL)
					{
						len = SendDlgItemMessage(hWnd, IDC_SEL, LB_GETCURSEL, 0, 0);
						id = 0;
						if(len != LB_ERR)
							id = SendDlgItemMessage(hWnd, IDC_SEL, LB_GETITEMDATA, len, 0);

						GetWindowText(GetDlgItem(hWnd, IDC_DIR), dw->szCurDir, MAX_PATH);
						SendDlgItemMessage(hWnd, IDC_SEL, LB_RESETCONTENT, 0, 0);
						PostMessage(hWnd, WM_FINDFIRSTFILE,	(WPARAM)0, id);
				 		SetFocus(GetDlgItem(hWnd, IDC_DIR));
					}
					break;
			}
			break;

		case WM_SIZE:
			GetClientRect(hWnd, &pRect);
			pRect.right -= pRect.left;
			pRect.bottom -= pRect.top;
			if((LOWORD(lParam) != pRect.right) || (HIWORD(lParam) != pRect.bottom))
				PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;

        case WM_DRAWITEM:
			if(((LPDRAWITEMSTRUCT)lParam)->CtlID == IDC_SEL)
				DrawSelDirListBoxItem((LPDRAWITEMSTRUCT)lParam);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}


static Graphics * gr = NULL;
static HDC hDc;
static Brush    * br_black = NULL;
static Brush    * br_white = NULL;
static Brush    * br_blue = NULL;
static Font     * fn = NULL;

static void DrawSelDirListBoxItem(LPDRAWITEMSTRUCT pItem)
{
	RectF        rf;
	RECT         crect;
	INT          i, n, size;
	StringFormat fmt;
	LPWSTR       pText;

	if(gr == NULL)
		return;

	pItem->rcItem.right -= pItem->rcItem.left;
	pItem->rcItem.bottom -= pItem->rcItem.top;

	rf.X = (float)pItem->rcItem.left;
	rf.Y = (float)pItem->rcItem.top;
	rf.Width = (float)pItem->rcItem.right;
	rf.Height = (float)pItem->rcItem.bottom;

	GetClientRect(pItem->hwndItem, &crect);
	n = SendMessage(pItem->hwndItem, LB_GETCOUNT, 0, 0);
	if((n < (crect.bottom / pItem->rcItem.bottom)) || (pItem->itemState & ODA_DRAWENTIRE))
	{
		for(i = n; i < (crect.bottom / pItem->rcItem.bottom); i++)
		{
			if(i % 2)
				gr->FillRectangle(br_blue, pItem->rcItem.left, i * pItem->rcItem.bottom, pItem->rcItem.right, pItem->rcItem.bottom);
			else
				gr->FillRectangle(br_white, pItem->rcItem.left, i * pItem->rcItem.bottom, pItem->rcItem.right, pItem->rcItem.bottom);
		}
	}

	size = 2 * SendMessage(pItem->hwndItem, LB_GETTEXTLEN, (WPARAM)pItem->itemID, 0) + 2;
	pText = (LPWSTR)GlobalAlloc(GPTR, size);
	SendMessage(pItem->hwndItem, LB_GETTEXT, (WPARAM)pItem->itemID, (LPARAM)pText);

	fmt.SetAlignment(StringAlignmentNear);
	fmt.SetTrimming(StringTrimmingNone);
	fmt.SetLineAlignment(StringAlignmentCenter);
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);
	if(pItem->itemState & ODS_SELECTED) 
	{
   		gr->FillRectangle(br_black, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.right, pItem->rcItem.bottom);
		gr->DrawString(pText, -1, fn, rf, &fmt, br_white);
	}
	else if(n != 0)
	{
		if(pItem->itemID % 2)
			gr->FillRectangle(br_blue, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.right, pItem->rcItem.bottom);
		else
			gr->FillRectangle(br_white, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.right, pItem->rcItem.bottom);

		gr->DrawString(pText, -1, fn, rf, &fmt, br_black);
	}
	GlobalFree(pText);
}

static void InitSelDirListBox(BOOL loadUnload, HWND lbox)
{
	if(loadUnload)
	{
		if(gr)       delete gr;
		if(hDc)		 ReleaseDC(lbox, hDc);
		if(br_black) delete br_black;
		if(br_white) delete br_white;
		if(br_blue)  delete br_blue;
		if(fn)       delete fn;

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

		fn = NULL;
		br_blue = NULL;
		br_white = NULL;
		br_black = NULL;
		gr = NULL;
	}
	else
	{
		delete gr;
		ReleaseDC(lbox, hDc);
		hDc = GetDC(lbox);
		gr = new Graphics(hDc, FALSE);
	}
}

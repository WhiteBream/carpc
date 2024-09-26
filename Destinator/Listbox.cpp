// Listbox.cpp: implementation of the Listbox library.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Listbox.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif


typedef struct _ListboxInfo
{
	INT			offset;
	Graphics	*lbGr;
	Pen			*lbPn;
	INT			visibleCount;
	INT			range;
    INT			curtop;
	BOOL		vOwnerDrawn;
	struct _draw
	{
		Graphics	*gr;
		HDC			hDc;
		Brush		*br_black;
		Brush		*br_white;
		Brush		*br_blue;
		Font		*fn;
	} draw;
} ListboxInfo, *PListboxInfo;

#define LBOXID		((int)GetMenu(hWnd))
#define SCROLLID	((int)GetMenu(hWnd) + 1)


LRESULT CALLBACK DestListboxProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void DrawSortListBox(PListboxInfo pLb, HWND hWnd);
static void DrawSortListBoxItem(PListboxInfo pLb, LPDRAWITEMSTRUCT pItem);
static void InitSortListBox(PListboxInfo pLb, BOOL loadUnload, HWND lbox);


//////////////////////////////////////////////////////////////////////
int ListboxInit(HINSTANCE hInstance)
{
	WNDCLASSEX     wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= DestListboxProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(LONG_PTR);
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("WB_LISTBOX");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}


LRESULT CALLBACK DestListboxProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPCREATESTRUCT cs;
    RECT		   ourRect, lbRect;
	DWORD          style;
	PListboxInfo   pLb;
	LRESULT        vRet;
	PAINTSTRUCT	   ps;
	INT            height;

	GetClientRect(hWnd, &ourRect);
	pLb = (PListboxInfo)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (message)
	{
		case WM_CREATE:
			pLb = (PListboxInfo)GlobalAlloc(GPTR, sizeof(ListboxInfo));
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pLb);

			cs = (LPCREATESTRUCT)lParam;
			pLb->offset = (INT)cs->lpCreateParams;
			style = cs->style & (LBS_NOTIFY | LBS_SORT | LBS_HASSTRINGS);
			pLb->vOwnerDrawn = (cs->style & LBS_OWNERDRAWFIXED ? TRUE : FALSE);

			CreateWindow(TEXT("LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | LBS_OWNERDRAWFIXED | style, 1, 1,
				cs->cx - pLb->offset - 2, cs->cy - 2, hWnd, cs->hMenu, cs->hInstance, NULL);

			pLb->visibleCount = (cs->cy - 2) / SendDlgItemMessage(hWnd, (int)cs->hMenu, LB_GETITEMHEIGHT, 0, 0);
			height = pLb->visibleCount * SendDlgItemMessage(hWnd, (int)cs->hMenu, LB_GETITEMHEIGHT, 0, 0);
			SetWindowPos(hWnd, NULL, 0, 0, cs->cx, height + 2, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
			InitSortListBox(pLb, TRUE, GetDlgItem(hWnd, LBOXID));

			CreateWindow(TEXT("WB_DESTSCROLL"), NULL, WS_CHILD | WS_VISIBLE, cs->cx - pLb->offset, 0,
				pLb->offset, height + 2, hWnd, (HMENU)((int)cs->hMenu + 1), cs->hInstance, NULL);
			SendDlgItemMessage(hWnd, SCROLLID, TBM_SETRANGE, TRUE, MAKELONG(0, (pLb->range = 0)));
			SendDlgItemMessage(hWnd, SCROLLID, TBM_SETPAGESIZE, 0, (LPARAM)pLb->visibleCount - 1);

			pLb->lbGr = new Graphics(hWnd, FALSE);
			pLb->lbPn = new Pen(Color(255, 0, 0, 0), 1);
			break;

		case WM_DESTROY:
			if(pLb)
			{
				InitSortListBox(pLb, FALSE, NULL);
				if(pLb->lbGr) delete pLb->lbGr;
				if(pLb->lbPn) delete pLb->lbPn;
				GlobalFree((HGLOBAL)pLb);
			}
			break;

		/* Generic listbox intermediates, parent > listbox */
		case LB_SETTOPINDEX:
			pLb->curtop = (INT)lParam;
			SendDlgItemMessage(hWnd, SCROLLID, TBM_SETPOS, TRUE, MAKELONG(0, pLb->curtop));
			return(SendDlgItemMessage(hWnd, LBOXID, message, wParam, lParam));

		case LB_SETITEMDATA:
		case LB_GETITEMDATA:
		case LB_GETITEMHEIGHT:
		case LB_GETCOUNT:
		case LB_GETTOPINDEX:
		case LB_GETTEXTLEN:
		case LB_GETTEXT:
		case LB_SETSEL:
		case LB_GETSEL:
		case LB_SETCURSEL:
		case LB_GETCURSEL:
		case LB_FINDSTRING:
			return(SendDlgItemMessage(hWnd, LBOXID, message, wParam, lParam));

		case WM_MOUSEWHEEL:
			pLb->curtop += -GET_WHEEL_DELTA_WPARAM(wParam) / 40;
			if(pLb->curtop < 0) pLb->curtop = 0;
			if(pLb->curtop > (pLb->range - pLb->visibleCount)) pLb->curtop = pLb->range - pLb->visibleCount;
			SendDlgItemMessage(hWnd, LBOXID, LB_SETTOPINDEX, (LPARAM)pLb->curtop, 0);
			SendDlgItemMessage(hWnd, SCROLLID, TBM_SETPOS, TRUE, pLb->curtop);
			return(0);

		case LB_SETITEMHEIGHT:
			vRet = SendDlgItemMessage(hWnd, LBOXID, message, wParam, lParam);
			GetClientRect(GetDlgItem(hWnd, LBOXID), &lbRect);
			pLb->visibleCount = lbRect.bottom / SendDlgItemMessage(hWnd, LBOXID, LB_GETITEMHEIGHT, 0, 0);
			height = pLb->visibleCount * SendDlgItemMessage(hWnd, LBOXID, LB_GETITEMHEIGHT, 0, 0);
			SetWindowPos(hWnd, NULL, 0, 0, ourRect.right, height + 2, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
			lbRect.right += 2;
			SetWindowPos(GetDlgItem(hWnd, SCROLLID), NULL, lbRect.right, 0, ourRect.right - lbRect.right, height + 2, SWP_NOZORDER | SWP_NOMOVE);
			InitSortListBox(pLb, FALSE, GetDlgItem(hWnd, LBOXID));
			SendDlgItemMessage(hWnd, SCROLLID, TBM_SETPAGESIZE, 0, (LPARAM)pLb->visibleCount - 1);
			return(vRet);

		/* Size adjusting listbox intermediates, parent > listbox */
		case LB_ADDSTRING:
		case LB_INSERTSTRING:
			vRet = SendDlgItemMessage(hWnd, LBOXID, message, wParam, lParam);
			if(++pLb->range > pLb->visibleCount)
				SendDlgItemMessage(hWnd, SCROLLID, TBM_SETRANGE, TRUE, MAKELONG(0, pLb->range - pLb->visibleCount));
			else
				SendDlgItemMessage(hWnd, SCROLLID, TBM_SETRANGE, TRUE, MAKELONG(0, 0));
			return(vRet);

		case LB_DELETESTRING:
			vRet = SendDlgItemMessage(hWnd, LBOXID, message, wParam, lParam);
			if(--pLb->range > pLb->visibleCount)
				SendDlgItemMessage(hWnd, SCROLLID, TBM_SETRANGE, TRUE, MAKELONG(0, pLb->range - pLb->visibleCount));
			else
				SendDlgItemMessage(hWnd, SCROLLID, TBM_SETRANGE, TRUE, MAKELONG(0, 0));
			return(vRet);

		case LB_RESETCONTENT:
			vRet = SendDlgItemMessage(hWnd, LBOXID, message, wParam, lParam);
			SendDlgItemMessage(hWnd, SCROLLID, TBM_SETPOS, TRUE, MAKELONG(0, (pLb->curtop = 0)));
			SendDlgItemMessage(hWnd, SCROLLID, TBM_SETRANGE, TRUE, MAKELONG(0, (pLb->range = 0)));
			return(vRet);

		/* likewise, listbox > parent */
		case WM_DRAWITEM:
			if(((LPDRAWITEMSTRUCT)lParam)->rcItem.top == 0)
			{
				vRet = SendDlgItemMessage(hWnd, LBOXID, LB_GETTOPINDEX, 0, 0);
				if(vRet != pLb->curtop)
					SendDlgItemMessage(hWnd, SCROLLID, TBM_SETPOS, TRUE, vRet);
				pLb->curtop = vRet;
			}
			if(pLb->vOwnerDrawn)
			{
				((LPDRAWITEMSTRUCT)lParam)->rcItem.top++;
				((LPDRAWITEMSTRUCT)lParam)->rcItem.bottom++;
				((LPDRAWITEMSTRUCT)lParam)->rcItem.left++;
				((LPDRAWITEMSTRUCT)lParam)->rcItem.right++;
				return(SendMessage(GetAncestor(hWnd, GA_PARENT), message, wParam, lParam));
			}
			else
				DrawSortListBoxItem(pLb, (LPDRAWITEMSTRUCT)lParam);
			break;

		case WM_DELETEITEM:
			return(SendMessage(GetAncestor(hWnd, GA_PARENT), message, wParam, lParam));

		case WM_COMMAND:
			return(SendMessage(GetAncestor(hWnd, GA_PARENT), message, wParam, lParam));

		/* Normal window handling */
		case WM_SIZE:
			SetWindowPos(GetDlgItem(hWnd, LBOXID), NULL, 0, 0, ourRect.right - pLb->offset - 2, ourRect.bottom - 2, SWP_NOZORDER | SWP_NOMOVE);
			InitSortListBox(pLb, FALSE, GetDlgItem(hWnd, LBOXID));
			GetClientRect(GetDlgItem(hWnd, LBOXID), &lbRect);
			pLb->visibleCount = lbRect.bottom / SendDlgItemMessage(hWnd, LBOXID, LB_GETITEMHEIGHT, 0, 0);
			height = pLb->visibleCount * SendDlgItemMessage(hWnd, LBOXID, LB_GETITEMHEIGHT, 0, 0);
			height += 2;
			lbRect.bottom += 2;
			lbRect.right += 2;
			SetWindowPos(hWnd, NULL, 0, 0, ourRect.right, height, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, SCROLLID), NULL, lbRect.right, 0, pLb->offset, height, SWP_NOZORDER);
			SendDlgItemMessage(hWnd, SCROLLID, TBM_SETPAGESIZE, 0, (LPARAM)pLb->visibleCount - 1);
			if(pLb->range > pLb->visibleCount)
				SendDlgItemMessage(hWnd, SCROLLID, TBM_SETRANGE, TRUE, MAKELONG(0, pLb->range - pLb->visibleCount));
			else
				SendDlgItemMessage(hWnd, SCROLLID, TBM_SETRANGE, TRUE, MAKELONG(0, 0));
			if(pLb->lbGr) delete pLb->lbGr;
			pLb->lbGr = new Graphics(hWnd, FALSE);
			break;

		case WM_PAINT:
			GetClientRect(GetDlgItem(hWnd, LBOXID), &lbRect);
			BeginPaint(hWnd, &ps);
			if(pLb)
				pLb->lbGr->DrawRectangle(pLb->lbPn, 0, 0, lbRect.right + 1, lbRect.bottom + 1);
			EndPaint(hWnd, &ps);
			SendDlgItemMessage(hWnd, LBOXID, message, wParam, lParam);
			if(pLb && pLb->vOwnerDrawn)
				SendMessage(GetAncestor(hWnd, GA_PARENT), WM_PAINTOWNERDRAWN, (WPARAM)GetDlgItem(hWnd, LBOXID), 0);
			else
				DrawSortListBox(pLb, GetDlgItem(hWnd, LBOXID));
			break;

		case WM_VSCROLL:
			if((LOWORD(wParam) == TB_THUMBTRACK) || (LOWORD(wParam) == TB_ENDTRACK ))
			{
				if((pLb->curtop = HIWORD(wParam)) == 0) // TB_ENDTRACK
					pLb->curtop = SendDlgItemMessage(hWnd, SCROLLID, TBM_GETPOS, 0, 0);
				SendDlgItemMessage(hWnd, LBOXID, LB_SETTOPINDEX, (LPARAM)pLb->curtop, 0);
			}
			break;

		default:
			return(DefWindowProc(hWnd, message, wParam, lParam));
   }
   return(0);
}

static void DrawSortListBox(PListboxInfo pLb, HWND hWnd)
{
	RECT vListBox, vItem;
	INT  i, vCount;
	LONG vTop;

	if(!pLb)
		return;

	GetClientRect(hWnd, &vListBox);
	vCount = SendMessage(hWnd, LB_GETCOUNT, 0, 0);
	vItem.bottom = SendMessage(hWnd, LB_GETITEMHEIGHT, 0, 0);

	if(vCount < (vListBox.bottom / vItem.bottom))
	{
		for(i = vCount; i < (vListBox.bottom / vItem.bottom); i++)
		{
			vTop = vListBox.top + i * vItem.bottom;
			if(i % 2)
				pLb->draw.gr->FillRectangle(pLb->draw.br_blue, vListBox.left, vTop, vListBox.right, vItem.bottom);
			else
				pLb->draw.gr->FillRectangle(pLb->draw.br_white, vListBox.left + vItem.bottom, vTop, vListBox.right - vItem.bottom, vItem.bottom);
		}
	}
}

static void DrawSortListBoxItem(PListboxInfo pLb, LPDRAWITEMSTRUCT pItem)
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
				pLb->draw.gr->FillRectangle(pLb->draw.br_blue, pItem->rcItem.left, top, pItem->rcItem.right, pItem->rcItem.bottom);
			else
				pLb->draw.gr->FillRectangle(pLb->draw.br_white, pItem->rcItem.left, top, pItem->rcItem.right, pItem->rcItem.bottom);
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
   		pLb->draw.gr->FillRectangle(pLb->draw.br_black, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.right, pItem->rcItem.bottom);
		if(pText)
			pLb->draw.gr->DrawString(pText, -1, pLb->draw.fn, rf, &fmt, pLb->draw.br_white);
	}
	else if(n != 0)
	{
		if(pItem->itemID % 2)
			pLb->draw.gr->FillRectangle(pLb->draw.br_blue, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.right, pItem->rcItem.bottom);
		else
			pLb->draw.gr->FillRectangle(pLb->draw.br_white, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.right, pItem->rcItem.bottom);

		// Item text
		if(pText)
			pLb->draw.gr->DrawString(pText, -1, pLb->draw.fn, rf, &fmt, pLb->draw.br_black);
	}
	if(pText)
		GlobalFree(pText);
}

static void InitSortListBox(PListboxInfo pLb, BOOL loadUnload, HWND lbox)
{
	if(loadUnload)
	{
		pLb->draw.hDc = GetDC(lbox);
		pLb->draw.gr = new Graphics(pLb->draw.hDc, FALSE);
		if(!pLb->draw.br_black) pLb->draw.br_black = new SolidBrush(Color(255, 0, 0, 0));
		if(!pLb->draw.br_white) pLb->draw.br_white = new SolidBrush(Color(255, 255, 255, 255));
		if(!pLb->draw.br_blue) pLb->draw.br_blue = new SolidBrush(Color(255, 201, 211, 227));
		if(!pLb->draw.fn) pLb->draw.fn = new Font(L"Arial", 16);
	}
	else if(lbox == NULL)
	{
		delete pLb->draw.fn;
		delete pLb->draw.br_blue;
		delete pLb->draw.br_white;
		delete pLb->draw.br_black;
		delete pLb->draw.gr;
	//	ReleaseDC(lbox, pLb->draw.hDc);
	}
	else
	{
		delete pLb->draw.gr;
		ReleaseDC(lbox, pLb->draw.hDc);
		pLb->draw.hDc = GetDC(lbox);
		pLb->draw.gr = new Graphics(pLb->draw.hDc, FALSE);
	}
}

// Dialog.cpp: implementation of the Dialog library.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Dialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

typedef struct _DialogInfo
{
} DialogInfo;

LRESULT CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#define CLASSNAME		TEXT("WB_DIALOG")

//////////////////////////////////////////////////////////////////////
int DialogInit(HINSTANCE hInstance, BOOL vLoadUnload)
{
	WNDCLASSEX wcex;

	if(vLoadUnload)
	{
		wcex.cbSize			= sizeof(WNDCLASSEX);
		wcex.style			= 0;
		wcex.lpfnWndProc	= DialogProc;
		wcex.cbClsExtra		= NULL;
		wcex.cbWndExtra		= sizeof(LONG_PTR);
		wcex.hInstance		= hInstance;
		wcex.hIcon			= NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= NULL;
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= CLASSNAME;
		wcex.hIconSm		= NULL;
		return(RegisterClassEx(&wcex));
	}
	else
	{
		return(UnregisterClass(CLASSNAME, hInstance));
	}
}

INT DestDialog(HWND hWnd, LPCSTR lpszText, LPCTSTR lpszTitle, INT y)
{
	DialogInfo dialogInfo;

	CreateWindow(CLASSNAME, lpszTitle,     hWnd,    &dialogInfo);
}

LRESULT CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	KeyboardInfo  * ki;
	LPCREATESTRUCT  cs;
	TCHAR           vKey[2];
	int             i, pos, pos2;

	if(message == WM_CREATE)
	{
		ki = (KeyboardInfo *)GlobalAlloc(GPTR, sizeof(KeyboardInfo));
		if(ki == NULL)
			return(1L);	// Error
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)ki);

		cs = (LPCREATESTRUCT)lParam;
		ki->id = (int)cs->hMenu;
		ki->hWnd = hWnd;
		ki->hParent = cs->hwndParent;
		ki->hTgt = NULL;
		ki->hInst = cs->hInstance;
		ki->size.left = cs->x;
		ki->size.top = cs->y;
		ki->size.right = cs->cx;
		ki->size.bottom = cs->cy;
		ki->vCaps = ki->vShift = FALSE;

		SetupKeyboardInfo(ki);

		return(0);
	}

	ki = (KeyboardInfo *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if((ki == NULL)
		return(DefWindowProc(hWnd, message, wParam, lParam));

	switch (message)
	{
		case WM_DESTROY:
			GlobalFree((void *)ki);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)0);
			break;

		case WM_SETOSKTARGET:
			ki->hTgt = (HWND)lParam;
			SendMessage(ki->hTgt, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
			SetFocus(ki->hTgt);
			break;

		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
					i = LOWORD(wParam);
					switch(vKeyCodes[i])
					{
						case CAPS:
							if(SendMessage(ki->key[i].hWnd, BM_GETCHECK, (WPARAM)0, 0) == BST_CHECKED)
								ki->vCaps = FALSE;
							else
								ki->vCaps = TRUE;
							SendMessage(ki->key[i].hWnd, BM_SETCHECK, (WPARAM)(ki->vCaps ? BST_CHECKED : BST_UNCHECKED), 0);
							SetKeyLabels(ki);
							break;

						case SHIFT:
							if(SendMessage(ki->key[i].hWnd, BM_GETCHECK, (WPARAM)0, 0) == BST_CHECKED)
								ki->vShift = -1;
							else
								ki->vShift = i;
							SendMessage(ki->key[i].hWnd, BM_SETCHECK, (WPARAM)(ki->vShift >= 0 ? BST_CHECKED : BST_UNCHECKED), 0);
							SetKeyLabels(ki);
							break;

						case LEFT: /* < */
						case RIGHT: /* > */
							SendMessage(ki->hTgt, EM_GETSEL, (WPARAM)&pos, 0);
							if(i == RIGHT)
								pos++;
							else
								pos--;
							SendMessage(ki->hTgt, EM_SETSEL, (WPARAM)pos, (LPARAM)pos);
							break;

						case DEL:
							SetFocus(hWnd);
							SendMessage(ki->hTgt, EM_GETSEL, (WPARAM)&pos, 0);
							pos++;
							SendMessage(ki->hTgt, EM_SETSEL, (WPARAM)pos, (LPARAM)pos);
							SendMessage(ki->hTgt, EM_GETSEL, (WPARAM)&pos2, 0);
							if(pos2 == pos)
								SendMessage(ki->hTgt, WM_CHAR, (WPARAM)TEXT('\b'), 0);
							SetFocus(ki->hTgt);
							break;

						default:
							if((vKey[0] = vKeyCodes[i]) == 0)
								GetWindowText(ki->key[i].hWnd, vKey, 2);
							if(ki->hTgt)
							{
								SetFocus(ki->hTgt);
								SendMessage(ki->hTgt, WM_CHAR, (WPARAM)vKey[0], 0);
							}
							else if(ki->hParent)
							{
								SetFocus(ki->hParent);
								SendMessage(ki->hParent, WM_CHAR, (WPARAM)vKey[0], 0);
							}
							break;
					}
					if((vKeyCodes[i] != SHIFT) && (ki->vShift >= 0))
					{
						SendMessage(ki->key[ki->vShift].hWnd, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
						ki->vShift = -1;
						SetKeyLabels(ki);
					}
					break;
			}
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}

// Keyboard.cpp: implementation of the Keyboard library.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Keyboard.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

typedef struct _DESTKEY
{
	HWND  hWnd;
	void  (*pFunc)(void);
	POINT vPos;
} DestKey;

typedef struct _KeyboardInfo
{
	int            id;
	HINSTANCE	   hInst;
	HWND           hWnd;
	HWND           hParent;
	HWND           hTgt;
	HDC            hdc;
	RECT		   size;
	DestKey		   key[14*4];
	BOOL           vCaps;
	INT            keyCaps;
	INT            vShift;
} KeyboardInfo;

LRESULT CALLBACK KeyboardProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#define CLASSNAME		TEXT("WB_KEYBOARD")

//////////////////////////////////////////////////////////////////////
int KeyboardInit(HINSTANCE hInstance, BOOL vLoadUnload)
{
	WNDCLASSEX wcex;

	if(vLoadUnload)
	{
		wcex.cbSize			= sizeof(WNDCLASSEX);
		wcex.style			= 0;
		wcex.lpfnWndProc	= KeyboardProc;
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

const TCHAR vKeyNames[14*4][6] = 
{
	TEXT("`"), TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4"), TEXT("5"), TEXT("6"), TEXT("7"), /*15*/
		TEXT("8"), TEXT("9"), TEXT("0"), TEXT("-"), TEXT("="), TEXT("del"), TEXT("bcksp"),
	TEXT("caps"), TEXT("q"), TEXT("w"), TEXT("e"), TEXT("r"), TEXT("t"), TEXT("y"), /*14*/
		TEXT("u"), TEXT("i"), TEXT("o"), TEXT("p"), TEXT("["), TEXT("]"), TEXT("#"),
	TEXT("space"), TEXT("a"), TEXT("s"), TEXT("d"), TEXT("f"), TEXT("g"), TEXT("h"), /*14*/
		TEXT("j"), TEXT("k"), TEXT("l"), TEXT(";"), TEXT("\'"), TEXT("enter"), TEXT(""),
	TEXT("shift"), TEXT("z"), TEXT("x"), TEXT("c"), TEXT("v"), TEXT("b"), TEXT("n"), /*13*/
		TEXT("m"), TEXT(","), TEXT("."), TEXT("/"), TEXT(""), TEXT("")
};
const TCHAR vKeyNamesShift[14*4][6] = 
{
	TEXT("~"), TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4"), TEXT("5"), TEXT("6"), TEXT("7"), 
		TEXT("8"), TEXT("9"), TEXT("0"), TEXT("_"), TEXT("+"), TEXT("del"), TEXT("bcksp"),
	TEXT("caps"), TEXT("Q"), TEXT("W"), TEXT("E"), TEXT("R"), TEXT("T"), TEXT("Y"), 
		TEXT("U"), TEXT("I"), TEXT("O"), TEXT("P"), TEXT("{"), TEXT("}"), TEXT("@"),
	TEXT("space"), TEXT("A"), TEXT("S"), TEXT("D"), TEXT("F"), TEXT("G"), TEXT("H"), 
		TEXT("J"), TEXT("K"), TEXT("L"), TEXT(":"), TEXT("\""), TEXT("enter"), TEXT(""),
	TEXT("shift"), TEXT("Z"), TEXT("X"), TEXT("C"), TEXT("V"), TEXT("B"), TEXT("N"), 
		TEXT("M"), TEXT("<"), TEXT(">"), TEXT("\\"), TEXT(""), TEXT("")
};

#define CAPS	255
#define SHIFT	254
#define LEFT	253
#define RIGHT	252
#define DEL		251

const TCHAR vKeyCodes[14*4] = 
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, DEL, TEXT('\b'),
	CAPS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	TEXT(' '), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, TEXT('\n'), 0,
	SHIFT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LEFT, RIGHT
};

void SetupKeyboardInfo(KeyboardInfo * ki)
{
	int i;
	int vHeight = ki->size.bottom / 4;
	int vRight = ki->size.right;
	int vWidth;
	int vX;
	int vY = 0;

	for(i = 0; i < (14*4); i++)
	{
		if(vKeyCodes[i] == CAPS)
			ki->keyCaps = i;
	}

	/* First row */
	vWidth = (2 * vRight) / 31;
	vX = 0;
	for(i = 0; i < 14; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		ki->key[i].hWnd = CreateWindow(TEXT("WB_DESTBUTTON"), vKeyNames[i], WS_CHILD | WS_VISIBLE, 
										vX, vY, vWidth, vHeight, ki->hWnd, (HMENU)i, ki->hInst, NULL);
		vX += vWidth;
	}
	for(i = 14; i < 15; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		ki->key[i].hWnd = CreateWindow(TEXT("WB_DESTBUTTON"), vKeyNames[i], WS_CHILD | WS_VISIBLE,
										vX, vY, vRight - vX, vHeight, ki->hWnd, (HMENU)i, ki->hInst, NULL);
	}
	vY += vHeight;

	/* Second row */
	vWidth = (2 * vRight) / 29;
	vX = 0;
	for(i = 15; i < 16; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		ki->key[i].hWnd = CreateWindow(TEXT("WB_DESTBUTTON"), vKeyNames[i], WS_CHILD | WS_VISIBLE, 
										vX, vY, vWidth + vWidth / 3, vHeight, ki->hWnd, (HMENU)i, ki->hInst, NULL);
		vX += vWidth + vWidth / 3;
	}
	for(i = 16; i < 28; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		ki->key[i].hWnd = CreateWindow(TEXT("WB_DESTBUTTON"), vKeyNames[i], WS_CHILD | WS_VISIBLE, 
										vX, vY, vWidth, vHeight, ki->hWnd, (HMENU)i, ki->hInst, NULL);
		vX += vWidth;
	}
	for(i = 28; i < 29; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		ki->key[i].hWnd = CreateWindow(TEXT("WB_DESTBUTTON"), vKeyNames[i], WS_CHILD | WS_VISIBLE,
										vX, vY, vRight - vX, vHeight, ki->hWnd, (HMENU)i, ki->hInst, NULL);
	}
	vY += vHeight;

	/* Third row */
	vX = 0;
	for(i = 29; i < 30; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		ki->key[i].hWnd = CreateWindow(TEXT("WB_DESTBUTTON"), vKeyNames[i], WS_CHILD | WS_VISIBLE, 
										vX, vY, 15 * vWidth / 8, vHeight, ki->hWnd, (HMENU)i, ki->hInst, NULL);
		vX += 15 * vWidth / 8;
	}
	for(i = 30; i < 41; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		ki->key[i].hWnd = CreateWindow(TEXT("WB_DESTBUTTON"), vKeyNames[i], WS_CHILD | WS_VISIBLE, 
										vX, vY, vWidth, vHeight, ki->hWnd, (HMENU)i, ki->hInst, NULL);
		vX += vWidth;
	}
	for(i = 41; i < 42; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		ki->key[i].hWnd = CreateWindow(TEXT("WB_DESTBUTTON"), vKeyNames[i], WS_CHILD | WS_VISIBLE,
										vX, vY, vRight - vX, vHeight, ki->hWnd, (HMENU)i, ki->hInst, NULL);
	}
	vY += vHeight;

	/* Last row */
	vX = 0;
	for(i = 43; i < 44; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		ki->key[i].hWnd = CreateWindow(TEXT("WB_DESTBUTTON"), vKeyNames[i], WS_CHILD | WS_VISIBLE, 
										vX, vY, 7 * vWidth / 3, vHeight, ki->hWnd, (HMENU)i, ki->hInst, NULL);
		vX += 7 * vWidth / 3;
	}
	for(i = 44; i < 54; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		ki->key[i].hWnd = CreateWindow(TEXT("WB_DESTBUTTON"), vKeyNames[i], WS_CHILD | WS_VISIBLE, 
										vX, vY, vWidth, vHeight, ki->hWnd, (HMENU)i, ki->hInst, NULL);
		vX += vWidth;
	}
	for(i = 54; i < 55; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		vWidth = (vRight - vX) / 2;
		ki->key[i].hWnd = CreateWindow(TEXT("WB_DESTBUTTON"), vKeyNames[i], WS_CHILD | WS_VISIBLE,
										vX, vY, vWidth, vHeight, ki->hWnd, (HMENU)i, ki->hInst, NULL);
		vX += vWidth;
	}
	for(i = 55; i < 56; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		ki->key[i].hWnd = CreateWindow(TEXT("WB_DESTBUTTON"), vKeyNames[i], WS_CHILD | WS_VISIBLE,
										vX, vY, vRight - vX, vHeight, ki->hWnd, (HMENU)i, ki->hInst, NULL);
	}


	for(i = 0; i < (14*4); i++)
	{
		HICON ico = NULL;

		switch(vKeyCodes[i])
		{
			case LEFT:
				ico = LoadIcon(ki->hInst, (LPCTSTR)IDI_LEFT);
				break;
			case RIGHT:
				ico = LoadIcon(ki->hInst, (LPCTSTR)IDI_RIGHT);
				break;
		}
		if(ico)
		{
			SendMessage(ki->key[i].hWnd, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
			DestroyIcon(ico);
		}
	}
}

void SetupKeyboardPos(KeyboardInfo * ki)
{
	int i;
	int vHeight = ki->size.bottom / 4;
	int vRight = ki->size.right;
	int vWidth;
	int vX;
	int vY = 0;

	/* First row */
	vWidth = (2 * vRight) / 31;
	vX = 0;
	for(i = 0; i < 14; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		SetWindowPos(ki->key[i].hWnd, 0, ki->key[i].vPos.x, ki->key[i].vPos.y, vWidth, vHeight, SWP_NOZORDER);
		vX += vWidth;
	}
	for(i = 14; i < 15; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		SetWindowPos(ki->key[i].hWnd, 0, ki->key[i].vPos.x, ki->key[i].vPos.y, vRight - vX, vHeight, SWP_NOZORDER);
	}
	vY += vHeight;

	/* Second row */
	vWidth = (2 * vRight) / 29;
	vX = 0;
	for(i = 15; i < 16; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		SetWindowPos(ki->key[i].hWnd, 0, ki->key[i].vPos.x, ki->key[i].vPos.y, vWidth + vWidth / 3, vHeight, SWP_NOZORDER);
		vX += vWidth + vWidth / 3;
	}
	for(i = 16; i < 28; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		SetWindowPos(ki->key[i].hWnd, 0, ki->key[i].vPos.x, ki->key[i].vPos.y, vWidth, vHeight, SWP_NOZORDER);
		vX += vWidth;
	}
	for(i = 28; i < 29; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		SetWindowPos(ki->key[i].hWnd, 0, ki->key[i].vPos.x, ki->key[i].vPos.y, vRight - vX, vHeight, SWP_NOZORDER);
	}
	vY += vHeight;

	/* Third row */
	vX = 0;
	for(i = 29; i < 30; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		SetWindowPos(ki->key[i].hWnd, 0, ki->key[i].vPos.x, ki->key[i].vPos.y, 15 * vWidth / 8, vHeight, SWP_NOZORDER);
		vX += 15 * vWidth / 8;
	}
	for(i = 30; i < 41; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		SetWindowPos(ki->key[i].hWnd, 0, ki->key[i].vPos.x, ki->key[i].vPos.y, vWidth, vHeight, SWP_NOZORDER);
		vX += vWidth;
	}
	for(i = 41; i < 42; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		SetWindowPos(ki->key[i].hWnd, 0, ki->key[i].vPos.x, ki->key[i].vPos.y, vRight - vX, vHeight, SWP_NOZORDER);
	}
	vY += vHeight;

	/* Last row */
	vX = 0;
	for(i = 43; i < 44; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		SetWindowPos(ki->key[i].hWnd, 0, ki->key[i].vPos.x, ki->key[i].vPos.y, 7 * vWidth / 3, vHeight, SWP_NOZORDER);
		vX += 7 * vWidth / 3;
	}
	for(i = 44; i < 54; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		SetWindowPos(ki->key[i].hWnd, 0, ki->key[i].vPos.x, ki->key[i].vPos.y, vWidth, vHeight, SWP_NOZORDER);
		vX += vWidth;
	}
	for(i = 54; i < 55; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		vWidth = (vRight - vX) / 2;
		SetWindowPos(ki->key[i].hWnd, 0, ki->key[i].vPos.x, ki->key[i].vPos.y, vWidth, vHeight, SWP_NOZORDER);
		vX += vWidth;
	}
	for(i = 55; i < 56; i++)
	{
		ki->key[i].vPos.x = vX;
		ki->key[i].vPos.y = vY;
		SetWindowPos(ki->key[i].hWnd, 0, ki->key[i].vPos.x, ki->key[i].vPos.y, vRight - vX, vHeight, SWP_NOZORDER);
	}
}

void SetKeyLabels(KeyboardInfo * ki)
{
	int i;

	for(i = 0; i < 14*4; i++)
	{
		if(ki->key[i].hWnd)
		{
			if(ki->vCaps == (ki->vShift >= 0))
				SetWindowText(ki->key[i].hWnd, vKeyNames[i]);
			else
				SetWindowText(ki->key[i].hWnd, vKeyNamesShift[i]);
			SendMessage(ki->key[i].hWnd, WM_PAINT, 0, 0);
		}
	}
	UpdateWindow(ki->hWnd);
}

LRESULT CALLBACK KeyboardProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	KeyboardInfo  * ki;
	LPCREATESTRUCT  cs;
	TCHAR           vKey[2];
	int             i, pos, pos2;
//	INPUT           kbSet[2];

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
	//	if(GetAsyncKeyState(VK_CAPITAL) & 0x8000)
	//	{
	//		ki->vCaps = TRUE;
	//		SendDlgItemMessage(hWnd, ki->keyCaps, BM_SETCHECK, (WPARAM)(ki->vCaps ? BST_CHECKED : BST_UNCHECKED), 0);
	//		SetKeyLabels(ki);
	//	}
		return(0);
	}

	ki = (KeyboardInfo *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if(ki == NULL)
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

					//		kbSet[0].type = INPUT_KEYBOARD;
					//		kbSet[0].ki.wVk = VK_CAPITAL;
					//		kbSet[0].ki.dwFlags = 0;
					//		kbSet[0].ki.dwExtraInfo = GetMessageExtraInfo();
					//		kbSet[1].type = INPUT_KEYBOARD;
					//		kbSet[1].ki.wVk = VK_CAPITAL;
					//		kbSet[1].ki.dwFlags = KEYEVENTF_KEYUP;
					//		kbSet[1].ki.dwExtraInfo = GetMessageExtraInfo();
					//		SendInput(2, kbSet, sizeof(INPUT));
							break;

						case SHIFT:
							if(SendMessage(ki->key[i].hWnd, BM_GETCHECK, (WPARAM)0, 0) == BST_CHECKED)
								ki->vShift = -1;
							else
								ki->vShift = i;
							SendMessage(ki->key[i].hWnd, BM_SETCHECK, (WPARAM)(ki->vShift >= 0 ? BST_CHECKED : BST_UNCHECKED), 0);
							SetKeyLabels(ki);

					//		kbSet[0].type = INPUT_KEYBOARD;
					//		kbSet[0].ki.wVk = VK_SHIFT;
					//		kbSet[0].ki.dwFlags = (ki->vShift == -1 ? KEYEVENTF_KEYUP : 0);
					//		kbSet[0].ki.dwExtraInfo = GetMessageExtraInfo();
					//		SendInput(1, kbSet, sizeof(INPUT));
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
								PostMessage(ki->hTgt, WM_CHAR, (WPARAM)vKey[0], 0);
							}
							else if(ki->hParent)
							{
								SetFocus(ki->hParent);
								PostMessage(ki->hParent, WM_CHAR, (WPARAM)vKey[0], 0);
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

		case WM_SHOWWINDOW:
			if(wParam)
				SetTimer(hWnd, 1, 100, NULL);
			else
				KillTimer(hWnd, 1);
			break;

	//	case WM_TIMER:
	//		kbState = GetAsyncKeyState(VK_CAPITAL);
	//		if(kbState & 0x8000)
	//		{
	//			ki->vCaps = TRUE;
	//			SendDlgItemMessage(hWnd, ki->keyCaps, BM_SETCHECK, (WPARAM)(ki->vCaps ? BST_CHECKED : BST_UNCHECKED), 0);
	//			SetKeyLabels(ki);
	//		}
	//		else if(kbState & 0x0001)
	//		{
	//			ki->vCaps = TRUE;
	//			SendDlgItemMessage(hWnd, ki->keyCaps, BM_SETCHECK, (WPARAM)(ki->vCaps ? BST_CHECKED : BST_UNCHECKED), 0);
	//			SetKeyLabels(ki);
	//		}
	//		SetTimer(hWnd, 1, 100, NULL);
	//		return(0);

		case WM_SIZE:
			ki->size.right = LOWORD(lParam);
			ki->size.bottom = HIWORD(lParam);
			SetupKeyboardPos(ki);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}

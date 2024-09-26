// Button.cpp: implementation of the Button library.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Destinator.h"
#include "Button.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

typedef struct _ButtonClsInfo
{
    Bitmap *destImage[3];
	Brush  *fontColor[3];
	Bitmap *destButton[3];
	Font   *defFont;
} ButtonClsInfo;

typedef struct _ButtonWndInfo
{
	INT				id;
	POINT			pWnd;
	HWND			hParent;
	HDC				hdc;
	Graphics		*btnGraphics;
	Font			*font;
	RectF			*fontRect[3];
	Bitmap			*cachedButton[3];
	StringFormat	*fontFormat;
	INT				fontSize;
	HICON			hIco;
	POINT			pIco;
	BYTE			sIco;
	INT				state;
	INT				pressed;
	INT				vRepeat;
	BOOL			vTimerFired;
} ButtonWndInfo;

void DrawImage(ButtonWndInfo *bi, ButtonClsInfo *bci, INT mode);
LRESULT CALLBACK DestButtonProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void DestResourceImages(HINSTANCE hInst, ButtonClsInfo * bci);
static Bitmap *CreateImage(HINSTANCE hInst, Bitmap * myImage);

#define MAX_WIDTH		400
#define MAX_HEIGHT		150

#define IDLE			0
#define PRESSED			1
#define DISABLED		2
#define PRESSED_LEFT	10

#define CLASSNAME		TEXT("WB_DESTBUTTON")

//////////////////////////////////////////////////////////////////////
int ButtonInit(HINSTANCE hInstance, BOOL vLoadUnload)
{
	WNDCLASSEX     wcex;
	ButtonClsInfo * bci;
	HWND            tmp;
	INT             ret, n;

	if(vLoadUnload)
	{
		wcex.cbSize			= sizeof(WNDCLASSEX);
		wcex.style			= 0;
		wcex.lpfnWndProc	= DestButtonProc;
		wcex.cbClsExtra		= sizeof(LONG_PTR);
		wcex.cbWndExtra		= sizeof(LONG_PTR);
		wcex.hInstance		= hInstance;
		wcex.hIcon			= NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= NULL;
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= CLASSNAME;
		wcex.hIconSm		= NULL;
		ret = RegisterClassEx(&wcex);

		tmp = CreateWindow(CLASSNAME, NULL, WS_DISABLED , 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
		bci = (ButtonClsInfo*)GlobalAlloc(GPTR, sizeof(ButtonClsInfo));
		if(bci)
		{
			SetClassLongPtr(tmp, 0, (LONG_PTR)bci);
			DestResourceImages(hInstance, bci);
			DestroyWindow(tmp);
			for(n = 0; n < 3; n++)
				bci->destButton[n] = CreateImage(hInstance, bci->destImage[n]);
			bci->fontColor[IDLE] = new SolidBrush(Color(255, 255, 255, 255));
			bci->fontColor[PRESSED] = new SolidBrush(Color(255, 0, 0, 0));
			bci->fontColor[DISABLED] = new SolidBrush(Color(255, 80, 80, 80));
			bci->defFont = new Font(L"Arial", (float)16);
		}
	}
	else
	{
		tmp = CreateWindow(CLASSNAME, NULL, WS_DISABLED , 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
		bci = (ButtonClsInfo*)GetClassLongPtr(tmp, 0);
		DestroyWindow(tmp);
		for(n = 0; n < 3; n++)
		{
			if(bci->destImage[n])
				delete bci->destImage[n];
			if(bci->destButton[n])
				delete bci->destButton[n];
			if(bci->fontColor[n])
				delete bci->fontColor[n];
		}
		if(bci->defFont)
			delete bci->defFont;
		GlobalFree(bci);
		ret = UnregisterClass(CLASSNAME, hInstance);
	}
	return(ret);
}

LRESULT CALLBACK DestButtonProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT     ps;
	ButtonWndInfo * bi;
	ButtonClsInfo * bci;
	LPCREATESTRUCT  cs;
	POINTS          pt;
	WCHAR           wt[100];
	Font           *ft;
	INT             n;

	wt[0] = L'\00';

	if(message == WM_CREATE)
	{
		bi = (ButtonWndInfo *)GlobalAlloc(GPTR, sizeof(ButtonWndInfo));
		if(bi == NULL)
			return(1L);	// Error
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)bi);

		cs = (LPCREATESTRUCT)lParam;
		bi->hParent = cs->hwndParent;
		bi->id = (int)cs->hMenu;
		bi->state = IDLE;
		bi->vRepeat = 0;
		bi->fontSize = 16;
		if((cs->cy / 3) < bi->fontSize)
		{
			bi->fontSize = cs->cy / 3;
			bi->font = new Font(L"Arial", (float)bi->fontSize);
		}
		bi->fontFormat = new StringFormat();
		bi->fontFormat->SetAlignment(StringAlignmentCenter);
		bi->fontFormat->SetTrimming(StringTrimmingNone);
		bi->fontFormat->SetLineAlignment(StringAlignmentCenter);
		bi->fontFormat->SetFormatFlags(StringFormatFlagsNoWrap);
		bi->fontRect[IDLE] = new RectF(4, 4, (float)cs->cx - 8, (float)cs->cy - 8);
		bi->fontRect[PRESSED] = new RectF(5, 5, (float)cs->cx - 8, (float)cs->cy - 8);
		bi->fontRect[DISABLED] = new RectF(5, 5, (float)cs->cx - 8, (float)cs->cy - 8);
		bi->pressed = FALSE;
		bi->btnGraphics = new Graphics(hWnd, FALSE);
		bi->pWnd.x = cs->cx;
		bi->pWnd.y = cs->cy;
		bi->cachedButton[IDLE] = bi->cachedButton[PRESSED] = bi->cachedButton[DISABLED] = NULL;

		bi->hIco = NULL;
		bi->sIco = ((cs->cx > cs->cy) ? cs->cy : cs->cx) < 48 ? 16 : 32;
		bi->pIco.x = (cs->cx - bi->sIco) >> 1;
		bi->pIco.y = (cs->cy - bi->sIco) >> 1;
		return(0);
	}

	bi = (ButtonWndInfo *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if(bi == NULL)
		return(DefWindowProc(hWnd, message, wParam, lParam));
	bci = (ButtonClsInfo*)GetClassLongPtr(hWnd, 0);

	if(bi->font)
		ft = bi->font;
	else
		ft = bci->defFont;

	switch (message)
	{
		case WM_DESTROY:
			if(bi->font) delete bi->font;
			if(bi->fontFormat) delete bi->fontFormat;
			for(n = 0; n < 3; n++)
			{
				if(bi->cachedButton[n]) delete bi->cachedButton[n];
				if(bi->fontRect[n]) delete bi->fontRect[n];
			}
			if(bi->hIco) DestroyIcon(bi->hIco);
			if(bi->btnGraphics) delete bi->btnGraphics;
			GlobalFree((void *)bi);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)0);
			break;

		case WM_LBUTTONDOWN:
			if(bi->state != DISABLED)
			{
				DrawImage(bi, bci, PRESSED);
				bi->hdc = GetDC(hWnd);
				if(bi->hIco) 
					DrawIconEx(bi->hdc, bi->pIco.x + 1, bi->pIco.y + 1, bi->hIco, bi->sIco, bi->sIco, NULL, NULL, DI_NORMAL);

				GetWindowTextW(hWnd, wt, 99);
				if(wt[0]) 
					bi->btnGraphics->DrawString(wt, -1, ft, *bi->fontRect[PRESSED], bi->fontFormat, bci->fontColor[PRESSED]);

				ReleaseDC(hWnd, bi->hdc);
				SetCapture(hWnd);
				bi->pressed = PRESSED;
				SendMessage(bi->hParent, WM_COMMAND, MAKEWPARAM(bi->id, BN_PUSHED), (LPARAM)hWnd);
				if(bi->vRepeat)
					SetTimer(hWnd, 1, bi->vRepeat, NULL);
				bi->vTimerFired = FALSE;
			}
			SendMessage(bi->hParent, WM_PARENTNOTIFY, MAKEWPARAM(message, bi->id), lParam);
			break;

		case WM_MOUSEMOVE:
			pt = MAKEPOINTS(lParam);
			if((pt.x < 0) || (pt.y < 0) || (pt.x >= bi->pWnd.x) || (pt.y >= bi->pWnd.y))
			{
				if(bi->pressed != PRESSED_LEFT)
				{
					bi->pressed = PRESSED_LEFT;
					SendMessage(hWnd, WM_PAINT, 0, 0);
				}
			}
			else if(bi->pressed == PRESSED_LEFT)
			{
				DrawImage(bi, bci, PRESSED);

				bi->hdc = GetDC(hWnd);
				if(bi->hIco) 
					DrawIconEx(bi->hdc, bi->pIco.x + 1, bi->pIco.y + 1, bi->hIco, bi->sIco, bi->sIco, NULL, NULL, DI_NORMAL);

				GetWindowTextW(hWnd, wt, 99);
				if(wt[0]) 
					bi->btnGraphics->DrawString(wt, -1, ft, *bi->fontRect[PRESSED], bi->fontFormat, bci->fontColor[PRESSED]);

				ReleaseDC(hWnd, bi->hdc);
				bi->pressed = PRESSED;
			}
			SendMessage(bi->hParent, WM_PARENTNOTIFY, MAKEWPARAM(message, bi->id), lParam);
			break;

		case WM_LBUTTONUP:
			if(bi->vRepeat)
				KillTimer(hWnd, 1);
			if((bi->pressed == PRESSED) && (bi->vTimerFired == FALSE))
				SendMessage(bi->hParent, WM_COMMAND, MAKEWPARAM(bi->id, BN_CLICKED), (LPARAM)hWnd);
			bi->pressed = IDLE;
			ReleaseCapture();
			SendMessage(hWnd, WM_PAINT, 0, 0);
			SendMessage(bi->hParent, WM_PARENTNOTIFY, MAKEWPARAM(message, bi->id), lParam);
			break;

		case WM_TIMER:
			if(bi->pressed == PRESSED)
			{
				bi->vTimerFired = TRUE;
				SendMessage(bi->hParent, WM_COMMAND, MAKEWPARAM(bi->id, BN_CLICKED), (LPARAM)hWnd);
			}
			break;

		case WM_PAINT:
			BeginPaint(hWnd, &ps);
			bi->hdc = GetDC(hWnd);
			DrawImage(bi, bci, bi->state);
			if(bi->hIco)
			{
				if(bi->state == IDLE)
					DrawIconEx(bi->hdc, bi->pIco.x, bi->pIco.y, bi->hIco, bi->sIco, bi->sIco, NULL, NULL, DI_NORMAL);
				else
					DrawIconEx(bi->hdc, bi->pIco.x + 1, bi->pIco.y + 1, bi->hIco, bi->sIco, bi->sIco, NULL, NULL, DI_NORMAL);
			}
			GetWindowTextW(hWnd, wt, 99);
			if(wt[0]) 
				bi->btnGraphics->DrawString(wt, -1, ft, *bi->fontRect[bi->state], bi->fontFormat, bci->fontColor[bi->state]);

			ReleaseDC(hWnd, bi->hdc);
			EndPaint(hWnd, &ps);
			break;

		case WM_SIZE:
			if(bi)
			{
				if(bi->btnGraphics) delete bi->btnGraphics;
				bi->btnGraphics = new Graphics(hWnd, FALSE);
				for(n = 0; n < 3; n++)
				{
					if(bi->cachedButton[n])
						delete bi->cachedButton[n];
					bi->cachedButton[n] = NULL;
					if(bi->fontRect[n])
						bi->fontRect[n]->Offset((REAL)(LOWORD(lParam) - bi->pWnd.x) / 2, (REAL)(HIWORD(lParam) - bi->pWnd.y) / 2);
				}
				bi->pIco.x = (LOWORD(lParam) - bi->sIco) >> 1;
				bi->pIco.y = (HIWORD(lParam) - bi->sIco) >> 1;

				bi->pWnd.x = LOWORD(lParam);
				bi->pWnd.y = HIWORD(lParam);
			}
			break;

		case BM_SETIMAGE:
			if(bi->hIco) DestroyIcon(bi->hIco);
			bi->hIco = NULL;

			if(wParam == IMAGE_BITMAP)
				bi->hIco = (HICON)CopyImage((HICON)lParam, IMAGE_BITMAP, bi->sIco, bi->sIco, LR_COPYFROMRESOURCE);
			else if(wParam == IMAGE_ICON)
				bi->hIco = (HICON)CopyImage((HICON)lParam, IMAGE_ICON, bi->sIco, bi->sIco, LR_COPYFROMRESOURCE);

			SendMessage(hWnd, WM_PAINT, 0, 0);
			break;

		case BM_SETCHECK:
			switch(wParam)
			{
				case BST_CHECKED:
					bi->state = PRESSED;
					break;
				case BST_UNCHECKED:
					bi->state = IDLE;
					break;
				case BST_INDETERMINATE:
					bi->state = DISABLED;
					break;
			}
			SendMessage(hWnd, WM_PAINT, 0, 0);
			break;

		case BM_GETCHECK:
			switch(bi->state)
			{
				case PRESSED:	return(BST_CHECKED);
				case IDLE:		return(BST_UNCHECKED);
				case DISABLED:	return(BST_INDETERMINATE);
			}
			break;

		case WB_GETTEXTSIZE:
			return(bi->fontSize);

		case WB_SETTEXTSIZE:
			if(lParam)
			{
				bi->fontSize = (INT)lParam;
				if(bi->font) delete bi->font;
				bi->font = new Font(L"Arial", (float)bi->fontSize);
			}
			break;

		case WM_USERALIGN:
			switch(wParam)
			{
				case ALIGNLEFT:		bi->fontFormat->SetAlignment(StringAlignmentNear);
					break;
				case ALIGNRIGHT:	bi->fontFormat->SetAlignment(StringAlignmentFar);
					break;
				default:			bi->fontFormat->SetAlignment(StringAlignmentCenter);
					break;
			}
			break;

		case WM_SETREPEAT:
			bi->vRepeat = (INT)lParam;
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}

void DrawImage(ButtonWndInfo *bi, ButtonClsInfo *bci, INT mode)
{
	INT       x, y, w1, w2, h1, h2;
	Graphics *pGr;

	w1 = bi->pWnd.x / 2;
	h1 = bi->pWnd.y / 2;
	w2 = bi->pWnd.x - w1;
	h2 = bi->pWnd.y - h1;

	if(bi->cachedButton[mode] == NULL)
	{
		bi->cachedButton[mode] = new Bitmap(bi->pWnd.x, bi->pWnd.y, PixelFormat24bppRGB);
		if(bi->cachedButton[mode] == NULL)
			return;

		pGr = new Graphics(bi->cachedButton[mode]);
		if(pGr == NULL)
		{
			delete bi->cachedButton[mode];
			bi->cachedButton[mode] = NULL;
			return;
		}

		/* Top left corner */
		x = y = 0;
		pGr->DrawImage(bci->destButton[mode], 0, 0, x, y, w1, h1, UnitPixel);
		/* Top right corner */
		x = MAX_WIDTH - w2;
		pGr->DrawImage(bci->destButton[mode], w1, 0, x, y, w2, h1, UnitPixel);
		/* Bottom right corner */
		y = MAX_HEIGHT - h2;
		pGr->DrawImage(bci->destButton[mode], w1, h1, x, y, w2, h2, UnitPixel);
		/* Bottom left corner */
		x = 0;
		pGr->DrawImage(bci->destButton[mode], 0, h1, x, y, w1, h2, UnitPixel);

		delete pGr;
	}

	if(bi->cachedButton[mode])
		bi->btnGraphics->DrawImage(bi->cachedButton[mode], 0, 0);
}

static Bitmap *CreateImage(HINSTANCE hInst, Bitmap * myImage)
{
	Bitmap * retBtn;
	Rect     srcRect;
	Color    pixColor;
	INT      x, y;

	srcRect.X = srcRect.Y = 0;
	srcRect.Width = myImage->GetWidth();
	srcRect.Height = myImage->GetHeight();

	// Create a scaled bitmap portion
	retBtn = new Bitmap(MAX_WIDTH, MAX_HEIGHT, PixelFormat24bppRGB);
	if(retBtn == NULL)
	{
		MessageBox(NULL, TEXT("Could not create new bitmap"), TEXT("Error"), MB_OK);
		return(NULL);
	}
	for(y = 0; y < (srcRect.Height >> 1); y++)
	{
		// Left upper corner
		for(x = 0; x < (srcRect.Width >> 1); x++)
		{
			myImage->GetPixel(x + srcRect.X, y + srcRect.Y, &pixColor);
			retBtn->SetPixel(x, y, pixColor);
		}
		// Mid upper edge
		myImage->GetPixel(srcRect.X + (srcRect.Width >> 1), y + srcRect.Y, &pixColor);
		for(x = (srcRect.Width >> 1); x < (MAX_WIDTH - (srcRect.Width >> 1)); x++)
		{
			retBtn->SetPixel(x, y, pixColor);
		}
		// Right upper corner
		for(x = (srcRect.Width >> 1); x < srcRect.Width; x++)
		{
			myImage->GetPixel(x + srcRect.X, y + srcRect.Y, &pixColor);
			retBtn->SetPixel(x + (MAX_WIDTH - srcRect.Width), y, pixColor);
		}
	}

	// Left edge
	for(x = 0; x < (srcRect.Width >> 1); x++)
	{
		myImage->GetPixel(x + srcRect.X, (srcRect.Width >> 1), &pixColor); //!!!
		for(y = (srcRect.Height >> 1); y < (MAX_HEIGHT - (srcRect.Height >> 1)); y++)
		{
			retBtn->SetPixel(x, y, pixColor);
		}
	}
	// Mid area
	myImage->GetPixel(srcRect.X + (srcRect.Width >> 1) - 1, (srcRect.Height >> 1) + srcRect.Y - 1, &pixColor);
	for(x = (srcRect.Width >> 1); x < (MAX_WIDTH - (srcRect.Width >> 1)); x++)
	{
		for(y = (srcRect.Height >> 1); y < (MAX_HEIGHT - (srcRect.Height >> 1)); y++)
		{
			retBtn->SetPixel(x, y, pixColor);
		}
	}

	// Right edge
	for(x = (srcRect.Width >> 1); x < srcRect.Width; x++)
	{
		myImage->GetPixel(x + srcRect.X, (srcRect.Height >> 1) + srcRect.Y, &pixColor);
		for(y = (srcRect.Height >> 1); y < (MAX_HEIGHT - (srcRect.Height >> 1)); y++)
		{
			retBtn->SetPixel(x + (MAX_WIDTH - srcRect.Height), y, pixColor);
		}
	}

	for(y = (srcRect.Height >> 1); y < srcRect.Height; y++)
	{
		// Left lower corner
		for(x = 0; x < (srcRect.Width >> 1); x++)
		{
			myImage->GetPixel(x + srcRect.X, y + srcRect.Y, &pixColor);
			retBtn->SetPixel(x, y + (MAX_HEIGHT - srcRect.Height), pixColor);
		}
		// Mid lower edge
		myImage->GetPixel(srcRect.X + (srcRect.Width >> 1), y + srcRect.Y, &pixColor);
		for(x = (srcRect.Width >> 1); x < (MAX_WIDTH - (srcRect.Width >> 1)); x++)
		{
			retBtn->SetPixel(x, y + (MAX_HEIGHT - srcRect.Height), pixColor);
		}
		// Right lower corner
		for(x = (srcRect.Width >> 1); x < srcRect.Width; x++)
		{
			myImage->GetPixel(x + srcRect.X, y + srcRect.Y, &pixColor);
			retBtn->SetPixel(x + (MAX_WIDTH - srcRect.Width), y + (MAX_HEIGHT - srcRect.Height), pixColor);
		}
	}
	return(retBtn);
}

static void DestResourceImages(HINSTANCE hInst, ButtonClsInfo * bci)
{
   LONG    retVal;
   DWORD   registryResult, idSize, regValueType;
   HKEY    regKeyHandle;
   WCHAR   files[3][MAX_PATH];
   Rect    sizes[3];

   /* Read the registry stuff */
   retVal = RegCreateKeyEx(HKEY_LOCAL_MACHINE,		//HKEY hKey,                        // handle to open key
							HKEY_WHITEBREAM,		//LPCTSTR lpSubKey,                 // subkey name
							0,						//DWORD Reserved,                   // reserved
							TEXT(""),				//LPTSTR lpClass,                   // class string
							REG_OPTION_NON_VOLATILE,//DWORD dwOptions,                  // special options
							KEY_QUERY_VALUE,		//REGSAM samDesired,                // desired security access
							NULL,					//LPSECURITY_ATTRIBUTES lpSecAttr,	// inheritance
							&regKeyHandle,			//PHKEY phkResult,                  // key handle
							&registryResult);		//LPDWORD lpdwDisposition           // disposition value buffer

   regValueType = REG_SZ;
   idSize = sizeof(files[0]);

   retVal = RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
							L"IdleFile",			//LPCTSTR lpValueName,  // value name
							NULL,					//LPDWORD lpReserved,   // reserved
							&regValueType,			//LPDWORD lpType,       // type buffer
							(LPBYTE)files[0],		//LPBYTE lpData,        // data buffer
							&idSize);				//LPDWORD lpcbData      // size of data buffer

   retVal = RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
							L"PressedFile",			//LPCTSTR lpValueName,  // value name
							NULL,					//LPDWORD lpReserved,   // reserved
							&regValueType,			//LPDWORD lpType,       // type buffer
							(LPBYTE)files[1],		//LPBYTE lpData,        // data buffer
							&idSize);				//LPDWORD lpcbData      // size of data buffer

   retVal = RegQueryValueExW(regKeyHandle,			//HKEY hKey,            // handle to key
							L"DisabledFile",		//LPCTSTR lpValueName,  // value name
							NULL,					//LPDWORD lpReserved,   // reserved
							&regValueType,			//LPDWORD lpType,       // type buffer
							(LPBYTE)files[2],		//LPBYTE lpData,        // data buffer
							&idSize);				//LPDWORD lpcbData      // size of data buffer

   if((retVal == ERROR_SUCCESS) && (registryResult == REG_OPENED_EXISTING_KEY))
   {
		bci->destImage[0] = new Bitmap(files[0]);
		bci->destImage[1] = new Bitmap(files[1]);
		bci->destImage[2] = new Bitmap(files[2]);
   }
   else
   {
		bci->destImage[0] = new Bitmap(hInst, (WCHAR *)IDB_DESTUP);
		bci->destImage[1] = new Bitmap(hInst, (WCHAR *)IDB_DESTDOWN);
		bci->destImage[2] = new Bitmap(hInst, (WCHAR *)IDB_DESTOFF);
   }
   RegCloseKey(regKeyHandle);
}


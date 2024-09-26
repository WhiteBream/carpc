// Spectrum.cpp: implementation of the Spectrum library.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Spectrum.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define BARCOUNT		7

typedef struct _SpectrumInfo
{
	INT		  id;
	HWND      hParent;
	HDC       hdc;
	Graphics *g;
	Graphics * graph;
	Image    * image;
	Color    *back;
	Brush    *white;
	LinearGradientBrush *paint;
	Pen      *pn;
	Rect      rf[BARCOUNT];
	RECT      wr;
} SpectrumInfo, * pSpectrumInfo;

LRESULT CALLBACK SpectrumProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////////////////////////
INT SpectrumInit(HINSTANCE hInstance)
{
	WNDCLASSEX     wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= SpectrumProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(LONG_PTR);
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("WB_SPECTRUM");
	wcex.hIconSm		= NULL;

	return(RegisterClassEx(&wcex));
}

LRESULT CALLBACK SpectrumProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT         ps;
	LPCREATESTRUCT      cs;
	pSpectrumInfo	    ti;
	Rect				ss;
	INT					i;
	INT 				x, w;
	BYTE				vBuf[10];
	const CHAR			vBarSize[7] = {70, 70, 70, 70, 60, 50, 40};

	ti = (pSpectrumInfo)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (message)
	{
		case WM_CREATE:
			ti = (pSpectrumInfo)GlobalAlloc(GPTR, sizeof(SpectrumInfo));
			if(ti == NULL)
				return(1L);	// Error
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)ti);
			cs = (LPCREATESTRUCT)lParam;
			ti->hParent = cs->hwndParent;
			ti->id = (INT)cs->hMenu;
			// GDI+ drawing class
			ti->wr.left = ti->wr.top = 0;
			ti->wr.bottom = cs->cy;
			ti->wr.right = cs->cx;
			ti->g = new Graphics(hWnd, FALSE);
			ti->image = new Bitmap(ti->wr.right, ti->wr.bottom);
			ti->graph = new Graphics(ti->image);
			ti->hdc = GetDC(hWnd);
			// Background brushes & pen
			ti->pn = new Pen(Color(255, 0, 0, 0), 1);
			ti->back = new Color(255, 255, 255, 255);
			ti->white = new SolidBrush(Color(255, 255, 255, 255));
			ti->paint = new LinearGradientBrush(Point(0, ti->wr.bottom - 2), Point(0, 3), Color(255, 0, 255, 0), Color(255, 255, 0, 0));

			PipeIo("MXwP1", 6, vBuf, sizeof(vBuf));
			SetTimer(hWnd, 1, 100, NULL);
			break;

		case WM_DESTROY:
			PipeIo("MXwP0", 6, vBuf, sizeof(vBuf));
			if(ti->hdc) ReleaseDC(hWnd, ti->hdc);
			KillTimer(hWnd, 1);
			if(ti->g) delete ti->g;
			if(ti->graph) delete ti->graph;
			if(ti->image) delete ti->image;
			if(ti->back) delete ti->back;
			if(ti->white) delete ti->white;
			if(ti->paint) delete ti->paint;
			if(ti->pn) delete ti->pn;
			GlobalFree((HGLOBAL)ti);
			ti = NULL;
			break;

		case WM_SIZE:
			GetClientRect(hWnd, &ti->wr);
			// Update GDI+ drawing class
			if(ti->g) delete ti->g;
			ti->g = new Graphics(hWnd, FALSE);
			if(ti->image) delete ti->image;
			ti->image = new Bitmap(ti->wr.right, ti->wr.bottom);
			if(ti->graph) delete ti->graph;
			ti->graph = new Graphics(ti->image);

			x = (ti->wr.right - (2 * ti->wr.right / (3 * BARCOUNT))) / BARCOUNT;
			w = x - ti->wr.right / (3 * BARCOUNT);//* 0.7;
			for(i = 0; i < BARCOUNT; i++)
			{
				ti->rf[i].X = i * x + ti->wr.right / (3 * BARCOUNT);
				ti->rf[i].X += 1 + i;
				ti->rf[i].Width = w;
				ti->rf[i].Y = 3;
				ti->rf[i].Height = ti->wr.bottom - 5;
			}
			if(ti->paint) delete ti->paint;
			ti->paint = new LinearGradientBrush(Point(0, ti->wr.bottom - 2), Point(0, 3), Color(255, 0, 255, 0), Color(255, 255, 0, 0));
			break;

		case WM_PAINT:
			// Do the initial displaying part
			ti->hdc = BeginPaint(hWnd, &ps);
			ti->g->ResetClip();
			ti->g->Clear(*ti->back);
			ti->g->DrawRectangle(ti->pn, 0, 0, ti->wr.right - 1, ti->wr.bottom - 1);
			ti->g->SetClip(Rect(1, 1, ti->wr.right - 2, ti->wr.bottom - 2));
			EndPaint(hWnd, &ps);
			break;

		case WM_TIMER:
			// Get data
			if(!PipeIo("MXrP", 5, vBuf, sizeof(vBuf)))
				for(i = 0; i < BARCOUNT; i++) vBuf[i + 2] = 0;

			// Do the displaying part
			for(i = 0; i < BARCOUNT; i++)
			{
				INT val = vBuf[i + 2] * (ti->rf[i].Height - 1) / vBarSize[i];

				if(val >= ti->rf[i].Height) val = ti->rf[i].Height -1;
				w = ti->rf[i].Height - val;
				if(w < 0) w = 0;
				x = w;
				ti->graph->FillRectangle(ti->white, ti->rf[i].X, ti->rf[i].Y, ti->rf[i].Width, w);
				w = ti->rf[i].Height - x;
				ti->graph->FillRectangle(ti->paint, ti->rf[i].X, ti->rf[i].Y + x, ti->rf[i].Width, w);
			}
			ti->g->DrawImage(ti->image, 0, 0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return(0);
}




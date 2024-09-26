// CdPlayer.cpp: implementation of the CdPlayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CdPlayer.h"
#include "Ticker.h"

#include "commctrl.h"

#pragma comment(lib, "Strmiids.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

typedef struct _ComingUp
{
   LPWSTR   szFile;
} ComingUp, *pComingUp;

#define BG_WHITE     0
#define BG_GREEN     1
#define BG_LBLUE     2
#define BG_DBLUE     3

int NowPlaying(void * cbParam, int colCount, char ** colData, char ** colNames);
LRESULT CALLBACK CdPlayerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static void DrawListBoxItem(LPDRAWITEMSTRUCT pItem);
static void InitListBox(BOOL loadUnload, HWND lbox);

static CdPlayer * myPtr;

static Database * dBase;
static WCHAR      szCurrent[66];
static WCHAR      szInfo[66];
static WCHAR      szFile[1024];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CdPlayer::CdPlayer(HINSTANCE hInstance, HWND hParent, int bottomOffset, Database * pDB)
{
   RECT  pRect;
   HICON ico;
   FontFamily * fm;

   myPtr = this;

   hInst = hInstance;
   hPrnt = hParent;
   offset = bottomOffset;
   row1 = (2 * bottomOffset) / 3;
   row2 = (5 * bottomOffset) / 2;
   myGraphics = NULL;
   dBase = pDB;
   autoLast = 0;
   muted = FALSE;
   saved = FALSE;

   pGraph = NULL;
   pConfig = NULL;
   pControl = NULL;
   pEvent = NULL;
   pSeek = NULL;
   pWcontrol = NULL;
   pVmr = (IBaseFilter *)-1;
   pAudio = NULL;

   hHeap = GetProcessHeap();
   MyRegisterClass(hInstance);
   TickerInit(hInstance);
   GetClientRect(hParent, &pRect);

   bg[BG_WHITE] = new SolidBrush(Color(255, 255, 255, 255));
   bg[BG_GREEN] = new SolidBrush(Color(255, 200, 255, 200));
   bg[BG_LBLUE] = new SolidBrush(Color(255, 201, 211, 227));
   bg[BG_DBLUE] = new SolidBrush(Color(255, 0, 0, 192));
   pn = new Pen(Color(255, 0, 0, 0), 1);

   fm = new FontFamily(L"Arial");
   fn[0] = new Font(fm, (float)row1 + 2, FontStyleRegular, UnitPixel);
   fn[1] = new Font(fm, (float)(bottomOffset >> 1) - 7, FontStyleRegular, UnitPixel);
   delete fm;

   hPlay = CreateWindow(L"CRPC_PLYR", L"CarPC play window", WS_CHILD | WS_CLIPCHILDREN,
      0, 0, pRect.right, pRect.bottom - bottomOffset, hParent, NULL, hInstance, NULL);

   CreateWindow(L"WB_DESTBUTTON", NULL, WS_CHILD | WS_VISIBLE, 0 * bottomOffset, row2,
      bottomOffset, bottomOffset, hPlay, (HMENU)IDC_PREV, hInstance, NULL);
   ico = LoadIcon(hInstance, (LPCTSTR)IDI_PREV);
   SendDlgItemMessage(hPlay, IDC_PREV, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
   DestroyIcon(ico);

   CreateWindow(L"WB_DESTBUTTON", NULL, WS_CHILD | WS_VISIBLE, 1 * bottomOffset, row2,
      bottomOffset, bottomOffset, hPlay, (HMENU)IDC_PLAY, hInstance, NULL);
   ico = LoadIcon(hInstance, (LPCTSTR)IDI_PLAY);
   SendDlgItemMessage(hPlay, IDC_PLAY, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
   DestroyIcon(ico);

   CreateWindow(L"WB_DESTBUTTON", NULL, WS_CHILD | WS_VISIBLE, 2 * bottomOffset, row2,
      bottomOffset, bottomOffset, hPlay, (HMENU)IDC_STOP, hInstance, NULL);
   ico = LoadIcon(hInstance, (LPCTSTR)IDI_STOP);
   SendDlgItemMessage(hPlay, IDC_STOP, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
   DestroyIcon(ico);

   CreateWindow(L"WB_DESTBUTTON", NULL, WS_CHILD | WS_VISIBLE, 3 * bottomOffset, row2,
      bottomOffset, bottomOffset, hPlay, (HMENU)IDC_PAUSE, hInstance, NULL);
   ico = LoadIcon(hInstance, (LPCTSTR)IDI_PAUSE);
   SendDlgItemMessage(hPlay, IDC_PAUSE, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
   DestroyIcon(ico);

   CreateWindow(L"WB_DESTBUTTON", NULL, WS_CHILD | WS_VISIBLE, 4 * bottomOffset, row2,
      bottomOffset, bottomOffset, hPlay, (HMENU)IDC_NEXT, hInstance, NULL);
   ico = LoadIcon(hInstance, (LPCTSTR)IDI_NEXT);
   SendDlgItemMessage(hPlay, IDC_NEXT, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
   DestroyIcon(ico);

   CreateWindow(L"WB_DESTBUTTON", L"XF", WS_CHILD | WS_VISIBLE, pRect.right - 3 * bottomOffset, 0,
      bottomOffset, row1, hPlay, (HMENU)IDC_XFADE, hInstance, NULL);
   SendDlgItemMessage(hPlay, IDC_XFADE, BM_SETCHECK, BST_INDETERMINATE, 0);

   CreateWindow(L"WB_DESTBUTTON", L"SHF", WS_CHILD | WS_VISIBLE, pRect.right - 2 * bottomOffset, 0,
      bottomOffset, row1, hPlay, (HMENU)IDC_SHUFFLE, hInstance, NULL);

   CreateWindow(L"WB_DESTBUTTON", L"RPT", WS_CHILD | WS_VISIBLE, pRect.right - 1 * bottomOffset, 0,
      bottomOffset, row1, hPlay, (HMENU)IDC_REPEAT, hInstance, NULL);
   SendDlgItemMessage(hPlay, IDC_REPEAT, BM_SETCHECK, BST_INDETERMINATE, 0);

   CreateWindow(L"WB_DESTBUTTON", NULL, WS_CHILD | WS_VISIBLE, pRect.right - 3 * bottomOffset, row2,
      bottomOffset, bottomOffset, hPlay, (HMENU)IDC_PLAYNOW, hInstance, NULL);
   ico = LoadIcon(hInstance, (LPCTSTR)IDI_PLAYNOW);
   SendDlgItemMessage(hPlay, IDC_PLAYNOW, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
   DestroyIcon(ico);
   SendDlgItemMessage(hPlay, IDC_PLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);

   CreateWindow(L"WB_DESTBUTTON", NULL, WS_CHILD | WS_VISIBLE, pRect.right - 2 * bottomOffset, row2,
      bottomOffset, bottomOffset, hPlay, (HMENU)IDC_PLAYNEXT, hInstance, NULL);
   ico = LoadIcon(hInstance, (LPCTSTR)IDI_PLAYNEXT);
   SendDlgItemMessage(hPlay, IDC_PLAYNEXT, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
   DestroyIcon(ico);
   SendDlgItemMessage(hPlay, IDC_PLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);

   CreateWindow(L"WB_DESTBUTTON", NULL, WS_CHILD | WS_VISIBLE, pRect.right - 1 * bottomOffset, row2,
      bottomOffset, bottomOffset, hPlay, (HMENU)IDC_REMOVE, hInstance, NULL);
   ico = LoadIcon(hInstance, (LPCTSTR)IDI_DEL);
   SendDlgItemMessage(hPlay, IDC_REMOVE, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
   DestroyIcon(ico);
   SendDlgItemMessage(hPlay, IDC_REMOVE, BM_SETCHECK, BST_INDETERMINATE, 0);

   CreateWindow(L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_OWNERDRAWFIXED | WS_VSCROLL | LBS_HASSTRINGS, 1,  row2 + bottomOffset + 1,
      pRect.right - 2, pRect.bottom - 2 * bottomOffset - row2 - 2, hPlay, (HMENU)IDC_COMINGNEXT, hInstance, NULL);
   InitListBox(TRUE, GetDlgItem(hPlay, IDC_COMINGNEXT));
   SendDlgItemMessage(hPlay, IDC_COMINGNEXT, LB_SETITEMHEIGHT, 0, (LPARAM)24);

   CreateWindow(L"WB_DESTBUTTON", L"MP3", WS_CHILD | WS_VISIBLE, 0 * bottomOffset, 0,
      bottomOffset, row1, hPlay, (HMENU)IDC_MODEMP3, hInstance, NULL);
   SendDlgItemMessage(hPlay, IDC_MODEMP3, BM_SETCHECK, BST_CHECKED, 0);

   CreateWindow(L"WB_DESTBUTTON", NULL, WS_CHILD | WS_VISIBLE, 1 * bottomOffset, 0,
      bottomOffset, row1, hPlay, (HMENU)IDC_MODECD, hInstance, NULL);
   ico = LoadIcon(hInstance, (LPCTSTR)IDI_CD);
   SendDlgItemMessage(hPlay, IDC_MODECD, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
   DestroyIcon(ico);
   SendDlgItemMessage(hPlay, IDC_MODECD, BM_SETCHECK, BST_INDETERMINATE, 0);

   CreateWindow(L"WB_DESTBUTTON", L"FM", WS_CHILD | WS_VISIBLE, 2 * bottomOffset, 0,
      bottomOffset, row1, hPlay, (HMENU)IDC_MODERADIO, hInstance, NULL);
   SendDlgItemMessage(hPlay, IDC_MODERADIO, BM_SETCHECK, BST_INDETERMINATE, 0);

   CreateWindow(L"WB_DESTSLIDER", NULL, WS_CHILD | WS_VISIBLE, 3 * bottomOffset + 2, 3 * row1 - 5,
      ourRect.right - 3 * bottomOffset - 4, (bottomOffset >> 1) + 2, hPlay, (HMENU)IDC_SEEK, hInstance, NULL);
   SendDlgItemMessage(hPlay, IDC_SEEK, TBM_SETRANGE, TRUE, MAKELONG(0, 10000));

   CreateWindow(L"WB_TICKERBOX", NULL, WS_CHILD | WS_VISIBLE, 3 * bottomOffset + 2, row1 + 2,
      ourRect.right - 3 * bottomOffset - 4, row1 + 6, hPlay, (HMENU)IDC_TICKER, hInstance, NULL);

   CreateWindow(L"WB_TICKERBOX", NULL, WS_CHILD | WS_VISIBLE, 3 * bottomOffset + 2, 2 * row1 + 7,
      ourRect.right - 3 * offset - 4, (bottomOffset >> 1) - 8, hPlay, (HMENU)IDC_SUBTICKER, hInstance, NULL);

   pPopup = new Popup(hInstance, hPlay, bottomOffset, &hPopup);

   HRESULT hr = CoInitialize(NULL);
   if(FAILED(hr))
   {
      MessageBox(NULL, L"CoInit failed", L"Error", MB_OK);
   }
   else
   {
      DShowInit(TRUE);
      tTime = 0;
      pSource = NULL;

      // By now every should have been loaded...
      RetrieveSettings();
      FillPlayList();
   }
}

CdPlayer::~CdPlayer()
{
   InitListBox(FALSE, NULL);
   delete pPopup;
   delete bg[BG_GREEN];
   delete bg[BG_DBLUE];
   delete bg[BG_LBLUE];
   delete bg[BG_WHITE];
   delete pn;
   delete fn[0];
   delete fn[1];
   DShowInit(FALSE);
   CoUninitialize();
   DestroyWindow(hPlay);
}

CdPlayer::DShowInit(BOOL loadUnload)
{
   IMediaEventEx * pEventEx;
    IVMRFilterConfig* pVmrConf;
   HRESULT hr;

   HANDLE logFile;

   logFile = CreateFile(L"DSHOW.LOG", FILE_ALL_ACCESS, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

   if(loadUnload)
   {
      hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
            IID_IGraphBuilder, (void **)&pGraph);
      if(FAILED(hr))
      {
         MessageBox(NULL, L"IID_IGraphBuilder failed", L"Error", MB_OK);
         return hr;
      }
      pGraph->SetLogFile((DWORD_PTR)logFile);

      hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
      if(FAILED(hr))
      {
         MessageBox(NULL, L"IID_IMediaControl failed", L"Error", MB_OK);
      }
      hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
      if(FAILED(hr))
      {
         MessageBox(NULL, L"IID_IMediaEvent failed", L"Error", MB_OK);
         return hr;
      }
      hr = pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pSeek);
      if(FAILED(hr))
      {
         MessageBox(NULL, L"IID_IMediaSeeking failed", L"Error", MB_OK);
         return hr;
      }
      hr = pGraph->QueryInterface(IID_IGraphConfig, (void **)&pConfig);
      if(FAILED(hr))
      {
         MessageBox(NULL, L"IID_IGraphConfig failed", L"Error", MB_OK);
         return hr;
      }
      hr = pGraph->QueryInterface(IID_IFilterGraph, (void **)&pFilter);
      if(FAILED(hr))
      {
         MessageBox(NULL, L"IID_IFilterGraph failed", L"Error", MB_OK);
         return hr;
      }
      hr = pGraph->QueryInterface(IID_IMediaEventEx, (void **)&pEventEx);
      if(FAILED(hr))
      {
         MessageBox(NULL, L"IID_IMediaEventEx failed", L"Error", MB_OK);
         return hr;
      }
      pEventEx->SetNotifyWindow((long)hPlay, WM_DIRECTSHOW, NULL);
      pEventEx->Release();
      hr = pGraph->QueryInterface(IID_IBasicAudio, (void **)&pAudio);
      if(FAILED(hr))
      {
         MessageBox(NULL, L"IID_IBasicAudio failed", L"Error", MB_OK);
         return hr;
      }
      pAudio->put_Volume(0);


      // Create the VMR.
      hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL,
         CLSCTX_INPROC, IID_IBaseFilter, (void**)&pVmr);
      if(FAILED(hr))
      {
         MessageBox(NULL, L"CLSID_VideoMixingRenderer failed", L"Error", MB_OK);
         return hr;
      }
      // Add the VMR to the filter graph.
      hr = pGraph->AddFilter(pVmr, L"Video Mixing Renderer");
      if(FAILED(hr))
      {
         MessageBox(NULL, L"AddFilter on VMR failed", L"Error", MB_OK);
         pVmr->Release();
         return hr;
      }
      // Set the rendering mode.
      hr = pVmr->QueryInterface(IID_IVMRFilterConfig, (void**)&pVmrConf);
      if(SUCCEEDED(hr))
      {
         hr = pVmrConf->SetRenderingMode(VMRMode_Windowless);
         if(FAILED(hr))
         {
            MessageBox(NULL, L"SetRenderingMode failed", L"Error", MB_OK);
            return(hr);
         }
         pVmrConf->Release();
      }
      else
      {
         MessageBox(NULL, L"IID_IVMRFilterConfig failed", L"Error", MB_OK);
         return(hr);
      }

      hr = pVmr->QueryInterface(IID_IVMRWindowlessControl, (void**)&pWcontrol);
        if(FAILED(hr))
      {
         MessageBox(NULL, L"IID_IVMRWindowlessControl failed", L"Error", MB_OK);
         return(hr);
      }
      // Find the native video size.
      hr = pWcontrol->SetAspectRatioMode(VMR_ARMODE_LETTER_BOX);
        if(FAILED(hr))
      {
         MessageBox(NULL, L"SetAspectRatioMode failed", L"Error", MB_OK);
         return(hr);
      }
      // Set the target window temporary to ours
      hr = pWcontrol->SetVideoClippingWindow(hPlay);
        if(FAILED(hr))
      {
         MessageBox(NULL, L"SetVideoClippingWindow failed", L"Error", MB_OK);
         return(hr);
      }
       pVmr->Release();
      pVmr = NULL;
      return(hr);
   }
   else
   {
      if(pControl)  pControl->Stop();

      if(pAudio)    pAudio->Release(); pAudio = NULL;
      if(pSource)   pSource->Release();   pSource = NULL;
      if(pWcontrol) pWcontrol->Release(); pWcontrol = NULL;
      if(pEvent)    pEvent->Release();    pEvent = NULL;
      if(pSeek)     pSeek->Release();     pSeek = NULL;
      if(pControl)  pControl->Release();  pControl = NULL;
      if(pFilter)   pFilter->Release();   pFilter = NULL;
      if(pConfig)   pConfig->Release();   pConfig = NULL;
      if(pGraph)    pGraph->Release();    pGraph = NULL;
      return(0);
   }
}

CdPlayer::Show(BOOL yesNo)
{
   if(yesNo)
   {
      ShowWindow(hPlay, SW_SHOW);
      UpdateWindow(hPlay);
      PostMessage(hPlay, WM_PAINT, 0, 0);
   }
   else
   {
      ShowWindow(hPlay, SW_HIDE);
   }
}

ATOM CdPlayer::MyRegisterClass(HINSTANCE hInstance)
{
   WNDCLASSEX wcex;

   wcex.cbSize = sizeof(WNDCLASSEX);
   wcex.style        = CS_HREDRAW | CS_VREDRAW;
   wcex.lpfnWndProc  = CdPlayerProc;
   wcex.cbClsExtra      = 0;
   wcex.cbWndExtra      = 0;
   wcex.hInstance    = hInstance;
   wcex.hIcon        = NULL;
   wcex.hCursor      = LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground   = NULL;
   wcex.lpszMenuName = NULL;
   wcex.lpszClassName   = L"CRPC_PLYR";
   wcex.hIconSm      = NULL;

   return RegisterClassEx(&wcex);
}

void CdPlayer::InitDC(BOOL updateDC)
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

void CdPlayer::Paint()
{
   RECT  lbRect;

   if(myGraphics == NULL)
      return;

   myGraphics->FillRectangle(bg[BG_DBLUE], 0, 0, ourRect.right, row1);
   myGraphics->FillRectangle(bg[BG_GREEN], 0, row1, ourRect.right, ourRect.bottom - row1);

   // Draw the rectangle around the NowPlaying listbox
   GetClientRect(GetDlgItem(hPlay, IDC_COMINGNEXT), &lbRect);
   myGraphics->DrawRectangle(pn, 0,  row2 + offset, ourRect.right - 1, lbRect.bottom + 1);

   if(szCurrent[0] != L'\00')
   {
      SetDlgItemText(hPlay, IDC_TICKER, szCurrent);
      SetWindowText(hPopup, szCurrent);
   }
   else
   {
      SetDlgItemText(hPlay, IDC_TICKER, L"CarPC media player");
      SetWindowText(hPopup, L"CarPC media player");
   }

   if(szInfo[0] != L'\00')
      SetDlgItemText(hPlay, IDC_SUBTICKER, szInfo);
   else
      SetDlgItemText(hPlay, IDC_SUBTICKER, L"");

   PaintTime();
}

void CdPlayer::PaintTime()
{
   PointF point;
   REFERENCE_TIME units;
   int pos, time;
   WCHAR  vBuffer[20];
   StringFormat fmt;
   HICON ico;
   OAFilterState  state;

   fmt.SetAlignment(StringAlignmentFar);
   fmt.SetTrimming(StringTrimmingNone);
   fmt.SetFormatFlags(StringFormatFlagsNoWrap);

   if(myGraphics == NULL)
      return;

   if(pSeek) pSeek->GetDuration(&tTime);

   myGraphics->FillRectangle(bg[BG_WHITE], 2, row1 + 2, 3 * offset - 3, row2 - row1 - 5);
   myGraphics->DrawRectangle(pn, 2, row1 + 2, 3 * offset - 3, row2 - row1 - 5);

   // Print total track time
   point.X = (float)3 * offset - 4;
   point.Y = (float)(3 * row1 - 2);
   time = (int)(tTime / 10000000);  // from 100ns to 1s
   if(time > 3600)
      wsprintfW(vBuffer, L"%ld:%02ld:%02ld", time / 3600, (time % 3600) / 60, time % 60);
   else
      wsprintfW(vBuffer, L"%ld:%02ld", time / 60, time % 60);
   myGraphics->DrawString(vBuffer, -1, fn[1], point, &fmt, bg[BG_DBLUE]);

   // Print remaining track time
   if(pSeek) pSeek->GetCurrentPosition(&units); else units = 0;
   point.Y = (float)(2 * row1 + 9);
   pos = (int)(units / 10000000);   // from 100ns to 1s
   if(time > 3600)
      wsprintfW(vBuffer, L"-%ld:%02ld:%02ld", (time - pos) / 3600, ((time - pos) % 3600) / 60, (time - pos) % 60);
   else
      wsprintfW(vBuffer, L"-%ld:%02ld", (time - pos) / 60, (time - pos) % 60);
   myGraphics->DrawString(vBuffer, -1, fn[1], point, &fmt, bg[BG_DBLUE]);

   // Print current track position
   point.X = (float)3 * offset;
   point.Y = (float)(row1 + 2);
   if(time > 3600)
      wsprintfW(vBuffer, L"%ld:%02ld:%02ld", pos / 3600, (pos % 3600) / 60, pos % 60);
   else
      wsprintfW(vBuffer, L"%ld:%02ld", pos / 60, pos % 60);
   myGraphics->DrawString(vBuffer, -1, fn[0], point, &fmt, bg[BG_DBLUE]);

   if(tTime)
      SendDlgItemMessage(hPlay, IDC_SEEK, TBM_SETPOS, TRUE, (pos * 10000) / time);
   else
      SendDlgItemMessage(hPlay, IDC_SEEK, TBM_SETPOS, TRUE, 0);

   if(myPtr->pControl)
   {
      myPtr->pControl->GetState(10, &state);
      switch(state)
      {
         case State_Paused:
            ico = LoadIcon(hInst, (LPCTSTR)IDI_PAUSED);
            break;
         case State_Running:
            ico = LoadIcon(hInst, (LPCTSTR)IDI_PLAYING);
            break;
         case State_Stopped:
         default:
            ico = LoadIcon(hInst, (LPCTSTR)IDI_STOPPED);
            break;
      }
      Bitmap bitmap(ico);
      myGraphics->DrawImage(&bitmap, 5, row2 - offset + 5);
      DestroyIcon(ico);
   }

   // Restart refresh timer
   SetTimer(hPlay, 1, 1000, NULL);
}

CdPlayer::FillPlayList()
{
   LPSTR    pFile, pDisplay, pTemp;
   LPWSTR      pText;
   INT         i, n, size;
   RECT     lbRect;
   LONG     idNew;
   pComingUp   pNew;

   GetWindowRect(GetDlgItem(hPlay, IDC_COMINGNEXT), &lbRect);
   lbRect.bottom -= lbRect.top;
   i = SendDlgItemMessage(hPlay, IDC_COMINGNEXT, LB_GETITEMHEIGHT, 0, 0);
   if(i > 0)
      n = lbRect.bottom / i;
   else n = 0;

   // Limit dbase search to 256 queries
   i = 0;
   while((SendDlgItemMessage(hPlay, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0) < n) && (i++ < 256))
   {
      if(SendDlgItemMessage(hPlay, IDC_SHUFFLE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)   // NO shuffle
      {
         pFile = dBase->GetAutoPlay(FALSE, &autoLast, &pDisplay);
      }
      else
      {
         pFile = dBase->GetAutoPlay(TRUE, &autoLast, &pDisplay);
      }

      if(pFile == NULL)
      {
         pFile = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 6);
         sprintf(pFile, "null");
      }
      else if((pTemp = strrchr(pFile, '.')) != NULL)
      {
         // Only allow mp3 files in auto play
         if(strcmp(pTemp, ".MP3") == 0)   /* (extensions are uppercased by settings.cpp, this should change one day!!!) */
         {
            if(pDisplay == NULL)
            {
               size = strlen(pFile) + 1;
               pDisplay = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, size);
               sprintf(pDisplay, pFile);
            }

            size = 2 * strlen(pDisplay) + 2;
            pText = (LPWSTR)HeapAlloc(myPtr->hHeap, HEAP_ZERO_MEMORY, size);
            wsprintf(pText, L"%hs", pDisplay);
            HeapFree(hHeap, 0, pDisplay);
            idNew = SendDlgItemMessage(hPlay, IDC_COMINGNEXT, LB_ADDSTRING, NULL, (LPARAM)pText);

            pNew = (pComingUp)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(ComingUp));
            size = 2 * strlen(pFile) + 2;
            pNew->szFile = (LPWSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, size);
            wsprintf(pNew->szFile, L"%hs", pFile);
            HeapFree(hHeap, 0, pFile);
            SendDlgItemMessage(hPlay, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)idNew, (LPARAM)pNew);
         }
      }
   }
   if((SendDlgItemMessage(hPlay, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0) - SendDlgItemMessage(hPlay, IDC_COMINGNEXT, LB_GETTOPINDEX, 0, 0)) < n)
      SendDlgItemMessage(hPlay, IDC_COMINGNEXT, LB_SETTOPINDEX, (WPARAM)(SendDlgItemMessage(hPlay, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0) - n), 0);
}

CdPlayer::PlayNow(LPWSTR pFile)
{
   REFERENCE_TIME pos = 0;
   HRESULT        hr;
   FILE         * f;
   CHAR           sTag[4] = "";
   CHAR           sArtist[31] = "";
   CHAR           sTitle[31] = "";
   LPSTR          sFile;
   LONG           lWidth, lHeight;
   LPWSTR         szSong = NULL;
   INT            i;

   DShowInit(FALSE);
   DShowInit(TRUE);

   // Clear current title
   for(i = 0; i < sizeof(szCurrent) / sizeof(WCHAR); i++) szCurrent[i] = L'\00';
   for(i = 0; i < sizeof(szInfo) / sizeof(WCHAR); i++) szInfo[i] = L'\00';

   sFile = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lstrlen(pFile) + 2);
   sprintf(sFile, "%ws", pFile);
   wsprintf(szFile, L"%s", pFile);

   f = fopen(sFile, "r");
   HeapFree(hHeap, 0, sFile);
   if(f != NULL)
   {
      fseek(f, -128, SEEK_END);
      fread(sTag, 3, 1, f);
      if(strncmp(sTag, "TAG", 3) == 0)
      {
         // Read the artist and track name
         fread(sTitle, 30, 1, f);
         Strip(sTitle);
         fread(sArtist, 30, 1, f);
         Strip(sArtist);
         wsprintf(szCurrent, L"%hs - %hs", sArtist, sTitle);

         for(i = 0; i < 31; i++) sArtist[i] = '\0';
         for(i = 0; i < 31; i++) sTitle[i] = '\0';
         // Read the album name and year
         fread(sArtist, 30, 1, f);
         Strip(sArtist);
         fread(sTitle, 4, 1, f);
         if(sArtist[0] && sTitle[0])
            wsprintf(szInfo, L" %hs - %hs", sArtist, sTitle);
         else if(sArtist[0])
            wsprintf(szInfo, L" %hs", sArtist);
         else if(sTitle[0])
            wsprintf(szInfo, L" %hs", sTitle);
         else
            wsprintf(szInfo, L"");
      }
      fclose(f);

      if(szCurrent[0] == L'\00')
      {
         // Remove the directory name
         szSong = wcsrchr(szFile, L'\\');
         szSong++;
         // Print up to 64 characters
         wsprintf(szCurrent, L"%-64ls",  szSong);
         // Remove the extension
         szSong = wcsrchr(szCurrent, L'.');
         *szSong = L'\00';
      }

      if(pGraph && szFile)
         pGraph->RenderFile(szFile, 0);
      if(pSeek)
      {
         pSeek->GetDuration(&tTime);
         pSeek->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
      }
      Paint();

      if(pWcontrol)
      {
         hr = pWcontrol->GetNativeVideoSize(&lWidth, &lHeight, 0, 0);
         if((hr != E_NOINTERFACE) && (lWidth != 0) && (lHeight != 0))
         {
            // If this video is encoded with an unsupported codec,
            // we won't see any video, although the audio will work if it is
            // of a supported format.
            SendMessage(hPrnt, WM_SHOWVIDEOWINDOW, 0, 0);
         }
      }

      pPopup->Show(TRUE);

      if(pAudio)
         pAudio->put_Volume(muted ? -2000 : 0);

      if(pControl)
         pControl->Run();
   }
   FillPlayList();
}

CdPlayer::Enqueue(LPWSTR pFile, INT pos)
{
   FILE     * f;
   CHAR     sTag[4] = "";
   CHAR     sArtist[31] = "";
   CHAR     sTitle[31] = "";
   LPSTR    sFile;
   LPWSTR      szEntry = NULL, szSong;
   LONG     idNew;
   pComingUp   pCup;

   sFile = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lstrlen(pFile) + 2);
   sprintf(sFile, "%ws", pFile);

   f = fopen(sFile, "r");
   HeapFree(hHeap, 0, sFile);
   if(f != NULL)
   {
      fseek(f, -128, SEEK_END);
      fread(sTag, 3, 1, f);
      if(strncmp(sTag, "TAG", 3) == 0)
      {
         fread(sTitle, 30, 1, f);
         Strip(sTitle);
         fread(sArtist, 30, 1, f);
         Strip(sArtist);
         szEntry = (LPWSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 2 * 65);
         wsprintf(szEntry, L"%hs - %hs", sArtist, sTitle);
      }
      fclose(f);
   }

   if(szEntry == NULL)
   {
      szSong = wcsrchr(pFile, L'\\');
      szSong++;
      szEntry = (LPWSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 2 * lstrlen(szSong) + 2);
      wsprintf(szEntry, L"%-64ws", szSong);
      szSong = wcsrchr(szEntry, L'.');
      *szSong = L'\00';
   }

   idNew = SendDlgItemMessage(hPlay, IDC_COMINGNEXT, LB_INSERTSTRING, (WPARAM)pos, (LPARAM)szEntry);
   HeapFree(hHeap, 0, szEntry);

   pCup = (pComingUp)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(ComingUp));
   pCup->szFile = (LPWSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 2 * lstrlen(pFile) + 2);
   wsprintf(pCup->szFile, L"%ws", pFile);

   SendDlgItemMessage(hPlay, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)idNew, (LPARAM)pCup);
}

void CdPlayer::SaveSettings()
{
   BOOL        toggle;
   OAFilterState  state;
   REFERENCE_TIME units;
   LONG        lastPos;
   LPWSTR         pText;
   pComingUp      pCup;
   INT            i, size;

   // Save the button states
   toggle = (SendDlgItemMessage(hPlay, IDC_SHUFFLE, BM_GETCHECK, 0, 0) == BST_CHECKED ? TRUE : FALSE);
   dBase->SetConfig("Shuffle", (LPVOID)&toggle, CONFIGTYPE_BOOL);

   // Save the current playing file & position
   if(pControl)
   {
      pControl->GetState(10, &state);
      if(state == State_Running)
         pControl->Stop();
   }
   dBase->SetConfig("CurrentState", (LPVOID)&state, CONFIGTYPE_LONG);

   toggle = FALSE;
   if(szCurrent && (szCurrent[0] != L'\00'))
   {
      dBase->SetConfig("CurrentTitle", (LPVOID)&szCurrent, CONFIGTYPE_UNICODE);
      dBase->SetConfig("CurrentSubtitle", (LPVOID)&szInfo, CONFIGTYPE_UNICODE);

      dBase->SetConfig("CurrentFile", (LPVOID)&szFile, CONFIGTYPE_UNICODE);
      toggle = TRUE;
   }
   dBase->SetConfig("CurrentValid", (LPVOID)&toggle, CONFIGTYPE_BOOL);

   if(pSeek)
      pSeek->GetCurrentPosition(&units);
   else
      units = 0;
   units /= 10000000;
   lastPos = (LONG)units;
   dBase->SetConfig("CurrentPosition", (LPVOID)&lastPos, CONFIGTYPE_LONG);

   // Save the current playlist
   for(i = 0; i < SendDlgItemMessage(hPlay, IDC_COMINGNEXT, LB_GETCOUNT, 0, 0); i++)
   {
      pCup = (pComingUp)SendDlgItemMessage(hPlay, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)i, 0);
      size = 2 * SendDlgItemMessage(hPlay, IDC_COMINGNEXT, LB_GETTEXTLEN, (WPARAM)i, 0) + 2;
      pText = (LPWSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, size);
      SendDlgItemMessage(hPlay, IDC_COMINGNEXT, LB_GETTEXT, (WPARAM)i, (LPARAM)pText);

      dBase->SaveNowPlaying(pText, pCup->szFile);

      HeapFree(hHeap, 0, pText);
   }

}

void CdPlayer::RetrieveSettings()
{
   LPBOOL           lpToggle = NULL;
   OAFilterState  * state = NULL;
   LPLONG           lastPos = NULL;
   REFERENCE_TIME   units;
   LPWSTR           szPtr;

   // Restore the button states
   dBase->GetConfig("Shuffle", (LPVOID*)&lpToggle, CONFIGTYPE_BOOL);
   if(*lpToggle)
      SendDlgItemMessage(hPlay, IDC_SHUFFLE, BM_SETCHECK, BST_CHECKED, 0);
   else
      SendDlgItemMessage(hPlay, IDC_SHUFFLE, BM_SETCHECK, BST_UNCHECKED, 0);

   // Save the current playing file & position
   dBase->GetConfig("CurrentValid", (LPVOID*)&lpToggle, CONFIGTYPE_BOOL);
   if(*lpToggle)
   {
      szPtr = &szCurrent[0];
      dBase->GetConfig("CurrentTitle", (LPVOID*)&szPtr, CONFIGTYPE_UNICODE);

      szPtr = &szInfo[0];
      dBase->GetConfig("CurrentSubtitle", (LPVOID*)&szPtr, CONFIGTYPE_UNICODE);

      szPtr = &szFile[0];
      dBase->GetConfig("CurrentFile", (LPVOID*)&szPtr, CONFIGTYPE_UNICODE);
      if(pGraph && szFile)
         pGraph->RenderFile(szFile, 0);

      dBase->GetConfig("CurrentPosition", (LPVOID*)&lastPos, CONFIGTYPE_LONG);
      units = *lastPos;
      units *= 10000000;
      dBase->Free((LPVOID)lastPos);
      if(pSeek)
      {
         pSeek->GetDuration(&tTime);
         pSeek->SetPositions(&units, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
      }

      dBase->GetConfig("CurrentState", (LPVOID*)&state, CONFIGTYPE_LONG);
      if(*state == State_Running)
         pControl->Run();
      dBase->Free((LPVOID)state);
   }

   // Retrieve the current playlist
   dBase->GetNowPlaying(NowPlaying);
   dBase->ClearNowPlaying();

   dBase->Free(lpToggle);
}

int NowPlaying(void * cbParam, int colCount, char ** colData, char ** colNames)
{
   LPWSTR      pText;
   INT         size;
   LONG     idNew;
   pComingUp   pNew;

   if(colCount != 2) return(0);
   if(colData == NULL) return(0);
   if(colData[0] == NULL) return(0);
   if(colData[1] == NULL) return(0);
   if(colData[0][0] == '\0') return(0);
   if(colData[1][0] == '\0') return(0);

   size = 2 * strlen(colData[0]) + 2;
   pText = (LPWSTR)HeapAlloc(myPtr->hHeap, HEAP_ZERO_MEMORY, size);
   wsprintf(pText, L"%hs", colData[0]);
   idNew = SendDlgItemMessage(myPtr->hPlay, IDC_COMINGNEXT, LB_ADDSTRING, 0, (LPARAM)pText);
   HeapFree(myPtr->hHeap, 0, pText);

   pNew = (pComingUp)HeapAlloc(myPtr->hHeap, HEAP_ZERO_MEMORY, sizeof(ComingUp));
   size = 2 * strlen(colData[1]) + 2;
   pNew->szFile = (LPWSTR)HeapAlloc(myPtr->hHeap, HEAP_ZERO_MEMORY, size);

   // Determine width of number marker
   wsprintf(pNew->szFile, L"%hs", colData[1]);
   SendDlgItemMessage(myPtr->hPlay, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)idNew, (LPARAM)pNew);
   return(0);
}

LRESULT CALLBACK CdPlayerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   PAINTSTRUCT    ps;
    RECT       ourRect;
   REFERENCE_TIME pos;
   OAFilterState  state;
   DWORD       Position;
   LONG        curSel;
   LPWSTR         pText;
   LONG        lEvCode, lParam1, lParam2;
// HRESULT        hr;
   INT            size;
   pComingUp      pCup;

   GetClientRect(hWnd, &ourRect);
   if(myPtr->pControl)
      myPtr->pControl->GetState(10, &state);

   switch(message)
   {
      case WM_CREATE:
         myPtr->hCbWnd = hWnd;
         myPtr->InitDC(FALSE);
         SendMessage(myPtr->hPrnt, WM_REGISTERFINALWND, (WPARAM)0, (LPARAM)hWnd);
         break;

      case WM_COMMAND:
         switch(HIWORD(wParam))
         {
            case BN_CLICKED:
               switch(LOWORD(wParam))
               {
                  case IDC_PLAY:
                     SendDlgItemMessage(hWnd, IDC_SEEK, TBM_SETRANGE, TRUE, MAKELONG(0, 10000));
                     myPtr->pControl->Run();
                     SendDlgItemMessage(hWnd, IDC_PAUSE, BM_SETCHECK, BST_UNCHECKED, 0);
                     SendDlgItemMessage(myPtr->hPopup, IDC_PAUSE, BM_SETCHECK, BST_UNCHECKED, 0);
                     // Reenable the skip buttons
                     SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, BST_UNCHECKED, 0);
                     SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, BST_UNCHECKED, 0);
                     break;
                  case IDC_STOP:
                     myPtr->pControl->Stop();
                     pos = 0;
                     myPtr->pSeek->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
                     SendDlgItemMessage(hWnd, IDC_PAUSE, BM_SETCHECK, BST_UNCHECKED, 0);
                     SendDlgItemMessage(myPtr->hPopup, IDC_PAUSE, BM_SETCHECK, BST_UNCHECKED, 0);
                     // Reenable the skip buttons
                     SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, BST_UNCHECKED, 0);
                     SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, BST_UNCHECKED, 0);
                     break;
                  case IDC_PAUSE:
                     if(state == State_Running)
                     {
                        myPtr->pControl->Pause();
                        // Disable the skip buttons
                        SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, BST_INDETERMINATE, 0);
                        SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
                        // Set the pause button activated
                        SendDlgItemMessage(hWnd, IDC_PAUSE, BM_SETCHECK, BST_CHECKED, 0);
                        SendDlgItemMessage(myPtr->hPopup, IDC_PAUSE, BM_SETCHECK, BST_CHECKED, 0);
                     }
                     else
                     {
                        myPtr->pControl->Run();
                        // Reenable the skip buttons
                        SendDlgItemMessage(hWnd, IDC_PREV, BM_SETCHECK, BST_UNCHECKED, 0);
                        SendDlgItemMessage(hWnd, IDC_NEXT, BM_SETCHECK, BST_UNCHECKED, 0);
                        // Put the pause btton back to default state
                        SendDlgItemMessage(hWnd, IDC_PAUSE, BM_SETCHECK, BST_UNCHECKED, 0);
                        SendDlgItemMessage(myPtr->hPopup, IDC_PAUSE, BM_SETCHECK, BST_UNCHECKED, 0);
                     }
                     break;
                  case IDC_NEXT:
                     pCup = (pComingUp)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)0, 0);
                     myPtr->PlayNow(pCup->szFile);
                     SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)0, 0);
                     myPtr->FillPlayList();
                     break;
                  case IDC_PREV:
                     myPtr->PlayNow(L"C:\Henk\Prive\WOPR\wopr.720x416.avi");
//                   myPtr->PlayNow(L"c:\\download\\divx\\Astérix - Astérix et les Indiens - 1994 - Fr DivX.avi");
                     break;

                  case IDC_XFADE:
                     if(SendDlgItemMessage(hWnd, IDC_XFADE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
                        SendDlgItemMessage(hWnd, IDC_XFADE, BM_SETCHECK, BST_CHECKED, 0);
                     else
                        SendDlgItemMessage(hWnd, IDC_XFADE, BM_SETCHECK, BST_UNCHECKED, 0);
                     break;
                  case IDC_SHUFFLE:
                     if(SendDlgItemMessage(hWnd, IDC_SHUFFLE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
                        SendDlgItemMessage(hWnd, IDC_SHUFFLE, BM_SETCHECK, BST_CHECKED, 0);
                     else
                        SendDlgItemMessage(hWnd, IDC_SHUFFLE, BM_SETCHECK, BST_UNCHECKED, 0);
                     break;
                  case IDC_REPEAT:
                     if(SendDlgItemMessage(hWnd, IDC_REPEAT, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
                        SendDlgItemMessage(hWnd, IDC_REPEAT, BM_SETCHECK, BST_CHECKED, 0);
                     else
                        SendDlgItemMessage(hWnd, IDC_REPEAT, BM_SETCHECK, BST_UNCHECKED, 0);
                     break;

                  // Now playing controls
                  case IDC_PLAYNOW:
                     curSel = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCURSEL, 0, 0);
                     if(curSel != LB_ERR)
                     {
                        pCup = (pComingUp)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)curSel, 0);
                        myPtr->PlayNow(pCup->szFile);
                        SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)curSel, 0);
                        SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETCURSEL, (WPARAM)-1, 0);
                        SendDlgItemMessage(hWnd, IDC_PLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
                        SendDlgItemMessage(hWnd, IDC_PLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
                        SendDlgItemMessage(hWnd, IDC_REMOVE, BM_SETCHECK, BST_INDETERMINATE, 0);
                        myPtr->FillPlayList();
                     }
                     break;
                  case IDC_PLAYNEXT:
                     curSel = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCURSEL, 0, 0);
                     if(curSel != LB_ERR)
                     {
                        // Get the original text
                        size = 2 * SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETTEXTLEN, (WPARAM)curSel, 0) + 2;
                        pText = (LPWSTR)HeapAlloc(myPtr->hHeap, HEAP_ZERO_MEMORY, size);
                        SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETTEXT, (WPARAM)curSel, (LPARAM)pText);
                        // Get the original data block
                        pCup = (pComingUp)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)curSel, 0);
                        SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)curSel, (LPARAM)NULL);

                        // Remove the old line
                        SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)curSel, 0);

                        // Insert the data at top location
                        SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_INSERTSTRING, (WPARAM)0, (LPARAM)pText);
                        SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETITEMDATA, (WPARAM)0, (LPARAM)pCup);

                        // Change the buton states
                        SendDlgItemMessage(hWnd, IDC_PLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
                        SendDlgItemMessage(hWnd, IDC_PLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
                        SendDlgItemMessage(hWnd, IDC_REMOVE, BM_SETCHECK, BST_INDETERMINATE, 0);
                        myPtr->FillPlayList();
                     }
                     break;
                  case IDC_REMOVE:
                     curSel = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCURSEL, 0, 0);
                     if(curSel != LB_ERR)
                     {
                        SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)curSel, 0);
                        myPtr->FillPlayList();
                        SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETCURSEL, (WPARAM)curSel, 0);
                     }
                     break;
                  default:
                     if(IsChild(hWnd, (HWND)lParam))
                     {
                        // Pass though to parent
                        PostMessage(myPtr->hPrnt, message, wParam, lParam);
                     }
                     break;
               }
               break;

            // Nowplaying listbox events
            case LBN_DBLCLK:
         /*    curSel = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCURSEL, 0, 0);
               if(curSel != LB_ERR)
               {
                  szText = (LPWSTR)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)curSel, 0);
                  myPtr->PlayNow(&szText[lstrlen(szText) + 1]);
                  SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)curSel, 0);
                  SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_SETCURSEL, (WPARAM)-1, 0);
                  SendDlgItemMessage(hWnd, IDC_PLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
                  SendDlgItemMessage(hWnd, IDC_PLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
                  SendDlgItemMessage(hWnd, IDC_REMOVE, BM_SETCHECK, BST_INDETERMINATE, 0);
                  myPtr->FillPlayList();
               }
         */    break;
            case LBN_SELCHANGE:
               curSel = SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETCURSEL, 0, 0);
               if(curSel == LB_ERR)
               {
                  SendDlgItemMessage(hWnd, IDC_PLAYNOW, BM_SETCHECK, BST_INDETERMINATE, 0);
                  SendDlgItemMessage(hWnd, IDC_PLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
                  SendDlgItemMessage(hWnd, IDC_REMOVE, BM_SETCHECK, BST_INDETERMINATE, 0);
               }
               else
               {
                  SendDlgItemMessage(hWnd, IDC_PLAYNOW, BM_SETCHECK, BST_UNCHECKED, 0);
                  if(curSel != 0)
                     SendDlgItemMessage(hWnd, IDC_PLAYNEXT, BM_SETCHECK, BST_UNCHECKED, 0);
                  else
                     SendDlgItemMessage(hWnd, IDC_PLAYNEXT, BM_SETCHECK, BST_INDETERMINATE, 0);
                  SendDlgItemMessage(hWnd, IDC_REMOVE, BM_SETCHECK, BST_UNCHECKED, 0);
               }
               break;
         }
         break;

      case WM_HSCROLL:
         if((LOWORD(wParam) == TB_THUMBTRACK) || (LOWORD(wParam) == TB_ENDTRACK ))
         {
            myPtr->pControl->Pause();
            if((Position = HIWORD(wParam)) == 0)
               Position  = SendDlgItemMessage(hWnd, IDC_SEEK, TBM_GETPOS, 0, 0);
            pos = (myPtr->tTime / 10000) * Position;
            if(myPtr->pSeek) myPtr->pSeek->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
            if(LOWORD(wParam) == TB_ENDTRACK)
               myPtr->pControl->Run();
         }
         break;

      case WM_PAINT:
         GetClientRect(myPtr->hPrnt, &ourRect);
         ourRect.bottom -= myPtr->offset;
         SetWindowPos(hWnd, 0, 0, 0, ourRect.right, ourRect.bottom, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
            BeginPaint(hWnd, &ps);
         myPtr->Paint();
         EndPaint(hWnd, &ps);
         break;

      case WM_SIZE:
         SetWindowPos(GetDlgItem(hWnd, IDC_PREV), 0, 0 * myPtr->offset, myPtr->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
         SetWindowPos(GetDlgItem(hWnd, IDC_PLAY), 0, 1 * myPtr->offset, myPtr->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
         SetWindowPos(GetDlgItem(hWnd, IDC_STOP), 0, 2 * myPtr->offset, myPtr->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
         SetWindowPos(GetDlgItem(hWnd, IDC_PAUSE), 0, 3 * myPtr->offset, myPtr->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
         SetWindowPos(GetDlgItem(hWnd, IDC_NEXT), 0, 4 * myPtr->offset, myPtr->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

         SetWindowPos(GetDlgItem(hWnd, IDC_XFADE), 0, ourRect.right - 3 * myPtr->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
         SetWindowPos(GetDlgItem(hWnd, IDC_SHUFFLE), 0, ourRect.right - 2 * myPtr->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
         SetWindowPos(GetDlgItem(hWnd, IDC_REPEAT), 0, ourRect.right - 1 * myPtr->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

         SetWindowPos(GetDlgItem(hWnd, IDC_PLAYNOW), 0, ourRect.right - 3 * myPtr->offset, myPtr->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
         SetWindowPos(GetDlgItem(hWnd, IDC_PLAYNEXT), 0, ourRect.right - 2 * myPtr->offset, myPtr->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
         SetWindowPos(GetDlgItem(hWnd, IDC_REMOVE), 0, ourRect.right - 1 * myPtr->offset, myPtr->row2, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

         SetWindowPos(GetDlgItem(hWnd, IDC_COMINGNEXT), 0, 1, myPtr->offset + myPtr->row2 + 1, ourRect.right - 2, ourRect.bottom - myPtr->offset - myPtr->row2 - 2, SWP_NOACTIVATE | SWP_NOZORDER);
         InitListBox(FALSE, GetDlgItem(hWnd, IDC_COMINGNEXT));
         myPtr->FillPlayList();

         SetWindowPos(GetDlgItem(hWnd, IDC_MODEMP3), 0, 0 * myPtr->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
         SetWindowPos(GetDlgItem(hWnd, IDC_MODECD), 0, 1 * myPtr->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
         SetWindowPos(GetDlgItem(hWnd, IDC_MODERADIO), 0, 2 * myPtr->offset, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

         SetWindowPos(GetDlgItem(hWnd, IDC_SEEK), 0, 3 * myPtr->offset + 2, 3 * myPtr->row1 - 5, ourRect.right - 3 * myPtr->offset - 4, (myPtr->offset >> 1) + 2, SWP_NOACTIVATE | SWP_NOZORDER);
         SetWindowPos(GetDlgItem(hWnd, IDC_TICKER), 0, 3 * myPtr->offset + 2, myPtr->row1 + 2, ourRect.right - 3 * myPtr->offset - 4, myPtr->row1 + 6, SWP_NOACTIVATE | SWP_NOZORDER);
         SetWindowPos(GetDlgItem(hWnd, IDC_SUBTICKER), 0, 3 * myPtr->offset + 2, 2 * myPtr->row1 + 7, ourRect.right - 3 * myPtr->offset - 4, (myPtr->offset >> 1) - 8, SWP_NOACTIVATE | SWP_NOZORDER);

         myPtr->InitDC(TRUE);
         myPtr->Paint();
         break;

      case WM_SETVIDEOWINDOW:
   //    while(myPtr->pVmr != NULL) ;
   //    hr = InitWindowlessVMR((HWND)wParam, myPtr->pWcontrol);
   //    myPtr->hVideo = (HWND)wParam;
   //    myPtr->dcVid = GetDC(myPtr->hVideo);
         break;

      case WM_PAINTVIDEOWINDOW:
         if(myPtr->pWcontrol != NULL)
            myPtr->pWcontrol->RepaintVideo(myPtr->hVideo, myPtr->dcVid);
         break;

      case WM_DISPLAYCHANGE:
         if(myPtr->pWcontrol != NULL)
            myPtr->pWcontrol->DisplayModeChanged();
         break;

      case WM_DIRECTSHOW:
         if(myPtr->pEvent)
            while(myPtr->pEvent->GetEvent(&lEvCode, &lParam1, &lParam2, 0) == S_OK)
            {
               if(lEvCode == EC_COMPLETE)
               {
                  pCup = NULL;
                  while(pCup == NULL)
                  {
                     pCup = (pComingUp)SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_GETITEMDATA, (WPARAM)0, 0);
                     if(pCup)
                        if(pCup->szFile == NULL) pCup = NULL;
                  }
                  myPtr->PlayNow(pCup->szFile);
                  SendDlgItemMessage(hWnd, IDC_COMINGNEXT, LB_DELETESTRING, (WPARAM)0, 0);
                  myPtr->FillPlayList();
               }
               myPtr->pEvent->FreeEventParams(lEvCode, lParam1, lParam2);
            }
         break;

      case WM_DIRECTSHOWMUTE:
         if((BOOL)wParam == TRUE)
            myPtr->pAudio->put_Volume(-2000);
         else
            myPtr->pAudio->put_Volume(0);
         myPtr->muted = (BOOL)wParam;
         break;

      case WM_LBUTTONDOWN:
         SetCapture(hWnd);
         break;

      case WM_LBUTTONUP:
         ReleaseCapture();
         break;

      case WM_TIMER:
         myPtr->PaintTime();
         break;

      case WM_DESTROY:
         KillTimer(hWnd, 1);
         break;

        case WM_DRAWITEM:
         DrawListBoxItem((LPDRAWITEMSTRUCT)lParam);
         break;

      case WM_DELETEITEM:
         pCup = (pComingUp)((PDELETEITEMSTRUCT)lParam)->itemData;
         if(pCup)
         {
            if(pCup->szFile)
               HeapFree(myPtr->hHeap, 0, pCup->szFile);
            HeapFree(myPtr->hHeap, 0, pCup);
         }
         break;

      case WM_PLAYNOW:
         myPtr->PlayNow((LPWSTR)wParam);
         break;

      case WM_PLAYNEXT:
         myPtr->Enqueue((LPWSTR)wParam, 0);
         break;

      case WM_PLAYQUEUE:
         myPtr->Enqueue((LPWSTR)wParam, -1);
         break;

      case WM_SAVEDATA:
         if(myPtr->saved == FALSE)
            myPtr->SaveSettings();
         myPtr->saved = TRUE;
         break;

      default:
         return(DefWindowProc(hWnd, message, wParam, lParam));
   }
   return(0);
}

static Graphics * gr;
static Brush    * br_black;
static Brush    * br_white;
static Brush    * br_blue;
static Font     * fn;

static void DrawListBoxItem(LPDRAWITEMSTRUCT pItem)
{
   RectF       rf;
   RECT         crect;
   INT            i, n, size;
   WCHAR       idTxt[12];
   StringFormat   fmt;
   LPWSTR         pText;

   int offset = myPtr->offset;

   pItem->rcItem.right -= pItem->rcItem.left;
   pItem->rcItem.bottom -= pItem->rcItem.top;

   rf.Y = (float)pItem->rcItem.top;
   rf.Height = (float)pItem->rcItem.bottom;

   GetClientRect(pItem->hwndItem, &crect);
   n = SendMessage(pItem->hwndItem, LB_GETCOUNT, 0, 0);
   if(pItem->itemID == -1)
   {
      GetClientRect(pItem->hwndItem, &crect);
      for(i = 0; i < (crect.bottom / pItem->rcItem.bottom); i++)
      {
//       gr->FillRectangle(br_blue, 0, i * pMeas->itemHeight, pMeas->itemWidth, pMeas->itemHeight);
//       gr->FillRectangle(br_blue, pItem->rcItem.left, i * pItem->rcItem.top, pItem->rcItem.right, pItem->rcItem.bottom);
      }
   }

   size = 2 * SendMessage(pItem->hwndItem, LB_GETTEXTLEN, (WPARAM)pItem->itemID, 0) + 2;
   pText = (LPWSTR)HeapAlloc(myPtr->hHeap, HEAP_ZERO_MEMORY, size);
   SendMessage(pItem->hwndItem, LB_GETTEXT, (WPARAM)pItem->itemID, (LPARAM)pText);

   fmt.SetAlignment(StringAlignmentNear);
   fmt.SetTrimming(StringTrimmingNone);
   fmt.SetLineAlignment(StringAlignmentCenter);
   fmt.SetFormatFlags(StringFormatFlagsNoWrap);
   if(pItem->itemState & ODS_SELECTED)
   {
      gr->FillRectangle(br_blue, pItem->rcItem.left, pItem->rcItem.top, pItem->rcItem.left + offset, pItem->rcItem.bottom);
      gr->FillRectangle(br_black, pItem->rcItem.left + offset, pItem->rcItem.top, pItem->rcItem.right - offset, pItem->rcItem.bottom);

      wsprintf(idTxt, L"%u", pItem->itemID + 1);
      rf.X = (float)pItem->rcItem.left;
      rf.Width = (float)offset;
      gr->DrawString(idTxt, -1, fn, rf, &fmt, br_black);
      rf.X += (float)offset;
      rf.Width = (float)pItem->rcItem.right - offset;
      gr->DrawString(pText, -1, fn, rf, &fmt, br_white);
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

      wsprintf(idTxt, L"%u", pItem->itemID + 1);
      rf.X = (float)pItem->rcItem.left;
      rf.Width = (float)offset;
      gr->DrawString(idTxt, -1, fn, rf, &fmt, br_black);
      rf.X += (float)offset;
      rf.Width = (float)pItem->rcItem.right - offset;
      gr->DrawString(pText, -1, fn, rf, &fmt, br_black);
   }
   HeapFree(myPtr->hHeap, 0, pText);
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

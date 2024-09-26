// CdPlayer.h: interface for the CdPlayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CDPLAYER_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_CDPLAYER_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DShow.h"
#include "Popup.h"

class CdPlayer
{
private:
   Graphics   * myGraphics;
   Brush      * bg[4];
   Pen        * pn;
   RECT         ourRect;
   HINSTANCE    hInst;
   Font       * fn[2];

   Popup * pPopup;

   LONG         autoLast;

   ATOM MyRegisterClass(HINSTANCE hInstance);
   DShowInit(BOOL loadUnload);

public:
   CdPlayer(HINSTANCE hInstance, HWND hParent, int bottomOffset, Database * pDB);
   virtual ~CdPlayer();

   int        row1, row2, offset;
   HWND       hPlay, hCbWnd, hPrnt;
   HWND    hPopup;

   HWND     hVideo;
   HDC      dcVid;
   REFERENCE_TIME tTime;
   BOOL     muted, saved;
   HANDLE   hHeap;

   // DirectShow stuff
   IGraphBuilder         * pGraph;
   IGraphConfig        * pConfig;
   IMediaControl         * pControl;
   IMediaEvent           * pEvent;
   IMediaSeeking       * pSeek;
    IVMRWindowlessControl * pWcontrol;
   IFilterGraph        * pFilter;
   IBaseFilter         * pSource;
    IBaseFilter           * pVmr;
   IBasicAudio         * pAudio;

   void InitDC(BOOL updateDC);
   void Paint();
   void PaintTime();
   void ShowMode(int vMode);
   void SaveSettings();
   void RetrieveSettings();
   virtual Show(BOOL yesNo);
   FillPlayList();
   PlayNow(LPWSTR pFile);
   Enqueue(LPWSTR pFile, INT pos);
};


#endif // !defined(AFX_CDPLAYER_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

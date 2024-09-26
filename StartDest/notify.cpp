//****************************************************************************
// Notify.cpp: Implements a simple event class for IDestEvents2.
//****************************************************************************

#include "stdafx.h"
#include <iostream.h>
#include <assert.h>
//using namespace std;

#include "DestDll_i.c"
#include "Notify.h"

extern IDest2 * m_pIDest;
extern HWND vDestinatorWnd;


//****************************************************************************
// CNotify::CNotify() Constructor
//****************************************************************************

CNotify::CNotify(): m_ByteConter(0){}

//****************************************************************************
// CNotify::~CNotify() Destructor
//****************************************************************************

CNotify::~CNotify(){}

//****************************************************************************
// CNotify::AddRef()
//****************************************************************************

ULONG CNotify::AddRef(){ return 1;}

//****************************************************************************
// CNotify::Release()
//****************************************************************************

ULONG CNotify::Release(){return 1;}

//****************************************************************************
// STDMETHODIMP CNotify::QueryInterface
//****************************************************************************

STDMETHODIMP CNotify::QueryInterface(REFIID riid, void** ppv)
{
   if(riid == IID_IUnknown || riid == IID__IDestEvents2)
   {
      *ppv = (_IDestEvents2*)this;
      return S_OK;
   }
   else
      return E_NOINTERFACE;
}

//****************************************************************************
// void CNotify::OnCloseMainDestinatorWindow
//****************************************************************************

void STDMETHODCALLTYPE CNotify::OnSwapMap(BSTR MapName)
{
   //perform your actions here.
}


void STDMETHODCALLTYPE CNotify::OnCreateDestinatorWindow(HWND hWnd)
{
   LONG        newStyle;
   WINDOWPLACEMENT winPlacement;

   /* Fill Windowplacement vars */
   winPlacement.length = sizeof(WINDOWPLACEMENT);
   winPlacement.flags = WPF_SETMINPOSITION;
   winPlacement.showCmd = SW_SHOW;
   winPlacement.ptMinPosition.y = GetSystemMetrics(SM_CYSCREEN);
   winPlacement.ptMinPosition.x = 0;

   newStyle = GetWindowLong(hWnd, GWL_STYLE);
   newStyle &= ~(WS_CAPTION | WS_THICKFRAME);
   newStyle |= WS_MAXIMIZE;
   newStyle &= ~WS_MINIMIZE;
   SetWindowLong(hWnd, GWL_STYLE, newStyle);
   SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW);
   SetWindowPlacement(hWnd, &winPlacement);

   SetFocus(hWnd);
   vDestinatorWnd = hWnd;
}


void STDMETHODCALLTYPE CNotify::OnYouHaveArrived(void)
{
   //perform your actions here.
}


void STDMETHODCALLTYPE CNotify::OnDestinatorError(long err)
{
   //perform your actions here.
}


void STDMETHODCALLTYPE CNotify::OnCloseMainDestinatorWindow(void)
{
   //perform your actions here.
   if(m_pIDest != NULL)
   {
      m_pIDest->Release();
      m_pIDest = NULL;
   }
   PostQuitMessage(0);

}


void STDMETHODCALLTYPE CNotify::OnGetDataFromCOMPort(long event, const BYTE * pData)
{
   //perform your actions here.
}

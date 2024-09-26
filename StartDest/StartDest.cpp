// StartDest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "DestDLL_i.c"
#include "DestDll.h"
#include "notify.h"
#include "stdio.h"
#include <iostream.h>
#include <assert.h>
#include "DestHook.h"

#define MAX_LOADSTRING 100

typedef VOID (* FNCBTHANDLE)(HANDLE);
typedef VOID (* FNIDSHANDLE)(DWORD, DWORD);

// Global Variables:
HINSTANCE hInst;                       // current instance
IDest2    * m_pIDest;
HWND        destWindow;
HHOOK       shellHk;
HMODULE     hinstDLL;
FNIDSHANDLE dllIds;
HWND		vDestinatorWnd;

// Foward declarations of functions included in this code module:
BOOL           InitInstance(HINSTANCE, int);
LRESULT CALLBACK  WndProc(HWND, UINT, WPARAM, LPARAM);
void                SetUpEventHandler(IDest2 *m_pIDest);
void           InitHook(void);


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	vDestinatorWnd = 0;

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	InitHook();

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_STARTDEST);

	// Main message loop:
	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(vDestinatorWnd != 0)
		{
			// Send on to destinator window
//			SendMessage(vDestinatorWnd, msg.message, msg.wParam, msg.lParam);
	//		msg.hwnd = vDestinatorWnd;
		}
		DispatchMessage(&msg);
	}
	return(msg.wParam);
}


//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   long CommandResult;

   hInst = hInstance; // Store instance handle in our global variable
   m_pIDest = NULL;

#ifdef _WIN32_WCE
   CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
   CoInitialize(NULL);
#endif

   CoGetClassObject(CLSID_Dest, CLSCTX_SERVER, NULL, IID_IDest2, (void**) &m_pIDest);
   if (!m_pIDest)
   {
      CoCreateInstance(CLSID_Dest, NULL, CLSCTX_SERVER , IID_IDest2,(void**) &m_pIDest);
      if (!m_pIDest)
     {
        MessageBox(NULL, "Could not instantiate Destinator application", "Error", MB_OK);
         return FALSE;
     }
   }

   SetUpEventHandler(m_pIDest);
   m_pIDest->CreateDestinatorWindow(F_DEFAULT, &CommandResult);

   return TRUE;
}

void InitHook(void)
{
   HOOKPROC dllProc;
   static HHOOK hhookSysMsg;
   FNCBTHANDLE dllHook;

   hinstDLL = LoadLibrary((LPCTSTR)"DestHook.dll");
   if(!hinstDLL)
     MessageBox(NULL, "Could not load DestHook.dll", "Error", MB_OK);

   dllIds = (FNIDSHANDLE)GetProcAddress(hinstDLL, (const char *)3);
   if(!dllIds)
     MessageBox(NULL, "Could not locate hook function CDestSetIds", "Error", MB_OK);
   else
     (dllIds)(GetCurrentThreadId(), GetCurrentProcessId());

   dllProc = (HOOKPROC)GetProcAddress(hinstDLL, (const char *)1);
   if(!dllProc)
     MessageBox(NULL, "Could not locate hook callback function CDestProc", "Error", MB_OK);
   else
      shellHk = SetWindowsHookEx(WH_CBT, dllProc, hinstDLL, 0);
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND - process the application menu
//  WM_PAINT   - Paint the main window
//  WM_DESTROY - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
      case WM_DESTROY:
         UnhookWindowsHookEx(shellHk);
         FreeLibrary(hinstDLL);
         PostQuitMessage(0);
         break;

      default:
         return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

void SetUpEventHandler(IDest2 *m_pIDest)
{
   IConnectionPointContainer *pIConnectionPointContainer = NULL;
   IConnectionPoint *pIConnectionPoint = NULL;
   CNotify *pCNotify = NULL;

   HRESULT hr;
   DWORD    dwCookie = 0;

   // Get a connection point container for the transport object.
   hr = m_pIDest->QueryInterface(IID_IConnectionPointContainer, (void**)&pIConnectionPointContainer);
   if(FAILED(hr)) {
      MessageBox(NULL, "Failed to get connection point container from dest object.", "Error", MB_OK);
   }

   // Get a connection point from the connection point container.
   hr = pIConnectionPointContainer->FindConnectionPoint(IID__IDestEvents2, &pIConnectionPoint);
   if (FAILED(hr)) {
      MessageBox(NULL, "Failed to get a connection point for the dest object.", "Error", MB_OK);
   }

   // Create the event
   pCNotify = new CNotify();
   if(!pCNotify) {
      MessageBox(NULL, "Failed to create a CNotify.", "Error", MB_OK);
   }

   // Advise the IDest object of the event sink.
   hr = pIConnectionPoint->Advise((IUnknown *)pCNotify, &dwCookie);
   if (FAILED(hr)) {
      MessageBox(NULL, "Failed to advise the dest object of the event handler.", "Error", MB_OK);
   }

} // end SetUpEventHandler.




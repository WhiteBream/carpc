//------------------------------------------------------------------------------
// File: PlayCap.h
//
// Desc: DirectShow sample code - header file for PlayCap previewer
//       application.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//#include <atlbase.h>
#include "stdafx.h"
#include <dshow.h>

#pragma comment(lib, "Strmiids.lib")

#pragma warning(disable : 4995)

//
// Function prototypes
//
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hInstP, LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndMainProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

HRESULT GetInterfaces(HWND hWnd);
HRESULT CaptureVideo(HWND hWnd);
HRESULT FindCaptureDevice(IBaseFilter ** ppSrcFilter);
HRESULT SetupVideoWindow(HWND hWnd);
HRESULT ChangePreviewState(HWND hWnd, int nShow);
HRESULT HandleGraphEvent(void);
void GraphNotify(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void PaintVideoWindow(HWND hWnd);
void ChangeVideoWindow(void);

void Msg(TCHAR *szFormat, ...);
void CloseInterfaces(void);
void ResizeVideoWindow(HWND hWnd);

HRESULT SetMirrorMode(BOOL vYesNo);


enum PLAYSTATE {Stopped, Paused, Running, Init};

//
// Macros
//
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

#define JIF(x) if (FAILED(hr=(x))) \
    {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n\0"), hr); return hr;}

//
// Constants
//
#define DEFAULT_VIDEO_WIDTH     640
#define DEFAULT_VIDEO_HEIGHT    480

#define APPLICATIONNAME TEXT("Video Capture Previewer (PlayCap)\0")
#define CLASSNAME       TEXT("VidCapPreviewer\0")

// Application-defined message to notify app of filtergraph events
#define WM_GRAPHNOTIFY  WM_APP+1

//
// Resource constants
//
#define IDI_VIDPREVIEW          100


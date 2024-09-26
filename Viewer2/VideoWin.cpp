// VideoWin.cpp: implementation of the VideoWin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VideoWin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

#define WINCLASS	TEXT("CRGVDWN")

LRESULT CALLBACK VideoWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

Brush    * ourBgnd2;
Graphics * ourGraphics2;
HWND       hPrnt2;
int        offset2;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VideoWin::VideoWin(PAppInfo pAppInfo, IBaseFilter *pSrcFilter, INT vNum)
{
	RECT pRect;
	CHAR vVar[20];

	pAi = pAppInfo;
	vNumber = vNum;

	pGraph = NULL;
	pControl = NULL;
	pEvent = NULL;
	pCapture = NULL;
	pVmr = NULL;
	pWcontrol = NULL;
	pWmixer = NULL;

	if(!pSrcFilter)
		delete this;

	RegWinClass(WINCLASS, VideoWinProc);
	GetClientRect(pAi->hApp, &pRect);

	hWnd = CreateWindow(WINCLASS, NULL, WS_CHILD | WS_CLIPCHILDREN,
		0, 0, pRect.right, pRect.bottom - pAi->vOffs, pAi->hApp, NULL, pAi->hInst, this);

	GetInterfaces(hWnd);

	// Add Capture filter to our graph.
	hr = g_pGraph->AddFilter(pSrcFilter, L"Video Capture");
	if (FAILED(hr))
	{
     //   MessageBox(TEXT("Couldn't add the capture filter to the graph!  hr=0x%x\r\n\r\n")
     //       TEXT("If you have a working video capture device, please make sure\r\n")
     //       TEXT("that it is connected and is not being used by another application.\r\n\r\n")
     //       TEXT("The sample will now close."), hr);
		pSrcFilter->Release();
		delete this;
	}

	// Render the preview pin on the video capture filter
	// Use this instead of g_pGraph->RenderFile
	hr = g_pCapture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pSrcFilter, NULL, pVmr);
	if (FAILED(hr))
	{
    //    Msg(TEXT("Couldn't render the video capture stream.  hr=0x%x\r\n")
    //        TEXT("The capture device may already be in use by another application.\r\n\r\n")
    //        TEXT("The sample will now close."), hr);
		pSrcFilter->Release();
		delete this;
	}
	pSrcFilter->Release();

	SetupVideoWindow(hWnd);

	sprintf(vVar, "Camera%dMirror", vNumber);
	vMirrorCurrent = pAi->dB->GetBool(vVar) ? TRUE : FALSE;
	SetMirrorMode(vMirrorCurrent);
}



VideoWin::~VideoWin()
{
	CloseInterfaces();
	DestroyWindow(hWnd);
}


VideoWin::Show(BOOL yesNo)
{
	if(yesNo)
	{
		ShowWindow(hWnd, SW_SHOW);
		ChangePreviewState(hWnd, TRUE);
		ResizeVideoWindow(hWnd);
	}
	else
	{
		ChangePreviewState(hWnd, FALSE);
		ShowWindow(hWnd, SW_HIDE);
	}
}


ATOM VideoWin::RegWinClass(LPTSTR pClassName, WNDPROC pClassCb)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= pClassCb;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= pAi->hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	//wcex.hbrBackground	= NULL;
	wcex.hbrBackground	= CreateSolidBrush(RGB(0, 0, 0));
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= pClassName;
	wcex.hIconSm		= NULL;

	return(RegisterClassEx(&wcex));
}


HRESULT VideoWin::GetInterfaces(HWND hWnd)
{
	HRESULT hr;
	IVMRFilterConfig* pVmrConf;

	// Create the filter graph
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                           IID_IGraphBuilder, (LPVOID*)&pGraph);
	if(FAILED(hr))
		return(hr);

    // Create the capture graph builder
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,
                           IID_ICaptureGraphBuilder2, (LPVOID*)&pCapture);
	if(FAILED(hr))
		return(hr);

    // Obtain interfaces for media control and Video Window
    hr = pGraph->QueryInterface(IID_IMediaControl,(LPVOID*)&pControl);
	if(FAILED(hr))
		return(hr);

    hr = pGraph->QueryInterface(IID_IMediaEvent, (LPVOID*)&pEvent);
	if(FAILED(hr))
		return(hr);

	hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC, IID_IBaseFilter, (LPVOID*)&pVmr);
	if(FAILED(hr))
	{
		MessageBox(NULL, TEXT("CLSID_VideoMixingRenderer failed"), TEXT("Error"), MB_OK);
		return(hr);
	}
	// Add the VMR to the filter graph.
	hr = pGraph->AddFilter(pVmr, L"Video Mixing Renderer");
	if(FAILED(hr))
	{
		MessageBox(NULL, TEXT("AddFilter on VMR failed"), TEXT("Error"), MB_OK);
		SAFE_RELEASE(pVmr);
		return hr;
	}
	// Set the rendering mode.
	hr = pVmr->QueryInterface(IID_IVMRFilterConfig, (LPVOID*)&pVmrConf);
	if(FAILED(hr))
	{
		MessageBox(NULL, TEXT("IID_IVMRFilterConfig failed"), TEXT("Error"), MB_OK);
		SAFE_RELEASE(pVmr);
		return(hr);
	}
	hr = pVmrConf->SetRenderingMode(VMRMode_Windowless);
	if(FAILED(hr))
	{
		MessageBox(NULL, TEXT("SetRenderingMode failed"), TEXT("Error"), MB_OK);
		SAFE_RELEASE(pVmrConf);
		SAFE_RELEASE(pVmr);
		return(hr);
	}
	// Activate mixer mode
	hr = pVmrConf->SetNumberOfStreams(1);
    if(FAILED(hr))
	{
		MessageBox(NULL, TEXT("SetNumberOfStreams failed"), TEXT("Error"), MB_OK);
		return(hr);
	}
	SAFE_RELEASE(pVmrConf);

	hr = pVmr->QueryInterface(IID_IVMRWindowlessControl, (LPVOID*)&pWcontrol);
    if(FAILED(hr))
	{
		MessageBox(NULL, TEXT("IID_IVMRWindowlessControl failed"), TEXT("Error"), MB_OK);
		return(hr);
	}

	hr = pVmr->QueryInterface(IID_IVMRMixerControl, (LPVOID*)&pWmixer);
    if(FAILED(hr))
	{
		MessageBox(NULL, TEXT("IID_IVMRMixerControl failed"), TEXT("Error"), MB_OK);
		return(hr);
	}

    // Attach the filter graph to the capture graph
    hr = pCapture->SetFiltergraph(pGraph);
    if(FAILED(hr))
    {
        MessageBox(NULL, TEXT("Failed to set capture filter graph!"), TEXT("Error"), MB_OK);
        return(hr);
    }

    // Set the window handle used to process graph events
    hr = pEvent->SetNotifyWindow((OAHWND)hWnd, WM_GRAPHNOTIFY, 0);
	return(hr);
}


void VideoWin::CloseInterfaces(void)
{
	// Stop previewing data
	if(pControl)
		pControl->StopWhenReady();

	// Stop receiving events
	if(pEvent)
		pEvent->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);

	// Release DirectShow interfaces
	SAFE_RELEASE(pWcontrol);
	SAFE_RELEASE(pWmixer);
	SAFE_RELEASE(pVmr);

	SAFE_RELEASE(pControl);
	SAFE_RELEASE(pEvent);
	SAFE_RELEASE(pGraph);
	SAFE_RELEASE(pCapture);

	if(hDcVid)
		ReleaseDC(hWnd, hDcVid);
}


HRESULT VideoWin::SetupVideoWindow(HWND hWnd)
{
    HRESULT hr = S_FALSE;
	long lWidth, lHeight;
	RECT rcSrc, rcDest;

	ghApp = hWnd;
	if(hDcVid) ReleaseDC(hWnd, hDcVid);
	hDcVid = GetDC(hWnd);
	if(pWcontrol)
	{
		hr = pWcontrol->SetVideoClippingWindow(hWnd);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("SetVideoClippingWindow failed"), TEXT("Error"), MB_OK);
			return(hr);
		}
		hr = pWcontrol->SetAspectRatioMode(VMR_ARMODE_NONE);
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("SetAspectRatioMode failed"), TEXT("Error"), MB_OK);
			return(hr);
		}

		hr = pWcontrol->GetNativeVideoSize(&lWidth, &lHeight, NULL, NULL);
		SetRect(&rcSrc, 0, 0, lWidth, lHeight);
		GetClientRect(hWnd, &rcDest);
		hr = pWcontrol->SetVideoPosition(&rcSrc, &rcDest);

		hr = pWcontrol->SetBorderColor(RGB(0,0,0));
		if(FAILED(hr))
		{
			MessageBox(NULL, TEXT("SetBorderColor failed"), TEXT("Error"), MB_OK);
			return(hr);
		}
	}
    return(hr);
}


void VideoWin::PaintVideoWindow(HWND hWnd)
{
	if(pWcontrol)
		pWcontrol->RepaintVideo(hWnd, hDcVid);
}


void VideoWin::ChangeVideoWindow(void)
{
	if(pWcontrol)
		pWcontrol->DisplayModeChanged();
}


void VideoWin::ResizeVideoWindow(HWND hWnd)
{
	long lWidth, lHeight;
	RECT rcSrc, rcDest;

    if(pWcontrol)
    {
		pWcontrol->GetNativeVideoSize(&lWidth, &lHeight, NULL, NULL);
		SetRect(&rcSrc, 0, 0, lWidth, lHeight);
		GetClientRect(hWnd, &rcDest);
		pWcontrol->SetVideoPosition(&rcSrc, &rcDest);
    }
}


HRESULT VideoWin::ChangePreviewState(HWND hWnd, BOOL vShow)
{
	HRESULT hr=S_OK;
	OAFilterState	state;

	// If the media control interface isn't ready, don't call it
	if(!pControl)
		return S_FALSE;

	pControl->GetState(10, &state);

	if(vShow && (state != State_Running))
		hr = pControl->Run();
	else if(!vShow && (state == State_Running))
		hr = pControl->StopWhenReady();

	return(hr);
}


HRESULT VideoWin::SetMirrorMode(BOOL vMirror)
{
	CHAR vVar[20];
	BOOL vCurrent;
	NORMALIZEDRECT  r;

	sprintf(vVar, "Camera%dMirror", vNumber);
	vMirrorCurrent = pAi->dB->GetBool(vVar) ? TRUE : FALSE;
	if(vMirror != vMirrorCurrent)
		pAi->dB->SetConfig(vVar, &vMirror, CONFIGTYPE_BOOL);
	vMirrorCurrent = vMirror;

	PostMessage(pAi->hPrnt, WM_MIRRORMODE, 0, (LPARAM)vMirrorCurrent);

	r.left = vMirror ? 1 : 0;
	r.right = vMirror ? 0 : 1;
	r.top = 0;
	r.bottom = 1;

	return(pWmixer->SetOutputRect(0, &r));
}

HRESULT VideoWin::Record(BOOL vYesNo)
{
	return(S_FALSE);
}



LRESULT CALLBACK VideoWinProc(HWND hWnd, UINT vMsg, WPARAM wParam, LPARAM lParam)
{
	VideoWin	*my;
	PAINTSTRUCT	ps;
    RECT		ourRect;

	my = (VideoWin*)GetWindowLong(hWnd, GWL_USERDATA);
	if((my == NULL) && (vMsg != WM_NCCREATE))
		return(DefWindowProc(hWnd, vMsg, wParam, lParam));

	switch(vMsg)
	{
		case WM_NCCREATE:
			my = (VideoWin*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLong(hWnd, GWL_USERDATA, (LONG)my);
			return(my ? DefWindowProc(hWnd, vMsg, wParam, lParam) : FALSE);

		case WM_SIZE:
		case WM_WINDOWPOSCHANGING:
			my->ResizeVideoWindow(hWnd);
			break;

		case WM_PAINT:
			BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			my->PaintVideoWindow(hWnd);
			break;

		case WM_WINDOWPOSCHANGED:
			if(((WINDOWPOS*)lParam)->cx > 100)
			{
				my->ChangePreviewState(hWnd, TRUE);
				my->ResizeVideoWindow(hWnd);
			}
			else
				my->ChangePreviewState(hWnd, FALSE);
			break;

		case WM_DISPLAYCHANGE:
			my->ChangeVideoWindow();
			break;

		default:
			return(DefWindowProc(hWnd, vMsg, wParam, lParam));
   }
   return(0);
}


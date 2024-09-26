//------------------------------------------------------------------------------
// File: PlayCap.cpp
//
// Desc: DirectShow sample code - a very basic application using video capture
//       devices.  It creates a window and uses the first available capture
//       device to render and preview video capture data.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------



#include "PlayCap.h"
#include <atlbase.h>

#include <D3d9.h>
#include <Vmr9.h>

//
// Global data
//
HWND ghApp=0;
DWORD g_dwGraphRegister=0;
HDC	hDcVid = NULL;

IMediaControl * g_pMC = NULL;
IMediaEventEx * g_pME = NULL;
IGraphBuilder * g_pGraph = NULL;
ICaptureGraphBuilder2 * g_pCapture = NULL;
PLAYSTATE g_psCurrent = Stopped;

IBaseFilter           * pVmr; 
IVMRWindowlessControl * pWcontrol;
IVMRMixerControl      * pWmixer; 

HRESULT CaptureVideo(HWND hWnd)
{
    HRESULT hr;
    IBaseFilter *pSrcFilter=NULL;

    // Get DirectShow interfaces
    hr = GetInterfaces(hWnd);
    if (FAILED(hr))
    {
        Msg(TEXT("Failed to get video interfaces!  hr=0x%x"), hr);
        return hr;
    }

    // Attach the filter graph to the capture graph
    hr = g_pCapture->SetFiltergraph(g_pGraph);
    if (FAILED(hr))
    {
        Msg(TEXT("Failed to set capture filter graph!  hr=0x%x"), hr);
        return hr;
    }

    // Set video window style and position
    hr = SetupVideoWindow(hWnd);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't initialize video window!  hr=0x%x"), hr);
        return hr;
    }

    // Use the system device enumerator and class enumerator to find
    // a video capture/preview device, such as a desktop USB video camera.
    hr = FindCaptureDevice(&pSrcFilter);
    if (FAILED(hr))
    {
        // Don't display a message because FindCaptureDevice will handle it
        return hr;
    }
   
    // Add Capture filter to our graph.
    hr = g_pGraph->AddFilter(pSrcFilter, L"Video Capture");
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't add the capture filter to the graph!  hr=0x%x\r\n\r\n") 
            TEXT("If you have a working video capture device, please make sure\r\n")
            TEXT("that it is connected and is not being used by another application.\r\n\r\n")
            TEXT("The sample will now close."), hr);
        pSrcFilter->Release();
        return hr;
    }

	// Render the preview pin on the video capture filter
    // Use this instead of g_pGraph->RenderFile
    hr = g_pCapture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pSrcFilter, NULL, pVmr);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't render the video capture stream.  hr=0x%x\r\n")
            TEXT("The capture device may already be in use by another application.\r\n\r\n")
            TEXT("The sample will now close."), hr);
        pSrcFilter->Release();
        return hr;
    }

    // Now that the filter has been added to the graph and we have
    // rendered its stream, we can release this reference to the filter.
    pSrcFilter->Release();

    // Set video window style and position
    hr = SetupVideoWindow(hWnd);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't initialize video window!  hr=0x%x"), hr);
        return hr;
    }

    // Start previewing video data
    hr = g_pMC->Run();
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't run the graph!  hr=0x%x"), hr);
        return hr;
    }

    // Remember current state
    g_psCurrent = Running;
        
    return S_OK;
}


HRESULT FindCaptureDevice(IBaseFilter ** ppSrcFilter)
{
    HRESULT hr;
    IBaseFilter * pSrc = NULL;
    CComPtr <IMoniker> pMoniker =NULL;
    ULONG cFetched;

    if (!ppSrcFilter)
        return E_POINTER;
   
    // Create the system device enumerator
    CComPtr <ICreateDevEnum> pDevEnum =NULL;

    hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
                           IID_ICreateDevEnum, (void **) &pDevEnum);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't create system enumerator!  hr=0x%x"), hr);
        return hr;
    }

    // Create an enumerator for the video capture devices
    CComPtr <IEnumMoniker> pClassEnum = NULL;

    hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't create class enumerator!  hr=0x%x"), hr);
        return hr;
    }

    // If there are no enumerators for the requested type, then 
    // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
    if (pClassEnum == NULL)
    {
        MessageBox(ghApp,TEXT("No video capture device was detected.\r\n\r\n")
                   TEXT("This sample requires a video capture device, such as a USB WebCam,\r\n")
                   TEXT("to be installed and working properly.  The sample will now close."),
                   TEXT("No Video Capture Hardware"), MB_OK | MB_ICONINFORMATION);
        return E_FAIL;
    }

    // Use the first video capture device on the device list.
    // Note that if the Next() call succeeds but there are no monikers,
    // it will return S_FALSE (which is not a failure).  Therefore, we
    // check that the return code is S_OK instead of using SUCCEEDED() macro.
    if (S_OK == (pClassEnum->Next (1, &pMoniker, &cFetched)))
    {
        // Bind Moniker to a filter object
        hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);
        if (FAILED(hr))
        {
            Msg(TEXT("Couldn't bind moniker to filter object!  hr=0x%x"), hr);
            return hr;
        }
    }
    else
    {
        Msg(TEXT("Unable to access video capture device!"));   
        return E_FAIL;
    }

    // Copy the found filter pointer to the output parameter.
    // Do NOT Release() the reference, since it will still be used
    // by the calling function.
    *ppSrcFilter = pSrc;

    return hr;
}

void GraphNotify(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//    if (g_pVW)
//        g_pVW->NotifyOwnerMessage((LONG_PTR) hwnd, message, wParam, lParam);
	//if(pWcontrol)
	//	pWcontrol->DisplayModeChanged();
}


HRESULT GetInterfaces(HWND hWnd)
{
	HRESULT hr;
	IVMRFilterConfig* pVmrConf; 

	// Create the filter graph
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                           IID_IGraphBuilder, (LPVOID*)&g_pGraph);
	if(FAILED(hr))
		return hr;

    // Create the capture graph builder
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,
                           IID_ICaptureGraphBuilder2, (LPVOID*)&g_pCapture);
    if (FAILED(hr))
        return hr;
    
    // Obtain interfaces for media control and Video Window
    hr = g_pGraph->QueryInterface(IID_IMediaControl,(LPVOID*)&g_pMC);
    if (FAILED(hr))
        return hr;

    hr = g_pGraph->QueryInterface(IID_IMediaEvent, (LPVOID*)&g_pME);
    if (FAILED(hr))
        return hr;

	hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC, IID_IBaseFilter, (LPVOID*)&pVmr); 
	if(FAILED(hr))
	{
		MessageBox(NULL, TEXT("CLSID_VideoMixingRenderer failed"), TEXT("Error"), MB_OK);
		return hr;
	}
	// Add the VMR to the filter graph.
	hr = g_pGraph->AddFilter(pVmr, L"Video Mixing Renderer"); 
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

    // Set the window handle used to process graph events
    hr = g_pME->SetNotifyWindow((OAHWND)hWnd, WM_GRAPHNOTIFY, 0);

    return hr;
}


void CloseInterfaces(void)
{
    // Stop previewing data
    if (g_pMC)
        g_pMC->StopWhenReady();

    g_psCurrent = Stopped;

    // Stop receiving events
    if (g_pME)
        g_pME->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);

    // Release DirectShow interfaces
    SAFE_RELEASE(pWcontrol);
	SAFE_RELEASE(pWmixer);
	SAFE_RELEASE(pVmr);
	
	SAFE_RELEASE(g_pMC);
    SAFE_RELEASE(g_pME);
    SAFE_RELEASE(g_pGraph);
    SAFE_RELEASE(g_pCapture);

	if(hDcVid) ReleaseDC(ghApp, hDcVid);
}


HRESULT SetupVideoWindow(HWND hWnd)
{
    HRESULT hr = S_OK;
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
    return hr;
}

void PaintVideoWindow(HWND hWnd)
{
	if(pWcontrol)
		pWcontrol->RepaintVideo(hWnd, hDcVid);
}

void ChangeVideoWindow(void)
{
	if(pWcontrol)
		pWcontrol->DisplayModeChanged();
}

void ResizeVideoWindow(HWND hWnd)
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


HRESULT ChangePreviewState(HWND hWnd, int nShow)
{
	HRESULT hr=S_OK;

	// If the media control interface isn't ready, don't call it
	if (!g_pMC)
		return S_OK;

	if(nShow)
	{
		if(g_psCurrent != Running)
		{
			// Start previewing video data
			hr = g_pMC->Run();
			g_psCurrent = Running;
		}
	}
	else
	{
		// Stop previewing video data
		hr = g_pMC->StopWhenReady();
		g_psCurrent = Stopped;
	}
	return hr;
}

HRESULT SetMirrorMode(BOOL vYesNo)
{
	NORMALIZEDRECT  r;

	r.left = vYesNo ? 1 : 0;
	r.right = vYesNo ? 0 : 1;
	r.top = 0;
	r.bottom = 1;

	return(pWmixer->SetOutputRect(0, &r));
}


void Msg(TCHAR *szFormat, ...)
{
	TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
	const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
	const int LASTCHAR = NUMCHARS - 1;

	// Format the input string
	va_list pArgs;
	va_start(pArgs, szFormat);

	// Use a bounded buffer size to prevent buffer overruns.  Limit count to
	// character size minus one to allow for a NULL terminating character.
	_vsntprintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
	va_end(pArgs);

	// Ensure that the formatted string is NULL-terminated
	szBuffer[LASTCHAR] = TEXT('\0');

	MessageBox(NULL, szBuffer, TEXT("PlayCap Message"), MB_OK | MB_ICONERROR);
}


HRESULT HandleGraphEvent(void)
{
    LONG evCode, evParam1, evParam2;
    HRESULT hr=S_OK;

    if (!g_pME)
        return E_POINTER;

    while(SUCCEEDED(g_pME->GetEvent(&evCode, (LONG_PTR *) &evParam1, 
                   (LONG_PTR *) &evParam2, 0)))
    {
        //
        // Free event parameters to prevent memory leaks associated with
        // event parameter data.  While this application is not interested
        // in the received events, applications should always process them.
        //
        hr = g_pME->FreeEventParams(evCode, evParam1, evParam2);
        
        // Insert event processing code here, if desired
    }

    return hr;
}





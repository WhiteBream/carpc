// ListAudioDevs.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <shlobj.h>


static HRESULT FindRadioInputDevice(void);
static HRESULT FindSoundDevice(void);


int _tmain(int argc, _TCHAR* argv[])
{
	HRESULT hr;

	hr = CoInitialize(NULL);
	if(FAILED(hr))
		printf("CoInitialize() failed");

	printf("Available input devices:\n");
	hr = FindRadioInputDevice();
    if(FAILED(hr))
        printf("FindRadioInputDevice failed with 0x%x\n", hr);

	printf("\nAvailable output devices:\n");
	hr = FindSoundDevice();
    if(FAILED(hr))
        printf("FindRadioInputDevice failed with 0x%x\n", hr);

	CoUninitialize();
	return 0;
}

#include "DShow.h"

#pragma comment(lib, "Strmiids.lib")


static HRESULT FindRadioInputDevice(void)
{
	HRESULT hr;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pClassEnum = NULL;
	IMoniker *pMoniker = NULL;
	ULONG cFetched;

	// Create the system device enumerator
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
						IID_ICreateDevEnum, (void **)&pDevEnum);
	if(FAILED(hr))
		return(hr);

    // Create an enumerator for the audio output devices
    hr = pDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pClassEnum, 0);
    if (FAILED(hr))
        return(hr);

	// If there are no enumerators for the requested type, then 
	// CreateClassEnumerator will succeed, but pClassEnum will be NULL.
	if(pClassEnum == NULL)
        return(hr);

	// Note that if the Next() call succeeds but there are no monikers,
	// it will return S_FALSE (which is not a failure).  Therefore, we
	// check that the return code is S_OK instead of using SUCCEEDED() macro.
	while(S_OK == (pClassEnum->Next(1, &pMoniker, &cFetched)))
	{
		IPropertyBag *pPropBag;
		pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
		if(SUCCEEDED(hr))
		{
			// To retrieve the filter's friendly name, do the following:
			VARIANT varName;
			VariantInit(&varName);
			hr = pPropBag->Read(L"FriendlyName", &varName, 0);
			if(SUCCEEDED(hr))			
				wprintf(_T(" %s\n"), varName.bstrVal);

			VariantClear(&varName);
			pPropBag->Release();
		}
		pMoniker->Release();
    }
	pClassEnum->Release();
	pDevEnum->Release();

	return(hr);
}

static HRESULT FindSoundDevice(void)
{
	HRESULT hr;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pClassEnum = NULL;
	IMoniker *pMoniker = NULL;
	ULONG cFetched;

	// Create the system device enumerator
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
						IID_ICreateDevEnum, (void **)&pDevEnum);
	if(FAILED(hr))
		return(hr);

    // Create an enumerator for the audio output devices
    hr = pDevEnum->CreateClassEnumerator(CLSID_AudioRendererCategory, &pClassEnum, 0);
    if (FAILED(hr))
        return(hr);

	// If there are no enumerators for the requested type, then 
	// CreateClassEnumerator will succeed, but pClassEnum will be NULL.
	if(pClassEnum == NULL)
        return(hr);

	// Note that if the Next() call succeeds but there are no monikers,
	// it will return S_FALSE (which is not a failure).  Therefore, we
	// check that the return code is S_OK instead of using SUCCEEDED() macro.
	while(S_OK == (pClassEnum->Next(1, &pMoniker, &cFetched)))
	{
		IPropertyBag *pPropBag;
		pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
		if(SUCCEEDED(hr))
		{
			// To retrieve the filter's friendly name, do the following:
			VARIANT varName;
			VariantInit(&varName);
			//http://msdn2.microsoft.com/en-us/library/ms787619.aspx
			hr = pPropBag->Read(L"FriendlyName", &varName, 0);
			if(SUCCEEDED(hr))
				wprintf(_T(" %s\n"), varName.bstrVal);

			VariantClear(&varName);
			pPropBag->Release();
		}
		pMoniker->Release();
    }
	pClassEnum->Release();
	pDevEnum->Release();

	return(hr);
}


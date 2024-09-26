//-----------------------------------------------------------------------
//  This file is part of the Destinator SDK Code Samples.
// 
//  Copyright (C) Paradigm Corporation.  All rights reserved.
// 
//This source code is intended only as a supplement to Destinator
//Development Tools and/or documentation.  See these other
//materials for detailed information regarding Destinator code samples.
//
//THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
//KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//PARTICULAR PURPOSE.
//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
// Destinator SDK comment
//
// you must include DestDll.h
// 
//-----------------------------------------------------------------------

#pragma once
//#include "DestDll_i.c"
#include "DestDll.h"


class CNotify : public _IDestEvents2
{
public:
	BYTE m_Buf[0xFFFF];
	long m_ByteConter;
	
	CNotify();
	~CNotify();

	// IUnknown methods
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP QueryInterface(REFIID riid, void** ppv);


	void STDMETHODCALLTYPE OnGetDataFromCOMPort(long NumberOfBytes, const BYTE * ByteArray);
	void STDMETHODCALLTYPE OnCloseMainDestinatorWindow();
	void STDMETHODCALLTYPE OnDestinatorError(long ErrorNotification);
	void STDMETHODCALLTYPE OnYouHaveArrived();
	void STDMETHODCALLTYPE OnCreateDestinatorWindow(HWND hWnd);
	void STDMETHODCALLTYPE OnSwapMap(BSTR MapName);

};




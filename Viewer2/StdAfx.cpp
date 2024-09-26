// stdafx.cpp : source file that includes just the standard includes
//	player.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

void StripA(LPSTR lpString)
{
	LPSTR  pTmp;
	size_t vLen;

	while(*lpString == ' ')
	{
		pTmp = lpString;
		for(vLen = strlen(lpString); vLen; vLen--)
            *pTmp++ = *(pTmp + 1);
	}

	pTmp = &lpString[strlen(lpString)];
	while((*pTmp == '\0') || (*pTmp == ' '))
	{
		*pTmp = '\0';
		if(pTmp == lpString) break;
		pTmp--;
	}
}

void StripW(LPWSTR lpString)
{
	LPWSTR  pTmp;
	size_t vLen;

	while(*lpString == L' ')
	{
		pTmp = lpString;
		for(vLen = wcslen(lpString); vLen; vLen--)
            *pTmp++ = *(pTmp + 1);
	}

	pTmp = &lpString[wcslen(lpString)];
	while((*pTmp == L'\00') || (*pTmp == L' '))
	{
		*pTmp = L'\00';
		if(pTmp == lpString) break;
		pTmp--;
	}
}

void AssignIcon(HINSTANCE hInstance, HWND hWnd, int vControl, int vIcon)
{
	HICON ico;

	if(vIcon)
	{
		ico = LoadIcon(hInstance, (LPCTSTR)vIcon);
		SendDlgItemMessage(hWnd, vControl, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)ico);
		DestroyIcon(ico);
	}
	else
		SendDlgItemMessage(hWnd, vControl, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)NULL);
}

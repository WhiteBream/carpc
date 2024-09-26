// stdafx.cpp : source file that includes just the standard includes
//	player.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

void Strip(LPSTR lpString)
{
	LPSTR pTmp = &lpString[strlen(lpString)];
	while((*pTmp == '\0') || (*pTmp == ' ')) 
	{
		*pTmp = '\0';
		if(pTmp == lpString) break;
		pTmp--;
	}
}

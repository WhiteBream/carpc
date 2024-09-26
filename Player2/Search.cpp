// Search.cpp: implementation of the Search class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Search.h"
#include "List.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define MAX_RECURSE_DEPTH       25
#define EXTENSION_SEPARATOR     TEXT('.')

typedef struct _SearchThreadParm
{
	HWND        hPrnt;
	HANDLE		*hSync;
	Database	*dBase;
	struct		_hSearch
	{
		HANDLE	hFind;
		LPTSTR	pFile;
	} hSearch[MAX_RECURSE_DEPTH];
} SearchThreadParm;


BOOL IsFileMedia(LPCTSTR lpFileName);
void AddAlbum(BOOL driveType, LPCTSTR lpPath, LPCTSTR mp3File);
DWORD WINAPI SearchThread(LPVOID lpParam);
SearchThreadParm * sp;

static LONG    fileCount = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Search::Search(HWND hParent, Database * pDB)
{
	HANDLE hEvent;

	sp = (SearchThreadParm *)GlobalAlloc(GPTR, sizeof(SearchThreadParm));
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	sp->hSync = &hEvent;
	sp->dBase = pDB;
	sp->hPrnt = hParent;
	hThread = CreateThread(NULL, 0, SearchThread, sp, 0, &vThread);
	if(hThread == NULL)
	{
		CloseHandle(hEvent);
		GlobalFree((HGLOBAL)sp);
		MessageBox(NULL, TEXT("Failed to start search engine"), TEXT("MediaPlayer"), MB_OK | MB_ICONSTOP);
	}
	else 
	{
		if(WaitForSingleObject(hEvent, 1000) != WAIT_OBJECT_0)
		{
			MessageBox(NULL, TEXT("Failed to sync search engine"), TEXT("MediaPlayer"), MB_OK | MB_ICONSTOP);
			CloseHandle(hThread);
			hThread = NULL;
		}
		else
			SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);

		CloseHandle(hEvent);
		sp->hSync = NULL;
	}

}

Search::~Search()
{
	HANDLE hStop;

	hStop = CreateEvent(NULL, FALSE, FALSE, NULL);
	sp->hSync = &hStop;

	if(PostThreadMessage(vThread, WM_QUIT, 0, 0) == 0)
		MessageBox(NULL, TEXT("PostThreadMessage failed"), TEXT("Quit searchthread"), MB_OK | MB_ICONSTOP);

	WaitForSingleObject(hStop, 2500);
	//if(WaitForSingleObject(hStop, 2500) == WAIT_TIMEOUT)
	//	MessageBox(NULL, TEXT("Timeout waiting for search to shutdown"), TEXT("Quit searchthread"), MB_OK | MB_ICONSTOP);

	CloseHandle(hStop);
}

void Search::GarbageCollection(void)
{
	if(PostThreadMessage(vThread, WM_GARBAGECOLLECTION, 0, 0) == 0)
		MessageBox(NULL, TEXT("PostThreadMessage failed"), TEXT("GarbageCollection"), MB_OK | MB_ICONSTOP);
}

void Search::AddFolder(BOOL vPrio, LPCTSTR pStr)
{
	LPTSTR pCopy;
	size_t vLen;

	if(pStr == NULL)
		return;

	StringCchLength(pStr, MAX_PATH, &vLen);
	if(vLen)
	{
		pCopy = (LPTSTR)GlobalAlloc(GPTR, ++vLen * sizeof(TCHAR));
		if(pCopy)
		{
			StringCchPrintf(pCopy, vLen, TEXT("%s"), pStr);
			if(PostThreadMessage(vThread, WM_RECEIVESTRING, (WPARAM)vPrio, (LPARAM)pCopy) == 0)
				MessageBox(NULL, TEXT("PostThreadMessage failed"), TEXT("list entry add"), MB_OK | MB_ICONSTOP);
		}
	}
}

bool DefaultProcessor(LPTSTR pFile, LPSTR pTitle, LPSTR pArtist, LPSTR pAlbum, INT vItemLen, LPSTR pYear, LPINT pGenre, LPINT pTrack)
{
	LPTSTR	pFileName;
	LPSTR	pFileTmp;

	if(pTitle[0] == '\0')
	{
		// Get filename without path
		pFileName = _tcsrchr(pFile, TEXT('\\'));
		if(!pFileName)
			pFileName = pFile;
		pFileName++;

#ifdef UNICODE
		StringCchPrintfA(pTitle, vItemLen, "%ls", pFileName);
#else
		StringCchPrintfA(pTitle, vItemLen, "%hs", pFileName);
#endif
		// Get rid of file extension
		pFileTmp = strrchr(pTitle, '.');
		if(pFileTmp)
			*pFileTmp = '\0';

		// Scan for internet escapes (%xx)
		pFileTmp = pTitle;
		do
		{
			if((*pFileTmp == '%') && ((pFileTmp[1] >= '0') && (pFileTmp[1] <= '9')))
			{
				CHAR vByte;
				if(sscanf(&pFileTmp[1], "%02X", &vByte) == 1)
				{
					*pFileTmp = vByte;

					for(LPSTR pScan = pFileTmp + 1; (pScan < (pTitle + vItemLen)) && *pScan; pScan++)
						*pScan = *(pScan + 2);
				}
			}
			pFileTmp++;
		} while((pFileTmp < (pTitle + vItemLen)) && *pFileTmp);

		//Try to find artist name in filename
		if(strchr(pTitle, '-'))
		{
			StringCchCopyA(pArtist, vItemLen, pTitle);
			pFileTmp = strchr(pArtist, '-');
			if(pFileTmp)
				*pFileTmp = '\0';
			StripA(pArtist);

			// Search for current artist string in dbase
			if(sp->dBase->FindString(FLD_ARTIST, pArtist) == 0)
			{
				// Strip leading numerics that are followed by a space
				pFileTmp = pArtist;
				while((*pFileTmp >= '0') && (*pFileTmp <= '9'))
				{
					if(pFileTmp[1] == ' ')
					{
						pFileTmp = pArtist;
						while((*pFileTmp >= '0') && (*pFileTmp <= '9'))
							*pFileTmp++ = ' ';
						break;
					}
					pFileTmp++;
				}
				StripA(pArtist);
				if(sp->dBase->FindString(FLD_ARTIST, pArtist) == 0)
				{
					// Strip trailing numerics that are separated with a leading space
					pFileTmp = &pArtist[strlen(pArtist) - 1];
					while((*pFileTmp >= '0') && (*pFileTmp <= '9'))
					{
						if(*(pFileTmp - 1) == ' ')
						{
							pFileTmp = &pArtist[strlen(pArtist) - 1];
							while((*pFileTmp >= '0') && (*pFileTmp <= '9'))
								*pFileTmp-- = ' ';
							break;
						}
						pFileTmp--;
					}
					StripA(pArtist);
				}
			}
			// Seems artist string was found...
			if(pArtist[0])
			{
				pFileTmp = pTitle;
				while(*pFileTmp != '-')
					*pFileTmp++ = ' ';
				*pFileTmp = ' ';
				StripA(pTitle);

				// Check if still some title left
				if(pTitle[0] == 0)
				{
					StringCchCopyA(pTitle, vItemLen, pArtist);
					memset(pArtist, 0, vItemLen);
				}
			}
		}
	}
	return(FALSE);
}

bool Mp3Processor(LPTSTR pFile, LPSTR pTitle, LPSTR pArtist, LPSTR pAlbum, INT vItemLen, LPSTR pYear, LPINT pGenre, LPINT pTrack)
{
	HANDLE	hFile;
	CHAR	sTag[3] = {0};
	DWORD	vReadLen;

	if(vItemLen < 30)
		return(FALSE);

	hFile = CreateFile(pFile, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile)
	{
		pTitle[0] = '\0';
		/* Read ID3V1 information */
		SetFilePointer(hFile, -128, NULL, FILE_END);
		ReadFile(hFile, sTag, sizeof(sTag), &vReadLen, NULL);
		if(StrCmpNA(sTag, "TAG", 3) == 0)
		{
			ReadFile(hFile, pTitle, 30, &vReadLen, NULL);
			StripA(pTitle);
			ReadFile(hFile, pArtist, 30, &vReadLen, NULL);
			StripA(pArtist);
			ReadFile(hFile, pAlbum, 30, &vReadLen, NULL);
			StripA(pAlbum);
			ReadFile(hFile, pYear, 4, &vReadLen, NULL);
			SetFilePointer(hFile, 28, NULL, FILE_CURRENT);
			ReadFile(hFile, pTrack, 1, &vReadLen, NULL);
			if(*pTrack == -256)
				ReadFile(hFile, pTrack, 1, &vReadLen, NULL);
			else *pTrack = -1;
			ReadFile(hFile, pGenre, 1, &vReadLen, NULL);
		}
	}
	CloseHandle(hFile);
	return(pTitle[0] ? TRUE : FALSE);
}

struct _FileTypeProcessor
{
	LPTSTR pExt;
	bool (*pProcessor)(LPTSTR pFile, LPSTR pTitle, LPSTR pArtist, LPSTR pAlbum, INT vItemLen, LPSTR pYear, LPINT pGenre, LPINT pTrack);
} vFileTypeProcessor[] = 
{
	{TEXT("MP3"), Mp3Processor},
	{NULL, DefaultProcessor},
	{NULL, NULL}
};

void FormatNames(LPSTR pTitle, LPSTR pArtist, LPSTR pAlbum, INT vItemLen)
{
	LPSTR	pCatText;
	volatile LPSTR	pTmpText;
	INT		vCountLow, vCountUp;
	size_t	vLen;

	vLen = 3 * (vItemLen + 1);
	pCatText = (LPSTR)GlobalAlloc(GPTR, vLen);
	if(pCatText)
	{
		// Concat all three strings
		if(pTitle)
		{
			StringCbCatA(pCatText, vLen, pTitle);
			StringCbCatA(pCatText, vLen, " ");
		}
		if(pArtist)
		{
			StringCbCatA(pCatText, vLen, pArtist);
			StringCbCatA(pCatText, vLen, " ");
		}
		if(pAlbum)
			StringCbCatA(pCatText, vLen, pAlbum);

		// Convert underscores to spaces if more that 1 exists.
		pTmpText = pCatText;
		vCountUp = 0;
		while((pTmpText = strchr(pTmpText, '_')) != NULL)
		{
			pTmpText++;
			vCountUp++;
		}
		if(vCountUp > 1)
		{
			pTmpText = pTitle;
			while((pTmpText = strchr(pTmpText, '_')) != NULL)
				*pTmpText++ = ' ';

			pTmpText = pArtist;
			while((pTmpText = strchr(pTmpText, '_')) != NULL)
				*pTmpText++ = ' ';

			pTmpText = pAlbum;
			while((pTmpText = strchr(pTmpText, '_')) != NULL)
				*pTmpText++ = ' ';
		}
		GlobalFree(pCatText);

		// Convert string to lowercase if only uppercase chars present
		pTmpText = pTitle;
		vCountLow = vCountUp = 0;
		while(*pTmpText)
		{
			if((*pTmpText >= 'a') && (*pTmpText <= 'z'))
				vCountLow++;
			if((*pTmpText >= 'A') && (*pTmpText <= 'Z'))
				vCountUp++;
			pTmpText++;
		}
		if((vCountLow == 0) && (vCountUp > 4))
			if(pTitle) strlwr(pTitle);

		pTmpText = pArtist;
		vCountLow = vCountUp = 0;
		while(*pTmpText)
		{
			if((*pTmpText >= 'a') && (*pTmpText <= 'z'))
				vCountLow++;
			if((*pTmpText >= 'A') && (*pTmpText <= 'Z'))
				vCountUp++;
			pTmpText++;
		}
		if((vCountLow == 0) && (vCountUp > 4))
			if(pArtist) strlwr(pArtist);

		pTmpText = pAlbum;
		vCountLow = vCountUp = 0;
		while(*pTmpText)
		{
			if((*pTmpText >= 'a') && (*pTmpText <= 'z'))
				vCountLow++;
			if((*pTmpText >= 'A') && (*pTmpText <= 'Z'))
				vCountUp++;
			pTmpText++;
		}
		if((vCountLow == 0) && (vCountUp > 4))
			if(pAlbum) strlwr(pAlbum);

		// Convert first characters to uppercase if only lowercase chars present
		pTmpText = pTitle;
		vCountLow = 0;
		while(*pTmpText)
		{
			if((*pTmpText >= 'A') && (*pTmpText <= 'Z'))
				vCountLow++;
			pTmpText++;
		}
		if(vCountLow == 0)
		{
			pTmpText = pTitle;
			if((*pTmpText >= 'a') && (*pTmpText <= 'z'))
				*pTmpText -= ('a' - 'A');
			while((pTmpText = strchr(pTmpText, ' ')) != NULL)
			{
				pTmpText++;
				if((*pTmpText >= 'a') && (*pTmpText <= 'z'))
					*pTmpText -= ('a' - 'A');
			}
		}
		pTmpText = pArtist;
		vCountLow = 0;
		while(*pTmpText)
		{
			if((*pTmpText >= 'A') && (*pTmpText <= 'Z'))
				vCountLow++;
			pTmpText++;
		}
		if(vCountLow == 0)
		{
			pTmpText = pArtist;
			if((*pTmpText >= 'a') && (*pTmpText <= 'z'))
				*pTmpText -= ('a' - 'A');
			while((pTmpText = strchr(pTmpText, ' ')) != NULL)
			{
				pTmpText++;
				if((*pTmpText >= 'a') && (*pTmpText <= 'z'))
					*pTmpText -= ('a' - 'A');
			}
		}
		pTmpText = pAlbum;
		vCountLow = 0;
		while(*pTmpText)
		{
			if((*pTmpText >= 'A') && (*pTmpText <= 'Z'))
				vCountLow++;
			pTmpText++;
		}
		if(vCountLow == 0)
		{
			pTmpText = pAlbum;
			if((*pTmpText >= 'a') && (*pTmpText <= 'z'))
				*pTmpText -= ('a' - 'A');
			while((pTmpText = strchr(pTmpText, ' ')) != NULL)
			{
				pTmpText++;
				if((*pTmpText >= 'a') && (*pTmpText <= 'z'))
					*pTmpText -= ('a' - 'A');
			}
		}
	}
}

#define ITEMSIZE	256
bool AddAlbum(Database * dBase, LPCTSTR lpPath, LPCTSTR mp3File)
{
	HANDLE hFile;
	CHAR sFile[MAX_PATH];
	TCHAR pFile[MAX_PATH];
	LPTSTR pExt;
	CHAR sTag[4] = {0};
	CHAR sTitle[ITEMSIZE] = {0};
	CHAR sArtist[ITEMSIZE] = {0};
	CHAR sAlbum[ITEMSIZE] = {0};
	INT  nGenre = 0;
	CHAR sYear[5] = {0};
	INT  nTrack = -1;
	INT  i, n;
	BYTE vSample[256];
	UINT vId = 0;
	bool retVal = FALSE;
	TCHAR vExtension[10];
	DWORD	vReadLen;

	if((lpPath == NULL) || (mp3File == NULL))
		return(FALSE);

#ifdef UNICODE
	StringCchPrintfA(sFile, sizeof(sFile), "%ls\\%ls", lpPath, mp3File);
	StringCchPrintf(pFile, sizeof(pFile), TEXT("%ls\\%ls"), lpPath, mp3File);
#else
	StringCchPrintfA(sFile, sizeof(sFile), "%hs\\%hs", lpPath, mp3File);
	StringCchPrintf(pFile, sizeof(pFile), TEXT("%hs\\%hs"), lpPath, mp3File);
#endif

	/* Create file id */
	n = 0; 
	hFile = CreateFile(pFile, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile)
	{
		vId = GetFileSize(hFile, NULL);
		SetFilePointer(hFile, vId / 25, NULL, FILE_BEGIN);
		do
		{
			UINT vSampleSet;

			if(!ReadFile(hFile, vSample, sizeof(vSample), &vReadLen, NULL))
				break;

			for(i = 0; i < (sizeof(vSample) - 1); i++)
			{
				if(vSample[i] && (vSample[i] != vSample[i + 1]))
				{
					vSampleSet = (vSampleSet << 8) | vSample[i];
					n++;
				}
				if((i % 4) == 3)
					vId ^= vSampleSet;
			}
			vId = (vId << 3) | (vId >> 29);
		} while(n < 256);
		CloseHandle(hFile);
	}
	else
		return(FALSE);

	if(vId)
	{
		pExt = _tcsrchr(pFile, EXTENSION_SEPARATOR);
		if(!pExt) pExt = pFile;
		pExt++;
		StringCbCopy(vExtension, sizeof(vExtension), pExt);
		CharUpper(vExtension);

		i = 0;
		while(vFileTypeProcessor[i].pProcessor)
		{
			if(vFileTypeProcessor[i].pExt == NULL)
			{
				if(vFileTypeProcessor[i].pProcessor(pFile, sTitle, sArtist, sAlbum, ITEMSIZE, sYear, &nGenre, &nTrack))
					break;
			}
			else if(_tcscmp(vExtension, vFileTypeProcessor[i].pExt) == 0)
				if(vFileTypeProcessor[i].pProcessor(pFile, sTitle, sArtist, sAlbum, ITEMSIZE, sYear, &nGenre, &nTrack))
					break;
			i++;
		}
		FormatNames(sTitle, sArtist, sAlbum, ITEMSIZE);
		if(dBase->AddAlbum(sArtist, sAlbum, sTitle, nGenre, atoi(sYear), sFile, nTrack, vId, FT_MP3))
			retVal = TRUE;
	}
	return(retVal);
}

bool AddVideo(Database * dBase, LPCTSTR lpPath, LPCTSTR mp3File)
{
	HANDLE hFile;
	CHAR sFile[MAX_PATH];
	TCHAR pFile[MAX_PATH];
	LPTSTR pExt;
	CHAR sTitle[ITEMSIZE] = {0};
	CHAR sArtist[ITEMSIZE] = {0};
	CHAR sAlbum[ITEMSIZE] = {0};
	INT  nGenre = 0;
	CHAR sYear[5] = {0};
	INT  nTrack = -1;
	INT  i, n;
	BYTE vSample[256];
	UINT vId = 0;
	bool retVal = FALSE;
	TCHAR vExtension[10];
	DWORD	vReadLen;

	if((lpPath == NULL) || (mp3File == NULL))
		return(FALSE);

#ifdef UNICODE
	StringCchPrintfA(sFile, sizeof(sFile), "%ls\\%ls", lpPath, mp3File);
	StringCchPrintf(pFile, sizeof(pFile), TEXT("%ls\\%ls"), lpPath, mp3File);
#else
	StringCchPrintfA(sFile, sizeof(sFile), "%hs\\%hs", lpPath, mp3File);
	StringCchPrintf(pFile, sizeof(pFile), TEXT("%hs\\%hs"), lpPath, mp3File);
#endif

	/* Create file id */
	n = 0; 
	hFile = CreateFile(pFile, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile)
	{
		vId = GetFileSize(hFile, NULL);
		SetFilePointer(hFile, vId / 25, NULL, FILE_BEGIN);
		do
		{
			UINT vSampleSet;

			if(!ReadFile(hFile, vSample, sizeof(vSample), &vReadLen, NULL))
				break;

			for(i = 0; i < (sizeof(vSample) - 1); i++)
			{
				if(vSample[i] && (vSample[i] != vSample[i + 1]))
				{
					vSampleSet = (vSampleSet << 8) | vSample[i];
					n++;
				}
				if((i % 4) == 3)
					vId ^= vSampleSet;
			}
			vId = (vId << 3) | (vId >> 29);
		} while(n < 256);
		CloseHandle(hFile);
	}
	else
		return(FALSE);

	if(vId)
	{
		pExt = _tcsrchr(pFile, EXTENSION_SEPARATOR);
		if(!pExt) pExt = pFile;
		pExt++;
		StringCbCopy(vExtension, sizeof(vExtension), pExt);
		CharUpper(vExtension);

		i = 0;
		while(vFileTypeProcessor[i].pProcessor)
		{
			if(vFileTypeProcessor[i].pExt == NULL)
			{
				if(vFileTypeProcessor[i].pProcessor(pFile, sTitle, sArtist, sAlbum, ITEMSIZE, sYear, &nGenre, &nTrack))
					break;
			}
			else if(_tcscmp(vExtension, vFileTypeProcessor[i].pExt) == 0)
				if(vFileTypeProcessor[i].pProcessor(pFile, sTitle, sArtist, sAlbum, ITEMSIZE, sYear, &nGenre, &nTrack))
					break;
			i++;
		}
		if(!sAlbum[0])
		{
			LPSTR pDir = NULL, pName, pTmp;
			// No album name, create one from directory name
			pName = strrchr(sFile, '\\'); // last slash
			pTmp = sFile;
			do
			{
				// Scan through directory separators
				pTmp = strchr(pTmp, '\\');
				if(pTmp++ != pName)
					pDir = pTmp;
				else
					pTmp = NULL;
			} while(pTmp);
			if(pDir)
			{
				if(*(pDir-1) != ':')
					StringCbCopyA(sAlbum, sizeof(sAlbum), pDir);
				pDir = strrchr(sAlbum, '\\');
				if(pDir)
					*pDir = '\0';
			}
			if(!sAlbum[0])
				StringCbCopyA(sAlbum, sizeof(sAlbum), "Video");
		}
		FormatNames(sTitle, sArtist, sAlbum, ITEMSIZE);
		if(dBase->AddAlbum(sArtist, sAlbum, sTitle, nGenre, atoi(sYear), sFile, nTrack, vId, FT_VIDEO))
			retVal = TRUE;
	}
	return(retVal);
}

BOOL IsFileMedia(LPCTSTR lpFileName, BOOL vVideo)
{
    LPCTSTR extMedia[] = {TEXT("MPA"), TEXT("MP1"), TEXT("MP2"), TEXT("MP3"), TEXT("WAV"), TEXT("SND"), 
							TEXT("AIF"), TEXT("AIFF"), TEXT("AIFC"), TEXT("AU"), TEXT("WMA"), NULL};
    LPCTSTR extVideo[] = {TEXT("AVI"), TEXT("MPG"), TEXT("MPE"), TEXT("M1V"), TEXT("MPEG"), TEXT("ASF"), 
							TEXT("WMV"), TEXT("MOV"), TEXT("QT"), TEXT("DIVX"), NULL};
	LPTSTR *extType;
    LPTSTR  extStart;
	TCHAR   extName[10];
    INT     i;

	if(vVideo)
		extType = (LPTSTR*)extVideo;
	else
		extType = (LPTSTR*)extMedia;

    extStart = _tcsrchr(lpFileName, EXTENSION_SEPARATOR);
	if(extStart == NULL)	// No extension at all
	{
		return(FALSE);
	}
    extStart++;
	StringCbCopy(extName, sizeof(extName), extStart);
	CharUpper(extName);

    i = 0;
    while(extType[i])
    {
        if(_tcscmp(extName, extType[i]) == 0)
            return(TRUE);
        i++;
    }
    return(FALSE);
}

void UpdateCurrentDirectory(LPTSTR *pCurDir, LPCTSTR lpDir)
{
	DWORD	strSize;
	LPTSTR	pTmp;

	StringCchLength(lpDir, STRSAFE_MAX_CCH, (size_t*)&strSize);
	pTmp = (LPTSTR)GlobalAlloc(GPTR, (strSize + 2) * sizeof(TCHAR));
	if(pTmp)
	{
		StringCchCopy(pTmp, strSize + 1, lpDir);
		if(lpDir[strSize - 1] != TEXT('\\'))
			StringCchCat(pTmp, (strSize + 2) * sizeof(TCHAR), TEXT("\\"));
		SetCurrentDirectory(pTmp);
	}
	else
		SetCurrentDirectory(lpDir);
	strSize = GetCurrentDirectory(0, NULL);
	if(*pCurDir)
		GlobalFree((HGLOBAL)*pCurDir); 
	*pCurDir = (LPTSTR)GlobalAlloc(GPTR, strSize * sizeof(TCHAR)); 
	GetCurrentDirectory(strSize, *pCurDir);

	SendMessage(sp->hPrnt, WM_SEARCHPATH, (WPARAM)*pCurDir, 0);
}

void IncreaseFileCount(BOOL vUpdated, BOOL vReset)
{
	static LONG    addCount = 0, updCount = 0;

	if(vReset)
		addCount = updCount = 0;
	else if(vUpdated)
		updCount++;
	else
		addCount++;

	SendMessage(sp->hPrnt, WM_SEARCHCOUNT, (WPARAM)addCount, (LPARAM)updCount);
}

DWORD WINAPI SearchThread(LPVOID lpParam) 
{
	SearchThreadParm	*sp = (SearchThreadParm *)lpParam;
	DWORD				hThis;
	MSG					vMsg;
    WIN32_FIND_DATA		findData;
	LPTSTR				szFind = NULL;
	LPWSTR				pCurDir = NULL;
    INT					nextFind;
	Queue				*cQueue = NULL;
	INT					i;

	/* Create mailbox and tell constructor to continue */
	PeekMessage(&vMsg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	SetEvent(*(sp->hSync));
	hThis = GetCurrentThreadId();
	cQueue = new Queue();
	pCurDir = (LPTSTR)GlobalAlloc(GPTR, MAX_PATH);

	for(i = 0; i < MAX_RECURSE_DEPTH; i++)
	{
		sp->hSearch[i].hFind = INVALID_HANDLE_VALUE;
		sp->hSearch[i].pFile = NULL;
	}
// FindFirstChangeNotification
	/* Thread message queue */
	while(GetMessage(&vMsg, NULL, 0, 0) > 0)
	{
		switch(vMsg.message)
		{
			case WM_GARBAGECOLLECTION:
				sp->dBase->GarbageCollection();
				break;

			case WM_RECEIVESTRING:
				if(vMsg.lParam)
				{
					if(cQueue->Empty() && (sp->hSearch[0].hFind == INVALID_HANDLE_VALUE))
						IncreaseFileCount(FALSE, TRUE);

					cQueue->Add((BOOL)vMsg.wParam, TRUE, (LPVOID)vMsg.lParam);
					if(sp->hSearch[0].hFind == INVALID_HANDLE_VALUE)
						PostThreadMessage(hThis, WM_FINDFIRSTFILE, (WPARAM)0, 0);
				}
				break;

			case WM_FINDFIRSTFILE:
				nextFind = (INT)vMsg.wParam;

				if(sp->hSearch[0].hFind == INVALID_HANDLE_VALUE)
				{
					if(cQueue->Get((LPVOID*)&szFind) == FALSE)
						break;
				}

				do
				{
					// First search (on root) should always return result...
					if(nextFind == 0)
						UpdateCurrentDirectory(&pCurDir, szFind);
					sp->hSearch[nextFind].hFind = FindFirstFile(TEXT("*"), &findData); 
					if(sp->hSearch[nextFind].hFind == INVALID_HANDLE_VALUE)
					{
						if(nextFind == 0)
						{
							// Enable the search button
							if(cQueue->Get((LPVOID*)&szFind))
								PostThreadMessage(hThis, WM_FINDNEXTFILE, (WPARAM)0, 0);
							break;
						}
						else
							PostThreadMessage(hThis, WM_FINDNEXTFILE, (WPARAM)--nextFind, 0);
					}

					// OK, We found a directory,
					if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
					{
						if(StrCmp(findData.cFileName, TEXT(".")) && StrCmp(findData.cFileName, TEXT("..")))	// !a || !b == a && b
						{
							if(++nextFind == MAX_RECURSE_DEPTH)
							{
								MessageBox(NULL, TEXT("Recurse depth over 25"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION); 
								break;
							}
							UpdateCurrentDirectory(&pCurDir, findData.cFileName);
						}
						else
						{
							PostThreadMessage(hThis, WM_FINDNEXTFILE, (WPARAM)nextFind, 0);
							break;
						}
					}
					else
					{
						// test if mediafile
						if(IsFileMedia(findData.cFileName, FALSE))
						{
							if(AddAlbum(sp->dBase, pCurDir, findData.cFileName))
								IncreaseFileCount(TRUE, FALSE);
							else
								IncreaseFileCount(FALSE, FALSE);
						}
						// test if videofile
						else if(IsFileMedia(findData.cFileName, TRUE))
						{
							if(AddVideo(sp->dBase, pCurDir, findData.cFileName))
								IncreaseFileCount(TRUE, FALSE);
							else
								IncreaseFileCount(FALSE, FALSE);
						}
						PostThreadMessage(hThis, WM_FINDNEXTFILE, (WPARAM)nextFind, 0);
						break;
					}
				} while(nextFind < MAX_RECURSE_DEPTH);
				break;

			case WM_FINDNEXTFILE:
				nextFind = (INT)vMsg.wParam;
				if(sp->hSearch[nextFind].hFind == INVALID_HANDLE_VALUE)
					break;

				if(FindNextFile(sp->hSearch[nextFind].hFind, &findData))
				{
					// OK, We found a directory,
					if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
					{
						if(StrCmp(findData.cFileName, TEXT(".")) && StrCmp(findData.cFileName, TEXT("..")))
						{
							if(++nextFind == MAX_RECURSE_DEPTH)
							{
								nextFind--;
								MessageBox(NULL, TEXT("Recurse depth maximum, skipping directory"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION); 
								PostThreadMessage(hThis, WM_FINDNEXTFILE, (WPARAM)nextFind, 0);
							}
							else
							{
								UpdateCurrentDirectory(&pCurDir, findData.cFileName);
								PostThreadMessage(hThis, WM_FINDFIRSTFILE, (WPARAM)nextFind, 0);
							}
						}
						else
							PostThreadMessage(hThis, WM_FINDNEXTFILE, (WPARAM)nextFind, 0);
					}
					else
					{
						// test if mediafile
						if(IsFileMedia(findData.cFileName, FALSE))
						{
							if(AddAlbum(sp->dBase, pCurDir, findData.cFileName))
								IncreaseFileCount(TRUE, FALSE);
							else
								IncreaseFileCount(FALSE, FALSE);
						}
						// test if videofile
						else if(IsFileMedia(findData.cFileName, TRUE))
						{
							if(AddVideo(sp->dBase, pCurDir, findData.cFileName))
								IncreaseFileCount(TRUE, FALSE);
							else
								IncreaseFileCount(FALSE, FALSE);
						}
						PostThreadMessage(hThis, WM_FINDNEXTFILE, (WPARAM)nextFind, 0);
					}
				}
				else
				{
					FindClose(sp->hSearch[nextFind].hFind);
					sp->hSearch[nextFind].hFind = INVALID_HANDLE_VALUE;
					if(nextFind == 0)
					{
						// Enable the search button
						GlobalFree(szFind);
						szFind = NULL;
						if(cQueue->Empty() == FALSE)
							PostThreadMessage(hThis, WM_FINDFIRSTFILE, (WPARAM)0, 0);
					}
					else
					{
						UpdateCurrentDirectory(&pCurDir, TEXT(".."));
						PostThreadMessage(hThis, WM_FINDNEXTFILE, (WPARAM)--nextFind, 0);
					}
				}
				break;

			default:
				break;
		}
	}
	if(szFind)
		GlobalFree((HGLOBAL)szFind);

	while(cQueue->Get((LPVOID*)&szFind))
	{
		//SendMessage(sp->hPrnt, WM_SAVESEARCHQUEUE, 0, (LPARAM)szFind);
		GlobalFree(szFind);
	}
	delete cQueue;

	for(i = 0; i < MAX_RECURSE_DEPTH; i++)
	{
		if(sp->hSearch[i].hFind != INVALID_HANDLE_VALUE)
		{
			FindClose(sp->hSearch[i].hFind);
			if(sp->hSearch[i].pFile)
			{
				//SendMessage(sp->hPrnt, WM_SAVESEARCHPATH, 0, (LPARAM)sp->hSearch[i].pFile);
				GlobalFree((HGLOBAL)sp->hSearch[i].pFile);
			}
		}
	}

	SetEvent(*sp->hSync);
	GlobalFree((HGLOBAL)pCurDir);
	GlobalFree((HGLOBAL)sp);
	ExitThread(0);
	return(0);
}

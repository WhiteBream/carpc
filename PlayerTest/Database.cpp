// Database.cpp: implementation of the Database class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Database.h"
#include <shlobj.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Database::Database()
{
	CHAR vPath[MAX_PATH];

	SYSTEMTIME time;
	GetLocalTime(&time);
    srand((time.wMilliseconds << 16) | time.wSecond);

	pUsr = NULL;
	pSys = NULL;
	pCfg = NULL;
	pCfgGlob = NULL;

	if(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, vPath) == S_OK)
		StringCbCatA(vPath, sizeof(vPath), "\\WhiteBream");
	else
		StringCbCopyA(vPath, sizeof(vPath), ".\\WhiteBream");
	CreateDirectoryA(vPath, NULL);

	StringCbPrintfA(vMediaPath, sizeof(vMediaPath), "%s\\MediaPlayer.sql", vPath);
	StringCbPrintfA(vConfigPath, sizeof(vConfigPath), "%s\\MediaPlayer.cfg", vPath);
	StringCbPrintfA(vGlobalsPath, sizeof(vGlobalsPath), "%s\\Cargo.cfg", vPath);
}

Database::~Database()
{
	CloseMdb();
	CloseCfg();
}

void Database::Free(LPVOID lpMem)
{
	sqlite3_free((char*)lpMem);
}

Database::Initialize()
{
	/* Version 2.0 */
	OpenMdb();
	OpenCfg();

	return(0);
}

void Database::Recreate(void)
{
	/* Version 2.0 */
	CloseMdb();
	// Delete the physical database
	DeleteFileA(vMediaPath);
	OpenMdb();
}

int Database::ShowError(LPSTR *szError)
{
	if(*szError)
	{
		MessageBoxA(NULL, *szError, "SQLite error", MB_OK | MB_ICONERROR);
		sqlite3_free(*szError);
		*szError = NULL;
		return(1);
	}
	return(0);
}


/* Version 2.0 */
typedef struct _ParamStruct
{
	Database	*pDb;
	HWND		hWnd;
	INT			vShow;
	BOOL		vNoRemove;
	INT			vInsertSeq;
} TParamStruct, *PParamStruct;

static const LPSTR szCreateMdbTables[] =
{
	"CREATE TABLE Ttitle(id INTEGER PRIMARY KEY, val VARCHAR(25));",
	"CREATE TABLE Tartist(id INTEGER PRIMARY KEY, val VARCHAR(25));",
	"INSERT INTO  Tartist(val) VALUES(\'\');",
	"CREATE TABLE Talbum(id INTEGER PRIMARY KEY, val VARCHAR(25));",
	"CREATE TABLE Tyear(id INTEGER PRIMARY KEY, val INTEGER);",
	"CREATE TABLE Tgenre(id INTEGER PRIMARY KEY, val VARCHAR(10), static INTEGER);",

	"CREATE TABLE Tfile(id INTEGER PRIMARY KEY, val VARCHAR(25));",
	"CREATE TABLE Ttype(id INTEGER PRIMARY KEY, val VARCHAR(25), mtype INTEGER, path VARCHAR(5));",
	"INSERT INTO  Ttype(val, mtype) VALUES(\'\', 0);",
	"CREATE TABLE Troot(id INTEGER PRIMARY KEY, val VARCHAR(25));",
	"CREATE TABLE Tfolder(id INTEGER PRIMARY KEY, val VARCHAR(100), root INTEGER);",
	"INSERT INTO  Tfolder(val, root) VALUES(\'\', 0);",

	"CREATE TABLE Tmedia(id INTEGER PRIMARY KEY, hash INTEGER, invalid INTEGER, "
						 "title INTEGER REFERENCES Ttitle(id), title_org INTEGER REFERENCES Ttitle(id), "
						 "artist INTEGER REFERENCES Tartist(id), artist_org INTEGER "
						 "REFERENCES Tartist(id), album INTEGER, album_org INTEGER, "
						 "track INTEGER, track_org INTEGER, "
						 "year INTEGER, year_org INTEGER, genre INTEGER, genre_org "
						 "INTEGER, type INTEGER, file INTEGER REFERENCES Tfile(id), "
						 "root, folder INTEGER REFERENCES Tfolder(id), playtime INTEGER, rating INTEGER);",
	"INSERT INTO Tmedia(id, title, title_org, artist, artist_org, file, folder, root) VALUES(0, 0, 0, 0, 0, 0, 0, 0);",
	NULL
};

void Database::OpenMdb(void)
{
	const LPSTR syncCmd = "PRAGMA synchronous = OFF;";
	FILE *f;
	BOOL  bIsNew;
	LPSTR szError = NULL;

	// Check the size of the database
	f = fopen(vMediaPath, "rb");
	bIsNew = (f ? FALSE : TRUE);
	if(f != NULL)
		fclose(f);

	if(sqlite3_open(vMediaPath, &pUsr) != SQLITE_OK)
	{
		MessageBoxA(NULL, sqlite3_errmsg(pUsr), "Sqlite error", MB_OK | MB_ICONERROR);
		CloseMdb();
		return;
	}
	sqlite3_busy_timeout(pUsr, 1000);
	sqlite3_exec(pUsr, syncCmd, NULL, NULL, &szError);
	if(szError)
		ShowError(&szError);

	if(sqlite3_open(vMediaPath, &pSys) != SQLITE_OK)
	{
		MessageBoxA(NULL, sqlite3_errmsg(pSys), "Sqlite error", MB_OK | MB_ICONERROR);
		CloseMdb();
		return;
	}
	sqlite3_busy_timeout(pSys, 2500);
	sqlite3_exec(pSys, syncCmd, NULL, NULL, &szError);
	if(szError)
		ShowError(&szError);

	// It seems there is a newly created database
	if(bIsNew == TRUE)
		CreateMdb();
}

void Database::CloseMdb(void)
{
	if(pUsr)
		sqlite3_close(pUsr);
	pUsr = NULL;
	if(pSys)
		sqlite3_close(pSys);
	pSys = NULL;
}

void Database::CreateMdb(void)
{
	LPSTR szError = NULL;
	INT   i;

	for(i = 0; szCreateMdbTables[i] != NULL; i++)
	{
		sqlite3_exec(pUsr, szCreateMdbTables[i], NULL, NULL, &szError);
		if(szError)
			ShowError(&szError);
	}
	CreateGenres();
}

PMediaDescr Database::CopyDescr(PMediaDescr pMedia)
{
	PMediaDescr pNew;

	if((pMedia == NULL) || ((INT)pMedia == LB_ERR))
		return(NULL);

	pNew = (PMediaDescr)GlobalAlloc(GPTR, sizeof(MediaDescr));
	if(pNew != NULL)
		memcpy(pNew, pMedia, sizeof(MediaDescr));
	return(pNew);
}

LPVOID Database::ParamStruct(INT vObjs)
{
	PParamStruct pData;

	pData = (PParamStruct)GlobalAlloc(GPTR, sizeof(TParamStruct) + vObjs * sizeof(LPVOID));
	pData->pDb = this;
	return((LPVOID)pData);
}

LPTSTR Database::DescriptorToFullString(PMediaDescr pSelect)
{
	INT          size;
	TCHAR        vTemp[128];
	LPTSTR       pText;
	INT          vLenTitle, vLenArtist, vLenAlbum;
	LPTSTR       pTitle, pArtist, pAlbum;
	LPSTR        sqlCmd;
	LPSTR       *pResult;
	INT          vRows, vCols;
	LPSTR        szError = NULL;
	INT          vExit, vTimeout = 0;

	if(pSelect == NULL)
		return(NULL);

	/* Format display string */
	pArtist = pTitle = NULL;
	vLenArtist = GetStringValue(pSelect, FLD_ARTIST, 0, &pArtist);
	vLenTitle = GetStringValue(pSelect, FLD_TITLE, 0, &pTitle);

	size = vLenTitle + vLenArtist + 256;
	pText = (LPTSTR)GlobalAlloc(GPTR, size * sizeof(TCHAR));
	if(vLenArtist > 1)
		StringCchCat(pText, size, pArtist);
	if(vLenArtist)
		GlobalFree((HGLOBAL)pArtist);

	if((vLenTitle > 1) && (vLenArtist > 1))
		StringCchCat(pText, size, TEXT(" - "));

	if(vLenTitle > 1)
		StringCchCat(pText, size, pTitle);
	if(vLenTitle)
		GlobalFree((HGLOBAL)pTitle);

	/* No title nor artist, try album name */
	if((vLenArtist <= 1) && (vLenTitle <= 1))
	{
		pAlbum = NULL;
		vLenAlbum = GetStringValue(pSelect, FLD_FILE, 0, &pAlbum);
		if(vLenAlbum > 1)
		{
			pTitle = _tcsrchr(pAlbum, '\\');
			StringCchCat(pText, size, ++pTitle);
		}
		GlobalFree((HGLOBAL)pAlbum);

//		pAlbum = NULL;
//		vLenAlbum = GetStringValue(pSelect, FLD_ALBUM, 0, &pAlbum);
//		if(vLenAlbum > 1)
//			StringCchCat(pText, size, pAlbum);
//		if(vLenAlbum)
//			GlobalFree((HGLOBAL)pAlbum);
	}
	else
	{
		/* Check number of occurences of this artist+title combination */
		sqlCmd = sqlite3_mprintf("SELECT COUNT(id) FROM Tmedia WHERE title=%d AND artist=%d;", pSelect->vTitleId, pSelect->vArtistId);
		while((vExit = sqlite3_get_table(pUsr, sqlCmd, &pResult, &vRows, &vCols, &szError)) == SQLITE_BUSY)
			if(++vTimeout > 100) break;
		if(vExit == SQLITE_OK)
		{
			if(vRows > 0)
			{
				if(pResult[vCols + 0])
					if(atoi(pResult[vCols + 0]) > 1)
					{
						LPTSTR pTemp;

						pAlbum = NULL;
						vTemp[0] = L'\0';
						vLenAlbum = GetStringValue(pSelect, FLD_ALBUM, 0, &pAlbum);
						if(vLenAlbum > 1)
							StringCbPrintf(vTemp, sizeof(vTemp), TEXT(" (%s)"), pAlbum);
						if(vLenAlbum)
							GlobalFree((HGLOBAL)pAlbum);

						size += vLenAlbum + 3;
						pTemp = (LPTSTR)GlobalAlloc(GPTR, size * sizeof(TCHAR));
						StringCchCat(pTemp, size, pText);
						GlobalFree((HGLOBAL)pText);
						StringCchCat(pTemp, size, vTemp);
						pText = pTemp;
					}
			}
			sqlite3_free_table(pResult);
		}
		sqlite3_free(sqlCmd);
		if(szError)
			ShowError(&szError);
	}

	/* Add trackduration to displaystring */
	if(pSelect->vPlayTime)
	{
		if(pSelect->vPlayTime >= 3600)
			StringCbPrintf(vTemp, sizeof(vTemp), TEXT(" [%u:%02u:%02u]"), pSelect->vPlayTime / 3600, (pSelect->vPlayTime - 3600) / 60, pSelect->vPlayTime % 60);
		else
			StringCbPrintf(vTemp, sizeof(vTemp), TEXT(" [%u:%02u]"), pSelect->vPlayTime / 60, pSelect->vPlayTime % 60);
		StringCchCat(pText, size, vTemp);
	}
	return(pText);
}

LPTSTR Database::DescriptorToSingleString(PMediaDescr pSelect, MediaField vLabel)
{
	PMediaDescr pSearch;
	LPTSTR      pText = NULL, pTemp, pArtist;
	INT         vLen, vArtistLen;

	if(pSelect == NULL)
		return(NULL);

	pSearch = CopyDescr(pSelect);
	pSearch->vMediaId = 0;

	/* Get desired string */
	if(vLabel & FLD_ARTIST)
	{
		if(pSearch->vArtistId == 1)	// Unknown
		{
			vLen = 10;
			pText = (LPTSTR)GlobalAlloc(GPTR, vLen * sizeof(TCHAR));
			StringCchPrintf(pText, vLen, TEXT("Unknown"));
		}
		else
			vLen = GetStringValue(pSearch, FLD_ARTIST, 0, &pText);
	}
	else if(vLabel & FLD_TITLE)
		vLen = GetStringValue(pSearch, FLD_TITLE, 0, &pText);
	else if(vLabel & FLD_ALBUM)
	{
		vLen = GetStringValue(pSearch, FLD_ALBUM, 0, &pText);
		if((vLen > 1) && pSearch->vArtistId)
		{
			pTemp = pText;
			vArtistLen = GetStringValue(pSearch, FLD_ARTIST, 0, &pArtist);
			vLen += vArtistLen + 5;
			pText = (LPTSTR)GlobalAlloc(GPTR, vLen * sizeof(TCHAR));
			StringCchPrintfW(pText, (size_t)vLen, TEXT("%s (%s)"), pTemp, pArtist);
			GlobalFree((HGLOBAL)pArtist);
			GlobalFree((HGLOBAL)pTemp);
		}
	}
	else if(vLabel & FLD_ROOT)
		vLen = GetStringValue(pSearch, FLD_ROOT, 0, &pText);
	else if(vLabel & FLD_FILE)
		vLen = GetStringValue(pSearch, FLD_FILE, 0, &pText);
	else if(vLabel & FLD_YEAR)
		vLen = GetStringValue(pSearch, FLD_YEAR, 0, &pText);

	GlobalFree((HGLOBAL)pSearch);

	if(vLen <= 1)
	{
		GlobalFree((HGLOBAL)pText);
		pText = NULL;
	}
	return(pText);
}

LPTSTR Database::DescriptorToDoubleString(PMediaDescr pSelect, MediaField vLabel)
{
	INT          size;
	TCHAR        vTemp[128];
	LPTSTR       pText;
	LPTSTR       pTitle = NULL, pArtist = NULL, pAlbum = NULL;
	INT          vLenTitle = 0, vLenArtist = 0, vLenAlbum = 0;
	LPSTR        sqlCmd;
	LPSTR       *pResult;
	INT          vLen, vRows, vCols;
	LPSTR        szError = NULL;
	INT          vExit, vTimeout;

	if(pSelect == NULL)
		return(NULL);

	vLen = (INT)vLabel;
	vLen &= ~(INT)(DISPLAY_FULL | DISPLAY_ITEM | DISPLAY_EXCL);
	vLabel = (MediaField)vLen;
	vLen = 0;

	if(vLabel & FLD_ARTIST)
		vLenArtist = GetStringValue(pSelect, FLD_ARTIST, 0, &pArtist);
	if(vLabel & FLD_TITLE)
		vLenTitle = GetStringValue(pSelect, FLD_TITLE, 0, &pTitle);
	if(vLabel & FLD_ALBUM)
		vLenAlbum = GetStringValue(pSelect, FLD_ALBUM, 0, &pAlbum);

	size = vLenTitle + vLenArtist + vLenAlbum + 30;
	pText = (LPTSTR)GlobalAlloc(GPTR, size * sizeof(TCHAR));

	/* Begin with artist, if enabled and if available */
	if(vLabel & FLD_ARTIST)
		if(vLenArtist > 1)
			StringCchCat(pText, size, pArtist);
	if(vLenArtist)
		GlobalFree((HGLOBAL)pArtist);

	/* Add the merging dash */
	if((vLenTitle > 1) && (vLenArtist > 1))
		StringCchCat(pText, size, TEXT(" - "));

	if(vLabel & FLD_TITLE)
		if(vLenTitle > 1)
			StringCchCat(pText, size, pTitle);
	if(vLenTitle)
		GlobalFree((HGLOBAL)pTitle);

	if((vLabel & FLD_ALBUM) || ((vLenArtist <= 1) && (vLenTitle <= 1)))
	{
		if(vLenAlbum > 1)
			StringCchCat(pText, size, pAlbum);
		GlobalFree((HGLOBAL)pAlbum);
	}

	pArtist = pTitle = NULL;

//	if(vLenArtist > 1)
//		StringCchCat(pText, size, pArtist);
//	if(vLenTitle > 1)
//		StringCchCat(pText, size, pTitle);

	/* No title nor artist, try album name */
	if((vLenArtist <= 1) && (vLenTitle <= 1))
	{
//		if(pAlbum)
//		{
//			if(vLenAlbum > 1)
//				StringCchCat(pText, size, pAlbum);
//			GlobalFree((HGLOBAL)pAlbum);
//		}
		vLenAlbum = GetStringValue(pSelect, FLD_FILE, 0, &pAlbum);
		if(vLenAlbum > 1)
		{
			pTitle = _tcsrchr(pAlbum, '\\');
			StringCchCat(pText, size, ++pTitle);
		}
		GlobalFree((HGLOBAL)pAlbum);
	}
	else
	{
		/* Check number of occurences of this artist+title combination */
		sqlCmd = sqlite3_mprintf("SELECT COUNT(id) FROM Tmedia WHERE title=%d AND artist=%d;", pSelect->vTitleId, pSelect->vArtistId);
		vTimeout = 0;
		while((vExit = sqlite3_get_table(pUsr, sqlCmd, &pResult, &vRows, &vCols, &szError)) == SQLITE_BUSY)
			if(++vTimeout > 100) break;
		sqlite3_free(sqlCmd);
		if(vExit == SQLITE_OK)
		{
			if(vRows > 0)
			{
				if(pResult[vCols + 0])
				{
					vRows = atoi(pResult[vCols + 0]);
					if(vRows > 1)
					{
						LPTSTR pTemp;

						pAlbum = NULL;
						vTemp[0] = L'\0';
						vLenAlbum = GetStringValue(pSelect, FLD_ALBUM, 0, &pAlbum);
						if(vLenAlbum > 1)
							StringCbPrintf(vTemp, sizeof(vTemp), TEXT(" (%s)"), pAlbum);
						if(vLenAlbum)
							GlobalFree((HGLOBAL)pAlbum);

						size += vLenAlbum + 3;
						pTemp = (LPTSTR)GlobalAlloc(GPTR, size * sizeof(TCHAR));
						StringCchCat(pTemp, size, pText);
						GlobalFree((HGLOBAL)pText);
						StringCchCat(pTemp, size, vTemp);
						pText = pTemp;
					}
				}
			}
			sqlite3_free_table(pResult);
		}
		if(szError)
			sqlite3_free(szError);
	}

	/* Add trackduration to displaystring */
	if(vLabel & FLD_PLAYTIME)
	{
		if(pSelect->vPlayTime)
		{
			if(pSelect->vPlayTime >= 3600)
				StringCbPrintf(vTemp, sizeof(vTemp), TEXT(" [%u:%02u:%02u]"), pSelect->vPlayTime / 3600, (pSelect->vPlayTime - 3600) / 60, pSelect->vPlayTime % 60);
			else
				StringCbPrintf(vTemp, sizeof(vTemp), TEXT(" [%u:%02u]"), pSelect->vPlayTime / 60, pSelect->vPlayTime % 60);
			StringCchCat(pText, size, vTemp);
		}
	}
	return(pText);
}

/* Callback for Database::QueryToListbox */
static INT QueryToListboxCb(LPVOID cbParam, INT colCount, LPSTR *colData, LPSTR *colNames)
{
	PParamStruct pData = (PParamStruct)cbParam;
	PMediaDescr  pSelect;
	LPTSTR       pText = NULL;
	LONG	 	 idNew;

	if((colCount < 1) || (colData == NULL) || (cbParam == NULL))
		return(1);

	pSelect = (PMediaDescr)GlobalAlloc(GPTR, sizeof(MediaDescr));
	if(pSelect == NULL)
		return(1);
	pSelect->vNoRemove = pData->vNoRemove;

	switch(colCount)
	{
		case 14:	if(colData[13])	pSelect->vRating = atoi(colData[13]);
		case 13:	if(colData[12])	pSelect->vPlayTime = atoi(colData[12]);
		case 12:	if(colData[11])	pSelect->vRootId = atoi(colData[11]);
		case 11:	if(colData[10])	pSelect->vFolderId = atoi(colData[10]);
		case 10:	if(colData[9])	pSelect->vFileId = atoi(colData[9]);
		case 9:		if(colData[8])	pSelect->vTypeId = atoi(colData[8]);
		case 8:		if(colData[7])	pSelect->vGenreId = atoi(colData[7]);
		case 7:		if(colData[6])	pSelect->vYearId = atoi(colData[6]);
		case 6:		if(colData[5])	pSelect->vTrack = atoi(colData[5]);
		case 5:		if(colData[4])	pSelect->vAlbumId = atoi(colData[4]);
		case 4:		if(colData[3])	pSelect->vArtistId = atoi(colData[3]);
		case 3:		if(colData[2])	pSelect->vTitleId = atoi(colData[2]);
		case 2:		if(colData[1])	pSelect->vInvalid = atoi(colData[1]);
		case 1:		if(colData[0])	pSelect->vMediaId = atoi(colData[0]);
			break;

		default:
			GlobalFree((HGLOBAL)pSelect);
			return(1);
	}

	switch(pData->vShow & DISPLAY_MASK)
	{
		case DISPLAY_FULL:
			pText = pData->pDb->DescriptorToFullString(pSelect);
			break;

		case DISPLAY_ITEM:
			pText = pData->pDb->DescriptorToSingleString(pSelect, (MediaField)pData->vShow);
			pSelect->vAux = pData->vShow;
			break;

		case DISPLAY_EXCL:
			pText = pData->pDb->DescriptorToDoubleString(pSelect, (MediaField)pData->vShow);
			pSelect->vAux = pData->vShow;
			break;

		default:
			return(1);
	}

	/* Send the displaystring and datablock to listbox. Datablock must be freed by receiver! */
	if(pText)
	{
		if(pData->vShow & DISPLAY_INSERT)
			idNew = SendMessage(pData->hWnd, LB_INSERTSTRING, pData->vInsertSeq++, (LPARAM)pText);
		else
			idNew = SendMessage(pData->hWnd, LB_ADDSTRING, 0, (LPARAM)pText);
		GlobalFree((HGLOBAL)pText);
		SendMessage(pData->hWnd, LB_SETITEMDATA, (WPARAM)idNew, (LPARAM)pSelect);
	}
	return(0);
}

/* QueryToListboxCb MUST be used with this input specifier ! */
static const CHAR queryToListbox[] = "id f0, invalid f1, title f2, artist f3, album f4, "
									 "track f5, year f6, genre f7, type f8, file f9, folder f10, "
									 "root f11, playtime f12, rating f13 FROM Tmedia";

/*
 * Fill the designated Listbox with a query matching pSelect criteria
 */
void Database::QueryToListbox(PMediaDescr pSelect, HWND hWnd)
{
	PParamStruct pCbParm;
	PMediaDescr  pInput;
	LPSTR        szError = NULL;
	LPSTR        sqlCmd, sqlIm1, sqlIm2;
	INT          cnt;
	INT          vTimeout = 0;

	if(hWnd == NULL)
		return;

	pInput = pSelect;
	if(pInput == NULL)
		pInput = (PMediaDescr)GlobalAlloc(GPTR, sizeof(MediaDescr));

	sqlIm1 = sqlite3_mprintf("");
	sqlCmd = "%s AND %s=%d";
	if(pInput->vTitleId)
	{
		sqlIm2 = sqlIm1;
		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "title", pInput->vTitleId);
		sqlite3_free(sqlIm2);
	}
	if(pInput->vArtistId)
	{
		sqlIm2 = sqlIm1;
		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "artist", pInput->vArtistId);
		sqlite3_free(sqlIm2);
	}
	if(pInput->vAlbumId)
	{
		sqlIm2 = sqlIm1;
		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "album", pInput->vAlbumId);
		sqlite3_free(sqlIm2);
	}
	if(pInput->vTrack)
	{
		sqlIm2 = sqlIm1;
		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "track", pInput->vTrack);
		sqlite3_free(sqlIm2);
	}
	if(pInput->vYearId)
	{
		sqlIm2 = sqlIm1;
		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "year", pInput->vYearId);
		sqlite3_free(sqlIm2);
	}
	if(pInput->vGenreId)
	{
		sqlIm2 = sqlIm1;
		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "genre", pInput->vGenreId);
		sqlite3_free(sqlIm2);
	}
	sqlIm2 = sqlIm1;
	sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "invalid", pInput->vInvalid);
	sqlite3_free(sqlIm2);

	StringCchLengthA(sqlIm1, STRSAFE_MAX_CCH, (size_t*)&cnt);
	if(cnt)
		sqlCmd = sqlite3_mprintf("SELECT %s WHERE %s;", queryToListbox, &sqlIm1[5]);
	else
		sqlCmd = sqlite3_mprintf("SELECT %s;", queryToListbox);
	sqlite3_free(sqlIm1);

	/* argument for callback function to access data within our class */
	pCbParm = (PParamStruct)ParamStruct(1);
	pCbParm->hWnd = hWnd;
	pCbParm->vShow = pInput->vAux;
	pCbParm->vInsertSeq = 0;

	while(sqlite3_exec(pUsr, sqlCmd, (sqlite3_callback)QueryToListboxCb, (void*)pCbParm, &szError) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;

	sqlite3_free(sqlCmd);
	GlobalFree((HGLOBAL)pCbParm);
	if(szError)
		ShowError(&szError);

	if(pSelect == NULL)
		GlobalFree((HGLOBAL)pInput);
}

/*
 * Fill the designated Listbox with a query matching pSelect criteria
 */
void Database::MediaDbQuery(LPTSTR pSearch, HWND hWnd)
{
	const CHAR sqlQuery[] = "SELECT %s WHERE title IN (SELECT id FROM Ttitle WHERE UPPER(val) LIKE \'%%%q%%\') OR "
							"artist IN (SELECT id FROM Tartist WHERE UPPER(val) LIKE \'%%%q%%\') LIMIT 1000;";
	PParamStruct pCbParm;
	LPSTR        szError = NULL;
	CHAR         vInStr[17];
	LPSTR        sqlCmd;
	INT          vTimeout = 0;

	if(hWnd == NULL)
		return;

	vInStr[0] = '\0';
	if(pSearch != NULL)
	{
#ifdef UNICODE
		StringCbPrintfA(vInStr, sizeof(vInStr), "%ws", pSearch);
#else
		StringCbPrintfA(vInStr, sizeof(vInStr), "%hs", pSearch);
#endif
		strupr(vInStr);
	}

	/* argument for callback function to access data within our class */
	pCbParm = (PParamStruct)ParamStruct(1);
	pCbParm->hWnd = hWnd;
	pCbParm->vShow = DISPLAY_FULL;

	sqlCmd = sqlite3_mprintf(sqlQuery, queryToListbox, vInStr, vInStr, vInStr);
	while(sqlite3_exec(pUsr, sqlCmd, (sqlite3_callback)QueryToListboxCb, (void*)pCbParm, &szError) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	GlobalFree((HGLOBAL)pCbParm);
	sqlite3_free(sqlCmd);
	if(szError)
		ShowError(&szError);
}

/*
 * Fill the designated Listbox with table contents matching pSelect criteria
 */
void Database::TableToListbox(PMediaDescr pSelect, HWND hWnd)
{
	// "SELECT id f0, invalid f1, title f2, artist f3, album f4, "
	// "track f5, year f6, genre f7, type f8, file f9, folder f10, "
	// "root f11, playtime f12, rating f13 FROM ";
	const CHAR sqlTable[] = "SELECT 0, 0, %s FROM %s;";

	PParamStruct pCbParm;
	LPSTR        szError = NULL;
	LPSTR        sqlCmd = NULL;
	INT          vDisplay;
	INT          vTimeout = 0;

	if((hWnd == NULL) || (pSelect == NULL))
		return;

	vDisplay = pSelect->vAux & FIELD_MASK;
	if(vDisplay)
	{
		if(vDisplay & FLD_TITLE)
		{
			sqlCmd = sqlite3_mprintf("SELECT DISTINCT 0, 0, title FROM Tmedia WHERE title IN (SELECT id FROM Ttitle);");
		}
		else if(vDisplay & FLD_ARTIST)
		{
			if(pSelect->vTitleId)
			{
				sqlCmd = sqlite3_mprintf("SELECT DISTINCT 0, 0, 0, artist FROM Tmedia WHERE title=%d "
										"ORDER BY (SELECT UPPER(val) FROM Tartist WHERE id=artist);",
										pSelect->vTitleId);
			}
			else if(pSelect->vAlbumId)
			{
				sqlCmd = sqlite3_mprintf("SELECT DISTINCT 0, 0, 0, artist FROM Tmedia WHERE album=%d "
										"ORDER BY (SELECT UPPER(val) FROM Tartist WHERE id=artist);",
										pSelect->vAlbumId);
			}
			else
			{
				sqlCmd = sqlite3_mprintf("SELECT DISTINCT 0, 0, 0, artist FROM Tmedia WHERE artist IN (SELECT id FROM Tartist);");
			}
		}
		else if(vDisplay & FLD_ALBUM)
		{
			if(pSelect->vTitleId)
			{
				sqlCmd = sqlite3_mprintf("SELECT 0, 0, 0, artist, album FROM (SELECT album, artist FROM Tmedia "
										"WHERE title=%d OR id=0 GROUP BY album HAVING COUNT(artist) > 1 UNION SELECT "
										"album, 0 artist FROM Tmedia WHERE title=%d OR id=0 GROUP BY album HAVING "
										"COUNT(artist) <= 1)ORDER BY (SELECT UPPER(val) FROM Talbum WHERE id=album);",
										pSelect->vTitleId, pSelect->vTitleId);
			}
			else if(pSelect->vArtistId)
			{
				sqlCmd = sqlite3_mprintf("SELECT DISTINCT 0, 0, 0, 0, album FROM Tmedia WHERE artist=%d ORDER BY "
										"(SELECT UPPER(val) FROM Talbum WHERE id=album);", pSelect->vArtistId);
			}
			else
			{
#define CLEANALB
#ifdef CLEANALB
				sqlCmd = sqlite3_mprintf("SELECT DISTINCT 0, 0, 0, 0, album FROM Tmedia WHERE album IN (SELECT id FROM Talbum);");
#else
				sqlCmd = sqlite3_mprintf("SELECT 0, 0, 0, artist, album FROM (SELECT album, artist FROM Tmedia "
										"WHERE album IN (SELECT album FROM (SELECT DISTINCT album, artist "
										"FROM Tmedia) GROUP BY album HAVING COUNT(artist) > 1) UNION SELECT "
										"album, 0 artist FROM Tmedia WHERE album IN (SELECT album FROM "
										"(SELECT DISTINCT album, artist FROM Tmedia) GROUP BY album HAVING "
										"COUNT(artist) <= 1)) ORDER BY (SELECT UPPER(val) FROM Talbum WHERE "
										"id=album);");
#endif
			}
		}
		else if(vDisplay & FLD_ROOT)
		{
			sqlCmd = sqlite3_mprintf("SELECT 0, 0, 0,0,0, 0,0,0,0,0 ,0, id FROM Troot;");
		}
		else if(vDisplay & FLD_FILE)
		{
			if(pSelect->vTitleId)
			{
				sqlCmd = sqlite3_mprintf("SELECT DISTINCT 0, 0, 0,0,0, 0,0,0 ,0, file FROM Tmedia WHERE title=%d ORDER BY "
										"(SELECT UPPER(val) FROM Tfile WHERE id=file);", pSelect->vTitleId);
			}
			else
			{
				sqlCmd = sqlite3_mprintf("SELECT 0, 0, 0,0,0, 0,0,0, ,0, file FROM Tmedia WHERE file IN (SELECT id FROM Tfile);");
			}
		}
	}
	else if(pSelect->vTitleId)
	{
		vDisplay = FLD_TITLE;
		sqlCmd = sqlite3_mprintf(sqlTable, "id", "Ttitle");
	}
	else if(pSelect->vArtistId)
	{
		vDisplay = FLD_ARTIST;
		sqlCmd = sqlite3_mprintf(sqlTable, "0, id", "Tartist");
	}
	else if(pSelect->vAlbumId)
	{
		vDisplay = FLD_ALBUM;
		sqlCmd = sqlite3_mprintf(sqlTable, "0, 0, id", "Talbum");
	}
	else if(pSelect->vRootId)
	{
		vDisplay = FLD_ROOT;
		sqlCmd = sqlite3_mprintf(sqlTable, "0,0,0, 0,0,0,0,0 ,0, id", "Troot");
	}

	if(sqlCmd)
	{
		/* argument for callback function to access data within our class */
		pCbParm = (PParamStruct)ParamStruct(1);
		pCbParm->hWnd = hWnd;
		pCbParm->vShow = DISPLAY_ITEM + (INT)vDisplay;

		while(sqlite3_exec(pUsr, sqlCmd, (sqlite3_callback)QueryToListboxCb, (void*)pCbParm, &szError) == SQLITE_BUSY)
			if(vTimeout++ > 100) break;

		sqlite3_free(sqlCmd);
		GlobalFree((HGLOBAL)pCbParm);
		if(szError)
			ShowError(&szError);
	}
}

void Database::GetAutoPlay(BOOL doShuffle, LPLONG lastIdx, HWND hWnd)
{
	PParamStruct	pCbParm;
	PMediaDescr		pMedia;
	LPSTR			szError = NULL;
	LPSTR			sqlCmd;
	LPSTR			*pResult;
	INT				nRows;
	INT				vExit, vLoopCnt = 0, vTimeout;
	INT				vMin = 0, vMax = 0;
	INT				vMedia;
	UINT			vCount = (UINT)-1;

	// A database and function pinter should already exist
	if(pUsr == NULL)
		return;

	do
	{
		/* Create a random number in the range of available media records */
		vTimeout = 0;
		while((vExit = sqlite3_get_table(pUsr, "SELECT MIN(id), MAX(id), COUNT(id) FROM Tmedia;", &pResult, &nRows, NULL, &szError)) == SQLITE_BUSY)
			if(++vTimeout > 100) break;
		if(szError)
			ShowError(&szError);
		if(vExit == SQLITE_OK)
		{
			if(nRows == 1)
			{
				if(pResult[3])
					vMin = atoi(pResult[3]);
				if(pResult[4])
					vMax = atoi(pResult[4]);
				if(pResult[5])
					vCount = atoi(pResult[5]);
			}
			sqlite3_free_table(pResult);
			if(vMax == vMin)
				return;

			if(doShuffle || (lastIdx == NULL))
				vMedia = vMin + ((rand() * rand()) % (vMax - vMin));
			else
			{
				vMedia = *lastIdx + 1;
				if(vMedia > vMax)
					vMedia = vMin;
			}
			*lastIdx = vMedia;
		}

		/* Check if the selected record is actually available */
		if(vMedia)
		{
			sqlCmd = sqlite3_mprintf("SELECT COUNT(id) FROM Tmedia WHERE id=%d;", vMedia);
			vTimeout = 0;
			while((vExit = sqlite3_get_table(pUsr, sqlCmd, &pResult, &nRows, NULL, &szError)) == SQLITE_BUSY)
				if(++vTimeout > 100) break;
			sqlite3_free(sqlCmd);
			if(szError)
				ShowError(&szError);
			if(vExit == SQLITE_OK)
			{
				sqlite3_free_table(pResult);
				if(nRows < 1)
					vMedia = 0;
			}
		}
		/* Check if the entry is already in nowplaying listbox */
		if(vMedia)
		{
			nRows = SendMessage(hWnd, LB_GETCOUNT, 0, 0);
			while(nRows && vMedia)
			{
				pMedia = (PMediaDescr)SendMessage(hWnd, LB_GETITEMDATA, (WPARAM)--nRows, 0);
				if(pMedia && ((LRESULT)pMedia != LB_ERR))
					if(pMedia->vMediaId == vMedia)
						vMedia = 0;
			}
		}
		/* Check if the entry is still in history */
		if(vMedia && (vCount > 200))
		{
			sqlCmd = sqlite3_mprintf("SELECT x FROM Thistory WHERE id=%d;", vMedia);
			vTimeout = 0;
			while((vExit = sqlite3_get_table(pCfg, sqlCmd, &pResult, &nRows, NULL, &szError)) == SQLITE_BUSY)
				if(++vTimeout > 100) break;
			sqlite3_free(sqlCmd);
			if(szError)
				ShowError(&szError);
			if(vExit == SQLITE_OK)
			{
				sqlite3_free_table(pResult);
				if(nRows > 0)
					vMedia = 0;
			}
		}
		/* Check if it's not a video file */
		if(vMedia && !GetBool("AutoVideo"))
		{
			sqlCmd = sqlite3_mprintf("SELECT type FROM Tmedia WHERE id=%d;", vMedia);
			vTimeout = 0;
			while((vExit = sqlite3_get_table(pUsr, sqlCmd, &pResult, &nRows, NULL, &szError)) == SQLITE_BUSY)
				if(++vTimeout > 100) break;
			sqlite3_free(sqlCmd);
			if(szError)
				ShowError(&szError);
			if(vExit == SQLITE_OK)
			{
				if(nRows == 1)
				{
					if(pResult[1])
					{
						nRows = atoi(pResult[1]);
						if(nRows == FT_VIDEO)
							vMedia = 0;
					}
				}
				sqlite3_free_table(pResult);
			}
		}
	} while((vMedia == 0) && (vLoopCnt++ < 100));

	/* argument for callback function to access data within our class */
	pCbParm = (PParamStruct)ParamStruct(1);
	pCbParm->hWnd = hWnd;
	pCbParm->vShow = DISPLAY_FULL;

	sqlCmd = sqlite3_mprintf("SELECT %s WHERE id=%d LIMIT 1;", queryToListbox, vMedia);
	vTimeout = 0;
	while(sqlite3_exec(pUsr, sqlCmd, (sqlite3_callback)QueryToListboxCb, (void*)pCbParm, &szError) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;

	GlobalFree((HGLOBAL)pCbParm);
	sqlite3_free(sqlCmd);
	if(szError)
		ShowError(&szError);
}

/*
  Update media rating score
 */
void Database::AdjustRating(PMediaDescr pSelect, MediaRating vRating)
{
	LPSTR szError = NULL;
	LPSTR sqlCmd;
	INT   vOldRating = 0;
	INT   vTimeout;
	LPSTR *pResult;
	INT   nRows;

	if(pSelect == NULL)
		return;
	if(pSelect->vMediaId == 0)
		return;

	sqlCmd = sqlite3_mprintf("SELECT rating FROM Tmedia WHERE id=%d;", pSelect->vMediaId);
	vTimeout = 0;
	while(sqlite3_get_table(pUsr, sqlCmd, &pResult, &nRows, NULL, &szError) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	if(nRows && pResult[1])
		vOldRating = atoi(pResult[1]);
	sqlite3_free_table(pResult);
	sqlite3_free(sqlCmd);

	sqlCmd = sqlite3_mprintf("UPDATE Tmedia SET rating=%d WHERE id=%d;", vOldRating + vRating, pSelect->vMediaId);
	vTimeout = 0;
	while(sqlite3_exec(pUsr, sqlCmd, NULL, NULL, &szError) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	sqlite3_free(sqlCmd);
	if(szError)
		ShowError(&szError);

	pSelect->vRating += vRating;

	sqlCmd = sqlite3_mprintf("SELECT rating FROM Tmedia WHERE id=%d;", pSelect->vMediaId);
	vTimeout = 0;
	while(sqlite3_get_table(pUsr, sqlCmd, &pResult, &nRows, NULL, &szError) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	if(nRows && pResult[1])
		vOldRating = atoi(pResult[1]);
	sqlite3_free_table(pResult);
	sqlite3_free(sqlCmd);
}

/*
 * Retrieve string for selected field in pSelect
 * if *pStr=NULL, function returns a GlobalHeap memory segment
 * returnvalue is stringlength with terminator
 */
INT Database::GetStringValue(PMediaDescr pSelect, MediaField vField, INT vMaxLen, LPTSTR * pStr)
{
	HRESULT  hr = -1;
	LPSTR    szError = NULL;
	LPSTR    sqlCmd, getString;
	LPSTR   *pResult;
	size_t   len;
	BOOL     claimMem = FALSE;
	INT      vRows, vExec;
	INT      vTimeout;

	if((pSelect == NULL) || (pStr == NULL))
		return(0);

	if((*pStr != NULL) && (vMaxLen >= 1))
		pStr[0] = TEXT('\0');
	else
		claimMem = TRUE;

	switch(vField)
	{
		/* These fields are already available in the pSelect structure, just do the appropriate string processing and return */
		case FLD_TRACK:
			if(claimMem)
				*pStr = (LPTSTR)GlobalAlloc(GPTR, 11 * sizeof(TCHAR));
			if(*pStr)
			{
				StringCchPrintf(*pStr, vMaxLen, TEXT("%d"), pSelect->vTrack);
				hr = StringCchLength(*pStr, vMaxLen, &len);
			}
			return(SUCCEEDED(hr) ? ++len : 0);

		case FLD_PLAYTIME:
			if(claimMem)
				*pStr = (LPTSTR)GlobalAlloc(GPTR, 15 * sizeof(TCHAR));
			if(*pStr)
			{
				if(pSelect->vPlayTime >= 3600)
					StringCbPrintf(*pStr, vMaxLen, TEXT("[%u:%02u:%02u]"), pSelect->vPlayTime / 3600, (pSelect->vPlayTime - 3600) / 60, pSelect->vPlayTime % 60);
				else
					StringCbPrintf(*pStr, vMaxLen, TEXT("[%u:%02u]"), pSelect->vPlayTime / 60, pSelect->vPlayTime % 60);
				hr = StringCchLength(*pStr, vMaxLen, &len);
			}
			return(SUCCEEDED(hr) ? ++len : 0);

		case FLD_RATING:
			if(claimMem)
				*pStr = (LPTSTR)GlobalAlloc(GPTR, 11 * sizeof(TCHAR));
			if(*pStr)
			{
				StringCchPrintf(*pStr, vMaxLen, TEXT("%d"), pSelect->vRating);
				hr = StringCchLength(*pStr, vMaxLen, &len);
			}
			return(SUCCEEDED(hr) ? ++len : 0);

		case FLD_TITLE:
		case FLD_ARTIST:
		case FLD_ALBUM:
		case FLD_YEAR:
		case FLD_GENRE:
		case FLD_FILE:
		case FLD_ROOT:
			if(pSelect->vMediaId)
			{
				getString = "SELECT val FROM %s WHERE id=(SELECT %s FROM Tmedia WHERE id=%d);";
				switch(vField)
				{
					/* The following fields must be retrieved through sql */
					case FLD_TITLE:		sqlCmd = sqlite3_mprintf(getString, "Ttitle",  "title",  pSelect->vMediaId);	break;
					case FLD_ARTIST:	sqlCmd = sqlite3_mprintf(getString, "Tartist", "artist", pSelect->vMediaId);	break;
					case FLD_ALBUM:		sqlCmd = sqlite3_mprintf(getString, "Talbum",  "album",  pSelect->vMediaId);	break;
					case FLD_YEAR:		sqlCmd = sqlite3_mprintf(getString, "Tyear",   "year",   pSelect->vMediaId);	break;
					case FLD_GENRE:		sqlCmd = sqlite3_mprintf(getString, "Tgenre",  "genre",  pSelect->vMediaId);	break;
					case FLD_FILE:		sqlCmd = sqlite3_mprintf(getString, "Tfile",   "file",   pSelect->vMediaId);	break;
					case FLD_ROOT:		sqlCmd = sqlite3_mprintf(getString, "Troot",   "root",   pSelect->vMediaId);	break;
				}
			}
			else
			{
				getString = "SELECT val FROM %s WHERE id=%d;";
				/* Get string directly from table */
				switch(vField)
				{
					/* The following fields must be retrieved through sql */
					case FLD_TITLE:		sqlCmd = sqlite3_mprintf(getString, "Ttitle",  pSelect->vTitleId);	break;
					case FLD_ARTIST:	sqlCmd = sqlite3_mprintf(getString, "Tartist", pSelect->vArtistId);	break;
					case FLD_ALBUM:		sqlCmd = sqlite3_mprintf(getString, "Talbum",  pSelect->vAlbumId);	break;
					case FLD_YEAR:		sqlCmd = sqlite3_mprintf(getString, "Tyear",   pSelect->vYearId);	break;
					case FLD_GENRE:		sqlCmd = sqlite3_mprintf(getString, "Tgenre",  pSelect->vGenreId);	break;
					case FLD_FILE:		sqlCmd = sqlite3_mprintf(getString, "Tfile",   pSelect->vFileId);	break;
					case FLD_ROOT:		sqlCmd = sqlite3_mprintf(getString, "Troot",   pSelect->vRootId);	break;
				}
			}
			break;

		default:
			return(0);
	}

	vTimeout = 0;
	while((vExec = sqlite3_get_table(pUsr, sqlCmd, &pResult, &vRows, NULL, &szError)) == SQLITE_BUSY)
		if(++vTimeout > 100) break;
	if(vExec == SQLITE_OK)
	{
		len = 0;
		if(vRows > 0)
		{
			LPSTR pCheck = pResult[1];
			if(SUCCEEDED(StringCchLengthA(pCheck, STRSAFE_MAX_CCH, &len)))
			{
				if(claimMem)
				{
					*pStr = (LPTSTR)GlobalAlloc(GPTR, ++len * sizeof(TCHAR));
					vMaxLen = len;
				}
				if(*pStr)
				{
					StringCchPrintf(*pStr, vMaxLen, TEXT("%hs"), pCheck);
					hr = StringCchLength(*pStr, vMaxLen, &len);
				}
			}
		}
		sqlite3_free_table(pResult);
	}
	sqlite3_free(sqlCmd);
	if(szError)
		ShowError(&szError);
	
	return(SUCCEEDED(hr) ? ++len : 0);
}

/* Get or create an id for the current pTextVal string in table pTable */
INT Database::GetCreateId(sqlite3 * pDb, LPCSTR pTable, LPCSTR pTextVal)
{
	LPSTR  szError = NULL;
	LPSTR  sqlCmd, sqlGetId;
	LPSTR *pResult;
	INT    nRows;
	INT    vRet = 0;
	INT    vExec, vTimeout;

	sqlGetId = sqlite3_mprintf("SELECT id FROM %s WHERE val=\'%q\';", pTable, pTextVal);
	while(1)
	{
		vTimeout = 0;
		while((vExec = sqlite3_get_table(pDb, sqlGetId, &pResult, &nRows, NULL, &szError)) == SQLITE_BUSY)
			if(vTimeout++ > 100) break;
		if(vExec == SQLITE_OK)
		{
			if(nRows == 1)
				vRet = atoi(pResult[1]);
			sqlite3_free_table(pResult);
			if(nRows == 0)
			{
				sqlCmd = sqlite3_mprintf("INSERT INTO %s (val) VALUES (\'%q\');", pTable, pTextVal);
				vTimeout = 0;
				while((vExec = sqlite3_exec(pDb, sqlCmd, NULL, NULL, &szError)) == SQLITE_BUSY)
					if(vTimeout++ > 100) break;
				sqlite3_free(sqlCmd);
				if(vExec != SQLITE_OK) //error
					break;
			}
			else break;
		}
	}
	sqlite3_free(sqlGetId);
	if(szError)
		ShowError(&szError);
	return(vRet);
}

INT Database::FindString(MediaField vField, LPSTR pName)
{
	LPSTR	pTable = NULL;
	LPSTR	szError = NULL;
	LPSTR	sqlGetId;
	LPSTR	*pResult;
	INT		nRows;
	INT		vRet = 0;
	INT		vExec, vTimeout;

	switch(vField)
	{
		case FLD_TITLE:		pTable = "Ttitle"; break;
		case FLD_ARTIST:	pTable = "Tartist"; break;
		case FLD_ALBUM:		pTable = "Talbum"; break;
		default:			return(0);
	}

	sqlGetId = sqlite3_mprintf("SELECT id FROM %s WHERE val=\'%q\';", pTable, pName);
	if(sqlGetId)
	{
		vTimeout = 0;
		while((vExec = sqlite3_get_table(pSys, sqlGetId, &pResult, &nRows, NULL, &szError)) == SQLITE_BUSY)
			if(vTimeout++ > 100) break;
		if(vExec == SQLITE_OK)
		{
			if(nRows == 1)
				vRet = atoi(pResult[1]);
			sqlite3_free_table(pResult);
		}
		sqlite3_free(sqlGetId);
		if(szError)
			ShowError(&szError);
	}
	return(vRet);
}

PMediaDescr Database::GetMedia(INT vMediaId)
{
	LPSTR        szError = NULL;
	LPSTR        sqlCmd;
	LPSTR       *pResult;
	INT          vRows, vCols;
	INT          vExec, vTimeout;
	PMediaDescr  pSelect = NULL;

	sqlCmd = sqlite3_mprintf("SELECT %s WHERE id=%d;", queryToListbox, vMediaId);

	vTimeout = 0;
	while((vExec = sqlite3_get_table(pUsr, sqlCmd, &pResult, &vRows, &vCols, &szError)) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	sqlite3_free(sqlCmd);
	if(szError)
		ShowError(&szError);
	if(vExec == SQLITE_OK)
	{
		if((vRows >= 1) && (vCols > 0))
		{
			pSelect = (PMediaDescr)GlobalAlloc(GPTR, sizeof(MediaDescr));
			if(pSelect != NULL)
			{
				switch(vCols)
				{
					default:
					case 14:	if(pResult[vCols + 13])	pSelect->vRating = atoi(pResult[vCols + 13]);
					case 13:	if(pResult[vCols + 12])	pSelect->vPlayTime = atoi(pResult[vCols + 12]);
					case 12:	if(pResult[vCols + 11])	pSelect->vRootId = atoi(pResult[vCols + 11]);
					case 11:	if(pResult[vCols + 10])	pSelect->vFolderId = atoi(pResult[vCols + 10]);
					case 10:	if(pResult[vCols + 9])	pSelect->vFileId = atoi(pResult[vCols + 9]);
					case 9:		if(pResult[vCols + 8])	pSelect->vTypeId = atoi(pResult[vCols + 8]);
					case 8:		if(pResult[vCols + 7])	pSelect->vGenreId = atoi(pResult[vCols + 7]);
					case 7:		if(pResult[vCols + 6])	pSelect->vYearId = atoi(pResult[vCols + 6]);
					case 6:		if(pResult[vCols + 5])	pSelect->vTrack = atoi(pResult[vCols + 5]);
					case 5:		if(pResult[vCols + 4])	pSelect->vAlbumId = atoi(pResult[vCols + 4]);
					case 4:		if(pResult[vCols + 3])	pSelect->vArtistId = atoi(pResult[vCols + 3]);
					case 3:		if(pResult[vCols + 2])	pSelect->vTitleId = atoi(pResult[vCols + 2]);
					case 2:		if(pResult[vCols + 1])	pSelect->vInvalid = atoi(pResult[vCols + 1]);
					case 1:		if(pResult[vCols + 0])	pSelect->vMediaId = atoi(pResult[vCols + 0]);
				}
			}
		}
		sqlite3_free_table(pResult);
	}
	return(pSelect);
}

BOOL Database::AddRemoveRootEntry(BOOL vAdd, LPTSTR pRoot)
{
	LPSTR   szError = NULL;
	LPSTR   sqlCmd, pChar;
	LPSTR  *pResult;
	INT     vRows;
	size_t  vLen;
	BOOL    vRet = FALSE;
	INT     vExec, vTimeout;

	if((pRoot == NULL) || (pUsr == NULL))
		return(FALSE);

	/* Make char string */
	StringCchLength(pRoot, MAX_PATH, &vLen);
	pChar = (LPSTR)GlobalAlloc(GPTR, ++vLen);
	if(pChar)
	{
#ifdef UNICODE
		StringCbPrintfA(pChar, vLen, "%ws", pRoot);
#else
		StringCbPrintfA(pChar, vLen, "%hs", pRoot);
#endif

		if(vAdd)
		{
			sqlCmd = sqlite3_mprintf("SELECT * FROM Troot WHERE val=\'%q\';", pChar);
			vTimeout = 0;
			while((vExec = sqlite3_get_table(pUsr, sqlCmd, &pResult, &vRows, NULL, &szError)) == SQLITE_BUSY)
				if(vTimeout++ > 100) break;
			sqlite3_free(sqlCmd);
			if(vExec == SQLITE_OK)
			{
				if(vRows == 0)
				{
					GetCreateId(pUsr, "Troot", pChar);
					vRet = TRUE;
				}
				sqlite3_free_table(pResult);
			}
		}
		else
		{
			sqlCmd = sqlite3_mprintf("DELETE FROM Troot WHERE val=\'%q\';", pChar);
			vTimeout = 0;
			while(sqlite3_exec(pUsr, sqlCmd, NULL, NULL, &szError) == SQLITE_BUSY)
				if(vTimeout++ > 100) break;
			sqlite3_free(sqlCmd);
			vRet = TRUE;
		}
		GlobalFree((HGLOBAL)pChar);
		if(szError)
			ShowError(&szError);
	}
	return(vRet);
}

static const LPSTR szCreateGenreTable[] =
{
/*0-4*/		"Blues", "Classic Rock", "Country", "Dance", "Disco", 
/*5-9*/		"Funk", "Grunge", "Hip-Hop", "Jazz", "Metal",
/*10-14*/	"New Age", "Oldies", "Other", "Pop", "R&B",
/*15-19*/	"Rap", "Reggae", "Rock", "Techno", "Industrial",
/*20-24*/	"Alternative", "Ska", "Death Metal", "Pranks", "Soundtrack",
/*25-29*/	"Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk",
/*30-34*/	"Fusion", "Trance", "Classical", "Instrumental", "Acid",
/*35-39*/	"House", "Game", "Sound Clip", "Gospel", "Noise",
/*40-44*/	"Alternative Rock", "Bass", "", "Punk", "Space",
/*45-49*/	"Meditative", "Instrumental Pop", "Instrumental Rock", "Ethnic", "Gothic",
/*50-54*/	"Darkwave", "Techno-Industrial", "Electronic", "Pop-Folk", "Eurodance",
/*55-59*/	"Dream", "Southern Rock", "Comedy", "Cult", "Gangsta", 
/*60-64*/	"Top 40", "Christian Rap", "Pop/Funk", "Jungle", "Native US", 
/*65-69*/	"Cabaret", "New Wave", "Psychadelic", "Rave", "Showtunes",
/*70-74*/	"Trailer", "Lo-Fi", "Tribal", "Acid Punk", "Acid Jazz",
/*75-79*/	"Polka", "Retro", "Musical", "Rock & Roll", "Hard Rock",
/*80-84*/	"Folk", "Folk-Rock", "National Folk", "Swing", "Fast Fusion",
/*85-89*/	"Bebob", "Latin", "Revival", "Celtic", "Bluegrass",
/*90-94*/	"Avantgarde", "Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock",
/*95-99*/	"Slow Rock", "Big Band", "Chorus", "Easy Listening", "Acoustic ",
/*100-104*/	"Humour", "Speech", "Chanson", "Opera", "Chamber Music", 
/*105-109*/	"Sonata", "Symphony", "Booty Bass", "Primus", "Porn Groove",
/*110-114*/	"Satire", "Slow Jam", "Club", "Tango", "Samba",
/*115-119*/	"Folklore", "Ballad", "Power Ballad", "Rhytmic Soul", "Freestyle",
/*120-124*/	"Duet", "Punk Rock", "Drum Solo", "Acapella", "Euro-House",
/*125-129*/	"Dance Hall", "Goa", "Drum & Bass", "Club-House", "Hardcore", 
/*130-134*/	"Terror", "Indie", "BritPop", "Negerpunk", "Polsk Punk",
/*135-139*/	"Beat", "Christian Gangsta", "Heavy Metal", "Black Metal", "Crossover",
/*140-144*/	"Contemporary C", "Christian Rock", "Merengue", "Salsa", "Thrash Metal",
/*145-147*/	"Anime", "JPop", "SynthPop",
	NULL
};

void Database::CreateGenres(void)
{
	LPSTR szError = NULL;
	LPSTR sqlCmd;
	INT   vTimeout;
	INT   i;

	/* Create temporary (memory) table */
	sqlite3_exec(pUsr, "CREATE TEMP TABLE TgenreTmp (id INTEGER PRIMARY KEY, val VARCHAR(10), static INTEGER);", NULL, NULL, &szError);
	if(szError)
		ShowError(&szError);

	/* Fill temporary table with records */
	for(i = 0; szCreateGenreTable[i] != NULL; i++)
	{
		sqlCmd = sqlite3_mprintf("INSERT INTO TgenreTmp (id, val, static) VALUES(%d, \'%q\', 1);", i, szCreateGenreTable[i]);
		vTimeout = 0;
		while(sqlite3_exec(pUsr, sqlCmd, NULL, NULL, &szError) == SQLITE_BUSY)
			if(vTimeout++ > 100) break;
		sqlite3_free(sqlCmd);
		if(szError)
			ShowError(&szError);
	}

	/* Copy temp table to config database */
	sqlite3_exec(pUsr, "INSERT INTO Tgenre (id, val, static) SELECT id, val, static FROM TgenreTmp;", NULL, NULL, &szError);
	if(szError)
		ShowError(&szError);
}

INT Database::AddAlbum(LPSTR szArtist, LPSTR szAlbum, LPSTR szTitle, INT nGenre, INT nYear, LPSTR szFile, INT vTrack, UINT vId, FileType vFt)
{
	const CHAR sqlSelHash[] = "SELECT invalid f0, title f1, artist f2, album f3, track f4, "
						      "year f5, genre f6, type f7, file f8, folder f9, root f10, "
						      "playtime f11, rating f12 FROM Tmedia WHERE hash=%lu;";
	PMediaDescr  pSelect;
	LPSTR        szError = NULL;
	LPSTR        sqlCmd, sqlIm1, sqlIm2;
	LPSTR       *pResult;
	INT          nRows, nCols, vRootCnt;
	INT          vStringId;
	INT          vExec, vTimeout;
	LPSTR        pTmp, pChar;
	INT          retVal = 0;

	// A database and function pinter should already exist
	if(pSys == NULL)
		return(-1);

	pSelect = (PMediaDescr)GlobalAlloc(GPTR, sizeof(MediaDescr));
	if(pSelect == NULL)
		return(-1);

	/* Search db for existing file by hash value*/
	sqlCmd = sqlite3_mprintf(sqlSelHash, vId);
	vTimeout = 0;
	while((vExec = sqlite3_get_table(pSys, sqlCmd, &pResult, &nRows, &nCols, &szError)) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	sqlite3_free(sqlCmd);
	if(vExec == SQLITE_OK)
	{
		if(nRows == 1)	/* Found */
		{
			if(pResult[nCols + 0])
				pSelect->vInvalid = *pResult[nCols + 0] ? TRUE : FALSE;
			if(pResult[nCols + 1])
				pSelect->vTitleId = atoi(pResult[nCols + 1]);
			if(pResult[nCols + 2])
				pSelect->vArtistId = atoi(pResult[nCols + 2]);
			if(pResult[nCols + 3])
				pSelect->vAlbumId = atoi(pResult[nCols + 3]);
			if(pResult[nCols + 4])
				pSelect->vTrack = atoi(pResult[nCols + 4]);
			if(pResult[nCols + 5])
				pSelect->vYearId = atoi(pResult[nCols + 5]);
			if(pResult[nCols + 6])
				pSelect->vGenreId = atoi(pResult[nCols + 6]);
			if(pResult[nCols + 7])
				pSelect->vTypeId = atoi(pResult[nCols + 7]);
			if(pResult[nCols + 8])
				pSelect->vFileId = atoi(pResult[nCols + 8]);
			if(pResult[nCols + 9])
				pSelect->vFolderId = atoi(pResult[nCols + 9]);
			if(pResult[nCols + 10])
				pSelect->vRootId = atoi(pResult[nCols + 10]);
			if(pResult[nCols + 11])
				pSelect->vPlayTime = atoi(pResult[nCols + 11]);
			if(pResult[nCols + 12])
				pSelect->vRating = atoi(pResult[nCols + 12]);
			retVal = 1; // Updated
		}
		sqlite3_free_table(pResult);
	}

	/* Get or create an id for the current title */
	vStringId = GetCreateId(pSys, "Ttitle", szTitle);
	if((vStringId != pSelect->vTitleId))
		pSelect->vTitleId = vStringId;

	/* Get or create an id for the current artist */
	vStringId = GetCreateId(pSys, "Tartist", szArtist);
	if((vStringId != pSelect->vArtistId))
		pSelect->vArtistId = vStringId;

	/* Get or create an id for the current album */
	vStringId = GetCreateId(pSys, "Talbum", szAlbum);
	if((vStringId != pSelect->vAlbumId))
		pSelect->vAlbumId = vStringId;

	/* Save value for the current track */
	if((vTrack != pSelect->vTrack))
		pSelect->vTrack = vTrack;

	/* Save value for the current genre */
	if((nGenre != pSelect->vGenreId))
		pSelect->vGenreId = nGenre;

	/* Get or create an id for the current year */
	szError = sqlite3_mprintf("%d", nYear);
	vStringId = GetCreateId(pSys, "Tyear", szError);
	if((vStringId != pSelect->vYearId))
		pSelect->vYearId = vStringId;
	sqlite3_free(szError);

	/* Get or create an id for the file */
	vStringId = GetCreateId(pSys, "Tfile", szFile);
	if((vStringId != pSelect->vFileId))
		pSelect->vFileId = vStringId;

	pSelect->vTypeId = (INT)vFt;

	/* Find the associated root id */
	pTmp = (LPSTR)GlobalAlloc(GPTR, strlen(szFile) + 1);
	if(pTmp)
	{
		memcpy(pTmp, szFile, strlen(szFile) + 1);
		vRootCnt = 0;
		while(vRootCnt == 0)
		{
			pChar = strrchr(pTmp, '\\');
			if(pChar)
			{
				pChar[0] = '\0';
				sqlCmd = sqlite3_mprintf("SELECT id FROM Troot WHERE val=\'%q\';", pTmp);
				vTimeout = 0;
				while((vExec = sqlite3_get_table(pSys, sqlCmd, &pResult, &vRootCnt, NULL, &szError)) == SQLITE_BUSY)
					if(vTimeout++ > 100) break;
				sqlite3_free(sqlCmd);
				if(vExec == SQLITE_OK)
				{
					if((vRootCnt >= 1) && (pResult[1] != NULL))
					{
						vStringId = atoi(pResult[1]);
						if((vStringId != pSelect->vRootId))
							pSelect->vRootId = vStringId;
					}
					sqlite3_free_table(pResult);
				}
			}
			if((pChar == NULL) || (vExec != SQLITE_OK))
				break;
		}
		GlobalFree((HGLOBAL)pTmp);
	}

	if(nRows == 0)	/* Not found */
	{
		/* Create the Tmedia entry for this files with empty fields */
		sqlCmd = sqlite3_mprintf("INSERT INTO Tmedia(hash, title, title_org, artist, artist_org, file, "
									"folder, root, invalid) VALUES (%lu, 0, 0, 0, 0, 0, 0, 0, 0);", vId);
		vTimeout = 0;
		while(sqlite3_exec(pSys, sqlCmd, NULL, NULL, &szError) == SQLITE_BUSY)
			if(vTimeout++ > 100) break;
		sqlite3_free(sqlCmd);
		if(szError)
			ShowError(&szError);
	}
	/* Now update the existing or just created record with the real data */
	sqlIm1 = sqlite3_mprintf("");
	sqlCmd = "%s, %s=%d, %s=%d";
	if(pSelect->vTitleId)
	{
		sqlIm2 = sqlIm1;
		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "title", pSelect->vTitleId, "title_org", pSelect->vTitleId);
		sqlite3_free(sqlIm2);
	}
	if(pSelect->vArtistId)
	{
		sqlIm2 = sqlIm1;
		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "artist", pSelect->vArtistId, "artist_org", pSelect->vArtistId);
		sqlite3_free(sqlIm2);
	}
	if(pSelect->vAlbumId)
	{
		sqlIm2 = sqlIm1;
		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "album", pSelect->vAlbumId, "album_org", pSelect->vAlbumId);
		sqlite3_free(sqlIm2);
	}
	if(pSelect->vTrack)
	{
		sqlIm2 = sqlIm1;
		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "track", pSelect->vTrack, "track_org", pSelect->vTrack);
		sqlite3_free(sqlIm2);
	}
	if(pSelect->vYearId)
	{
		sqlIm2 = sqlIm1;
		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "year", pSelect->vYearId, "year_org", pSelect->vYearId);
		sqlite3_free(sqlIm2);
	}
	if(pSelect->vGenreId)
	{
		sqlIm2 = sqlIm1;
		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "genre", pSelect->vGenreId, "genre_org", pSelect->vGenreId);
		sqlite3_free(sqlIm2);
	}
	sqlCmd = "%s, %s=%d";
	if(pSelect->vTypeId)
	{
		sqlIm2 = sqlIm1;
		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "type", pSelect->vTypeId);
		sqlite3_free(sqlIm2);
	}
	if(pSelect->vFileId)
	{
		sqlIm2 = sqlIm1;
		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "file", pSelect->vFileId);
		sqlite3_free(sqlIm2);
	}
//	if(pSelect->vFolderId)
//	{
//		sqlIm2 = sqlIm1;
//		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "folder", pSelect->vFolderId);
//		sqlite3_free(sqlIm2);
//	}
	if(pSelect->vRootId)
	{
		sqlIm2 = sqlIm1;
		sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "root", pSelect->vRootId);
		sqlite3_free(sqlIm2);
	}
	StringCchLengthA(sqlIm1, STRSAFE_MAX_CCH, (size_t*)&nCols);
	if(nCols)
	{
		sqlCmd = sqlite3_mprintf("UPDATE Tmedia SET %s WHERE hash=%lu;", &sqlIm1[2], vId);
		vTimeout = 0;
		while((vExec = sqlite3_exec(pSys, sqlCmd, NULL, NULL, &szError)) == SQLITE_BUSY)
			if(vTimeout++ > 100) break;
		sqlite3_free(sqlCmd);
		if(vExec != SQLITE_OK) //error
			if(szError)
				ShowError(&szError);
	}
	sqlite3_free(sqlIm1);
	return(retVal);
}

INT Database::ChangeTrack(PMediaDescr pSelect)
{
	const CHAR sqlSel[] = "SELECT title f0, title_org f1, artist f2, artist_org f3, album f4, "
						  "album_org f5, track f6, track_org f7, year f8, year_org f9, "
						  "genre f10, genre_org f11, invalid f12, playtime f13 FROM Tmedia WHERE id=%d;";
	PMediaDescr  pEdit;
	LPSTR        szError = NULL;
	LPSTR        sqlCmd, sqlIm1, sqlIm2;
	LPSTR       *pResult;
	INT          vRows, vCols, vVal;
	INT          vExec, vTimeout;

	// A database and function pinter should already exist
	if((pUsr == NULL) || (pSelect == NULL))
		return(1);

	/* Duplicate changing values into new structure */
	pEdit = (PMediaDescr)GlobalAlloc(GPTR, sizeof(MediaDescr));
	if(pEdit == NULL)
		return(1);
	pEdit->vMediaId = pSelect->vMediaId;
	pEdit->vInvalid = pSelect->vInvalid;

	/* Search db for existing file */
	sqlCmd = sqlite3_mprintf(sqlSel, pEdit->vMediaId);
	vTimeout = 0;
	while((vExec = sqlite3_get_table(pUsr, sqlCmd, &pResult, &vRows, &vCols, &szError)) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	sqlite3_free(sqlCmd);
	if(vExec == SQLITE_OK)
	{
		if(vRows == 1)
		{
			if(pResult[vCols + 0])
				vVal = atoi(pResult[vCols + 0]);
			if(vVal != pSelect->vTitleId)
			{
				if(pResult[vCols + 1])
					vVal = atoi(pResult[vCols + 1]);
				if(vVal != pSelect->vTitleId)
					pEdit->vTitleId = vVal;
			}

			if(pResult[vCols + 2])
				vVal = atoi(pResult[vCols + 2]);
			if(vVal != pSelect->vArtistId)
			{
				if(pResult[vCols + 3])
					vVal = atoi(pResult[vCols + 3]);
				if(vVal != pSelect->vArtistId)
					pEdit->vArtistId = vVal;
			}

			if(pResult[vCols + 4])
				vVal = atoi(pResult[vCols + 4]);
			if(vVal != pSelect->vAlbumId)
			{
				if(pResult[vCols + 5])
					vVal = atoi(pResult[vCols + 5]);
				if(vVal != pSelect->vAlbumId)
					pEdit->vAlbumId = vVal;
			}

			if(pResult[vCols + 6])
				vVal = atoi(pResult[vCols + 6]);
			if(vVal != pSelect->vTrack)
			{
				if(pResult[vCols + 7])
					vVal = atoi(pResult[vCols + 7]);
				if(vVal != pSelect->vTrack)
					pEdit->vTrack = vVal;
			}

			if(pResult[vCols + 8])
				vVal = atoi(pResult[vCols + 8]);
			if(vVal != pSelect->vYearId)
			{
				if(pResult[vCols + 9])
					vVal = atoi(pResult[vCols + 9]);
				if(vVal != pSelect->vYearId)
					pEdit->vYearId = vVal;
			}

			if(pResult[vCols + 10])
				vVal = atoi(pResult[vCols + 10]);
			if(vVal != pSelect->vGenreId)
			{
				if(pResult[vCols + 11])
					vVal = atoi(pResult[vCols + 11]);
				if(vVal != pSelect->vGenreId)
					pEdit->vGenreId = vVal;
			}

			vVal = 0;
			if(pResult[vCols + 13])
				vVal = atoi(pResult[vCols + 13]);
			if(vVal != pSelect->vPlayTime)
				pEdit->vPlayTime = pSelect->vPlayTime;

			/* Now update the exiting or just created record with the real data */
			sqlIm1 = sqlite3_mprintf("");
			sqlCmd = "%s, %s=%d";
			if(pEdit->vTitleId)
			{
				sqlIm2 = sqlIm1;
				sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "title", pEdit->vTitleId);
				sqlite3_free(sqlIm2);
			}
			if(pEdit->vArtistId)
			{
				sqlIm2 = sqlIm1;
				sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "artist", pEdit->vArtistId);
				sqlite3_free(sqlIm2);
			}
			if(pEdit->vAlbumId)
			{
				sqlIm2 = sqlIm1;
				sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "album", pEdit->vAlbumId);
				sqlite3_free(sqlIm2);
			}
			if(pEdit->vTrack)
			{
				sqlIm2 = sqlIm1;
				sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "track", pEdit->vTrack);
				sqlite3_free(sqlIm2);
			}
			if(pEdit->vYearId)
			{
				sqlIm2 = sqlIm1;
				sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "year", pEdit->vYearId);
				sqlite3_free(sqlIm2);
			}
			if(pEdit->vGenreId)
			{
				sqlIm2 = sqlIm1;
				sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "genre", pEdit->vGenreId);
				sqlite3_free(sqlIm2);
			}
			if(pEdit->vPlayTime)
			{
				sqlIm2 = sqlIm1;
				sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "playtime", pEdit->vPlayTime);
				sqlite3_free(sqlIm2);
			}
			sqlIm2 = sqlIm1;
			sqlIm1 = sqlite3_mprintf(sqlCmd, sqlIm2, "invalid", pEdit->vInvalid);
			sqlite3_free(sqlIm2);

			StringCchLengthA(sqlIm1, STRSAFE_MAX_CCH, (size_t*)&vVal);
			if(vVal)
			{
				sqlCmd = sqlite3_mprintf("UPDATE Tmedia SET %s WHERE id=%d;", &sqlIm1[2], pEdit->vMediaId);
				vTimeout = 0;
				while((vExec = sqlite3_exec(pUsr, sqlCmd, NULL, NULL, &szError)) == SQLITE_BUSY)
					if(vTimeout++ > 100) break;
				sqlite3_free(sqlCmd);
				if(vExec != SQLITE_OK) //error
					if(szError)
						ShowError(&szError);
			}
			sqlite3_free(sqlIm1);
		}
		sqlite3_free_table(pResult);
	}
	GlobalFree((HGLOBAL)pEdit);
	return(vRows ? 0 : 1);
}

void Database::KillTrack(PMediaDescr pSelect)
{
	LPSTR  szError = NULL;
	LPSTR  sqlCmd;
	INT    vTimeout;

	// A database and function pinter should already exist
	if((pUsr == NULL) || (pSelect == NULL))
		return;

	/* Search db for existing file */
	sqlCmd = sqlite3_mprintf("DELETE FROM Tmedia WHERE id=%d;", pSelect->vMediaId);
	vTimeout = 0;
	while(sqlite3_exec(pUsr, sqlCmd, NULL, NULL, &szError) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	sqlite3_free(sqlCmd);
	if(szError)
		ShowError(&szError);
}

void Database::GarbageCollection(void)
{
	LPSTR szError = NULL;
	INT   vTimeout;

	/* Search Tmedia entries with deleted root */
	vTimeout = 0;
	while(sqlite3_exec(pSys, "DELETE FROM Tmedia WHERE root NOT IN (SELECT id FROM Troot) and id <> 0;", NULL, NULL, &szError) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	if(szError)
		ShowError(&szError);

	/* Search Tmedia entries with deleted file */
	vTimeout = 0;
	while(sqlite3_exec(pSys, "DELETE FROM Tmedia WHERE file NOT IN (SELECT id FROM Tfile) and id <> 0;", NULL, NULL, &szError) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	if(szError)
		ShowError(&szError);

	/* Find orphanaged titles, artists, albums, genres and years */
	vTimeout = 0;
	while(sqlite3_exec(pSys, "DELETE FROM Ttitle WHERE id NOT IN (SELECT title FROM Tmedia);", NULL, NULL, &szError) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	if(szError)
		ShowError(&szError);

	vTimeout = 0;
	while(sqlite3_exec(pSys, "DELETE FROM Tartist WHERE id NOT IN (SELECT artist FROM Tmedia) and id <> 1;", NULL, NULL, &szError) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	if(szError)
		ShowError(&szError);

	vTimeout = 0;
	while(sqlite3_exec(pSys, "DELETE FROM Talbum WHERE id NOT IN (SELECT album FROM Tmedia);", NULL, NULL, &szError) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	if(szError)
		ShowError(&szError);

	vTimeout = 0;
	while(sqlite3_exec(pSys, "DELETE FROM Tyear WHERE id NOT IN (SELECT year FROM Tmedia);", NULL, NULL, &szError) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	if(szError)
		ShowError(&szError);

	vTimeout = 0;
	while(sqlite3_exec(pSys, "DELETE FROM Tgenre WHERE id NOT IN (SELECT genre FROM Tmedia) AND static=0;", NULL, NULL, &szError) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	if(szError)
		ShowError(&szError);
}

/******************************************************************************************************************/
/******************************************************************************************************************/
/******************************************************************************************************************/
static const LPSTR szCreateCfgTables[] =
{
	"CREATE TABLE Tconfig(tag VARCHAR(8) PRIMARY KEY, value VARCHAR(4));",
	"CREATE TABLE Tplaying(x INTEGER PRIMARY KEY, id INTEGER, prot INTEGER);",
	"CREATE TABLE Thistory(x INTEGER PRIMARY KEY, id INTEGER, prev INTEGER);",
	"CREATE TABLE Tstations(x INTEGER PRIMARY KEY, name  VARCHAR(20), channel INTEGER);",
	NULL
};

void Database::OpenCfg(void)
{
	FILE *f;
	BOOL  vExists;

	// Check the size of the database
	f = fopen(vConfigPath, "rb");
	vExists = (f ? FALSE : TRUE);
	if(f != NULL)
		fclose(f);

	if(sqlite3_open(vConfigPath, &pCfg) != SQLITE_OK)
	{
		MessageBoxA(NULL, sqlite3_errmsg(pCfg), "Sqlite error", MB_OK | MB_ICONERROR);
		CloseCfg();
	}
	else
		sqlite3_busy_timeout(pCfg, 1000);

	// It seems there is a newly created database
	if(vExists)
		CreateCfg();

	// Check the size of the global database
	f = fopen(vGlobalsPath, "rb");
	if(f)
	{
		fclose(f);

		if(sqlite3_open(vGlobalsPath, &pCfgGlob) == SQLITE_OK)
			sqlite3_busy_timeout(pCfgGlob, 1000);
	}
}

void Database::CloseCfg(void)
{
	if(pCfg)
		sqlite3_close(pCfg);
	pCfg = NULL;
	if(pCfgGlob)
		sqlite3_close(pCfgGlob);
	pCfgGlob = NULL;
}

void Database::CreateCfg(void)
{
	LPSTR szError = NULL;
	INT   i;
	INT   vTimeout;

	for(i = 0; szCreateCfgTables[i] != NULL; i++)
	{
		vTimeout = 0;
		while(sqlite3_exec(pCfg, szCreateCfgTables[i], NULL, NULL, &szError) == SQLITE_BUSY)
			if(vTimeout++ > 100) break;
		if(szError)
			ShowError(&szError);
	}
}

void Database::RecreateCfg(void)
{
	CloseCfg();
	DeleteFileA(vConfigPath);
	OpenCfg();
}

INT Database::GetConfig(LPSTR lpVar, LPVOID * lpData, tConfigType type)
{
	const CHAR inQuery[] = "SELECT value FROM Tconfig WHERE tag = \'%q\';";

	LPSTR	szError = NULL;
	LPSTR	outQuery;
	LPSTR	*pResult;
	int		vRows = 0;
	size_t	vLen;
	sqlite3	*pDatabase;

	// A database and function pinter should already exist
	if(pCfg == NULL)
		return(0);
	if(lpVar == NULL)
		return(0);

	for(pDatabase = pCfg; ; pDatabase = pCfgGlob)
	{
		if(pDatabase == 0)
			return(0);

		outQuery = sqlite3_mprintf(inQuery, lpVar);
		sqlite3_get_table(pDatabase, outQuery, &pResult, &vRows, NULL, &szError);
		sqlite3_free(outQuery);
		if(szError)
			ShowError(&szError);
		if(vRows == 1)
		{
			if(lpData)
			{
				switch(type)
				{
					case CONFIGTYPE_BOOL:
						if(*lpData == NULL)
							*lpData = malloc(sizeof(BOOL));
						*(LPBOOL)(*lpData) = (atoi(pResult[1]) ? TRUE : FALSE);
						break;

					case CONFIGTYPE_INT:
						if(*lpData == NULL)
							*lpData = malloc(sizeof(INT));
						*(LPINT)(*lpData) = atoi(pResult[1]);
						break;

					case CONFIGTYPE_LONG:
						if(*lpData == NULL)
							*lpData = malloc(sizeof(LONG));
						*(LPLONG)(*lpData) = atol(pResult[1]);
						break;

					case CONFIGTYPE_STRING:
						vLen = strlen(pResult[1]) + 1;
						if(*lpData == NULL)
							*lpData = malloc(vLen);
						StringCbPrintfA((LPSTR)*lpData, vLen, "%s", pResult[1]);
						break;

					case CONFIGTYPE_UNICODE:
						vLen = 2 * (strlen(pResult[1]) + 1);
						if(*lpData == NULL)
							*lpData = malloc(vLen);
						StringCbPrintfW((LPWSTR)*lpData, vLen, L"%hs", pResult[1]);
						break;

					default:
						MessageBoxA(NULL, "Invalid configuration type requested!", "Database error", MB_OK | MB_ICONERROR);
						vRows = 0;
						break;
				}
			}
		}
		if(vRows)
			break;
		if(pDatabase == pCfgGlob)
			break;
	} 
	sqlite3_free_table(pResult);
	return(vRows ? 1 : 0);
}

BOOL Database::GetBool(LPSTR lpVar)
{
	BOOL	vToggle = FALSE;
	LPBOOL	lpToggle = &vToggle;
	GetConfig(lpVar, (LPVOID*)&lpToggle, CONFIGTYPE_BOOL);
	return(vToggle);
}

INT Database::GetInt(LPSTR lpVar)
{
	INT		vInt = FALSE;
	LPINT	lpInt = &vInt;
	GetConfig(lpVar, (LPVOID*)&lpInt, CONFIGTYPE_INT);
	return(vInt);
}

INT Database::SetConfig(LPSTR lpVar, LPVOID lpData, tConfigType type)
{
	const CHAR fndQuery[] = "SELECT * FROM Tconfig WHERE tag = \'%s\';";
	const CHAR addQuery[] = "INSERT INTO Tconfig VALUES(\'%s\', \'%s\');";
	const CHAR updQuery[] = "UPDATE Tconfig SET value = \'%s\' WHERE tag = \'%s\';";

	LPSTR  szError = NULL;
	LPSTR  outQuery;
	LPSTR  pData, pText;
	LPSTR *pResult;
	INT    vRows;

	// A database and function pinter should already exist
	if(pCfg == NULL)
		return(0);
	if((lpData == NULL) || (lpVar == 0))
		return(0);

	outQuery = sqlite3_mprintf(fndQuery, lpVar);
	sqlite3_get_table(pCfg, outQuery, &pResult, &vRows, NULL, &szError);
	sqlite3_free_table(pResult);
	sqlite3_free(outQuery);
	if(szError)
		ShowError(&szError);

	switch(type)
	{
		case CONFIGTYPE_BOOL:
			pData = sqlite3_mprintf("%d", (*(LPBOOL)lpData ? 1 : 0));
			break;

		case CONFIGTYPE_INT:
			pData = sqlite3_mprintf("%d", *(LPINT)lpData);
			break;

		case CONFIGTYPE_LONG:
			pData = sqlite3_mprintf("%ld", *(LPLONG)lpData);
			break;

		case CONFIGTYPE_STRING:
			pData = sqlite3_mprintf("%q", lpData);
			break;

		case CONFIGTYPE_UNICODE:
			pText = (LPSTR)malloc(2 * lstrlen((LPWSTR)lpData) + 6);
			StringCbPrintfA(pText, 2 * lstrlen((LPWSTR)lpData) + 6, "%ws", lpData);
			pData = sqlite3_mprintf("%q", pText);
			sqlite3_free(pText);
			break;

		default:
			return(0);
			break;
	}

	if(vRows == 1)
		outQuery = sqlite3_mprintf(updQuery, pData, lpVar);
	else
		outQuery = sqlite3_mprintf(addQuery, lpVar, pData);
	sqlite3_free(pData);

	sqlite3_exec(pCfg, outQuery, NULL, NULL, &szError);
	sqlite3_free(outQuery);
	if(szError)
		ShowError(&szError);
	return(1);
}

void Database::GetNowPlaying(HWND hWnd)
{
	const CHAR fndQuery[] = "SELECT id, prot FROM Tplaying;";
	const CHAR delQuery[] = "DELETE FROM Tplaying;";
	const CHAR sqlQuery[] = "SELECT %s WHERE id=%d";

	PParamStruct  pCbParm;
	LPSTR         szError = NULL;
	LPSTR        *pResult;
	INT           i, vRows, vMedia;
	LPSTR         sqlCmd;
	INT           vExec;

	// A database and function pinter should already exist
	if((pCfg == NULL) || (hWnd == NULL))
		return;

	/* argument for callback function to access data within our class */
	pCbParm = (PParamStruct)ParamStruct(0);
	pCbParm->hWnd = hWnd;
	pCbParm->vShow = DISPLAY_FULL;

	/* Get the nowplaying list */
	vExec = sqlite3_get_table(pCfg, fndQuery, &pResult, &vRows, NULL, &szError);
	if(vExec == SQLITE_OK)
	{
		for(i = 2; i <= (2 * vRows); i += 2)
		{
			if(pResult[i])
			{
				vMedia = atoi(pResult[i]);
				if(pResult[i + 1])
					pCbParm->vNoRemove = atoi(pResult[i + 1]);
				else
					pCbParm->vNoRemove = 0;

				/* Find the descriptor for the ID */
				sqlCmd = sqlite3_mprintf(sqlQuery, queryToListbox, vMedia);
				sqlite3_exec(pUsr, sqlCmd, (sqlite3_callback)QueryToListboxCb, (void*)pCbParm, &szError);
				sqlite3_free(sqlCmd);
				if(szError)
					ShowError(&szError);
			}
		}
		sqlite3_free_table(pResult);
	}
	if(szError)
		ShowError(&szError);

	GlobalFree((HGLOBAL)pCbParm);

	sqlite3_exec(pCfg, delQuery, NULL, NULL, &szError);
	if(szError)
		ShowError(&szError);
}

void Database::SaveNowPlaying(PMediaDescr *pMedia)
{
	LPSTR szError = NULL;
	LPSTR sqlCmd;

	// A database and function pinter should already exist
	if((pCfg == NULL) || (pMedia == NULL))
		return;

	/* At least one entry in list? */
	if(*pMedia == NULL)
		return;

	/* Create temporary (memory) table */
	sqlite3_exec(pCfg, "CREATE TEMP TABLE TplayingTmp (x INTEGER PRIMARY KEY, id INTEGER, prot INTEGER);", NULL, NULL, &szError);
	if(szError)
		ShowError(&szError);

	/* Fill temporary table with records */
	while(*pMedia != NULL)
	{
		if((*pMedia)->vHistory == 0)
		{
			sqlCmd = sqlite3_mprintf("INSERT INTO TplayingTmp (id, prot) VALUES(%d, %d);", (*pMedia)->vMediaId, (*pMedia)->vNoRemove);
			sqlite3_exec(pCfg, sqlCmd, NULL, NULL, &szError);
			sqlite3_free(sqlCmd);
			if(szError)
				ShowError(&szError);
		}
		*pMedia++;
	}
	/* Copy temp table to config database */
	sqlite3_exec(pCfg, "INSERT INTO Tplaying (x, id, prot) SELECT x, id, prot FROM TplayingTmp;", NULL, NULL, &szError);
	if(szError)
		ShowError(&szError);
}

void Database::SaveHistory(PMediaDescr pMedia)
{
	LPSTR  szError = NULL;
	LPSTR  sqlCmd;
	LPSTR *pResult;
	INT    vRows, vCount, vTimeout;
	INT    vPrev = 0;

	if((pCfg == NULL) || (pMedia == NULL))
		return;

	/* Dont save when history id present (used to prevent saving records from Thistory) */
	if(pMedia->vHistory || pMedia->vInvalid)
		return;

	/* Add the record to history */
	sqlCmd = sqlite3_mprintf("INSERT INTO Thistory (id, prev) VALUES(%d, %d);", pMedia->vMediaId, vPrev);
	vTimeout = 0;
	while(sqlite3_exec(pCfg, sqlCmd, NULL, NULL, &szError) == SQLITE_BUSY)
		if(vTimeout++ > 100) break;
	sqlite3_free(sqlCmd);
	if(szError)
		ShowError(&szError);

	/* Read number of entries in history */
	vCount = 0;
	if(sqlite3_get_table(pCfg, "SELECT COUNT(x) FROM Thistory;", &pResult, &vRows, NULL, &szError) == SQLITE_OK)
	{
		if((vRows == 1) && pResult[1])
			vCount = atoi(pResult[1]);
		sqlite3_free_table(pResult);
	}

	/* Limit history depth to 100 entries */
	while((vCount - 100) > 0)
	{
		if(sqlite3_get_table(pCfg, "SELECT MIN(x) FROM Thistory;", &pResult, &vRows, NULL, &szError) == SQLITE_OK)
		{
			if((vRows == 1) && pResult[1])
				vPrev = atoi(pResult[1]);
			sqlite3_free_table(pResult);
		}
		sqlCmd = sqlite3_mprintf("DELETE FROM Thistory WHERE x=%d;", vPrev);
		sqlite3_exec(pCfg, sqlCmd, NULL, NULL, &szError);
		sqlite3_free(sqlCmd);
		vCount--;
	}
}

PMediaDescr Database::GetHistory(PMediaDescr pMedia)
{
	PMediaDescr pPrev;
	LPSTR  szError = NULL;
	LPSTR  sqlCmd;
	LPSTR *pResult;
	INT    vExec, vRows, vTimeout;
	INT    vPrev, vHistory, vStart = 0;

	if(pCfg == NULL)
		return(NULL);

	vPrev = (pMedia->vHistory - 1);
	if(vPrev == -1)
	{
		vTimeout = 0;
		while((vExec = sqlite3_get_table(pCfg, "SELECT MIN(x), MAX(x) FROM Thistory;", &pResult, &vRows, NULL, &szError)) == SQLITE_BUSY)
			if(vTimeout++ > 100) break;
		if(vExec == SQLITE_OK)
		{
			if(vRows == 1)
			{
				if(pResult[2])
					vStart = atoi(pResult[2]);
				if(pResult[3])
					vPrev = atoi(pResult[3]);
			}
			sqlite3_free_table(pResult);
		}
		if(szError)
			ShowError(&szError);
	}

	do
	{
		sqlCmd = sqlite3_mprintf("SELECT id FROM Thistory WHERE x=%d;", vPrev);
		vTimeout = 0;
		while((vExec = sqlite3_get_table(pCfg, sqlCmd, &pResult, &vRows, NULL, &szError)) == SQLITE_BUSY)
			if(vTimeout++ > 100) break;
		sqlite3_free(sqlCmd);
		if(vExec == SQLITE_OK)
		{
			if((vRows == 1) && pResult[1])
				vHistory = atoi(pResult[1]);
			sqlite3_free_table(pResult);
		}
		if(szError)
			ShowError(&szError);

		// Get & return descriptor for vMediaId=vHistory
		if(vHistory)
		{
			pPrev = GetMedia(vHistory);
			if(pPrev)
				pPrev->vHistory = vPrev;
			else
				vPrev--;
			if(vPrev < vStart)
				return(NULL);
		}
	} while(pPrev == NULL);
	return(pPrev);
}

void Database::GetStation(HWND hWnd)
{
	LPSTR	szError = NULL;
	LPSTR	*pResult;
	INT		i, vRows, vLen;
	INT		vExec;
	LPTSTR	pString;

	// A database and function pinter should already exist
	if((pCfg == NULL) || (hWnd == NULL))
		return;

	/* Get the nowplaying list */
	vExec = sqlite3_get_table(pCfg, "SELECT name, channel FROM Tstations;", &pResult, &vRows, NULL, &szError);
	if(vExec == SQLITE_OK)
	{
		for(i = 2; i <= (2 * vRows); i += 2)
		{
			if(pResult[i] && pResult[i+1])
			{
				vLen = (strlen(pResult[i]) + 2) * sizeof(TCHAR);
				pString = (LPTSTR)GlobalAlloc(GPTR, vLen);
				StringCbPrintf(pString, vLen, TEXT("%hs"), pResult[i]);
				SendMessage(hWnd, LB_INSERTSTRING, (WPARAM)0, (LPARAM)pString);
				GlobalFree(pString);
				SendMessage(hWnd, LB_SETITEMDATA, 0, atoi(pResult[i+1]));
			}
		}
		sqlite3_free_table(pResult);
	}
	if(szError)
		ShowError(&szError);
}

void Database::SaveStations(HWND hWnd)
{
	const CHAR delQuery[] = "DELETE FROM Tstations;";
	LPSTR	szError = NULL;
	LPSTR	sqlCmd;
	LPTSTR	pString;
	LPSTR	pText;
	INT		vIndex, vLen, vChannel;

	// A database and function pinter should already exist
	if((pCfg == NULL) || (hWnd == NULL))
		return;

	/* Create temporary (memory) table */
	sqlite3_exec(pCfg, "CREATE TEMP TABLE TstationsTmp (x INTEGER PRIMARY KEY, name VARCHAR(20), channel INTEGER);", NULL, NULL, &szError);
	if(szError)
		ShowError(&szError);

	/* Delete old list */
	sqlite3_exec(pCfg, delQuery, NULL, NULL, &szError);
	if(szError)
		ShowError(&szError);

	/* Fill temporary table with records */
	while((vIndex = SendMessage(hWnd, LB_GETCOUNT, 0, 0)))
	{
		vIndex--;
		vLen = SendDlgItemMessage(hWnd, IDC_STATIONS, LB_GETTEXTLEN, (WPARAM)vIndex, 0) + 2;
		pString = (LPTSTR)GlobalAlloc(GPTR, vLen * sizeof(TCHAR));
		pText = (LPSTR)GlobalAlloc(GPTR, vLen);
		if(pString && pText)
		{
			SendMessage(hWnd, LB_GETTEXT, (WPARAM)vIndex, (LPARAM)pString);
#ifdef UNICODE
			StringCchPrintfA(pText, vLen, "%ls", pString);
#else
			StringCchPrintfA(pText, vLen, "%hs", pString);
#endif
			GlobalFree(pString);
			vChannel = SendMessage(hWnd, LB_GETITEMDATA, (WPARAM)vIndex, 0);
			SendMessage(hWnd, LB_DELETESTRING, (WPARAM)vIndex, 0);
			sqlCmd = sqlite3_mprintf("INSERT INTO TstationsTmp (name, channel) VALUES(\"%q\", %d);", pText, vChannel);
			GlobalFree(pText);
			sqlite3_exec(pCfg, sqlCmd, NULL, NULL, &szError);
			sqlite3_free(sqlCmd);
			if(szError)
				ShowError(&szError);
		}
	}
	/* Copy temp table to config database */
	sqlite3_exec(pCfg, "INSERT INTO Tstations (name, channel) SELECT name, channel FROM TstationsTmp;", NULL, NULL, &szError);
	if(szError)
		ShowError(&szError);
}

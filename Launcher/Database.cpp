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

	pCfg = NULL;

	if(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, vPath) == S_OK)
		StringCbCatA(vPath, sizeof(vPath), "\\WhiteBream");
	else
		StringCbCopyA(vPath, sizeof(vPath), ".\\WhiteBream");
	CreateDirectoryA(vPath, NULL);

	StringCbPrintfA(vConfigPath, sizeof(vConfigPath), "%s\\Cargo.cfg", vPath);

	OpenCfg();
}

Database::~Database()
{
	CloseCfg();
}

void Database::Free(LPVOID lpMem)
{
	sqlite3_free((char*)lpMem);
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


/******************************************************************************************************************/
/******************************************************************************************************************/
/******************************************************************************************************************/
static const LPSTR szCreateCfgTables[] =
{
	"CREATE TABLE Tconfig(tag VARCHAR(8) PRIMARY KEY, value VARCHAR(4));",
	NULL
};

void Database::OpenCfg(void)
{
	FILE *f;
	BOOL  bIsNew;

	// Check the size of the database
	f = fopen(vConfigPath, "rb");
	bIsNew = (f ? FALSE : TRUE);
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
	if(bIsNew == TRUE)
		CreateCfg();
}

void Database::CloseCfg(void)
{
	if(pCfg)
		sqlite3_close(pCfg);
	pCfg = NULL;
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

	LPSTR   szError = NULL;
	LPSTR   outQuery;
	LPSTR  *pResult;
	int     vRows;
	size_t  vLen;

	// A database and function pinter should already exist
	if(pCfg == NULL)
		return(0);
	if(lpVar == NULL)
		return(0);

	outQuery = sqlite3_mprintf(inQuery, lpVar);
	sqlite3_get_table(pCfg, outQuery, &pResult, &vRows, NULL, &szError);
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
					if(*lpData == NULL)
						*lpData = malloc(2);
					vRows = 0;
					break;
			}
		}
	}
	else
	{
		if(lpData)
			if(*lpData == NULL)
				*lpData = malloc(2);
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

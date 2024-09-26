// ConfigDb.h: interface for the ConfigDb class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ConfigDb_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_ConfigDb_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\SqLite3\Sqlite3.h"
#pragma comment(lib, "..\\SqLite3\\Debug\\SqLite3.lib")


typedef int (*xCallback)(void * cbParam, int colCount, char ** colData, char ** colNames);

typedef enum _ConfigType
{
	CONFIGTYPE_NONE,
	CONFIGTYPE_BOOL = 1,
	CONFIGTYPE_INT,
	CONFIGTYPE_LONG,
	CONFIGTYPE_STRING,
	CONFIGTYPE_UNICODE
} tConfigType;


class ConfigDb
{
private:
    int		ShowError(LPSTR *szError);
	CHAR	vConfigPath[MAX_PATH];

	void CreateCfg(void);
	sqlite3  *pCfg;

public:
	ConfigDb();
	virtual ~ConfigDb();

	void Free(LPVOID lpMem);

	int  Initialize();

	void OpenCfg(void);
	void CloseCfg(void);
	void RecreateCfg(void);
	INT  GetConfig(LPSTR lpVar, LPVOID *lpData, tConfigType type);
	BOOL GetBool(LPSTR lpVar);
	INT  GetInt(LPSTR lpVar);
	INT  SetConfig(LPSTR lpVar, LPVOID lpData, tConfigType type);
};


#endif // !defined(AFX_ConfigDb_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

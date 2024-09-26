// Database.h: interface for the Database class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATABASE_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_DATABASE_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

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

/* Version 2.0 */
typedef struct _MediaDescr
{
	INT		vMediaId;
	BOOL	vInvalid;
	INT		vTitleId;
	INT		vArtistId;
	INT		vAlbumId;
	INT		vTrack;
	INT		vYearId;
	INT		vGenreId;
	INT		vTypeId;
	INT		vFileId;
	INT		vFolderId;
	INT		vRootId;
	UINT	vPlayTime;
	INT		vRating;
	INT		vAux;
	INT		vHistory;
	BOOL	vNoRemove;
} MediaDescr, * PMediaDescr;

typedef enum _FileType
{
	FT_NONE,
	FT_MP3,
	FT_VIDEO
} FileType;

typedef enum _MediaRating
{
	RATE_DECR = -5,
	RATE_REMOVE = -3,
	RATE_REMOVEALL = -2,
	RATE_NEXT = -1,
	RATE_NEUTRAL = 0,
	RATE_PLAYALBUM = +1,
	RATE_PLAYQUEUE = +2,
	RATE_PLAYNEXT = +3,
	RATE_PLAYNOW = +4,
	RATE_INCR = +5,
	RATE_MANUAL = +10,
} MediaRating;

#define FIELD_MASK      0x03FF
#define DISPLAY_MASK    0x3000
#define DISPLAY_FULL	0x0000
#define DISPLAY_ITEM	0x1000
#define DISPLAY_EXCL	0x2000
#define DISPLAY_INSERT	0x4000

typedef enum _MediaField
{
	FLD_TITLE = 1,
	FLD_ARTIST = 2,
	FLD_ALBUM = 4,
	FLD_TRACK = 8,
	FLD_YEAR = 16,
	FLD_GENRE = 32,
	FLD_FILE = 64,
	FLD_ROOT = 128,

	FLD_PLAYTIME = 256,
	FLD_RATING = 512
} MediaField;

class Database
{
private:
    int		ShowError(LPSTR *szError);
	CHAR	vMediaPath[MAX_PATH];
	CHAR	vConfigPath[MAX_PATH];
	CHAR	vGlobalsPath[MAX_PATH];

//	sqlite3 *pDB;

	/* Version 2.0 */
	void CreateMdb(void);

	INT GetCreateId(sqlite3 * pDb, LPCSTR pTable, LPCSTR pTextVal);
	LPVOID ParamStruct(INT vObjs);

	void CreateCfg(void);
	/* Version 2.0 */
	sqlite3  *pCfg;
	sqlite3  *pCfgGlob;
	sqlite3  *pUsr;
	sqlite3  *pSys;

public:
	Database();
	virtual ~Database();

	CHAR	vMediaDbRunning;

	void Free(LPVOID lpMem);

	int  Initialize();
	void Recreate();

	void OpenMdb(void);
	void CloseMdb(void);
	PMediaDescr CopyDescr(PMediaDescr pMedia);

	LPTSTR DescriptorToFullString(PMediaDescr pData);
	LPTSTR DescriptorToSingleString(PMediaDescr pSelect, MediaField vLabel);
	LPTSTR DescriptorToDoubleString(PMediaDescr pSelect, MediaField vLabel);
	void   QueryToListbox(PMediaDescr pSelect, HWND hWnd);
	void   MediaDbQuery(LPTSTR pSearch, HWND hWnd);
	BOOL   MediaDbStop(void);
	void   TableToListbox(PMediaDescr pSelect, HWND hWnd);
	void   GetAutoPlay(BOOL doShuffle, LPLONG lastIdx, HWND hWnd);
	void   AdjustRating(PMediaDescr pSelect, MediaRating vRating);
	INT    GetStringValue(PMediaDescr pSelect, MediaField vField, INT vMaxLen, LPTSTR * pStr);
	INT    FindString(MediaField vField, LPSTR pName);
	PMediaDescr GetMedia(INT vMediaId);
	BOOL   AddRemoveRootEntry(BOOL vAdd, LPTSTR pRoot);
	void   CreateGenres(void);
	INT    AddAlbum(LPSTR szArtist, LPSTR szAlbum, LPSTR szTitle, INT nGenre, INT nYear, LPSTR szFile, INT vTrack, UINT vId, FileType vFt);
	INT    ChangeTrack(PMediaDescr pSelect);
	void   KillTrack(PMediaDescr pSelect);
	void   GarbageCollection(void);

	void OpenCfg(void);
	void CloseCfg(void);
	void RecreateCfg(void);
	INT  GetConfig(LPSTR lpVar, LPVOID *lpData, tConfigType type);
	BOOL GetBool(LPSTR lpVar);
	INT  GetInt(LPSTR lpVar);
	INT  SetConfig(LPSTR lpVar, LPVOID lpData, tConfigType type);

	void GetNowPlaying(HWND hWnd);
	void SaveNowPlaying(PMediaDescr *pMedia);
	void SaveHistory(PMediaDescr pMedia);
	PMediaDescr GetHistory(PMediaDescr pMedia);
	void GetStation(HWND hWnd);
	void SaveStations(HWND hWnd);
};


#endif // !defined(AFX_DATABASE_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

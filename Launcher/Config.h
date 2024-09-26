// Config.h: implementation of the Configuration system.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

typedef enum _MENUTYPE
{
	MENU_EXE = 1,
	MENU_MAN,
	MENU_INT
} MENUTYPE;

typedef struct _menu
{
	BOOL		tValid;
	TCHAR		menuName[21];
	MENUTYPE	menuType;
	BOOL		autoStart;
	TCHAR		exeName[MAX_PATH];
	BOOL		setSize;
	BOOL		minimize;
} tMenu;

typedef enum _INFVALUES
{
	BUTTONHEIGHT = 1,
	STARTBUTTONWIDTH,
	MENUBUTTONWIDTH,
	MENUCOLUMS,
	MENUROWS,
	ICONNAME,
	BACKGROUNDFILE,
	SETSCREENSIZE,
	SCREENSIZE
} tInfVal;

extern tMenu sMenu[];

INT ConfigRead(LPTSTR lpIniFile);
void ConfigMakeFirst(LPTSTR iniFile);
INT ConfigGetInt(tInfVal infVal, INT * pData);
INT ConfigGetString(tInfVal infVal, LPTSTR pData, DWORD maxLength);
INT ConfigSetInt(tInfVal infVal, INT vData);

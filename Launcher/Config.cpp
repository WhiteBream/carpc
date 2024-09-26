// Config.cpp: implementation of the Configuration system.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Config.h"
#include "Setupapi.h"
#include "stdio.h"
#include <shlobj.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

#define INIFILE		TEXT("\\Cargo.ini")


#define MAXROWS		8
#define MAXCOLS		3

tMenu	sMenu[MAXROWS * MAXCOLS] = {{FALSE}};
TCHAR	pFile[MAX_PATH];

struct _config
{
	INT		buttonHeight;
	INT		startButtonWidth;
	INT		menuButtonWidth;
	INT		menuRows;
	INT		menuColumns;
	TCHAR	iconFile[MAX_PATH];
	TCHAR	bitmapFile[MAX_PATH];
} vConfig;

INT ConfigRead(LPTSTR lpIniFile)
{
	TCHAR	retKeyString[40] = {0};
	TCHAR	menuKey[20];
	INT		menuCnt;

	if(lpIniFile)
		StringCbCopy(pFile, sizeof(pFile), lpIniFile);
	else
	{
		if(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, pFile) == S_OK)
			StringCbCat(pFile, sizeof(pFile), TEXT("\\WhiteBream"));
		else
			StringCbCopy(pFile, sizeof(pFile), TEXT(".\\WhiteBream"));
		CreateDirectory(pFile, NULL);

		StringCbCat(pFile, sizeof(pFile), INIFILE);
	}

	GetPrivateProfileString(TEXT("White Bream"), TEXT("Application"), TEXT(""), retKeyString, sizeof(retKeyString)-1, pFile);
	if(StrCmp(retKeyString, TEXT("Cargo")) != 0)
		ConfigMakeFirst(pFile);

	GetPrivateProfileString(TEXT("Cargo"), TEXT("StartButtonIcon"), TEXT(""), vConfig.iconFile, sizeof(vConfig.iconFile), INIFILE);
	GetPrivateProfileString(TEXT("Desktop"), TEXT("Background"), TEXT(""), vConfig.bitmapFile, sizeof(vConfig.bitmapFile), INIFILE);

	vConfig.buttonHeight     = GetPrivateProfileInt(TEXT("Cargo"), TEXT("ButtonHeight")    , 60, pFile);
	vConfig.startButtonWidth = GetPrivateProfileInt(TEXT("Cargo"), TEXT("StartButtonWidth"), 75, pFile);
	vConfig.menuButtonWidth  = GetPrivateProfileInt(TEXT("Cargo"), TEXT("MenuButtonWidth") , 200, pFile);
	vConfig.menuRows         = GetPrivateProfileInt(TEXT("Cargo"), TEXT("MenuRows")        , 6, pFile);
	vConfig.menuColumns      = GetPrivateProfileInt(TEXT("Cargo"), TEXT("MenuColumns")     , 2, pFile);

	if(vConfig.menuRows > MAXROWS) vConfig.menuRows = MAXROWS;
	if(vConfig.menuColumns > MAXCOLS) vConfig.menuColumns = MAXCOLS;

	menuCnt = vConfig.menuRows * vConfig.menuColumns;
	for(; menuCnt > 0; menuCnt --)
	{
	   StringCchPrintf(menuKey, sizeof(menuKey), TEXT("menu #%d"), menuCnt);
	   GetPrivateProfileString(menuKey, TEXT("MenuName"), TEXT(""), sMenu[menuCnt - 1].menuName, sizeof(sMenu[menuCnt - 1].menuName)-1, pFile);
	   GetPrivateProfileString(menuKey, TEXT("ExeName"), TEXT(""), sMenu[menuCnt - 1].exeName, sizeof(sMenu[menuCnt - 1].exeName)-1, pFile);

	   GetPrivateProfileString(menuKey, TEXT("MenuType"), TEXT("EXE"), retKeyString, 20, pFile);
	   if(StrCmp(retKeyString, TEXT("EXE")) == 0)
		   sMenu[menuCnt - 1].menuType = MENU_EXE;
	   if(StrCmp(retKeyString, TEXT("MAN")) == 0)
		   sMenu[menuCnt - 1].menuType = MENU_MAN;
	   if(StrCmp(retKeyString, TEXT("INT")) == 0)
		   sMenu[menuCnt - 1].menuType = MENU_INT;

	   GetPrivateProfileString(menuKey, TEXT("AutoStart"), TEXT("N"), retKeyString, 5, pFile);
		sMenu[menuCnt - 1].autoStart = FALSE;
	   if(StrCmpN(retKeyString, TEXT("Y"), 1) == 0)
		   sMenu[menuCnt - 1].autoStart = TRUE;

	   GetPrivateProfileString(menuKey, TEXT("SetSize"), TEXT("N"), retKeyString, 5, pFile);
	   sMenu[menuCnt - 1].setSize = FALSE;
	   if(StrCmpN(retKeyString, TEXT("Y"), 1) == 0)
		   sMenu[menuCnt - 1].setSize = TRUE;

	   GetPrivateProfileString(menuKey, TEXT("Minimize"), TEXT("N"), retKeyString, 5, pFile);
	   sMenu[menuCnt - 1].minimize = FALSE;
	   if(StrCmpN(retKeyString, TEXT("Y"), 1) == 0)
		   sMenu[menuCnt - 1].minimize = TRUE;

	   sMenu[menuCnt - 1].tValid = FALSE;
	}

	return(0);
}

void ConfigMakeFirst(LPTSTR iniFile)
{
	FILE	*iniFirst;
#ifdef _DEBUG
	TCHAR	vMsgText[100];
#endif

	iniFirst = _tfopen(iniFile, TEXT("w"));
	if(iniFirst != NULL)
	{
		_ftprintf(iniFirst, TEXT("; configuration file for the Cargo application\n"));

		_ftprintf(iniFirst, TEXT("\n[White Bream]\n"));
		_ftprintf(iniFirst, TEXT("Application      = Cargo\n"));

		_ftprintf(iniFirst, TEXT("\n[Cargo]\n"));
		_ftprintf(iniFirst, TEXT("ButtonHeight     = 60\n"));
		_ftprintf(iniFirst, TEXT("StartButtonWidth = 75\n"));
		_ftprintf(iniFirst, TEXT("MenuButtonWidth  = 200\n"));
		_ftprintf(iniFirst, TEXT("MenuRows         = 6\n"));
		_ftprintf(iniFirst, TEXT("MenuColumns      = 2\n"));
		_ftprintf(iniFirst, TEXT("StartButtonIcon  = startglass.ico\n"));

		_ftprintf(iniFirst, TEXT("\n[Desktop]\n"));
		_ftprintf(iniFirst, TEXT("Background       = leon.jpg\n"));

		_ftprintf(iniFirst, TEXT("\n;Menu entries have the following form;\n"));
		_ftprintf(iniFirst, TEXT(";[menu #n] (eg [menu #1], [menu #10])\n"));
		_ftprintf(iniFirst, TEXT(";MenuName  = Text that appears in menu\n"));
		_ftprintf(iniFirst, TEXT(";MenuType  = EXE, MAN or INT\n"));
		_ftprintf(iniFirst, TEXT(";AutoStart = Y/N\n"));
		_ftprintf(iniFirst, TEXT(";ExeName   = Name and path of executable\n"));

		_ftprintf(iniFirst, TEXT("\n[menu #1]\n"));
		_ftprintf(iniFirst, TEXT("MenuName  = Destinator\n"));
		_ftprintf(iniFirst, TEXT("MenuType  = EXE\n"));
		_ftprintf(iniFirst, TEXT("AutoStart = No\n"));
		_ftprintf(iniFirst, TEXT("ExeName   = StartDest.exe\n"));
		_ftprintf(iniFirst, TEXT("SetSize   = Yes\n"));

		_ftprintf(iniFirst, TEXT("\n[menu #2]\n"));
		_ftprintf(iniFirst, TEXT("MenuName  = Engine\n"));
		_ftprintf(iniFirst, TEXT("MenuType  = EXE\n"));
		_ftprintf(iniFirst, TEXT("AutoStart = Yes\n"));
		_ftprintf(iniFirst, TEXT("ExeName   = \n"));
		_ftprintf(iniFirst, TEXT("SetSize   = No\n"));

		_ftprintf(iniFirst, TEXT("\n[menu #3]\n"));
		_ftprintf(iniFirst, TEXT("MenuName  = Music\n"));
		_ftprintf(iniFirst, TEXT("MenuType  = EXE\n"));
		_ftprintf(iniFirst, TEXT("AutoStart = Yes\n"));
		_ftprintf(iniFirst, TEXT("ExeName   = Player.exe\n"));
		_ftprintf(iniFirst, TEXT("SetSize   = Yes\n"));

		_ftprintf(iniFirst, TEXT("\n[menu #4]\n"));
		_ftprintf(iniFirst, TEXT("MenuName  = Telephone\n"));
		_ftprintf(iniFirst, TEXT("MenuType  = EXE\n"));
		_ftprintf(iniFirst, TEXT("ExeName   = \n"));

		_ftprintf(iniFirst, TEXT("\n[menu #5]\n"));
		_ftprintf(iniFirst, TEXT("MenuName  = Photos\n"));
		_ftprintf(iniFirst, TEXT("MenuType  = EXE\n"));
		_ftprintf(iniFirst, TEXT("ExeName   = Imager.exe\n"));
		_ftprintf(iniFirst, TEXT("SetSize   = Yes\n"));

		_ftprintf(iniFirst, TEXT("\n[menu #6]\n"));
		_ftprintf(iniFirst, TEXT("MenuName  = Rear Video\n"));
		_ftprintf(iniFirst, TEXT("MenuType  = EXE\n"));
		_ftprintf(iniFirst, TEXT("ExeName   = Viewer.exe\n"));
		_ftprintf(iniFirst, TEXT("SetSize   = Yes\n"));
		_ftprintf(iniFirst, TEXT("Minimize  = Yes\n"));

		_ftprintf(iniFirst, TEXT("\n[menu #7]\n"));
		_ftprintf(iniFirst, TEXT("MenuName  = Patience\n"));
		_ftprintf(iniFirst, TEXT("MenuType  = EXE\n"));
		_ftprintf(iniFirst, TEXT("AutoStart = No\n"));
		_ftprintf(iniFirst, TEXT("ExeName   = sol.exe\n"));
		_ftprintf(iniFirst, TEXT("SetSize   = Yes\n"));

		_ftprintf(iniFirst, TEXT("\n[menu #8]\n"));
		_ftprintf(iniFirst, TEXT("MenuName  = Taskman\n"));
		_ftprintf(iniFirst, TEXT("MenuType  = MAN\n"));
		_ftprintf(iniFirst, TEXT("ExeName   = taskmgr.exe\n"));

		_ftprintf(iniFirst, TEXT("\n[menu #9]\n"));
		_ftprintf(iniFirst, TEXT("MenuName  = Sound\n"));
		_ftprintf(iniFirst, TEXT("MenuType  = EXE\n"));
		_ftprintf(iniFirst, TEXT("AutoStart = Yes\n"));
		_ftprintf(iniFirst, TEXT("ExeName   = Mixer.exe\n"));
		_ftprintf(iniFirst, TEXT("SetSize   = Yes\n"));
		_ftprintf(iniFirst, TEXT("Minimize  = Yes\n"));

		_ftprintf(iniFirst, TEXT("\n[menu #10]\n"));
		_ftprintf(iniFirst, TEXT("MenuName  = Settings\n"));
		_ftprintf(iniFirst, TEXT("MenuType  = INT\n"));
		_ftprintf(iniFirst, TEXT("ExeName   = SETTINGS\n"));

		_ftprintf(iniFirst, TEXT("\n[menu #11]\n"));
		_ftprintf(iniFirst, TEXT("MenuName  = Show desktop\n"));
		_ftprintf(iniFirst, TEXT("MenuType  = INT\n"));
		_ftprintf(iniFirst, TEXT("ExeName   = DESKTOP\n"));

		_ftprintf(iniFirst, TEXT("\n[menu #12]\n"));
		_ftprintf(iniFirst, TEXT("MenuName  = Shutdown\n"));
		_ftprintf(iniFirst, TEXT("MenuType  = INT\n"));
		_ftprintf(iniFirst, TEXT("ExeName   = SHUTDOWN\n"));

		fclose(iniFirst);
#ifdef _DEBUG
		StringCbPrintf(vMsgText, sizeof(vMsgText), TEXT("Default configuration written to \n%s"), iniFile);
		MessageBox(NULL, vMsgText, TEXT("Info"), MB_OK | MB_ICONINFORMATION);
#endif
	}
#ifdef _DEBUG
	else
	{
		StringCbPrintf(vMsgText, sizeof(vMsgText), TEXT("Cannot create default configuration \n%s"), iniFile);
		MessageBox(NULL, vMsgText, TEXT("Error"), MB_OK | MB_ICONERROR);
	}
#endif
}

INT ConfigGetInt(tInfVal infVal, INT *pData)
{
	if(!pData)
		return(-1);

	switch(infVal)
	{
		case BUTTONHEIGHT:
			*pData = vConfig.buttonHeight;
			break;

		case STARTBUTTONWIDTH:
			*pData = vConfig.startButtonWidth;
			break;

		case MENUBUTTONWIDTH:
			*pData = vConfig.menuButtonWidth;
			break;

		case MENUROWS:
			*pData = vConfig.menuRows;
			break;

		case MENUCOLUMS:
			*pData = vConfig.menuColumns;
			break;

		case SETSCREENSIZE:
			*pData = GetPrivateProfileInt(TEXT("Desktop"), TEXT("SetScreenSize"), 0, pFile);
			break;

		case SCREENSIZE:
			*pData = GetPrivateProfileInt(TEXT("Desktop"), TEXT("ScreenSize"), 0, pFile);
			break;

		default:
			return(1);
	}
	return(0);
}

INT ConfigSetInt(tInfVal infVal, INT vData)
{
	TCHAR vValue[20];
	StringCbPrintf(vValue, sizeof(vValue), TEXT("%d"), vData);

	switch(infVal)
	{
		case SETSCREENSIZE:
			WritePrivateProfileString(TEXT("Desktop"), TEXT("SetScreenSize"), vValue, pFile);
			break;
		case SCREENSIZE:
			WritePrivateProfileString(TEXT("Desktop"), TEXT("ScreenSize"), vValue, pFile);
			break;
		default:
			return(0);
	}
	return(1);
}

INT ConfigGetString(tInfVal infVal, LPTSTR pData, DWORD maxLength)
{
	switch(infVal)
	{
		case ICONNAME:
			StringCbCopy(pData, maxLength, vConfig.iconFile);
			break;

		case BACKGROUNDFILE:
			StringCbCopy(pData, maxLength, vConfig.bitmapFile);
			break;

		default:
			return(1);
	}
	return(0);
}

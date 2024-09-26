// startbtn1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Config.h"
#include "Shutdown.h"
#include "Settings.h"

// Global Variables:
extern LPTSTR	pAppName;


#define HKEY_USERNAME	TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer")
#define HKEY_WINLOGON	TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define HKEY_EXPLPROC	TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced")
#define HKEY_DESKTOP	TEXT("Control Panel\\Desktop")
#define HKEY_SCHEMES	TEXT("AppEvents\\Schemes")
#define HKEY_SCHEMEAPPS	TEXT("AppEvents\\Schemes\\Apps")

void TestKeyEnum(HKEY hKey)
{
	DWORD vIndex, vType;
	DWORD vKeyLen, vValueLen;
	TCHAR vKey[256];
	TCHAR vValue[256];
	TCHAR vDebug[512];
	LONG retVal = ERROR_SUCCESS;

	RegQueryInfoKey(hKey, NULL, NULL, NULL, &vKeyLen, NULL, NULL, &vValueLen, NULL, NULL, NULL, NULL);
	StringCbPrintf(vDebug, sizeof(vDebug), TEXT("%d subkeys, %d values"), vKeyLen, vValueLen);
	if(MessageBox(NULL, vDebug, TEXT("Reg debug"), MB_OKCANCEL) == IDCANCEL)
		return;

	for(vIndex = 0; retVal != ERROR_NO_MORE_ITEMS; vIndex++)
	{
		vKeyLen = sizeof(vKey);
		vValueLen = sizeof(vValue);
		retVal = RegEnumValue(hKey, vIndex, vKey, &vKeyLen, NULL, &vType, (LPBYTE)vValue, &vValueLen);
		if(retVal == ERROR_SUCCESS)
		{
			switch(vType)
			{
				case REG_SZ:
					StringCbPrintf(vDebug, sizeof(vDebug), TEXT("Index %d: Name \"%s\" Value \"%s\""), vIndex, vKey, vValue);
					break;
				default:
					StringCbPrintf(vDebug, sizeof(vDebug), TEXT("Index %d: Name \"%s\" Value \"%d\""), vIndex, vKey, vValue);
					break;
			}
		}
		else
			StringCbPrintf(vDebug, sizeof(vDebug), TEXT("RegEnumValue failed at Index %d"), vIndex);
		if(MessageBox(NULL, vDebug, TEXT("Reg debug"), MB_OKCANCEL) == IDCANCEL)
			return;
	}
}

LONG RegReadValue(HKEY hKey, LPCTSTR vKeyName, LPTSTR vData, PLONG vDataLen)
{
	DWORD vIndex;
	DWORD vKeyLen, vValueLen;
	TCHAR vKey[256];
	LONG retVal = ERROR_SUCCESS;

	for(vIndex = 0; retVal != ERROR_NO_MORE_ITEMS; vIndex++)
	{
		vKeyLen = sizeof(vKey);
		vValueLen = *vDataLen;
		if((retVal = RegEnumValue(hKey, vIndex, vKey, &vKeyLen, NULL, NULL, (LPBYTE)vData, &vValueLen)) == ERROR_SUCCESS)
		{
			if(_tcscmp(vKey, vKeyName) == 0)
			{
				*vDataLen = vValueLen;
				return(ERROR_SUCCESS);
			}
		}
	}
	return(1L);
}

BOOL GetWindowsShell(void)
{
	HKEY	hReg = NULL;
	LONG	vLen;
	TCHAR	vReadString[100] = {0};
	BOOL	retVal = FALSE;
	LONG error;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_WINLOGON, 0, KEY_READ, &hReg) == ERROR_SUCCESS)
	{
		vLen = sizeof(vReadString);
		if((error = RegReadValue(hReg, TEXT("Shell"), vReadString, &vLen)) == ERROR_SUCCESS)
		{
			if(_tcscmp(vReadString, pAppName) == 0)
				retVal = TRUE;
		}
		RegCloseKey(hReg);
	}
	return(retVal);
}

BOOL SetWindowsShell(BOOL vYes)
{
	HKEY	hReg = NULL;
	BOOL	retVal = FALSE;
	size_t	vLen;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_WINLOGON, 0, KEY_WRITE, &hReg) == ERROR_SUCCESS)
	{
		if(vYes)
		{
			StringCchLength(pAppName, MAX_PATH, &vLen);
			vLen = (vLen + 1) * sizeof(TCHAR);
			if(RegSetValueEx(hReg, TEXT("Shell"), 0, REG_SZ, (BYTE*)pAppName, vLen) != ERROR_SUCCESS)
				MessageBox(NULL, TEXT("Could not set StartMenu.exe as the default windows shell..."), TEXT("CARGO Menu"), MB_OK | MB_ICONEXCLAMATION);
			else
				retVal = TRUE;
		}
		else
		{
			if(RegDeleteValue(hReg, TEXT("Shell")) != ERROR_SUCCESS)
				MessageBox(NULL, TEXT("Could not reset the default windows shell..."), TEXT("CARGO Menu"), MB_OK | MB_ICONEXCLAMATION);
			else
				retVal = TRUE;
		}
		RegCloseKey(hReg);
	}
	if(RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_EXPLPROC, 0, KEY_WRITE, &hReg) == ERROR_SUCCESS)
	{
		if(vYes)
			RegSetValueEx(hReg, TEXT("SeperateProcess"), 0, REG_SZ, (BYTE*)TEXT("1"), 2 * sizeof(TCHAR));
		else
			RegSetValueEx(hReg, TEXT("SeperateProcess"), 0, REG_SZ, (BYTE*)TEXT("0"), 2 * sizeof(TCHAR));
		RegCloseKey(hReg);
	}
	return(retVal);
}

void SetAutoLogon(BOOL vYes)
{
	HKEY	hReg = NULL;
	DWORD	vLen;
	TCHAR	vReadString[100] = {0};

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, HKEY_WINLOGON, 0, KEY_WRITE, &hReg) == ERROR_SUCCESS)
	{
		if(vYes)
		{
			vLen = sizeof(vReadString);
			GetUserName(vReadString, &vLen);
			StringCbLength(vReadString, sizeof(vReadString), (size_t*)&vLen);
			vLen += sizeof(TCHAR);

			if(RegSetValueEx(hReg, TEXT("DefaultUserName"), 0, REG_SZ, (BYTE*)vReadString, vLen) != ERROR_SUCCESS)
				MessageBox(NULL, TEXT("Could not set this useraccount as the default logon..."), TEXT("CARGO Menu"), MB_OK | MB_ICONEXCLAMATION);
			else
				RegSetValueEx(hReg, TEXT("AutoAdminLogon"), 0, REG_SZ, (BYTE*)TEXT("1"), 2 * sizeof(TCHAR));
		}
		else
		{
			RegSetValueEx(hReg, TEXT("AutoAdminLogon"), 0, REG_SZ, (BYTE*)TEXT("0"), 2 * sizeof(TCHAR));
			if(RegDeleteValue(hReg, TEXT("DefaultUserName")) != ERROR_SUCCESS)
				MessageBox(NULL, TEXT("Could not reset the default windows shell..."), TEXT("CARGO Menu"), MB_OK | MB_ICONEXCLAMATION);
		}
		RegCloseKey(hReg);
	}
	else
	{
		MessageBox(NULL, TEXT("Access denied\n\nCould not set this useraccount as the default logon..."), TEXT("CARGO Menu"), MB_OK | MB_ICONEXCLAMATION);
	}
}

BOOL ScreensaverActive(void)
{
	HKEY	hReg = NULL;
	LONG	vLen;
	TCHAR	vReadString[100] = {0};
	BOOL	retVal = FALSE;
	LONG error;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_DESKTOP, 0, KEY_READ, &hReg) == ERROR_SUCCESS)
	{
		vLen = sizeof(vReadString);
		if((error = RegReadValue(hReg, TEXT("ScreenSaveActive"), vReadString, &vLen)) == ERROR_SUCCESS)
		{
			if(_tcscmp(vReadString, TEXT("1")) == 0)
				retVal = TRUE;
		}
		RegCloseKey(hReg);
	}
	return(retVal);
}

BOOL DisableScreensaver(void)
{
	HKEY	hReg = NULL;
	BOOL	retVal = FALSE;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_DESKTOP, 0, KEY_WRITE, &hReg) == ERROR_SUCCESS)
	{
		if(RegSetValueEx(hReg, TEXT("ScreenSaveActive"), 0, REG_SZ, (BYTE*)TEXT("0"), 2 * sizeof(TCHAR)) != ERROR_SUCCESS)
			MessageBox(NULL, TEXT("Could not disable the Windows screensaver..."), TEXT("CARGO Menu"), MB_OK | MB_ICONEXCLAMATION);
		else
			retVal = TRUE;
		RegCloseKey(hReg);
	}
	return(retVal);
}

void SecureSystemShutdown(void)
{
	HKEY	hReg = NULL;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_DESKTOP, 0, KEY_WRITE, &hReg) == ERROR_SUCCESS)
	{
		RegSetValueEx(hReg, TEXT("AutoEndTasks"), 0, REG_SZ, (BYTE*)TEXT("1"), 2 * sizeof(TCHAR));
		RegSetValueEx(hReg, TEXT("WaitToKillAppTimeout"), 0, REG_SZ, (BYTE*)TEXT("5000"), 5 * sizeof(TCHAR));
		RegCloseKey(hReg);
	}
}

BOOL WindowsSoundsActive(void)
{
	HKEY	hReg = NULL;
	LONG	vLen;
	TCHAR	vReadString[100] = {0};
	BOOL	retVal = FALSE;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_SCHEMES, 0, KEY_READ, &hReg) == ERROR_SUCCESS)
	{
		vLen = sizeof(vReadString);
		if(RegQueryValue(hReg, NULL, vReadString, &vLen) == ERROR_SUCCESS)
		{
			if(_tcscmp(vReadString, TEXT(".None")))
				retVal = TRUE;
		}
		else
			MessageBox(NULL, TEXT("Could not read soundscheme entry"), TEXT("CARGO Menu"), MB_OK | MB_ICONEXCLAMATION);
		RegCloseKey(hReg);
	}
	return(retVal);
}

BOOL DisableWindowsSounds(void)
{
	HKEY	hReg = NULL;
	BOOL	retVal = FALSE;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_SCHEMES, 0, KEY_WRITE, &hReg) == ERROR_SUCCESS)
	{
		if(RegSetValueEx(hReg, NULL, 0, REG_SZ, (BYTE*)TEXT(".None"), 6 * sizeof(TCHAR)) != ERROR_SUCCESS)
			MessageBox(NULL, TEXT("Could not set the Windows soundscheme..."), TEXT("CARGO Menu"), MB_OK | MB_ICONEXCLAMATION);
		else
			retVal = TRUE;
		RegCloseKey(hReg);
	}
	if(RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_SCHEMEAPPS, 0, KEY_ALL_ACCESS, &hReg) == ERROR_SUCCESS)
	{
		LONG	retVal1 = ERROR_SUCCESS;
		DWORD	vIndex1;

		for(vIndex1 = 0; retVal1 != ERROR_NO_MORE_ITEMS; vIndex1++)
		{
			TCHAR	vKey[256];
			DWORD	vKeyLen = sizeof(vKey);

			//Traverse HKCU\AppEvents\Schemes\Apps
			if((retVal1 = RegEnumKeyEx(hReg, vIndex1, vKey, &vKeyLen, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
			{
				HKEY	hKey1;

				if(RegOpenKeyEx(hReg, vKey, 0, KEY_ALL_ACCESS, &hKey1) == ERROR_SUCCESS)
				{
					LONG	retVal2 = ERROR_SUCCESS;
					DWORD	vIndex2;

					for(vIndex2 = 0; retVal2 != ERROR_NO_MORE_ITEMS; vIndex2++)
					{
						//Traverse HKCU\AppEvents\Schemes\Apps\$App
						vKeyLen = sizeof(vKey);
						if((retVal2 = RegEnumKeyEx(hKey1, vIndex2, vKey, &vKeyLen, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
						{
							TCHAR	vKeyName[256];
							HKEY	hKey2;

							//Open HKCU\AppEvents\Schemes\Apps\$App\$Sound\.Current
							StringCbPrintf(vKeyName, sizeof(vKeyName), TEXT("%s\\.Current"), vKey);
							if(RegOpenKeyEx(hKey1, vKeyName, 0, KEY_WRITE, &hKey2) == ERROR_SUCCESS)
							{
								RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE*)TEXT(""), sizeof(TCHAR));
								RegCloseKey(hKey2);
							}
						}
						else if(retVal2 == ERROR_ACCESS_DENIED)
							retVal2 = ERROR_NO_MORE_ITEMS;
					}
					RegCloseKey(hKey1);
				}
			}
			else if(retVal1 == ERROR_ACCESS_DENIED)
				retVal1 = ERROR_NO_MORE_ITEMS;
		}
		RegCloseKey(hReg);
	}
	return(retVal);
}

extern void SetPower(void);

BOOL CheckXPIntegration(void)
{
	DWORD	vLen;
	TCHAR	vReadString[100] = {0};
	int		vRet;

	// Read logged-on username
	vLen = sizeof(vReadString);
	GetUserName(vReadString, &vLen);
	CharUpper(vReadString);
	if(_tcscmp(vReadString, TEXT("CARGO")) && _tcscmp(vReadString, TEXT("UNIGO")) && _tcscmp(vReadString, TEXT("MERGO")))
		return(FALSE);

	if(GetWindowsShell())
		return(FALSE);

	SetPower();

	if(MessageBox(NULL, TEXT("Shall we set the CARGO Menu System as the default for this user account?"), TEXT("CARGO Menu"), MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		SetWindowsShell(TRUE);

	//	if(MessageBox(NULL, TEXT("Do you want this account to be the default on startup?\n\n(This only works if this username is not protected with a password)"), TEXT("CARGO Menu"), MB_YESNO | MB_ICONQUESTION) == IDYES)
	//		SetAutoLogon(TRUE);

		if(ScreensaverActive())
			if(MessageBox(NULL, TEXT("Disable Windows screensaver?"), TEXT("CARGO Menu"), MB_YESNO | MB_ICONQUESTION) == IDYES)
				DisableScreensaver();

		if(WindowsSoundsActive())
			if(MessageBox(NULL, TEXT("Disable Windows system sounds?"), TEXT("CARGO Menu"), MB_YESNO | MB_ICONQUESTION) == IDYES)
				DisableWindowsSounds();

		SecureSystemShutdown();

		if((vRet = MessageBox(NULL, TEXT("Shall we use a low-resolution displaymode for running Cargo?\n\n")
									TEXT("Yes: use 640 x 480 pixels\n")
									TEXT("No:  use 800 x 600 pixels\n")
									TEXT("Cancel: keep the current resolution\n\n")
									TEXT("(You can change this later in the settings window)"), TEXT("CARGO Menu"), MB_YESNOCANCEL | MB_ICONQUESTION)) != IDCANCEL)
		{
			if(vRet == IDOK)
				ConfigSetInt(SCREENSIZE, 0);
			else
				ConfigSetInt(SCREENSIZE, 1);
			ConfigSetInt(SETSCREENSIZE, 1);
		}

		if(MessageBox(NULL, TEXT("Shall we logoff to activate these changes?"), TEXT("CARGO Menu"), MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
		{
			ExitWindowsEx(EWX_LOGOFF | EWX_FORCEIFHUNG, 0);
			return(TRUE);
		}
	}
	return(FALSE);
}

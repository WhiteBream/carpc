// AddPowerScheme.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <tchar.h>
#include <shlwapi.h>
#include <strsafe.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "Powrprof.h"
#ifdef __cplusplus
}
#endif

#pragma comment(lib, "Powrprof.lib")


#define CARGOSCHEMENAME		TEXT("Cargo, Always On")
#define CARGOSCHEMEDESCR	TEXT("Power scheme for Cargo car computer system. Keep monitor always on and never enter standby mode")

typedef struct _SchemeInfo
{
	BYTE	vSize;
	UINT	vCargoScheme;
	UINT	vLastScheme;
} SchemeInfo;

#define HKEY_POWER1A	TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\PowerCfg\\GlobalPowerPolicy")
#define HKEY_POWER1B	TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\PowerCfg\\PowerPolicies")
#define HKEY_POWER2A	TEXT(".DEFAULT\\Control Panel\\PowerCfg\\GlobalPowerPolicy")
#define HKEY_POWER2B	TEXT(".DEFAULT\\Control Panel\\PowerCfg\\PowerPolicies")


BOOLEAN CALLBACK PowerSchemeCallback(UINT uiIndex, DWORD dwName, LPTSTR sName, DWORD dwDesc, LPTSTR sDesc, PPOWER_POLICY pp, LPARAM lParam)
{
	SchemeInfo *pInfo = (SchemeInfo*)lParam;

	if(!pInfo)
		return(FALSE);
	if(pInfo->vSize != sizeof(SchemeInfo))
		return(FALSE);

	if(StrCmp(sName, CARGOSCHEMENAME) == 0)
		pInfo->vCargoScheme = uiIndex;

	pInfo->vLastScheme = uiIndex;
	return(TRUE);
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	TCHAR				error[100];
	SchemeInfo			vInfo;
	POWER_POLICY		vPowerPolicy;
	HKEY				hReg, hSubKey;
	SECURITY_DESCRIPTOR	vSec;
	LONG				retVal = ERROR_SUCCESS;
	DWORD				vIndex, vKeyLen;
	TCHAR				vKey[10];

	vInfo.vSize = sizeof(SchemeInfo);
	vInfo.vCargoScheme = (UINT)-1;
	vInfo.vLastScheme = 0;

	if(EnumPwrSchemes(PowerSchemeCallback, (LPARAM)&vInfo) == 0)
	{
		StringCbPrintf(error, sizeof(error), TEXT("Create powerscheme failed\n\nExitcode 0x%X"), GetLastError());
		MessageBox(NULL, error, TEXT("PowerCfg"), MB_OK | MB_ICONEXCLAMATION);
	}

	if(CanUserWritePwrScheme() == ERROR_ACCESS_DENIED)
	{
		MessageBox(NULL, TEXT("Access denied, \n\nplease run AddPowerScheme.exe as a system administrator"), TEXT("PowerCfg"), MB_OK | MB_ICONEXCLAMATION);
		return(1);
	}

	// Create new scheme if needed
	if(vInfo.vCargoScheme == (UINT)-1)
	{
		vInfo.vCargoScheme = vInfo.vLastScheme + 1;

		GetCurrentPowerPolicies(NULL, &vPowerPolicy);

		vPowerPolicy.user.IdleTimeoutAc = vPowerPolicy.user.IdleTimeoutDc = 0;
		vPowerPolicy.user.VideoTimeoutAc = vPowerPolicy.user.VideoTimeoutDc = 0;

		if(WritePwrScheme(&vInfo.vCargoScheme, CARGOSCHEMENAME, CARGOSCHEMEDESCR, &vPowerPolicy) == 0)
		{
			StringCbPrintf(error, sizeof(error), TEXT("Create powerscheme failed\n\nExitcode 0x%X"), GetLastError());
			MessageBox(NULL, error, TEXT("PowerCfg"), MB_OK | MB_ICONEXCLAMATION);
			return(1);
		}
	}

	// Create simple (all access) security descriptor for power control reg entries
	if(!InitializeSecurityDescriptor(&vSec, SECURITY_DESCRIPTOR_REVISION))
	{
		StringCbPrintf(error, sizeof(error), TEXT("InitializeSecurityDescriptor failed\n\nExitcode 0x%X"), GetLastError());
		MessageBox(NULL, error, TEXT("PowerCfg"), MB_OK | MB_ICONEXCLAMATION);
	}
	if(!SetSecurityDescriptorDacl(&vSec, TRUE, NULL, TRUE))
	{
		StringCbPrintf(error, sizeof(error), TEXT("SetSecurityDescriptorDacl failed\n\nExitcode 0x%X"), GetLastError());
		MessageBox(NULL, error, TEXT("PowerCfg"), MB_OK | MB_ICONEXCLAMATION);
	}

	// Set security descriptors to use for powercfg registry entries
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, HKEY_POWER1A, 0, KEY_ALL_ACCESS | WRITE_DAC, &hReg) == ERROR_SUCCESS)
	{
		RegSetKeySecurity(hReg, DACL_SECURITY_INFORMATION, &vSec);
		RegCloseKey(hReg);
	}
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, HKEY_POWER1B, 0, KEY_ALL_ACCESS | WRITE_DAC, &hReg) == ERROR_SUCCESS)
	{

		RegSetKeySecurity(hReg, DACL_SECURITY_INFORMATION, &vSec);
		// Also 'free' all the subkeys
		for(vIndex = 0; retVal != ERROR_NO_MORE_ITEMS; vIndex++)
		{
			vKeyLen = sizeof(vKey);
			if((retVal = RegEnumKeyEx(hReg, vIndex, vKey, &vKeyLen, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
			{
				if(RegOpenKeyEx(hReg, vKey, 0, KEY_ALL_ACCESS | WRITE_DAC, &hSubKey) == ERROR_SUCCESS)
				{
					RegSetKeySecurity(hSubKey, DACL_SECURITY_INFORMATION, &vSec);
					RegCloseKey(hSubKey);
				}
			}
			else if(retVal == ERROR_ACCESS_DENIED)
				MessageBox(NULL, TEXT("Access denied, \n\nplease run AddPowerScheme.exe as a system administrator"), TEXT("PowerCfg"), MB_OK | MB_ICONEXCLAMATION);
		}
		RegCloseKey(hReg);
	}
	if(RegOpenKeyEx(HKEY_USERS, HKEY_POWER2A, 0, KEY_ALL_ACCESS | WRITE_DAC, &hReg) == ERROR_SUCCESS)
	{
		RegSetKeySecurity(hReg, DACL_SECURITY_INFORMATION, &vSec);
		RegCloseKey(hReg);
	}
	if(RegOpenKeyEx(HKEY_USERS, HKEY_POWER2B, 0, KEY_ALL_ACCESS | WRITE_DAC, &hReg) == ERROR_SUCCESS)
	{
		RegSetKeySecurity(hReg, DACL_SECURITY_INFORMATION, &vSec);
		for(vIndex = 0; retVal != ERROR_NO_MORE_ITEMS; vIndex++)
		{
			vKeyLen = sizeof(vKey);
			if((retVal = RegEnumKeyEx(hReg, vIndex, vKey, &vKeyLen, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
			{
				if(RegOpenKeyEx(hReg, vKey, 0, KEY_ALL_ACCESS | WRITE_DAC, &hSubKey) == ERROR_SUCCESS)
				{
					RegSetKeySecurity(hSubKey, DACL_SECURITY_INFORMATION, &vSec);
					RegCloseKey(hSubKey);
				}
			}
			else if(retVal == ERROR_ACCESS_DENIED)
				MessageBox(NULL, TEXT("Access denied, \n\nplease run AddPowerScheme.exe as a system administrator"), TEXT("PowerCfg"), MB_OK | MB_ICONEXCLAMATION);
		}
		RegCloseKey(hReg);
	}

	return(0);
}

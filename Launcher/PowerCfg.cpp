
#include "stdafx.h"

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


static BOOLEAN CALLBACK PowerSchemeCallback(UINT uiIndex, DWORD dwName, LPTSTR sName, DWORD dwDesc, LPTSTR sDesc, PPOWER_POLICY pp, LPARAM lParam)
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

void SetPower(void)
{
	SchemeInfo			vInfo;
	UINT				vCurrentScheme;
	POWER_POLICY		vPowerPolicy;

	vInfo.vSize = sizeof(SchemeInfo);
	vInfo.vCargoScheme = (UINT)-1;
	vInfo.vLastScheme = 0;

	if(EnumPwrSchemes(PowerSchemeCallback, (LPARAM)&vInfo) == 0)
	{
		INT vError;
		vError = GetLastError();
	}

	// Create new scheme if needed
	if(vInfo.vCargoScheme == (UINT)-1)
	{
		if(CanUserWritePwrScheme() == ERROR_ACCESS_DENIED)
		{
			MessageBox(NULL, TEXT("This account has no privileges to change the power scheme of the computer.\nPlease set the power scheme manually so that the monitor will not be turned off."), TEXT("PowerCfg"), MB_OK | MB_ICONEXCLAMATION);
			return;
		}
		vInfo.vCargoScheme = vInfo.vLastScheme + 1;

		GetCurrentPowerPolicies(NULL, &vPowerPolicy);
		vPowerPolicy.user.IdleTimeoutAc = vPowerPolicy.user.IdleTimeoutDc = 0;
		vPowerPolicy.user.VideoTimeoutAc = vPowerPolicy.user.VideoTimeoutDc = 0;

		if(WritePwrScheme(&vInfo.vCargoScheme, CARGOSCHEMENAME, CARGOSCHEMEDESCR, &vPowerPolicy) == 0)
		{
			TCHAR error[100];
			StringCbPrintf(error, sizeof(error), TEXT("Create powerscheme failed\n\nExitcode 0x%X"), GetLastError());
			MessageBox(NULL, error, TEXT("PowerCfg"), MB_OK | MB_ICONEXCLAMATION);
			return;
		}
	}

	GetActivePwrScheme(&vCurrentScheme);
	if(vCurrentScheme != vInfo.vCargoScheme)
		SetActivePwrScheme(vInfo.vCargoScheme, NULL, NULL);
}


// VolKeysHook.cpp : Defines the entry point for the DLL application.
//

#include <windows.h>
#include "stdafx.h"
#include "VolKeysHook.h"

#pragma comment(lib, "Winmm.lib")

#pragma data_seg(".shared")
INT		vRef = 0;
HHOOK	hVolKeysHook = NULL;
HMIXER	hMixer = NULL;

HWND	pSubscribers[25] = {NULL,NULL,NULL,NULL,NULL, NULL,NULL,NULL,NULL,NULL, NULL,NULL,NULL,NULL,NULL, NULL,NULL,NULL,NULL,NULL, NULL,NULL,NULL,NULL,NULL};
#pragma data_seg()

DWORD	vMuteControl;
DWORD	vVolumeControl;
DWORD	vVolMin, vVolMax;

void InitHmixer(void);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			if(vRef++ == 0)
			{
				ZeroMemory(pSubscribers, sizeof(pSubscribers));
				hVolKeysHook = NULL;
				hMixer = NULL;
			}
			break;

		case DLL_PROCESS_DETACH:
			if(--vRef == 0)
			{
				if(hVolKeysHook)
					UnhookWindowsHookEx(hVolKeysHook);
				hVolKeysHook = NULL;
				if(hMixer)
					InitHmixer(FALSE);
			}
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}

HOOK_API VOID InitHmixer(BOOL vOnOff)
{
	if(vOnOff)
	{
		if(!hVolKeysHook)
			hVolKeysHook = SetWindowsHookEx(WH_SHELL, VolKeysHook, (HINSTANCE)GetModuleHandle(NULL), 0);

		if(mixerOpen(&hMixer, 0, NULL, 0L, 0) == MMSYSERR_NOERROR)
		{
			MIXERCAPS vMixCaps;
			if(mixerGetDevCaps((UINT_PTR)hMixer, &vMixCaps, sizeof(vMixCaps)) == MMSYSERR_NOERROR)
			{
				MIXERLINE vMixLine;
				MIXERCONTROL vMixControl;
				MIXERLINECONTROLS vMixControls;

				// get dwLineID
				vMixLine.cbStruct = sizeof(MIXERLINE);
				vMixLine.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
				mixerGetLineInfo((HMIXEROBJ)hMixer, &vMixLine, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE);

				// get dwControlID
				vMixControls.cbStruct = sizeof(vMixControls);
				vMixControls.dwLineID = vMixLine.dwLineID;
				vMixControls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
				vMixControls.cControls = 1;
				vMixControls.cbmxctrl = sizeof(vMixControl);
				vMixControls.pamxctrl = &vMixControl;
				mixerGetLineControls((HMIXEROBJ)hMixer, &vMixControls, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);

				// We need the ID in the callback
				vMuteControl = vMixControl.dwControlID;

				// get dwControlID
				vMixControls.cbStruct = sizeof(vMixControls);
				vMixControls.dwLineID = vMixLine.dwLineID;
				vMixControls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
				vMixControls.cControls = 1;
				vMixControls.cbmxctrl = sizeof(vMixControl);
				vMixControls.pamxctrl = &vMixControl;
				mixerGetLineControls((HMIXEROBJ)hMixer, &vMixControls, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);

				// We need the ID in the callback
				vVolumeControl = vMixControl.dwControlID;
				vVolMin = vMixControl.Bounds.dwMinimum;
				vVolMax = vMixControl.Bounds.dwMaximum;
			}
		}
	}
	else if(hMixer && !vOnOff)
	{
		mixerClose(hMixer);
		hMixer = NULL;
	}
}


HOOK_API INT SetWindow(HWND hWnd)
{
	INT i;

	for(i = 0; i < (sizeof(pSubscribers) / sizeof(pSubscribers[0])); i++)
	{
		if(pSubscribers[i] == 0)
		{
			pSubscribers[i] = hWnd;
			return(0);
		}
	}
	return(1);
}


HOOK_API LRESULT CALLBACK VolKeysHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_BOOLEAN mxcdMute;
	MIXERCONTROLDETAILS_UNSIGNED mxcdVolume;
	INT	i;

	if(nCode < 0)  // do not process message
		return(CallNextHookEx(hVolKeysHook, nCode, wParam, lParam));

	switch(nCode)
	{
		case HSHELL_APPCOMMAND:
			switch(GET_APPCOMMAND_LPARAM(lParam))
			{
				case APPCOMMAND_VOLUME_MUTE:
					if(hMixer)
					{
						// Get current mute setting
						mxcd.cbStruct = sizeof(mxcd);
						mxcd.dwControlID = vMuteControl;
						mxcd.cChannels = 1;
						mxcd.cMultipleItems = 0;
						mxcd.cbDetails = sizeof(mxcdMute);
						mxcd.paDetails = &mxcdMute;
						mixerGetControlDetails((HMIXEROBJ)hMixer, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
						mxcdMute.fValue = !mxcdMute.fValue;
						mixerSetControlDetails((HMIXEROBJ)hMixer, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
					}
					break;

				case APPCOMMAND_VOLUME_DOWN:
					if(hMixer)
					{
						// Get current volume setting
						mxcd.cbStruct = sizeof(mxcd);
						mxcd.dwControlID = vVolumeControl;
						mxcd.cChannels = 1;
						mxcd.cMultipleItems = 0;
						mxcd.cbDetails = sizeof(mxcdVolume);
						mxcd.paDetails = &mxcdVolume;
						mixerGetControlDetails((HMIXEROBJ)hMixer, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
						mxcdVolume.dwValue -= (vVolMax - vVolMin) / 25;
						if(mxcdVolume.dwValue < vVolMin) mxcdVolume.dwValue = vVolMin;
						if(mxcdVolume.dwValue > vVolMax) mxcdVolume.dwValue = vVolMax;
						mixerSetControlDetails((HMIXEROBJ)hMixer, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
					}
					break;

				case APPCOMMAND_VOLUME_UP:
					if(hMixer)
					{
						// Get current volume setting
						mxcd.cbStruct = sizeof(mxcd);
						mxcd.dwControlID = vVolumeControl;
						mxcd.cChannels = 1;
						mxcd.cMultipleItems = 0;
						mxcd.cbDetails = sizeof(mxcdVolume);
						mxcd.paDetails = &mxcdVolume;
						mixerGetControlDetails((HMIXEROBJ)hMixer, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
						mxcdVolume.dwValue += (vVolMax - vVolMin) / 25;
						if(mxcdVolume.dwValue < vVolMin) mxcdVolume.dwValue = vVolMin;
						if(mxcdVolume.dwValue > vVolMax) mxcdVolume.dwValue = vVolMax;
						mixerSetControlDetails((HMIXEROBJ)hMixer, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
					}
					break;

				default:
					for(i = 0; i < (sizeof(pSubscribers) / sizeof(pSubscribers[0])); i++)
						if(pSubscribers[i])
							if(PostMessage(pSubscribers[i], WM_USER + 29, wParam, lParam) == 0)
								pSubscribers[i] = NULL;
					break;
			}
			break;
	}
	return(CallNextHookEx(hVolKeysHook, nCode, wParam, lParam));
}

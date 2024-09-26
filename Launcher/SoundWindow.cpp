// SoundWindow.cpp: implementation of the SoundWindow Class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SoundWindow.h"
#include "Config.h"
#include <commctrl.h>
#include "mixerdll.h"
#include <shlobj.h>


#pragma comment(lib, "Winmm.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

HMIXER	hMixer;
HMODULE	hMixer1 = NULL;
HFNSET	hSetValue = NULL;

LRESULT CALLBACK SoundWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static SoundWindow	*my;

SoundWindow::SoundWindow(HINSTANCE hInstance, HWND hParent)
{
	LONG	newStyle;
	INT		width, height;
	BOOL	winMixer = TRUE;
	CHAR	vInpTxt[10];
	TCHAR	vPath[MAX_PATH];
	TCHAR	vText[10];

	my = this;
	hInst = hInstance;
	hWnd = NULL;
	hPrnt = hParent;
	hMixer = NULL;

	MyRegisterClass(hInstance);
	ConfigGetInt(BUTTONHEIGHT, &width);
	height = GetSystemMetrics(SM_CYSCREEN) - 2 * width;
	if(height > (6 * width))
		height = 6 * width;

	hWnd = CreateWindow(TEXT("CRPC_SNDWNDW"), TEXT("Volume"), WS_BORDER | WS_CLIPCHILDREN, 0, 0, 0, 0, NULL, NULL, hInst, NULL);

	newStyle = GetWindowLong(hWnd, GWL_STYLE);
	newStyle &= ~(WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_OVERLAPPED | WS_TILED | WS_DLGFRAME );
	SetWindowLong(hWnd, GWL_STYLE, newStyle);
	SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

	SetWindowPos(hWnd, HWND_NOTOPMOST, GetSystemMetrics(SM_CXSCREEN) - width, GetSystemMetrics(SM_CYSCREEN) - (width + height),
				width, height, SWP_FRAMECHANGED);

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, 0, height - width,
					width, width, hWnd, (HMENU)IDC_MUTE, hInst, NULL);
	AssignIcon(hInst, hWnd, IDC_MUTE, IDI_SPK_OFF);

	CreateWindow(WBFADER, NULL, WS_CHILD | WS_VISIBLE | TBS_VERT, 0, 0,
					width, height - width, hWnd, (HMENU)IDC_VOLUME, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_VOLUME, WB_SETTICKCOUNT, 12, 0);


	// If windows explorer is running, we simply assume that the APPCOMMAND keys are handled by some
	// windows feature already. If not, we start our own handler
	if(FindWindow(TEXT("Shell_TrayWnd"), TEXT("")) == NULL)
	{
		hinstDLL = LoadLibrary(TEXT("VolKeysHook.dll"));
		if(hinstDLL)
		{
			typedef VOID (* HFNINIT)(BOOL);
			HFNINIT fSetMixer = (HFNINIT)GetProcAddress(hinstDLL, "InitHmixer");
			fSetMixer(TRUE);
		}
	}

	if(PipeIo("SIrMI", 6, vInpTxt, sizeof(vInpTxt)))
	{
		if((vInpTxt[0] == 0x01)	|| (vInpTxt[0] == 0x05)) //CARGOICE or CARGOBUS?
		{
			SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, SHGFP_TYPE_CURRENT, vPath);
			PathAppend(vPath, TEXT("White Bream\\Mixer.dll"));
			hMixer1 = LoadLibrary(vPath);
			if(hMixer1)
			{
				HFNCB dllIds;
				dllIds = (HFNCB)GetProcAddress(hMixer1, "SetCallback");
				if(dllIds)
					(dllIds)(hWnd);

				hSetValue = (HFNSET)GetProcAddress(hMixer1, "SetMixer");
			}

			PipeIo("MXrM", 5, vInpTxt, sizeof(vInpTxt));
			SendDlgItemMessage(hWnd, IDC_MUTE, BM_SETCHECK, (vInpTxt[0] - '0' ? BST_CHECKED : BST_UNCHECKED), 0);
			SendMessage(hPrnt, WM_USER + 2, vInpTxt[0] - '0' ? TRUE : FALSE, 0);

			vVolMin = 25;
			vVolMax = 95;
			SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETRANGE, TRUE, MAKELONG(vVolMin, vVolMax));

			PipeIo("MXrV", 5, vInpTxt, sizeof(vInpTxt));
			//SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETPOS, TRUE, (vVolMax - vVolMin) - vInpTxt[0]);
			SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETPOS, TRUE, 40 - vInpTxt[0]);
			if(vInpTxt[0] > 0)
				StringCbPrintf(vText, sizeof(vText), TEXT("+%d"), vInpTxt[0]);
			else
				StringCbPrintf(vText, sizeof(vText), TEXT("%d"), vInpTxt[0]);
			SetWindowText(GetDlgItem(hWnd, IDC_VOLUME), vText);
			winMixer = FALSE;
		}
	}
	if(winMixer)
	{
		if(mixerOpen(&hMixer, 0, (DWORD)hWnd, 0L, CALLBACK_WINDOW) != MMSYSERR_NOERROR)
			MessageBox(hWnd, TEXT("mixerOpen failed"), TEXT("error"), MB_OK);
		else
		{
			MIXERCAPS vMixCaps;
			if(mixerGetDevCaps((UINT_PTR)hMixer, &vMixCaps, sizeof(vMixCaps)) != MMSYSERR_NOERROR)
				MessageBox(hWnd, TEXT("mixerGetDevCaps failed"), TEXT("error"), MB_OK);
			else
			{
				MIXERLINE vMixLine;
				MIXERCONTROL vMixControl;
				MIXERLINECONTROLS vMixControls;
				MIXERCONTROLDETAILS mxcd;

				// get dwLineID
				vMixLine.cbStruct = sizeof(MIXERLINE);
				vMixLine.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
				if(mixerGetLineInfo((HMIXEROBJ)hMixer, &vMixLine, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE) != MMSYSERR_NOERROR)
					MessageBox(hWnd, TEXT("mixerGetLineInfo failed"), TEXT("error"), MB_OK);

				// get dwControlID
				vMixControls.cbStruct = sizeof(vMixControls);
				vMixControls.dwLineID = vMixLine.dwLineID;
				vMixControls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
				vMixControls.cControls = 1;
				vMixControls.cbmxctrl = sizeof(vMixControl);
				vMixControls.pamxctrl = &vMixControl;
				if(mixerGetLineControls((HMIXEROBJ)hMixer, &vMixControls, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR)
					MessageBox(hWnd, TEXT("mixerGetLineControls failed"), TEXT("error"), MB_OK);

				// We need the ID in the callback
				vMuteControl = vMixControl.dwControlID;

				// Get current mute setting
				MIXERCONTROLDETAILS_BOOLEAN mxcdMute;
				mxcd.cbStruct = sizeof(mxcd);
				mxcd.dwControlID = vMuteControl;
				mxcd.cChannels = 1;
				mxcd.cMultipleItems = 0;
				mxcd.cbDetails = sizeof(mxcdMute);
				mxcd.paDetails = &mxcdMute;
				mixerGetControlDetails((HMIXEROBJ)hMixer, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);

				SendDlgItemMessage(hWnd, IDC_MUTE, BM_SETCHECK, (mxcdMute.fValue ? BST_CHECKED : BST_UNCHECKED), 0);
				SendMessage(hPrnt, WM_USER + 2, mxcdMute.fValue, 0);

				// get dwControlID
				vMixControls.cbStruct = sizeof(vMixControls);
				vMixControls.dwLineID = vMixLine.dwLineID;
				vMixControls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
				vMixControls.cControls = 1;
				vMixControls.cbmxctrl = sizeof(vMixControl);
				vMixControls.pamxctrl = &vMixControl;
				if(mixerGetLineControls((HMIXEROBJ)hMixer, &vMixControls, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR)
					MessageBox(hWnd, TEXT("mixerGetLineControls failed"), TEXT("error"), MB_OK);

				// We need the ID in the callback
				vVolumeControl = vMixControl.dwControlID;
				vVolMin = vMixControl.Bounds.dwMinimum;
				vVolMax = vMixControl.Bounds.dwMaximum;
				SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETRANGE, TRUE, MAKELONG(vVolMin, vVolMax));

				// Get current volume setting
				MIXERCONTROLDETAILS_UNSIGNED mxcdVolume;
				mxcd.cbStruct = sizeof(mxcd);
				mxcd.dwControlID = vVolumeControl;
				mxcd.cChannels = 1;
				mxcd.cMultipleItems = 0;
				mxcd.cbDetails = sizeof(mxcdVolume);
				mxcd.paDetails = &mxcdVolume;
				mixerGetControlDetails((HMIXEROBJ)hMixer, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
				SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETPOS, TRUE, (vVolMax - vVolMin) - mxcdVolume.dwValue);
			}
		}
	}
}

SoundWindow::~SoundWindow()
{
	if(hMixer)		mixerClose(hMixer);
	if(hMixer1)		FreeLibrary(hMixer1);
	if(hinstDLL)	FreeLibrary(hinstDLL);

	DestroyWindow(hWnd);
}

ATOM SoundWindow::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= 0;
	wcex.lpfnWndProc	= SoundWindowProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= CreateSolidBrush(RGB(200, 255, 200));
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPC_SNDWNDW");
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

static HWND hMoving = NULL;

LRESULT CALLBACK SoundWindowProc(HWND hWnd, UINT vMsg, WPARAM wParam, LPARAM lParam)
{
	INT		width, height;
	CHAR	vInpTxt[10];
	CHAR	vOutTxt[10];
	TCHAR	vText[10];

	switch(vMsg)
	{
		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				if(LOWORD(wParam) == IDC_MUTE)
				{
					BOOL mute;
					mute = SendDlgItemMessage(hWnd, IDC_MUTE, BM_GETCHECK, 0, 0) == BST_CHECKED ? FALSE : TRUE;
					if(my->hMixer)
					{
						MIXERCONTROLDETAILS_BOOLEAN mxcdMute = {mute};
						MIXERCONTROLDETAILS mxcd;
						mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
						mxcd.dwControlID = my->vMuteControl;
						mxcd.cChannels = 1;
						mxcd.cMultipleItems = 0;
						mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
						mxcd.paDetails = &mxcdMute;
						mixerSetControlDetails((HMIXEROBJ)my->hMixer, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
					}
					else
					{
						StringCbPrintfA(vOutTxt, sizeof(vOutTxt), "MXwM%c", mute ? '1' : '0');
						if(hSetValue) hSetValue(vOutTxt, (INT)strlen(vOutTxt) + 1);
						else
						{
							PipeIo(vOutTxt, (DWORD)strlen(vOutTxt) + 1, vInpTxt, sizeof(vInpTxt));
							SendDlgItemMessage(hWnd, IDC_MUTE, BM_SETCHECK, (mute ? BST_CHECKED : BST_UNCHECKED), 0);
							SendMessage(my->hPrnt, WM_USER + 2, mute, 0);
						}
					}
				}
				SendMessage(my->hPrnt, WM_USER + 1, 0, 0);
			}
			break;

		case WM_VSCROLL:
			if((LOWORD(wParam) == TB_THUMBTRACK) || (LOWORD(wParam) == TB_ENDTRACK ))
			{
				INT vPos;
				if(LOWORD(wParam) == TB_THUMBTRACK)
					vPos = HIWORD(wParam);
				else
					vPos = SendMessage((HWND)lParam, TBM_GETPOS, 0, 0) & 0xFFFF;

				hMoving = (HWND)lParam;
				if(LOWORD(wParam) == TB_ENDTRACK)
					hMoving = NULL;

				if(my->hMixer)
				{
					vPos = (my->vVolMax - my->vVolMin) - vPos;
					MIXERCONTROLDETAILS_UNSIGNED mxcdVolume = {vPos};
					MIXERCONTROLDETAILS mxcd;
					mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
					mxcd.dwControlID = my->vVolumeControl;
					mxcd.cChannels = 1;
					mxcd.cMultipleItems = 0;
					mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
					mxcd.paDetails = &mxcdVolume;
					mixerSetControlDetails((HMIXEROBJ)my->hMixer, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
				}
				else
				{
					vPos = 40 - vPos;
					StringCbPrintfA(vOutTxt, sizeof(vOutTxt), "MXwV%c", (CHAR)(vPos & 0xFF));
					if(hSetValue) hSetValue(vOutTxt, (INT)strlen(vOutTxt) + 1);
					else PipeIo(vOutTxt, (DWORD)strlen(vOutTxt) + 1, vInpTxt, sizeof(vInpTxt));
				}
				SendMessage(my->hPrnt, WM_USER + 3, 0, 0);
			}
			break;

		case WM_KILLFOCUS:
			if(IsWindowVisible(hWnd) && ((HWND)wParam !=  my->hPrnt))
				PostMessage(my->hPrnt, WM_USER + 1, 0, 0);
			break;

		case WM_DISPLAYCHANGE:
			ConfigGetInt(BUTTONHEIGHT, &width);
			height = GetSystemMetrics(SM_CYSCREEN) - 2 * width;
			if(height > (6 * width))
				height = 6 * width;
			SetWindowPos(hWnd, HWND_NOTOPMOST, GetSystemMetrics(SM_CXSCREEN) - width, GetSystemMetrics(SM_CYSCREEN) - (width + height),
				width, height, SWP_HIDEWINDOW);
			break;

		case MM_MIXM_CONTROL_CHANGE:
			if(wParam && (lParam == my->vMuteControl))
			{
				MIXERCONTROLDETAILS_BOOLEAN mxcdMute;
				MIXERCONTROLDETAILS mxcd;
				mxcd.cbStruct = sizeof(mxcd);
				mxcd.dwControlID = lParam;
				mxcd.cChannels = 1;
				mxcd.cMultipleItems = 0;
				mxcd.cbDetails = sizeof(mxcdMute);
				mxcd.paDetails = &mxcdMute;
				mixerGetControlDetails((HMIXEROBJ)wParam, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);

				SendDlgItemMessage(hWnd, IDC_MUTE, BM_SETCHECK, (mxcdMute.fValue ? BST_CHECKED : BST_UNCHECKED), 0);
				SendMessage(my->hPrnt, WM_USER + 2, mxcdMute.fValue, 0);
			}
			else if(wParam && (lParam == my->vVolumeControl))
			{
				MIXERCONTROLDETAILS_UNSIGNED mxcdVolume;
				MIXERCONTROLDETAILS mxcd;
				mxcd.cbStruct = sizeof(mxcd);
				mxcd.dwControlID = lParam;
				mxcd.cChannels = 1;
				mxcd.cMultipleItems = 0;
				mxcd.cbDetails = sizeof(mxcdVolume);
				mxcd.paDetails = &mxcdVolume;
				mixerGetControlDetails((HMIXEROBJ)wParam, &mxcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);

				if(GetDlgItem(hWnd, IDC_VOLUME) != hMoving)
					SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETPOS, TRUE, (my->vVolMax - my->vVolMin) - mxcdVolume.dwValue);
			}
			break;

		case WM_MIXERCHANGED:
			switch(wParam)
			{
				case 'V':
					if(GetDlgItem(hWnd, IDC_VOLUME) != hMoving)
						SendDlgItemMessage(hWnd, IDC_VOLUME, TBM_SETPOS, TRUE, 40 - (signed short)LOWORD(lParam));
					if((signed short)LOWORD(lParam) >= 0)
						StringCbPrintf(vText, sizeof(vText), TEXT("+%ddB"), (signed short)LOWORD(lParam));
					else
						StringCbPrintf(vText, sizeof(vText), TEXT("%ddB"), (signed short)LOWORD(lParam));
					SetWindowText(GetDlgItem(hWnd, IDC_VOLUME), vText);
					break;

				case 'M':
					SendDlgItemMessage(hWnd, IDC_MUTE, BM_SETCHECK, (LOWORD(lParam) - '0' ? BST_CHECKED : BST_UNCHECKED), 0);
					SendMessage(my->hPrnt, WM_USER + 2, LOWORD(lParam) - '0', 0);
					break;

				default:
					break;
			}
			break;

		case WM_CLOSE:
			if(hSetValue) hSetValue("MXwA", 5);
			else PipeIo("MXwA", 5, vInpTxt, sizeof(vInpTxt));	// Save All
			return(DefWindowProc(hWnd, vMsg, wParam, lParam));

		case WM_DESTROY:
			my->hWnd = NULL;
			SendMessage(my->hPrnt, WM_USER + 1, 0, 0);
			break;

		default:
			return DefWindowProc(hWnd, vMsg, wParam, lParam);
   }
   return(0);
}


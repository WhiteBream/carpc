// player.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MainWindow.h"
#include "mixerdll.h"
#include <shlobj.h>

#define DEFAULTSIZE		48


typedef struct _Clients
{
	Rect   bRect;
	HWND   hWnd, hCbWnd;
	BOOL   enabled;
	int    vControl, vIcon;
} Clients;

#include <initguid.h>
///#include <ntddmou.h>
//#include <Dbt.h>
#include <setupapi.h>
#include <ks.h>
#include <ksmedia.h>

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static HDEVINFO DoSoundDeviceEnum(GUID *InterfaceClassGuid);

AppInfo sAi;

static MainWindow *my;
static Clients     clients[IDW_max+1];

MainWindow::MainWindow(HINSTANCE hInstance, HWND hParent)
{
	HWND		hStart;
	LPVOID		pConfig;
	STARTUPINFO	supInfo;
	INT			X, Y, H, W;
	LPINT		pInt;
	TCHAR		vPath[MAX_PATH];
	CHAR		vInpTxt[10];


	sAi.hInst = hInstance;
	MyRegisterClass(sAi.hInst);
	my = this;
	hAppKeys = NULL;

	playWin = NULL;
	radioWin = NULL;
	listWin = NULL;
	sortWin = NULL;
	mediaDB = NULL;
	video = NULL;
	settWin = NULL;
	memset(clients, 0, sizeof(clients));

	hSetValue = NULL;

	/* Get the desired size of the buttons */
	hStart = FindWindow(TEXT("CRPC_STRTBTTN"), NULL);
	if(hStart)
		GetClientRect(hStart, &startRect);
	else
		startRect.bottom = startRect.left = startRect.right = startRect.top = 0;
	if(!startRect.bottom)
		startRect.bottom = DEFAULTSIZE;
	sAi.vOffs = startRect.bottom;

	/* Start database */
	sAi.hDb = new Database();
	if(sAi.hDb->Initialize())
		PostQuitMessage(1);

	memset(&supInfo, 0, sizeof(supInfo));
	GetStartupInfo(&supInfo);
	if(supInfo.dwFlags & STARTF_USESIZE)
	{
		H = supInfo.dwYSize;
		W = supInfo.dwXSize;
	}
	else
	{
		pInt = &W;
		if(!sAi.hDb->GetConfig("WindowWidth", (LPVOID*)&pInt, CONFIGTYPE_INT))
			W = 10 * startRect.bottom;

		pInt = &H;
		if(!sAi.hDb->GetConfig("WindowHeight", (LPVOID*)&pInt, CONFIGTYPE_INT))
			H = 3 * startRect.bottom / 4;

	}

	if(supInfo.dwFlags & STARTF_USEPOSITION)
	{
		X = supInfo.dwX;
		Y = supInfo.dwY;
	}
	else
	{
		pInt = &X;
		if(!sAi.hDb->GetConfig("WindowPosX", (LPVOID*)&pInt, CONFIGTYPE_INT))
			X = CW_USEDEFAULT;
		if(X < 0) X = CW_USEDEFAULT;

		pInt = &Y;
		if(!sAi.hDb->GetConfig("WindowPosH", (LPVOID*)&pInt, CONFIGTYPE_INT))
			Y = CW_USEDEFAULT;
		if(Y < 0) Y = CW_USEDEFAULT;
	}

	myGraphics = NULL;

	lastclicked = -1;

	current = 0;
	pConfig = &vRunMode;
	if(sAi.hDb->GetConfig("RunMode", (LPVOID*)&pConfig, CONFIGTYPE_INT))
	{
		if(vRunMode == MODE_AMFM)
			current = 1;
		vRunMode = (RUNMODE)-1;
	}
	sAi.vTA = sAi.hDb->GetBool("TrafficAnnouncement");

	/* Create main window */
	hMain = CreateWindow(TEXT("CRPCPLR_MN"), TEXT("CARGO Media Player"), WS_TILEDWINDOW | WS_CLIPCHILDREN,
						X, Y, W, H, hParent, NULL, sAi.hInst, NULL);

	// Unmute
	SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, SHGFP_TYPE_CURRENT, vPath);
	PathAppend(vPath, TEXT("White Bream\\Mixer.dll"));
	hMixer = LoadLibrary(vPath);
	if(!hMixer)
	{
		StrCpy(vPath, TEXT("C:\\Henk\\WhiteBream\\Projects\\C401\\Drivers\\Cargo\\MixerDll\\Release\\Mixer.dll"));
		hMixer = LoadLibrary(vPath);
	}
	if(hMixer)
		hSetValue = GetProcAddress(hMixer, "SetMixer");

	if(hSetValue) ((HFNSET)hSetValue)("MXwM0", 6);
	else PipeIo("MXwM0", 6, vInpTxt, sizeof(vInpTxt));

	hAppKeys = LoadLibrary(TEXT("VolKeysHook.dll"));
	if(hAppKeys)
	{
		HFNCB fSetWindow = (HFNCB)GetProcAddress(hAppKeys, "SetWindow");
		fSetWindow(hMain);
	}

//	GUID WaveoutGuid = KSCATEGORY_AUDIO;
//	HDEVINFO NewDeviceInfoSet;

//	NewDeviceInfoSet = DoSoundDeviceEnum(&WaveoutGuid); 
//	SetupDiDestroyDeviceInfoList(NewDeviceInfoSet);
}

/* DoSoundDeviceEnum results:

:Cargo system with TVout board:
\\?\pci#ven_8086&dev_24c5&subsys_4740414c&rev_02#3&13c0b0c5&0&fd#{6994ad04-93ef-11d0-a3cc-00a0c9223196}\topology
Realtek AC'97 Audio
\\?\pci#ven_8086&dev_24c5&subsys_4740414c&rev_02#3&13c0b0c5&0&fd#{6994ad04-93ef-11d0-a3cc-00a0c9223196}\wave
Realtek AC'97 Audio
\\?\root#system#0000#{6994ad04-93ef-11d0-a3cc-00a0c9223196}\{2f412ab5-ed3a-4590-ab24-b0ce2aa77d3c}&{9b365890-165f-11d0-a195-0020afd156e4}
Plug and Play Software Device Enumerator (total 6x with slightly different paths)
\\?\usb#vid_08bb&pid_2904&mi_00#7&a76d6d2&0&0000#{6994ad04-93ef-11d0-a3cc-00a0c9223196}\global
USB Audio Device

So AC97 is a PCI device, we can safely search for paths of the device paths because no other PCI devices can be added anyway.

*/

static HDEVINFO DoSoundDeviceEnum( GUID *InterfaceClassGuid )
/*
Routine Description:
    Retrieves the device information set that contains that contains
    the devices of the specified class. 

Parameters:
    InterfaceClassGuid - The interface class GUID. 

Return Value:
    If the function succeeds, the return value is a handle to the 
    device information set.

    If the function fails, the return value is zero.
*/
{
    HDEVINFO DeviceInfoSet;
    HDEVINFO NewDeviceInfoSet;
	DWORD Err;
	DWORD MemberIndex ;
	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;

    // Create a device information set that will be the container for 
    // the device interfaces.
    DeviceInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);
    
    if(DeviceInfoSet == INVALID_HANDLE_VALUE)
	{
        Err = GetLastError();
        printf( "SetupDiCreateDeviceInfoList failed: %lx.\n", Err );
        return 0;
    }

    // Retrieve the device information set for the interface class.
    NewDeviceInfoSet = SetupDiGetClassDevsEx( InterfaceClassGuid,
        NULL,   NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE,
        DeviceInfoSet,
        NULL,    NULL
    );

    if(NewDeviceInfoSet == INVALID_HANDLE_VALUE) 
    {
        Err = GetLastError();
        printf( "SetupDiGetClassDevsEx failed: %lx.\n", Err );
		MessageBox(NULL, TEXT("SetupDiGetClassDevsEx error"), TEXT("Device"), MB_OK);
        return 0;
    }

	MemberIndex  = 0;
	DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	do
	{
		if(SetupDiEnumDeviceInterfaces(NewDeviceInfoSet, NULL, InterfaceClassGuid, MemberIndex++, &DeviceInterfaceData))
		{
			if(DeviceInterfaceData.Flags == SPINT_ACTIVE)
			{
				DWORD size = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + MAX_PATH * sizeof(TCHAR);
				PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)GlobalAlloc(GPTR, size);
				SP_DEVINFO_DATA DeviceInfo;
				DeviceInfo.cbSize = sizeof(SP_DEVINFO_DATA);
				pDeviceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

				if(SetupDiGetDeviceInterfaceDetail(NewDeviceInfoSet, &DeviceInterfaceData, pDeviceDetail, size, NULL, &DeviceInfo))
				{
				//	printf("%s\n", pDeviceDetail->DevicePath);
					MessageBox(NULL, pDeviceDetail->DevicePath, TEXT("Device"), MB_OK);

					// Parse the path for good devices (USB audio on certain USB controller+port as 
					// RearAudioDev, any PCI audio device for FrontAudioDev). Keep the match devicedescription
					// to search for corresponding FriendlyNames with DirectShow functions.
					if(1)
					{
						TCHAR vBuffer[256];
					//	if(SetupDiGetDeviceRegistryProperty(NewDeviceInfoSet, &DeviceInfo, SPDRP_CLASSGUID, NULL,
					//										(PBYTE)vBuffer, sizeof(vBuffer), NULL))
					//	{
 					//		MessageBox(NULL, vBuffer, TEXT("SPDRP_CLASSGUID"), MB_OK);
					//	}
					//	if(SetupDiGetDeviceRegistryProperty(NewDeviceInfoSet, &DeviceInfo, SPDRP_FRIENDLYNAME, NULL,
					//										(PBYTE)vBuffer, sizeof(vBuffer), NULL))
					//	{
 					//		MessageBox(NULL, vBuffer, TEXT("Device Name"), MB_OK);
					//	}
						if(SetupDiGetDeviceRegistryProperty(NewDeviceInfoSet, &DeviceInfo, SPDRP_DEVICEDESC, NULL,
															(PBYTE)vBuffer, sizeof(vBuffer), NULL))
						{
 							MessageBox(NULL, vBuffer, TEXT("SPDRP_DEVICEDESC"), MB_OK);
						}
					}
				}
				else MessageBox(NULL, TEXT("SetupDiGetDeviceInterfaceDetail error"), TEXT("Device"), MB_OK);
				GlobalFree(pDeviceDetail);

			}
			else
			{
			//	printf("not active\n");
				MessageBox(NULL, TEXT("not active"), TEXT("Device"), MB_OK);
			}
		}
	} while(GetLastError() != ERROR_NO_MORE_ITEMS);
	printf("\n\n");

    return NewDeviceInfoSet;
}

MainWindow::~MainWindow()
{
	DestroyWindow(hMain);
	if(myGraphics) delete myGraphics;
	if(font) delete font;
	if(yellow) delete yellow;
	if(bkGnd) delete bkGnd;
	GdiplusShutdown(gdiplusToken);

	if(hSetValue) hSetValue = NULL;
	if(hMixer) FreeLibrary(hMixer);

	if(hAppKeys) FreeLibrary(hAppKeys);
	DestinatorFree(sAi.hInst);

	delete sAi.hDb;
}


void MainWindow::SaveMode(void)
{
	LPVOID	pConfig = NULL;
	RECT	winRect;
	LONG	winStyle;

	if(sAi.hDb->GetConfig("RunMode", (LPVOID*)&pConfig, CONFIGTYPE_INT))
	{
		if(vRunMode != *(RUNMODE*)pConfig)
			sAi.hDb->SetConfig("RunMode", (LPVOID)&vRunMode, CONFIGTYPE_INT);
		sAi.hDb->Free(pConfig);
	}
	else
		sAi.hDb->SetConfig("RunMode", (LPVOID)&vRunMode, CONFIGTYPE_INT);

	winStyle = GetWindowLong(hMain, GWL_STYLE);
	if(!(winStyle & (WS_MAXIMIZE | WS_MINIMIZE)))
	{
		GetWindowRect(hMain, &winRect);
		sAi.hDb->SetConfig("WindowPosX", (LPVOID)&winRect.left, CONFIGTYPE_INT);
		sAi.hDb->SetConfig("WindowPosH", (LPVOID)&winRect.top, CONFIGTYPE_INT);
		winRect.right -= winRect.left;
		winRect.bottom -= winRect.top;
		sAi.hDb->SetConfig("WindowWidth", (LPVOID)&winRect.right, CONFIGTYPE_INT);
		sAi.hDb->SetConfig("WindowHeight", (LPVOID)&winRect.bottom, CONFIGTYPE_INT);
	}
}


ATOM MainWindow::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= MainWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PLR));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("CRPCPLR_MN");
	wcex.hIconSm		= (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_PLR), IMAGE_ICON, 16, 16, 0);

	return RegisterClassEx(&wcex);
}


void MainWindow::InitDC(BOOL updateDC)
{
	if(updateDC && myGraphics)
	   delete myGraphics;
	myGraphics = new Graphics(hCbWnd, FALSE);

	GetClientRect(hCbWnd, &ourRect);
}


void MainWindow::GdiInit(HINSTANCE hInstance, HWND hSrc)
{
	GdiplusStartupInput gdiplusStartupInput;
	Rect                btnPlace;
	INT                 i;

	// Initialize GDI+.
	gdiplusStartupInput.DebugEventCallback = NULL;
	gdiplusStartupInput.GdiplusVersion = 1;
	gdiplusStartupInput.SuppressBackgroundThread = FALSE;
	gdiplusStartupInput.SuppressExternalCodecs = FALSE;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	bkGnd = new SolidBrush(Color(255, 0, 0, 192));
	yellow = new SolidBrush(Color(255, 255, 255, 80));
	font = new Font(TEXT("Arial"), 32);

	DestinatorInit(hInstance);

	GetClientRect(hSrc, &ourRect);

	btnPlace.X = startRect.right;
	btnPlace.Y = ourRect.bottom - sAi.vOffs;

	clients[0].vControl = IDC_PLR;
	clients[0].vIcon = IDI_PLR;
	clients[1].vControl = IDC_RAD;
	clients[1].vIcon = IDI_RAD;
	clients[2].vControl = IDC_LST;
	clients[2].vIcon = IDI_LST;
	clients[3].vControl = IDC_SRT;
	clients[3].vIcon = IDI_SRT;
	clients[4].vControl = IDC_MDB;
	clients[4].vIcon = IDI_LIB;
	clients[5].vControl = IDC_VID;
	clients[5].vIcon = IDI_VID;
	clients[6].vControl = IDC_MXR;
	clients[6].vIcon = IDI_MXR;
	clients[7].vControl = 0;

	for(i = 0; clients[i].vControl != 0; i++)
	{
		clients[i].hWnd = CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE, btnPlace.X, btnPlace.Y, 
							sAi.vOffs, sAi.vOffs, hSrc, (HMENU)clients[i].vControl, hInstance, NULL);
		AssignIcon(hInstance, hSrc, clients[i].vControl, clients[i].vIcon);
		btnPlace.X += sAi.vOffs;
	}

	CreateWindow(WBBUTTON, NULL, WS_CHILD | WS_VISIBLE,
				btnPlace.X, btnPlace.Y, sAi.vOffs, sAi.vOffs, hSrc, (HMENU)IDC_MUTE, hInstance, NULL);
	AssignIcon(hInstance, hSrc, IDC_MUTE, IDI_SPK_ON);

	/* Some (temporary) default disabled */
	SendDlgItemMessage(hSrc, IDC_LST, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, 0);
}


void MainWindow::Source(BOOL vTuner)
{
	CHAR	vRsp[10];
	LPSTR	vCommand;

	// Check which audio device we're looking for, on cargo I/O version <1.2, front is the USB device
	PipeIo("SIrVI", 6, vRsp, sizeof(vRsp));
	if(vRsp[1] == '.')
	{
		if((vRsp[0] <= '1') && (vRsp[2] <= '1'))
			vCommand = "MXwS1";	// USBDev = front on version < 1.2
		else
			vCommand = "MXwS0"; // From v1.2, front is AC97

		//if((vRsp[0] <= '1') && (vRsp[2] <= '1'))
		//	MessageBox(NULL, L"Source set to USB", L"Cargo Mixer", MB_OK);
		//else
		//	MessageBox(NULL, L"Source set to AC97", L"Cargo Mixer", MB_OK);

		if(hSetValue) ((HFNSET)hSetValue)(vCommand, 6);
		else PipeIo(vCommand, 6, vRsp, sizeof(vRsp));
	}
	//else
	//	MessageBox(NULL, L"Set source failed", L"Cargo Mixer", MB_OK);
}


void MainWindow::Paint()
{
	RectF       vRect;
    PointF      point;
	BOOL		vDemo = TRUE;
	CHAR		vInpTxt[10];

	if(myGraphics == NULL)
		return;

	myGraphics->FillRectangle(bkGnd, 0, 0, ourRect.right, ourRect.bottom);

	if(PipeIo("SIrMI", 6, vInpTxt, sizeof(vInpTxt)))
		vDemo = FALSE;
	if(vDemo)
	{
		point.X = 0;
		point.Y = 0;
		myGraphics->MeasureString(L"Demo", -1, font, point, &vRect);
		point.Y = ourRect.bottom - sAi.vOffs + 2 + (sAi.vOffs - vRect.Height) / 2;
		point.X = (REAL)startRect.right + 7 * sAi.vOffs;
		myGraphics->DrawString(L"Demo", -1, font, point, yellow);
	}
}


void MainWindow::Client(UINT n)
{
	RUNMODE newMode = vRunMode;

	if(n != current)
	{
		if(n == -1)
			n = current;

		switch(n)
		{
		//	case IDW_PLR: if(playWin)  playWin->Show(TRUE);  newMode = MODE_MP3;  break;
		//	case IDW_RAD: if(radioWin) radioWin->Show(TRUE); newMode = MODE_AMFM; break;
		//	case IDW_LST: if(listWin)  listWin->Show(TRUE);  break;
		//	case IDW_SRT: if(sortWin)  sortWin->Show(TRUE);  break;
		//	case IDW_MDB: if(mediaDB)  mediaDB->Show(TRUE);  break;
		//	case IDW_VID: if(video)    video->Show(TRUE);    break;
		//	case IDW_MXR: if(settWin)  settWin->Show(TRUE);  break;
			case IDW_PLR: newMode = MODE_MP3;  break;
			case IDW_RAD: newMode = MODE_AMFM; break;
		}
		if(vRunMode != newMode)
		{
			switch(vRunMode)
			{
				case MODE_MP3:  if(playWin)  playWin->Pause(TRUE); break;
				case MODE_AMFM: if(radioWin) radioWin->Mute(TRUE); break;
			}
			switch(newMode)
			{
				case MODE_MP3:  if(playWin)  playWin->Pause(FALSE); Source(FALSE); break;
				case MODE_AMFM: if(radioWin) radioWin->Mute(FALSE); Source(TRUE); break;
			}
			vRunMode = newMode;
		}
		switch(n)
		{
			case IDW_PLR: if(playWin)  playWin->Show(TRUE);  break;
			case IDW_RAD: if(radioWin) radioWin->Show(TRUE); break;
			case IDW_LST: if(listWin)  listWin->Show(TRUE);  break;
			case IDW_SRT: if(sortWin)  sortWin->Show(TRUE);  break;
			case IDW_MDB: if(mediaDB)  mediaDB->Show(TRUE);  break;
			case IDW_VID: if(video)    video->Show(TRUE);    break;
			case IDW_MXR: if(settWin)  settWin->Show(TRUE);  break;
		}
		if(n != current)
		{
			switch(current)
			{
				case IDW_PLR: if(playWin)  playWin->Show(FALSE);  break;
				case IDW_RAD: if(radioWin) radioWin->Show(FALSE); break;
				case IDW_LST: if(listWin)  listWin->Show(FALSE);  break;
				case IDW_SRT: if(sortWin)  sortWin->Show(FALSE);  break;
				case IDW_MDB: if(mediaDB)  mediaDB->Show(FALSE);  break;
				case IDW_VID: if(video)    video->Show(FALSE);    break;
				case IDW_MXR: if(settWin)  settWin->Show(FALSE);  break;
			}
			current = n;
		}

		SendMessage(clients[0].hWnd, BM_SETCHECK, (WPARAM)((current == 0) ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(clients[1].hWnd, BM_SETCHECK, (WPARAM)((current == 1) ? BST_CHECKED : BST_UNCHECKED), 0);
//		SendMessage(clients[2].hWnd, BM_SETCHECK, (WPARAM)((current == 2) ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(clients[3].hWnd, BM_SETCHECK, (WPARAM)((current == 3) ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(clients[4].hWnd, BM_SETCHECK, (WPARAM)((current == 4) ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(clients[5].hWnd, BM_SETCHECK, (WPARAM)((current == 5) ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(clients[6].hWnd, BM_SETCHECK, (WPARAM)((current == 6) ? BST_CHECKED : BST_UNCHECKED), 0);
	}
}


LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT	ps;
	int			h, w, i;
	UINT		n;
	RECT		ourRect, wr;
	CHAR		vInpTxt[10];
	LRESULT		retVal;
	INT			r;

	h = sAi.vOffs ? sAi.vOffs : DEFAULTSIZE;
	w = my->startRect.right;

	switch (message)
	{
		case WM_CREATE:
			my->hCbWnd = hWnd;
			sAi.hApp = hWnd;
			my->InitDC(FALSE);
			my->GdiInit(sAi.hInst, hWnd);
			my->playWin  = new PlayWin(&sAi);
			my->video    = new Video(&sAi);
			my->radioWin = new Radio(&sAi);
			my->listWin  = new Playlist(&sAi);
			my->sortWin  = new Sorted(&sAi);
			my->mediaDB  = new MediaDB(&sAi);
			my->settWin  = new Settings(&sAi);
			ShowWindow(hWnd, SW_SHOW);
			break;

		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				switch(LOWORD(wParam))
				{
					case IDC_PLR:	n = IDW_PLR;	break;
					case IDC_RAD:	n = IDW_RAD;	break;
					case IDC_LST:	n = IDW_LST;	break;
					case IDC_SRT:	n = IDW_SRT;	break;
					case IDC_MDB:	n = IDW_MDB;	break;
					case IDC_VID:	n = IDW_VID;	break;
					case IDC_MXR:	n = IDW_MXR;	break;
					default:
						n = my->current;
						break;
				}
				my->Client(n);

				switch(LOWORD(wParam))
				{
					case IDC_SPK:
					case IDC_MUTE:
						if(SendDlgItemMessage(hWnd, IDC_MUTE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
						{
							AssignIcon(sAi.hInst, hWnd, IDC_MUTE, IDI_SPK_OFF);
							SendDlgItemMessage(hWnd, IDC_MUTE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
							if(my->hSetValue) r = ((HFNSET)(my->hSetValue))("MXwM1", 6);
							else r = PipeIo("MXwM1", 6, vInpTxt, sizeof(vInpTxt));
							if(!r)
								PostMessage(clients[IDW_PLR].hCbWnd, WM_DIRECTSHOWMUTE, (WPARAM)TRUE, 0);
						}
						else
						{
							AssignIcon(sAi.hInst, hWnd, IDC_MUTE, IDI_SPK_ON);
							SendDlgItemMessage(hWnd, IDC_MUTE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							if(my->hSetValue) r = ((HFNSET)(my->hSetValue))("MXwM0", 6);
							else r = PipeIo("MXwM0", 6, vInpTxt, sizeof(vInpTxt));
							if(!r)
								PostMessage(clients[IDW_PLR].hCbWnd, WM_DIRECTSHOWMUTE, (WPARAM)FALSE, 0);
						}
						break;
					default:
						if(IsChild(hWnd, (HWND)lParam))
						{
							// Pass the command to the playwindow
							SendMessage(clients[IDW_PLR].hCbWnd, message, wParam, lParam);
						}
						break;
				}
			}
			break;

		case WM_SYSCOMMAND:
			if(wParam == SC_CLOSE)
			{
				for(i = 0; clients[i].vControl != 0; i++)
					if(clients[i].hCbWnd)
						SendMessage(clients[i].hCbWnd, WM_SAVEDATA, 0, 0);
			}
			return(DefWindowProc(hWnd, message, wParam, lParam));

		case WM_SIZE:
			GetClientRect(hWnd, &ourRect);
			//if((ourRect.right < (w + (9 * h))) || (ourRect.right < (10 * h)) || (ourRect.bottom < (2 * (w + (9 * h)) / 3)))
			if((ourRect.right < (10 * h)) || (ourRect.bottom < (3 * (10 * h) / 4)))
			{
				GetWindowRect(hWnd, &wr);
				wr.right -= wr.left;
				wr.right -= ourRect.right; // result is width overhead
				wr.bottom -= wr.top;
				wr.bottom -= ourRect.bottom; // result is height overhead

				//if(ourRect.right < (w + (8 * h))) ourRect.right = w + (9 * h);
				if(ourRect.right < (10 * h)) ourRect.right = 10 * h;
				//if(ourRect.bottom < (2 * (w + (9 * h)) / 3)) ourRect.bottom = 2 * (w + (9 * h)) / 3;
				if(ourRect.bottom < (3 * (10 * h) / 4)) ourRect.bottom = 3 * (10 * h) / 4;

				SetWindowPos(hWnd, 0, 0, 0, ourRect.right + wr.right, ourRect.bottom + wr.bottom,
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
				GetClientRect(hWnd, &ourRect);
			}
			// Reposition client select buttons
			for(i = 0; clients[i].vControl != 0; i++)
				SetWindowPos(clients[i].hWnd, NULL, w + i * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hWnd, IDC_MUTE), 0, ourRect.right - h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

			// Reposition&size client windows
			for(i = 0; clients[i].vControl != 0; i++)
				SetWindowPos(clients[i].hCbWnd, NULL, 0, 0, ourRect.right, ourRect.bottom - h, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
			
			my->InitDC(TRUE);
			my->Paint();
			my->Client(-1);
			break;

		case WM_PAINT:
            BeginPaint(hWnd, &ps);
			my->Paint();
			EndPaint(hWnd, &ps);
			break;

		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
			break;

		case WM_LBUTTONUP:
			ReleaseCapture();
			break;

		case WM_SETVIDEOWINDOW:
		case WM_PAINTVIDEOWINDOW:
		case WM_DISPLAYCHANGE:
		case WM_PLAYNOW:
		case WM_PLAYNEXT:
		case WM_PLAYQUEUE:
			SendMessage(clients[IDW_PLR].hCbWnd, message, wParam, lParam);
			break;

		case WM_MMBUTTON:
			for(i = IDW_PLR; i < IDW_max; i++)
				SendMessage(clients[i].hCbWnd, WM_MMBUTTON, wParam, lParam);
			break;

		case WM_APPCOMMAND:
			retVal = 0;
			for(i = IDW_PLR; i < IDW_max; i++)
				retVal |= SendMessage(clients[i].hCbWnd, WM_MMBUTTON, wParam, lParam);
			if(retVal)
                return(retVal);
			else
				return(DefWindowProc(hWnd, message, wParam, lParam));

		case WM_PAUSE:
		case WM_DSHOWTIME:
		case WM_DSHOWPOS:
			for(i = IDW_PLR; i < IDW_max; i++)
				SendMessage(clients[i].hCbWnd, message, wParam, lParam);
			break;

		case WM_MUTERADIO:
			if(my->vRunMode == MODE_AMFM)
			{
				if(my->radioWin)
					my->radioWin->Mute(TRUE);
				if(my->playWin)
					my->playWin->Pause(FALSE);
				my->vRunMode = MODE_MP3;
			}
			break;

		case WM_RADIOTA:
			if(wParam)
				my->vTaIntercept = my->current;

			if(sAi.vTA)
			{
				if(lParam)
				{
					my->vTaIntercept = my->current;
					if(my->current != IDW_RAD)
					{
						my->Client(IDW_RAD);
						if(my->radioWin)
							my->radioWin->Mute(FALSE);
						if(my->playWin)
							my->playWin->Pause(TRUE);
					}
				}
				else
				{
					if((my->vTaIntercept != IDW_RAD) && (my->vTaIntercept <= IDW_MXR))
					{
						my->Client(my->vTaIntercept);
						if(my->radioWin)
							my->radioWin->Mute(TRUE);
						if(my->playWin)
							my->playWin->Pause(FALSE);
					}
				}
			}
			break;

		case WM_SHOWPLAYERWINDOW:
			my->Client(IDW_PLR);
			break;

		case WM_SHOWVIDEOWINDOW:
			if(lParam)
				my->Client(IDW_VID);
			SendMessage(clients[IDW_VID].hCbWnd, WM_SHOWVIDEOWINDOW, wParam, lParam);
			break;

		case WM_SETREGISTERFINALWND:
			clients[LOWORD(wParam)].hCbWnd = (HWND)lParam;
			if(clients[IDW_MXR].hCbWnd)
				SendMessage(clients[IDW_PLR].hCbWnd, WM_INITDONE, 0, 0);
			break;

		case WM_GETREGISTERFINALWND:
			if(lParam)
				*(HWND*)lParam = clients[LOWORD(wParam)].hCbWnd;
			break;

		case WM_REALIGNVIDEOWINDOW:
			GetClientRect(hWnd, &ourRect);
			SetWindowPos(GetDlgItem(hWnd, IDC_VID), 0, w + 4 * h, ourRect.bottom - h, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			break;

		case WM_ENDSESSION:
			if((BOOL)wParam == TRUE)
			{
				for(i = 0; clients[i].vControl != 0; i++)
					if(clients[i].hCbWnd)
						SendMessage(clients[i].hCbWnd, WM_SAVEDATA, 0, 0);
				my->SaveMode();
			}
			break;

		case WM_CLOSE:
			for(i = 0; clients[i].vControl != 0; i++)
				if(clients[i].hCbWnd)
					SendMessage(clients[i].hCbWnd, WM_SAVEDATA, 0, 0);
			return(DefWindowProc(hWnd, message, wParam, lParam));

		case WM_DESTROY:
			my->SaveMode();
			if(my->settWin)  delete my->settWin;
			if(my->video)    delete my->video;
			if(my->mediaDB)  delete my->mediaDB;
			if(my->listWin)  delete my->listWin;
			if(my->sortWin)  delete my->sortWin;
			if(my->radioWin) delete my->radioWin;
			if(my->playWin)  delete my->playWin;
			PostQuitMessage(0);
			break;

		default:
			return(DefWindowProc(hWnd, message, wParam, lParam));
   }
   return(0);
}

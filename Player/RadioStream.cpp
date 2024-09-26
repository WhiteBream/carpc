// FMRadioDevice.cpp: implementation of the CFMRadioDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RadioStream.h"
#include "math.h"
#include "aclapi.h"

#include <string>
#include <vector>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define RADIO_TIMER_PERIOD 45 /* Timer will fire every RADIO_TIMER_PERIOD ms */

static VOID CALLBACK RadioTimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    if (lpParam) {
		((RadioStream*)lpParam)->StreamAudio();
    }
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RadioStream::RadioStream(bool GetRDSText)
{
	//Initialize the critical section variable used for exclusivity
	InitializeCriticalSection(&gWaveCriticalSection);

	//Make the handles NULL to begin with
	m_FMRadioAudioHandle = NULL;
	m_FMRadioDataHandle = NULL;
	m_SoundCardHandle = NULL;

	//The radio is not streaming or tuning initially
	m_StreamingAllowed = false;
	m_Streaming = false;

	//Setup the wave format based on our defined audio data
	m_FMRadioWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_FMRadioWaveFormat.nSamplesPerSec = SAMPLES_PER_SECOND;
	m_FMRadioWaveFormat.wBitsPerSample = BITS_PER_SAMPLE;
	m_FMRadioWaveFormat.nChannels = CHANNELS;

	// Opening the audio stream from a DLL requires this?
	m_FMRadioWaveFormat.nBlockAlign= 0xcccc;
	m_FMRadioWaveFormat.nAvgBytesPerSec = 0xcccccccc;
	m_FMRadioWaveFormat.cbSize = 0xcccc;

	//Set the last known radio index to negative since there hasnt been a radio attached yet
	m_LastKnownRadioIndex = -1;

	//The current block starts at zero, and all blocks are initially free
	m_CurrentBlock = 0;
	m_FreeBlock = 0;
	gWaveFreeBlockCount = BLOCK_COUNT;

	//Allocate memory for the blocks of audio data to stream
	m_OutputHeader = AllocateBlocks(BLOCK_SIZE, BLOCK_COUNT);

	m_WaveformBuffer = (char*)malloc(BLOCK_SIZE);
	m_InputHeader.lpData = m_WaveformBuffer;
	m_InputHeader.dwBufferLength = BLOCK_SIZE;
	m_InputHeader.dwFlags = 0;

	// Initialize our radio timer to NULL
	h_radioTimer = NULL;

	// Initialize the previous process priority to 0
	m_previous_process_priority = 0;
	m_process_priority_set = false;

	// We may or may not want to change the process priority
	change_process_priority = true;
}

RadioStream::~RadioStream()
{
	//Free all allocated memory when destroyed
	FreeBlocks(m_OutputHeader);
	free(m_WaveformBuffer);
}

BYTE RadioStream::OpenFMRadio(RadioData* radioData)
{
	//Check that radio data is not NULL
	if (radioData == NULL) {
		return (STATUS_ERROR);
	}

	//Try opening all valid pipes
	if (!OpenFMRadioData()) {
		return (STATUS_FMRADIODATA_ERROR);
	}

	//Get the radio data from the device's scratch page
	if (!GetRadioData(radioData)) {
		return (STATUS_ERROR);
	}

	//Initialize the radio with the current radio data
	if (!InitializeRadioData(radioData)) {
		return (STATUS_ERROR);
	}

	//Open the FM Radio audio input
	if (!OpenFMRadioAudio()) {
		return (STATUS_FMRADIOAUDIO_ERROR);
	}

	//Open the sound card
	if (!OpenSoundCard()) {
		return (STATUS_OUTPUTAUDIO_ERROR);
	}

	return (STATUS_OK);
}

bool RadioStream::CloseFMRadio()
{
	bool status = false;

	//Close all pipes
	CloseFMRadioAudio();
	CloseSoundCard();
	CloseFMRadioData();

	status = true;

	return status;
}

bool RadioStream::OpenFMRadioAudio()
{
	bool status = false;

	//Check the last known radio index, if it is not negative attemp to
	//open this device, it will probably be the radio
	if (m_LastKnownRadioIndex >= 0)
	{
		//If the open succeeds, then return status true, otherwise set the radio index to -1 to try again
		if (waveInOpen(&m_FMRadioAudioHandle,
					   m_LastKnownRadioIndex,
					   &m_FMRadioWaveFormat,
					   NULL, NULL, CALLBACK_NULL) == MMSYSERR_NOERROR)
		{
			status = true;
		}
		else
		{
			m_LastKnownRadioIndex = -1;
		}
	}

	//If status isn't true, then the radio didn't open successfully, or we are opening
	//the radio for the first time
	if (!status)
	{
		//Get the index of the audio device
		m_LastKnownRadioIndex = GetAudioDeviceIndex();

		//If a valid index is returned open the audio device
		if (m_LastKnownRadioIndex >= 0)
		{
			DWORD ret = waveInOpen(&m_FMRadioAudioHandle,
									m_LastKnownRadioIndex,
									&m_FMRadioWaveFormat,
									NULL, NULL, CALLBACK_NULL);

			//If the audio device opens successfully, then return true
			if (ret == MMSYSERR_NOERROR)
			{
				status = true;
			}
		}
	}

	return status;
}

int RadioStream::GetAudioDeviceIndex()
{
	//This function is designed to open up the audio handle to the USB Radio. In
	//Windows XP, the audio portion of the USB Radio will come up as "FM Radio". If
	//this string is found, the radio will open the first one immediately, and begin
	//to play it. If it isn't found then it will go through the device list and look
	//for "USB Audio Device", which shows up in Windows 2000. If only one is found, then
	//it will go ahead and open it immediately as well since the device will be the radio.
	//However if more devices (or none) are found then it will prompt with a list for the
	//user to select the correct USB Audio device from the list.

	int index = -1;
	std::vector<std::string> deviceList;

	//Obtain the number of input devices on the system
	DWORD numWaveInputDevices = waveInGetNumDevs();

	//Scan through each input device to see if we can find the FM Radio
	for (DWORD i = 0; i < numWaveInputDevices; i++)
	{
		WAVEINCAPS waveInputCapabilities;

		//Get the device capabilities of the currently indexed device
		if (waveInGetDevCaps(i, &waveInputCapabilities, sizeof(waveInputCapabilities)) == MMSYSERR_NOERROR)
		{
			//If FM Radio is found, open the radio immediately, this is our device
			if (!strcmp(waveInputCapabilities.szPname, TEXT("FM Radio")))
			{
				//Set the current index to i, and set i to numWaveInputDevices to break
				//out of the for loop
				index = i;
				i = numWaveInputDevices;
			}
			else
			{
				//Otherwise push back the string of the device on the list
				deviceList.push_back(waveInputCapabilities.szPname);
			}
		}
	}

	//If we haven't found a valid index, then start looking at the strings
	if (index < 0)
	{
		DWORD usbAudioDeviceNum = 0;

		//Go through the list of device strings
		for (i = 0; i < deviceList.size(); i++)
		{
			//See if a "USB Audio Device" is found
			if (deviceList[i].find("USB Audio Device", 0))
			{
				//Increment the usb audio device number, and set our current index
				usbAudioDeviceNum++;
				index = i;
			}
		}
#if 0
		//If more than one (or no deivices) are found, then the user needs
		//to select their audio input device from the list
		if (usbAudioDeviceNum != 1)
		{
			//Reset the index to -1, invalid
			index = -1;
			//Bring up the device select dialog by passing it the audio input
			//device list
			CDeviceSelectDlg deviceSelectDlg(&deviceList);

			if (deviceSelectDlg.DoModal() == IDOK)
			{
				//If OK is pressed, get the index selected
				index = deviceSelectDlg.GetIndex();
			}
		}
#endif
	}

	//Return the index that will be used
	return index;
}

int	RadioStream::GetLastKnownRadioIndex()
{
	//Gets the last known radio index variable
	return m_LastKnownRadioIndex;
}

void RadioStream::SetNewRadioIndex(int index)
{
	//Disable audio
	bool reEnableAudio = m_Streaming;
	m_Tuning = true;
	CloseFMRadioAudio();

	//Set the new index
	m_LastKnownRadioIndex = index;

	//Open the audio again (the new index will be used)
	OpenFMRadioAudio();

	//Enable audio again if we were streaming
	if (reEnableAudio)
	{
		InitializeStream();
	}

	m_Tuning = false;
}

bool RadioStream::OpenSoundCard()
{
	bool status = false;
	DWORD res;

	//Open a handle to the default wave output device (sound card)
	if ((res = waveOutOpen(&m_SoundCardHandle, WAVE_MAPPER,
						   &m_FMRadioWaveFormat, (DWORD)waveOutProc,
						   (DWORD)&gWaveFreeBlockCount, CALLBACK_FUNCTION)) == MMSYSERR_NOERROR)
	{
		status = true;
	}

	return status;
}

void RadioStream::InitializeStream()
{
	waveInStart(m_FMRadioAudioHandle);

	//Reset block status to "empty" the buffer (simply starting over and refilling)
	m_CurrentBlock = 0;
	m_FreeBlock = 0;
	gWaveFreeBlockCount = BLOCK_COUNT;

	//Fill the audio buffer to initialize the stream
	while (gWaveFreeBlockCount > (BLOCK_COUNT - BUFFER_PADDING))
	{
		StreamAudioIn();
		Sleep(50);
	}

	//Skip the first chunk (at least half) of the padded buffer to eliminate any audio glitches
	if (BUFFER_PADDING > 10)
	{
		m_CurrentBlock = 5;
		gWaveFreeBlockCount = 5;
	}

	//Set streaming to true
	m_Streaming = true;
}

void RadioStream::StreamAudio()
{
	//If our timer thread is being stopped, then don't do this
	if (!m_StreamingAllowed) {
		return;
	}

	//If a tune isn't being performed, then stream in and out
	if (!m_Tuning)
	{
		//If we are not already streaming, initialize the stream
		if (!m_Streaming)
			InitializeStream();

		//Check that the handles arent NULL
		if ((m_FMRadioAudioHandle) && (m_SoundCardHandle))
		{
			//If there are any free blocks, then stream audio in.
			if (gWaveFreeBlockCount) {
				StreamAudioIn();
			}

			if (gWaveFreeBlockCount) {
				StreamAudioIn();
			}

			//If there are any blocks ready for output, then stream audio out
			if (gWaveFreeBlockCount < BLOCK_COUNT) {
				StreamAudioOut();
			}
		}
	}
}

bool RadioStream::IsStreaming()
{
	return m_Streaming;
}

bool RadioStream::StreamAudioIn()
{
	bool status = false;

	//Unprepare header to begin preperation process
	waveInUnprepareHeader(m_FMRadioAudioHandle, &m_OutputHeader[m_FreeBlock], sizeof(m_OutputHeader[m_FreeBlock]));

	//Prepare the header for streaming in
	if (waveInPrepareHeader(m_FMRadioAudioHandle, &m_OutputHeader[m_FreeBlock], sizeof(m_OutputHeader[m_FreeBlock])) == MMSYSERR_NOERROR)
	{
		//Get the buffer of audio in the input header
		if (waveInAddBuffer(m_FMRadioAudioHandle, &m_OutputHeader[m_FreeBlock], sizeof(m_OutputHeader[m_FreeBlock])) == MMSYSERR_NOERROR)
		{
			//Enter the critical section to decrement the free block count
			EnterCriticalSection(&gWaveCriticalSection);
			gWaveFreeBlockCount--;
			LeaveCriticalSection(&gWaveCriticalSection);

			//Increment the free block index, and scale it
			m_FreeBlock++;
			m_FreeBlock %= BLOCK_COUNT;

			status = true;
		}
	}

	return status;
}

static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	//If a WaveOut Done message comes back, then enter the critical section to
	//increment the free block counter
	if (uMsg == WOM_DONE)
	{
		EnterCriticalSection(&gWaveCriticalSection);
		int* freeBlockCounter = (int*)dwInstance;
		(*freeBlockCounter)++;
		LeaveCriticalSection(&gWaveCriticalSection);
	}
}

bool RadioStream::StreamAudioOut()
{
	bool status = false;

	//Unprepare header to begin preparation to stream out
	waveOutUnprepareHeader(m_SoundCardHandle, &m_OutputHeader[m_CurrentBlock], sizeof(WAVEHDR));

	//Prepare the header for streaming out
	if (waveOutPrepareHeader(m_SoundCardHandle, &m_OutputHeader[m_CurrentBlock], sizeof(WAVEHDR)) == MMSYSERR_NOERROR)
	{
		//Write the sound to the sound card
		if (waveOutWrite(m_SoundCardHandle, &m_OutputHeader[m_CurrentBlock], sizeof(WAVEHDR)) == MMSYSERR_NOERROR)
		{
			//waveOutProc callback function will get called, and free block counter gets incremented

			//Increment the index of the current block to be played
			m_CurrentBlock++;
			m_CurrentBlock %= BLOCK_COUNT;

			status = true;
		}
	}

	return status;
}

BYTE RadioStream::GetWaveOutVolume()
{
	DWORD level = VOLUME_MIN;

	//This gets the current wave output volume
	waveOutGetVolume(m_SoundCardHandle, &level);

	//This determines the level of one channel
	level = level & 0xFFFF;

	//If the level is above 0, then calculate it's percentage (0-100%)
	if (level)
		level = (DWORD)((double)(level / (double)0xFFFF) * 100.0);

	//Return the percentage level of volume
	return (BYTE)(level & 0xFF);
}

bool RadioStream::SetWaveOutVolume(BYTE level)
{
	bool status = false;
	DWORD setLevel = 0x00000000;

	//Don't set the volume to anything greater than the max level
	if (level > VOLUME_MAX)
		level = VOLUME_MAX;

	//Calculate a value based on the percentage input of one channel
	if (level)
		setLevel = (DWORD)(((double)level / 100.0) * (double)0xFFFF);

	//Set the volume for L and R channels
	setLevel = (setLevel << 16) | setLevel;

	//Set the volume
	if (waveOutSetVolume(m_SoundCardHandle, setLevel) == MMSYSERR_NOERROR)
		status = true;

	return status;
}

bool RadioStream::CloseFMRadioAudio()
{
	bool status = false;

	//Stop the input from the device
	waveInStop(m_FMRadioAudioHandle);

	//Reset the device
	waveInReset(m_FMRadioAudioHandle);

	//Close the device
	waveInClose(m_FMRadioAudioHandle);

	//Reset handles and variables
	m_FMRadioAudioHandle = NULL;
	m_CurrentBlock = 0;
	m_FreeBlock = 0;
	gWaveFreeBlockCount = BLOCK_COUNT;
	m_Streaming = false;
	status = true;

	return status;
}

bool RadioStream::CloseSoundCard()
{
	bool status = false;

	//Reset the device
	waveOutReset(m_SoundCardHandle);

	//Close the device
	waveOutClose(m_SoundCardHandle);

	//Reset the handle
	m_SoundCardHandle = NULL;
	status = true;

	return status;
}

WAVEHDR* RadioStream::AllocateBlocks(int size, int count)
{
	char* buffer;
	WAVEHDR* blocks = NULL;
	DWORD totalBufferSize = (size + sizeof(WAVEHDR)) * count;

	//Allocate zero initialized memory the size of our total buffer
	if (buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, totalBufferSize))
	{
		blocks = (WAVEHDR*)buffer;
		buffer += sizeof(WAVEHDR) * count;

		//Fill the headers out based on our allocated space
		for (int i = 0; i < count; i++)
		{
			blocks[i].dwBufferLength = size;
			blocks[i].lpData = buffer;
			buffer += size;
		}
	}

	return blocks;
}

void RadioStream::FreeBlocks(WAVEHDR* blockArray)
{
	//Free the heap memory from the pointer provided
	HeapFree(GetProcessHeap(), 0, blockArray);
}

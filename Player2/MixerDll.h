// MixerDll.h : Defines the entry point for the console application.
//

extern "C"
{

#ifdef HOOK_EXPORTS
#define HOOK_API __declspec(dllexport)
#else
#define HOOK_API __declspec(dllimport)
#endif


#define WM_MIXERCHANGED		(WM_USER + 10000)

typedef INT (* HFNCB)(HWND);
typedef INT (* HFNSET)(LPSTR, INT);


HOOK_API INT SetCallback(HWND hWnd);
HOOK_API INT SetMixer(LPSTR pMsg, INT vLen);

}

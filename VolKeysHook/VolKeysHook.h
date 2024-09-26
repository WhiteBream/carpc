
// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the DESTHOOK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// DESTHOOK_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
extern "C"
{

#ifdef HOOK_EXPORTS
#define HOOK_API __declspec(dllexport)
#else
#define HOOK_API __declspec(dllimport)
#endif


typedef VOID (* HFNINIT)(BOOL);
typedef INT (* HFNCB)(HWND);


HOOK_API VOID InitHmixer(BOOL vOnOff);
HOOK_API INT SetWindow(HWND hWnd);

HOOK_API LRESULT CALLBACK VolKeysHook(int nCode, WPARAM wParam, LPARAM lParam);

}

// Playlist.h: interface for the Playlist class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYLIST_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_PLAYLIST_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Playlist
{
private:
	Graphics   * myGraphics;
	Brush      * bkGnd;
	RECT         ourRect;
	HINSTANCE    hInst;
	Font       * font;

	ATOM MyRegisterClass(HINSTANCE hInstance);

public:
	Playlist(PAppInfo pAppInfo);
	virtual ~Playlist();

	PAppInfo		pAi;
	int        offset;
	HWND       hList, hCbWnd, hPrnt, hText;

	void InitDC(BOOL updateDC);
	void Paint();
	virtual Show(BOOL yesNo);
	int OSK();
};


#endif // !defined(AFX_PLAYLIST_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

// About.h: interface for the About class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ABOUT_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_ABOUT_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct _AboutParm
{
	INT        offset;
} AboutParm;


ATOM RegisterAboutClass(HINSTANCE hInstance);


#endif // !defined(AFX_ABOUT_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

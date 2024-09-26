// Help.h: interface for the Help class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HELP_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_HELP_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct _HelpParm
{
	INT        offset;
} HelpParm;


ATOM RegisterHelpClass(HINSTANCE hInstance);


#endif // !defined(AFX_HELP_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

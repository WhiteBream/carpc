// Remove.h: interface for the Remove class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REMOVE_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_REMOVE_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct _RemoveParm
{
	INT        offset;
	Database   *hDb;
	MediaDescr *pMedia;
} RemoveParm;


ATOM RegisterRemoveClass(HINSTANCE hInstance);


#endif // !defined(AFX_REMOVE_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

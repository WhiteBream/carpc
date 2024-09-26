// Search.h: interface for the Search class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SEARCH_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_SEARCH_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Search
{
private:
	HANDLE hThread;
	DWORD  vThread;

public:
	Search(HWND hParent, Database * pDB);
	virtual ~Search();

	void AddFolder(BOOL vPrio, LPCTSTR pStr);
	void GarbageCollection(void);
};


#endif // !defined(AFX_SEARCH_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

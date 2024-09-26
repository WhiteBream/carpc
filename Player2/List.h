// Settings.h: interface for the Settings class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LIST_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)
#define AFX_LIST_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct _ListHead
{
	struct _ListHead *pPrev;
	struct _ListHead *pNext;
	BOOL      vType;
	LPVOID    pData;
} ListHead;

class Queue
{
private:
    void _add(ListHead *pNew, ListHead *pPrev, ListHead *pNext);
	void _del(ListHead * pPrev, ListHead * pNext);

	ListHead  vHead;
	ListHead *pHead;

public:
	Queue(void);
	virtual ~Queue();

	void Add(BOOL vPrio, BOOL vType, LPVOID pData);
	BOOL Get(LPVOID *ppData);
	BOOL Empty(void);
};

#endif // !defined(AFX_LIST_H__5D076E58_675D_4587_8163_EBE154EE5DA3__INCLUDED_)

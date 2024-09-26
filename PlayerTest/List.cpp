// List.cpp: implementation of the List class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "List.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Queue::Queue(void)
{
	vHead.pData = NULL;
	vHead.vType = FALSE;
	pHead = &vHead;
	pHead->pNext = pHead;
	pHead->pPrev = pHead;
}

Queue::~Queue()
{
}

void Queue::_add(ListHead *pNew, ListHead *pPrev, ListHead *pNext)
{
	pNext->pPrev = pNew;
	pNew->pNext = pNext;
	pNew->pPrev = pPrev;
	pPrev->pNext = pNew;
}

void Queue::_del(ListHead * pPrev, ListHead * pNext)
{
	pNext->pPrev = pPrev;
	pPrev->pNext = pNext;
}

void Queue::Add(BOOL vPrio, BOOL vType, LPVOID pData)
{
	ListHead * pNew;

	pNew = (ListHead*)GlobalAlloc(GPTR, sizeof(ListHead));
	if(pNew)
	{
		if(vPrio)
			_add(pNew, pHead, pHead->pNext);
		else
			_add(pNew, pHead->pPrev, pHead);

		pNew->pData = pData;
		pNew->vType = vType;
	}
}

BOOL Queue::Get(LPVOID *ppData)
{
	ListHead *pGet = NULL;
	BOOL      ret = FALSE;

	if(Empty() == FALSE)
	{
		pGet = pHead->pNext;
		*ppData = pGet->pData;

		_del(pGet->pPrev, pGet->pNext);
		GlobalFree(pGet);
		ret = TRUE;
	}
	else *ppData = NULL;
	return(ret);
}

BOOL Queue::Empty(void)
{
	return(pHead->pNext == pHead ? TRUE : FALSE);
}

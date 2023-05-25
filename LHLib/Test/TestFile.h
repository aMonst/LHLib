#pragma once
#include "../Public/LHLib.h"

class CLHTextFile : public CLHFile
{
public:
	CLHTextFile() :
		CLHFile()
	{
	}

	CLHTextFile(LPCWSTR lpszFileName, BOOL bUseIOCP = TRUE) :
		CLHFile(lpszFileName, bUseIOCP)
	{
	}

	virtual ~CLHTextFile()
	{

	}

protected:
	// 派生类需要重写这些方法以响应完成通知或者其他别的通知
	// 派生类通常在这些方法中来清理对应的缓冲

	virtual void OnCreate(LPCWSTR lpszFileName) override
	{
	}

	virtual void OnClose(LPCWSTR lpszFileName) override
	{
	}

	virtual BOOL OnWrite(LPCWSTR lpszFileName, void* pData, DWORD dwLen, DWORD dwTrans, void* pUseData) override
	{
		if (NULL != pData)
		{
			::HeapFree(GetProcessHeap(), 0, pData);
			pData = NULL;
		}
		return TRUE;
	}

	virtual BOOL OnRead(LPCWSTR lpszFileName, void* pData, DWORD dwLen, DWORD dwTrans, void* pUseData) override
	{
		if (NULL != pData)
		{
			::HeapFree(GetProcessHeap(), 0, pData);
			pData = NULL;
		}

		return TRUE;
	}

	virtual void OnError(LPCWSTR lpszFileName, DWORD dwErrCode, UINT nOpType, void* pData, DWORD dwLen, DWORD dwTransLen, void* pUseData) override
	{
		if (NULL != pData)
		{
			::HeapFree(GetProcessHeap(), 0, pData);
		}
	}
};

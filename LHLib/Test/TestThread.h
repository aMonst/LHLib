#pragma once
#include "Public/LHLib.h"
#include <iostream>

class CTestThread : public CLHThread
{
public:
	virtual UINT __stdcall Run(LPVOID pParam) override
	{
		CLHString sInfo;
		sInfo.Format(L"CTestThread对象[A:0x%08x]线程[H:0x%08x ID:0x%x]函数ThreadProc运行\n", this, m_hThread, m_dwThreadID);
		std::wcout << sInfo;
		return CLHThread::Run(pParam);
	}
};

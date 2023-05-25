//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：Windows线程封装类头文件
//模块名称: CLHLib 线程模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-20 17:30:27
//修改日志:
//===================================================================================

#pragma once
#ifndef __LH_THREAD_H__
#define __LH_THREAD_H__
#include "LH_Win.h"
#include "LHException.h"

class CLHThread : public CLHObject
{
public:
	CLHThread() :
		m_dwThreadID(0),
		m_hThread(NULL)
	{

	}

	virtual ~CLHThread() = default;

	// Win32线程相关操作已经被封装到该线程类中，因此这里为了安全，暂时不提供访问线程句柄的方法
	DWORD GetID() const
	{
		return m_dwThreadID;
	}

	BOOL CreateThread(DWORD dwCreateFlags = 0, UINT nStackSize = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL)
	{
		typedef UINT(__stdcall* StdThreadFunc)(LPVOID);
		typedef UINT(__stdcall CLHThread::* ThreadFunc)(LPVOID);

		BOOL bRet = FALSE;

		try
		{
			ThreadFunc pMemberFunc = &CLHThread::Run;

			// 这里将类成员函数强转成普通函数
			// 类成员函数与普通函数的区别就在于在调用时会默认传递 this 指针。线程函数在线程运行时被调用并被传入this指针，这个行为与类成员函数相同，因此这里可以这么使用
			m_hThread = (HANDLE)_beginthreadex(lpSecurityAttrs, nStackSize, *(StdThreadFunc*)(&pMemberFunc), this, dwCreateFlags | CREATE_SUSPENDED, (unsigned int*) & m_dwThreadID);
			if (m_hThread == NULL || INVALID_HANDLE_VALUE == m_hThread)
			{
				throw CLHException(GetLastError());
			}

			//判断创建标志中是否有暂停标志，如果没有就启动线程
			if (!(dwCreateFlags & CREATE_SUSPENDED))
			{
				bRet = (1 >= ::ResumeThread(m_hThread));
			}
		}
		catch (CLHException& e)
		{
			bRet = FALSE;
			LH_DBGOUT_EXPW(CreateThread, e);
		}

		return bRet;
	}

	BOOL Wait(DWORD dwWaitTime)
	{
		BOOL bRet = FALSE;
		LH_ASSERT(NULL != m_hThread);
		try
		{
			auto ret = WaitForSingleObject(m_hThread, dwWaitTime);
			if (ret == WAIT_OBJECT_0 || WAIT_TIMEOUT == ret)
			{
				bRet = TRUE;
			}
			else
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			bRet = FALSE;
			LH_DBGOUT_EXPW(Wait, e);
		}

		return bRet;
	}

	DWORD ResumeThread()
	{
		// 使线程的暂停计数减一
		LH_ASSERT(NULL != m_hThread);
		return ::ResumeThread(m_hThread);
	}

	DWORD SuspendThread()
	{
		//暂停线程，同时使暂停计数加一
		LH_ASSERT(NULL != m_hThread);
		return ::SuspendThread(m_hThread);
	}

	int GetThreadPriority()
	{
		// 得到线程优先级
		LH_ASSERT(NULL != m_hThread);
		return ::GetThreadPriority(m_hThread);
	}

	BOOL SetThreadPriority(int nPriority)
	{
		// 设置线程优先级
		LH_ASSERT(NULL != m_hThread);
		return ::SetThreadPriority(m_hThread, nPriority);
	}

	BOOL SetThreadContext(const CONTEXT* lpContext)
	{
		// 设置线程的寄存器环境
		LH_ASSERT(NULL != m_hThread);
		return ::SetThreadContext(m_hThread, lpContext);
	}

	BOOL GetThreadContext(CONTEXT& context)
	{
		// 得到线程的寄存器环境值
		LH_ASSERT(NULL != m_hThread);
		return ::GetThreadContext(m_hThread, &context);
	}

	BOOL SetAffinityCPU(DWORD_PTR dwCPUIndex)
	{
		//设置线程亲缘性到一个指定的CPU索引上
		// 一般情况下建议如果要指定亲缘性,那么就让线程和CPU一对一去绑定
		BOOL bRet = TRUE;
		try
		{
			DWORD_PTR dwProcessMark = 0;
			DWORD_PTR dwSystemMark = 0;
			if (!GetProcessAffinityMask(GetCurrentProcess(), &dwProcessMark, &dwSystemMark))
			{
				throw CLHException(GetLastError());
			}
			
			DWORD_PTR dwMask = dwProcessMark & ((DWORD_PTR)1 << dwCPUIndex);
			if (!::SetThreadAffinityMask(m_hThread, dwMask))
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(SetAffinityCPU, e);
			bRet = FALSE;
		}

		return bRet;
	}
protected:
	virtual UINT __stdcall Run(LPVOID pParam)
	{
		// 这里不做任何事，使用时从该类继承并重写该函数
		return 0;
	}

public:
	virtual BOOL PostThreadMessage(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = NULL)
	{
		if (0 == m_dwThreadID)
		{
			return FALSE;
		}

		return 0 != ::PostThreadMessage(m_dwThreadID, uMsg, wParam, lParam);
	}
protected:
	DWORD m_dwThreadID;
	HANDLE m_hThread;
};
#endif

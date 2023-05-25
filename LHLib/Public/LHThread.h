//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�������Windows�̷߳�װ��ͷ�ļ�
//ģ������: CLHLib �߳�ģ��
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-5-20 17:30:27
//�޸���־:
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

	// Win32�߳���ز����Ѿ�����װ�����߳����У��������Ϊ�˰�ȫ����ʱ���ṩ�����߳̾���ķ���
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

			// ���ｫ���Ա����ǿת����ͨ����
			// ���Ա��������ͨ����������������ڵ���ʱ��Ĭ�ϴ��� this ָ�롣�̺߳������߳�����ʱ�����ò�������thisָ�룬�����Ϊ�����Ա������ͬ��������������ôʹ��
			m_hThread = (HANDLE)_beginthreadex(lpSecurityAttrs, nStackSize, *(StdThreadFunc*)(&pMemberFunc), this, dwCreateFlags | CREATE_SUSPENDED, (unsigned int*) & m_dwThreadID);
			if (m_hThread == NULL || INVALID_HANDLE_VALUE == m_hThread)
			{
				throw CLHException(GetLastError());
			}

			//�жϴ�����־���Ƿ�����ͣ��־�����û�о������߳�
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
		// ʹ�̵߳���ͣ������һ
		LH_ASSERT(NULL != m_hThread);
		return ::ResumeThread(m_hThread);
	}

	DWORD SuspendThread()
	{
		//��ͣ�̣߳�ͬʱʹ��ͣ������һ
		LH_ASSERT(NULL != m_hThread);
		return ::SuspendThread(m_hThread);
	}

	int GetThreadPriority()
	{
		// �õ��߳����ȼ�
		LH_ASSERT(NULL != m_hThread);
		return ::GetThreadPriority(m_hThread);
	}

	BOOL SetThreadPriority(int nPriority)
	{
		// �����߳����ȼ�
		LH_ASSERT(NULL != m_hThread);
		return ::SetThreadPriority(m_hThread, nPriority);
	}

	BOOL SetThreadContext(const CONTEXT* lpContext)
	{
		// �����̵߳ļĴ�������
		LH_ASSERT(NULL != m_hThread);
		return ::SetThreadContext(m_hThread, lpContext);
	}

	BOOL GetThreadContext(CONTEXT& context)
	{
		// �õ��̵߳ļĴ�������ֵ
		LH_ASSERT(NULL != m_hThread);
		return ::GetThreadContext(m_hThread, &context);
	}

	BOOL SetAffinityCPU(DWORD_PTR dwCPUIndex)
	{
		//�����߳���Ե�Ե�һ��ָ����CPU������
		// һ������½������Ҫָ����Ե��,��ô�����̺߳�CPUһ��һȥ��
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
		// ���ﲻ���κ��£�ʹ��ʱ�Ӹ���̳в���д�ú���
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

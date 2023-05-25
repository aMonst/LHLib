//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�������CLHIOCPBase ��ķ�װ��������Ϊ����IOCP Tcp �����Ļ�����
//ģ�����ƣ�����ģ��
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-6-03 09:53:16
//�޸���־:
//===================================================================================

#pragma once

#include "LH_Win.h"
#include "LH_Debug.h"
#include "LHObject.h"
#include "LHString.h"
#include "LHException.h"
#include "LHSystemInfo.h"

#define IOCP_QUIT_MSG	0xffeeddcc

class CLHIOCPBase : public CLHObject
{
public:
	CLHIOCPBase():
		m_hIOCP(NULL),
		m_dwConcurrent(0),
		m_dwThreadCnt(0),
		m_hThreadAry(NULL),
		m_nThreadIDAry(NULL),
		m_dwWaitTime(INFINITE)
	{
	}

	~CLHIOCPBase()
	{

	}

	// ��ȡ��󲢷��߳���
	DWORD GetConcurrent() const
	{
		return m_dwConcurrent;
	}

	// ��ȡ�̳߳�������߳���
	DWORD GetThreadCount() const
	{
		return m_dwThreadCnt;
	}

	//���ó�ʱʱ�䣬����ԭʼ�ĳ�ʱʱ��
	DWORD SetWaitTime(DWORD dwWaitTime)
	{
		DWORD dwOldTime = m_dwWaitTime;
		m_dwWaitTime = dwWaitTime;
		return dwOldTime;
	}

	DWORD GetWaitTime() const
	{
		return m_dwWaitTime;
	}

	virtual BOOL IsValid() const
	{
		//�ж�IOCP�̳߳��Ƿ�������Ч�ķ���,�ú���Ϊ�麯��,�������������Լ����ж�������
		return (m_hIOCP != NULL && m_hIOCP != INVALID_HANDLE_VALUE && m_dwThreadCnt > 0 && m_hThreadAry != NULL && m_nThreadIDAry != NULL);
	}

	BOOL Create(LPVOID pUserData, DWORD dwConcurrent = 0, DWORD dwThreadCnt = 0, LPSECURITY_ATTRIBUTES lpSAThread = NULL)
	{
		BOOL bRet = TRUE;
		try
		{
			CLHSystemInfo& sysInfo = CLHSystemInfo::GetSystemInfo();
			// ��󲢷��߳���Ĭ����CPU����
			if (0 == dwConcurrent)
			{
				m_dwConcurrent = sysInfo.GetSysInfo()->dwNumberOfProcessors;
			}
			else
			{
				m_dwConcurrent = dwConcurrent;
			}

			// ����߳�����Ĭ����CPU����������
			if (0 == dwThreadCnt)
			{
				m_dwThreadCnt = 2 * sysInfo.GetSysInfo()->dwNumberOfProcessors;
			}
			else
			{
				m_dwThreadCnt = dwThreadCnt;
			}

			//������󲢷������ܳ������е�����߳���
			m_dwConcurrent = min(m_dwThreadCnt, m_dwConcurrent);

			//����IOCP����
			m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)this, m_dwConcurrent);
			if (NULL == m_hIOCP)
			{
				throw CLHException(GetLastError());
			}

			//�����̳߳�
			if (!BeginThreadPool(lpSAThread))
			{
				throw CLHException(L"IOCP�̳߳��̴߳���ʧ��,�޷�������������!\n");
			}

			//�����麯������������������Լ�ѡ�����ز�ʵ���Լ���һЩ����
			OnCreateIOCP(pUserData);
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPBase::Create, e);
			bRet = FALSE;
		}

		return bRet;
	}

	BOOL Close(LPVOID pUserData)
	{
		// �Ѿ��ǹر�״̬��ֱ�ӷ���TRUE
		if (!IsValid())
		{
			return TRUE;
		}

		BOOL bRet = TRUE;
		try
		{
			EndThreadPool();
			if (NULL != m_hIOCP)
			{
				CloseHandle(m_hIOCP);
				m_hIOCP = NULL;
			}
			OnCloseIOCP(pUserData);
			
			m_dwConcurrent = 0;
			m_dwThreadCnt = 0;
			m_dwWaitTime = INFINITE;
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPBase::Close, e);
			bRet = FALSE;
		}

		return bRet;
	}

	void AddHandle2IOCP(HANDLE hObject)
	{
		LH_ASSERT(m_hIOCP != NULL);
		LH_ASSERT(hObject != INVALID_HANDLE_VALUE);

		try
		{
			if (!CreateIoCompletionPort(hObject, m_hIOCP, NULL, m_dwConcurrent))
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPBase::AddHandle2IOCP, e);
		}
	}

protected:
	BOOL BeginThreadPool(LPSECURITY_ATTRIBUTES lpSAThread)
	{
		typedef UINT (__stdcall *StdFun)(LPVOID);
        typedef UINT (__stdcall CLHIOCPBase::*ThreadFun)(LPVOID);

		BOOL bRet = TRUE;

		try 
		{
			//�����߳�
			ThreadFun pMemFunc = &CLHIOCPBase::IOCPThreadProc;
			m_hThreadAry = (HANDLE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(HANDLE) * m_dwThreadCnt);
			m_nThreadIDAry = (DWORD*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(HANDLE) * m_dwThreadCnt);

			for (int i = 0; i < m_dwThreadCnt; i++)
			{
				m_hThreadAry[i] = (HANDLE)_beginthreadex(lpSAThread, 0, *(StdFun*)(&pMemFunc), (void*)this, CREATE_SUSPENDED, (UINT*)(&m_nThreadIDAry[i]));
				if (m_hThreadAry[i] == NULL || INVALID_HANDLE_VALUE == m_hThreadAry[i])
				{
					throw CLHException(GetLastError());
				}
			}

			for (int i = 0; i < m_dwThreadCnt; i++)
			{
				::ResumeThread(m_hThreadAry[i]);
			}
		}
		catch (CLHException& e)
		{
			bRet = FALSE;
			LH_DBGOUT_EXPW(CLHIOCPBase::BeginThreadPool, e);
		}
		return bRet;
	}

	BOOL EndThreadPool()
	{
		BOOL bRet = TRUE;
		try
		{
			OVERLAPPED ol = { 0 };
			for (DWORD i = 0; i < m_dwThreadCnt; i++)
			{
				PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)IOCP_QUIT_MSG, &ol);
			}
			WaitForMultipleObjects(m_dwThreadCnt, m_hThreadAry, TRUE, INFINITE);

			if (m_hThreadAry != NULL)
			{
				HeapFree(GetProcessHeap(), 0, m_hThreadAry);
				m_hThreadAry = NULL;
			}

			if (m_nThreadIDAry)
			{
				HeapFree(GetProcessHeap(), 0, m_nThreadIDAry);
				m_nThreadIDAry = NULL;
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPBase::EndThreadPool, e);
			bRet = FALSE;
		}

		return bRet;
	}

	//�̳߳غ���
	UINT __stdcall IOCPThreadProc(void* pParam)
	{
		try
		{
			BOOL bRet = FALSE;
			DWORD dwTransBytes = 0;
			ULONG_PTR key = NULL;
			OVERLAPPED* pOl = NULL;
			BOOL bLoop = TRUE;

			do
			{
				try
				{
					bRet = GetQueuedCompletionStatus(m_hIOCP, &dwTransBytes, &key, &pOl, m_dwWaitTime);
					if (!bRet)
					{
						if (pOl == NULL)
						{
							//�ȴ���ʱ
							OnWaitTimeOut(m_dwWaitTime);
							continue;
						}
						else
						{
							OnIOError(GetLastError(), dwTransBytes, pOl);
							throw CLHException(GetLastError());
						}
					}

					if (IOCP_QUIT_MSG == key)
					{
						LH_DBGOUT(_T("IOCP�߳�[ID:%u]�ӵ��˳���Ϣ......\n"), GetCurrentThreadId());
						bLoop = FALSE;
					}
					else
					{
						OnIOComplete(dwTransBytes, pOl);
					}
				}
				catch (CLHException& e)
				{
					LH_DBGOUT_EXPW(CLHIOCPBase::IOCPThreadProc, e);
				}
			} while (bLoop);
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPBase::IOCPThreadProc, e);
		}

		return 0;
	}

	protected:
    virtual VOID OnCreateIOCP(VOID*pUserData)
    {//IOCP Thread Pool���ɹ�����,����������������г�ʼ���Լ�������

    }
    virtual VOID OnCloseIOCP(VOID*pUserData)
    {//IOCP Thread Pool�Ѿ��ر�,������������������ͷ��Լ���������Դ

    }
    virtual VOID OnIOComplete(DWORD dwTransBytes,LPOVERLAPPED pOL)
    {//IO��ɴ���
        //��������������������������CONTAINING_RECORD,��OVERLAPPED�ṹָ��õ��Զ���ṹ��ָ��
        //ST_MY_OVERLAPPED*  pMYOL = CONTAINING_RECORD(pOL,ST_MY_OVERLAPPED,m_OL);
    }
    virtual VOID OnIOError(DWORD dwErrorCode,DWORD dwTransBytes,LPOVERLAPPED pOL)
    {//������
        //��������������������������CONTAINING_RECORD,��OVERLAPPED�ṹָ��õ��Զ���ṹ��ָ��
        //ST_MY_OVERLAPPED*  pMYOL = CONTAINING_RECORD(pOL,ST_MY_OVERLAPPED,m_OL);
    }

	virtual VOID OnWaitTimeOut(DWORD dwWaitTime)
    {//GetQueuedCompletionStatus�ȴ���ʱ
    }

protected:
	HANDLE m_hIOCP; //IOCP ���
	DWORD m_dwConcurrent; // ��󲢷��߳���
	DWORD m_dwThreadCnt; // �̳߳���ʵ���߳���

	//����������������ͬ��id�;����Ӧͬһ���߳�
	HANDLE* m_hThreadAry; // ��ǰ�߳̾������
	DWORD* m_nThreadIDAry; //��ǰ�߳�ID����

	DWORD   m_dwWaitTime; //�ȴ���ʱʱ��
};

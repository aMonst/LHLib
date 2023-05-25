//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：CLHIOCPBase 类的封装，该类作为后续IOCP Tcp 操作的基础类
//模块名称：网络模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-6-03 09:53:16
//修改日志:
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

	// 获取最大并发线程数
	DWORD GetConcurrent() const
	{
		return m_dwConcurrent;
	}

	// 获取线程池中最大线程数
	DWORD GetThreadCount() const
	{
		return m_dwThreadCnt;
	}

	//设置超时时间，返回原始的超时时间
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
		//判定IOCP线程池是否正常有效的方法,该函数为虚函数,派生类可以添加自己的判定在里面
		return (m_hIOCP != NULL && m_hIOCP != INVALID_HANDLE_VALUE && m_dwThreadCnt > 0 && m_hThreadAry != NULL && m_nThreadIDAry != NULL);
	}

	BOOL Create(LPVOID pUserData, DWORD dwConcurrent = 0, DWORD dwThreadCnt = 0, LPSECURITY_ATTRIBUTES lpSAThread = NULL)
	{
		BOOL bRet = TRUE;
		try
		{
			CLHSystemInfo& sysInfo = CLHSystemInfo::GetSystemInfo();
			// 最大并发线程数默认是CPU核数
			if (0 == dwConcurrent)
			{
				m_dwConcurrent = sysInfo.GetSysInfo()->dwNumberOfProcessors;
			}
			else
			{
				m_dwConcurrent = dwConcurrent;
			}

			// 最大线程数，默认是CPU核数的两倍
			if (0 == dwThreadCnt)
			{
				m_dwThreadCnt = 2 * sysInfo.GetSysInfo()->dwNumberOfProcessors;
			}
			else
			{
				m_dwThreadCnt = dwThreadCnt;
			}

			//调整最大并发数不能超过池中的最大线程数
			m_dwConcurrent = min(m_dwThreadCnt, m_dwConcurrent);

			//创建IOCP对象
			m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)this, m_dwConcurrent);
			if (NULL == m_hIOCP)
			{
				throw CLHException(GetLastError());
			}

			//创建线程池
			if (!BeginThreadPool(lpSAThread))
			{
				throw CLHException(L"IOCP线程池线程创建失败,无法继续创建对象!\n");
			}

			//调用虚函数，后续派生类可以自己选择重载并实现自己的一些操作
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
		// 已经是关闭状态，直接返回TRUE
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
			//创建线程
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

	//线程池函数
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
							//等待超时
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
						LH_DBGOUT(_T("IOCP线程[ID:%u]接到退出消息......\n"), GetCurrentThreadId());
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
    {//IOCP Thread Pool被成功创建,派生类在这个方法中初始化自己的数据

    }
    virtual VOID OnCloseIOCP(VOID*pUserData)
    {//IOCP Thread Pool已经关闭,派生类在这个方法中释放自己创建的资源

    }
    virtual VOID OnIOComplete(DWORD dwTransBytes,LPOVERLAPPED pOL)
    {//IO完成处理
        //派生类中类似像下面这样调用CONTAINING_RECORD,从OVERLAPPED结构指针得到自定义结构的指针
        //ST_MY_OVERLAPPED*  pMYOL = CONTAINING_RECORD(pOL,ST_MY_OVERLAPPED,m_OL);
    }
    virtual VOID OnIOError(DWORD dwErrorCode,DWORD dwTransBytes,LPOVERLAPPED pOL)
    {//出错处理
        //派生类中类似像下面这样调用CONTAINING_RECORD,从OVERLAPPED结构指针得到自定义结构的指针
        //ST_MY_OVERLAPPED*  pMYOL = CONTAINING_RECORD(pOL,ST_MY_OVERLAPPED,m_OL);
    }

	virtual VOID OnWaitTimeOut(DWORD dwWaitTime)
    {//GetQueuedCompletionStatus等待超时
    }

protected:
	HANDLE m_hIOCP; //IOCP 句柄
	DWORD m_dwConcurrent; // 最大并发线程数
	DWORD m_dwThreadCnt; // 线程池中实际线程数

	//以下数组中索引相同的id和句柄对应同一个线程
	HANDLE* m_hThreadAry; // 当前线程句柄数组
	DWORD* m_nThreadIDAry; //当前线程ID数组

	DWORD   m_dwWaitTime; //等待超时时间
};

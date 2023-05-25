//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：CLHFile类定义文件 
//			CLHFile类是类库中关于Windows文件的封装类
//			CLHFile类带IOCP方式操作文件支持
//模块名称：文件管理
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-22 09:13:29
//修改日志:
//===================================================================================

#pragma once
#ifndef __LH__FILE_H__
#define __LH__FILE_H__
#include "LH_Win.h"
#include "LH_Def.h"
#include "LH_Debug.h"
#include "LHHeap.h"
#include "LHException.h"
#include "LHSystemInfo.h"

class CLHFile
{
	enum EM_FILE_OPERATOR
	{
		FOP_READ = 0x01,
		FOP_WRITE = 0x02,
		FOP_EXIT = 0x03
	};

	typedef struct _ST_LH_FILE_OVERLAPPED
	{
		OVERLAPPED m_ol; //overlapped 结构
		CLHFile* m_pThis; //指向文件对象的指针
		EM_FILE_OPERATOR m_OpType; //文件操作类型
		DWORD m_dwRW; //获取实际读写的字符数
		LPVOID m_pData; //IO操作的数据缓冲指针
		DWORD m_dataLen; //数据长度
		LPVOID m_pUserData; //调用者绑定在每个IO操作上的任意数据指针
	}ST_LH_FILE_OVERLAPPED, * LPST_LH_FILE_OVERLAPPED;
public:
	CLHFile() :
		m_hFile(INVALID_HANDLE_VALUE),
		m_bUseIOCP(FALSE),
		m_phIocpThread(NULL),
		m_Heap()
	{
		ZeroMemory(m_szFileName, sizeof(wchar_t) * MAX_PATH);
		m_FilePointer.QuadPart = 0;
	}

	CLHFile(LPCWSTR pFileName, BOOL bUseIOCP = TRUE) :
		m_hFile(INVALID_HANDLE_VALUE),
		m_bUseIOCP(bUseIOCP),
		m_phIocpThread(NULL),
		m_Heap()
	{
		ZeroMemory(m_szFileName, sizeof(wchar_t) * MAX_PATH);
		m_FilePointer.QuadPart = 0i64;
		Create(pFileName, m_bUseIOCP);
	}

	virtual ~CLHFile()
	{
		if (IsOpen())
		{
			Close();
		}
	}

protected:
	virtual LPST_LH_FILE_OVERLAPPED AllocOl()
	{
		return (LPST_LH_FILE_OVERLAPPED)m_Heap.Alloc(sizeof(ST_LH_FILE_OVERLAPPED));
	}

	virtual BOOL FreeOl(LPST_LH_FILE_OVERLAPPED pOl)
	{
		if (NULL != pOl)
		{
			return m_Heap.Free(pOl);
		}
		return TRUE;
	}

	BOOL BeginIOCP()
	{
		BOOL bRet = TRUE;
		CLHSystemInfo& si = CLHSystemInfo::GetSystemInfo();
		try
		{
			//创建IOCP对象
			m_hIOCP = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)this, si.GetSysInfo()->dwNumberOfProcessors);
			if (NULL == m_hIOCP)
			{
				LH_DBGOUT(_T("CLHFile::BeginIOCP() 调用 CreateIoCompletionPort() 失败\r\n"));
				throw CLHException(GetLastError());
			}

			// 分配线程句柄数组
			m_phIocpThread = (HANDLE*)m_Heap.Alloc(sizeof(HANDLE) * 2 * si.GetSysInfo()->dwNumberOfProcessors);

			// 启动线程
			for (unsigned int i = 0; i < 2 * si.GetSysInfo()->dwNumberOfProcessors; i++)
			{
				m_phIocpThread[i] = (HANDLE)_beginthreadex(NULL, 0, CLHFile::S_FileIOCPThread, m_hIOCP, CREATE_SUSPENDED, NULL);
				if (NULL == m_phIocpThread[i])
				{
					LH_DBGOUT(_T("CLHFile::BeginIOCP() 调用 _beginthreadex() 失败\r\n"));
					throw CLHException(GetLastError());
				}

				::ResumeThread(m_phIocpThread[i]);
			}
			// 绑定IOCP和线程
			if (NULL == ::CreateIoCompletionPort(m_hFile, m_hIOCP, NULL, 0))
			{
				LH_DBGOUT(_T("CLHFile::BeginIOCP() 调用 CreateIoCompletionPort() 绑定文件句柄失败\r\n"));
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			bRet = FALSE;
			LH_DBGOUT_EXPW(BeginIOCP, e);
			EndIOCP();
		}

		return bRet;
	}

	BOOL EndIOCP()
	{
		BOOL bRet = TRUE;
		CLHSystemInfo& si = CLHSystemInfo::GetSystemInfo();
		try
		{
			if (NULL != m_phIocpThread)
			{
				LPST_LH_FILE_OVERLAPPED pExitOl = NULL;
				DWORD dwActiveThread = 0;
				//1. 为每个激活的线程发送一个退出的消息
				for (DWORD i = 0; i < si.GetSysInfo()->dwNumberOfProcessors * 2; i++)
				{
					if (m_phIocpThread[i] != NULL)
					{
						pExitOl = AllocOl();
						pExitOl->m_pThis = this;
						pExitOl->m_OpType = FOP_EXIT;
						PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)this, (LPOVERLAPPED)pExitOl);
						++dwActiveThread;
					}
				}

				//2. 等待线程退出
				WaitForMultipleObjects(dwActiveThread, m_phIocpThread, TRUE, INFINITE);

				//3. 关闭线程句柄
				for (DWORD i = 0; i < dwActiveThread; i++)
				{
					if (NULL != m_phIocpThread[i])
					{
						CloseHandle(m_phIocpThread[i]);
					}
				}

				//4. 释放句柄数组
				m_Heap.Free(m_phIocpThread);
				m_phIocpThread = NULL;

				//5. 关闭IOCP句柄
				if (NULL != m_hIOCP)
				{
					CloseHandle(m_hIOCP);
					m_hIOCP = NULL;
				}
			}
		}

		catch (CLHException& e)
		{
			bRet = FALSE;
			LH_DBGOUT_EXPW(EndIOCP, e);
		}

		return bRet;
	}

public:
	BOOL Create(LPCWSTR lpszFileName, BOOL bIOCP)
	{
		BOOL bRet = TRUE;
		try
		{
			if (IsOpen())
			{
				Close();
			}

			ZeroMemory(m_szFileName, MAX_PATH * sizeof(wchar_t));
			if (FAILED(StringCchCopy(m_szFileName, MAX_PATH, lpszFileName)))
			{
				throw CLHException(L"文件名[%s]超过指定的长度[%d]", lpszFileName, MAX_PATH);
			}

			DWORD dwFlagAttribute = FILE_ATTRIBUTE_SYSTEM;
			m_bUseIOCP = bIOCP;
			if (m_bUseIOCP)
			{
				dwFlagAttribute |= FILE_FLAG_OVERLAPPED;
			}

			m_hFile = ::CreateFile(m_szFileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, dwFlagAttribute, NULL);
			if (INVALID_HANDLE_VALUE == m_hFile)
			{
				throw CLHException(GetLastError());
			}

			m_FilePointer.LowPart = ::GetFileSize(m_hFile, (DWORD*) & m_FilePointer.HighPart);
			if (0 == SetFilePointerEx(m_hFile, m_FilePointer, &m_FilePointer, FILE_BEGIN))
			{
				throw CLHException(GetLastError());
			}

			if (m_bUseIOCP)
			{
				if (!BeginIOCP())
				{
					throw CLHException(L"启动IOCP线程池失败!");
				}
			}

			OnCreate(m_szFileName);
		}
		catch (CLHException& e)
		{
			bRet = FALSE;
			LH_DBGOUT_EXPW(CLHFile::Create, e);
		}

		return bRet;
	}

	BOOL Close()
	{
		if (!IsOpen())
		{
			return TRUE;
		}

		BOOL bRet = TRUE;
		try
		{
			if (!CloseHandle(m_hFile))
			{
				throw CLHException(GetLastError());
			}

			m_hFile = INVALID_HANDLE_VALUE;

			if (m_bUseIOCP)
			{
				if (!EndIOCP())
				{
					throw CLHException(L"关闭IOCP线程失败!");
				}
			}

			OnClose(m_szFileName);
		}
		catch (CLHException& e)
		{
			bRet = FALSE;
			LH_DBGOUT_EXPW(CLHFile::Close, e);
		}

		return bRet;
	}

	BOOL IsOpen() const
	{
		return INVALID_HANDLE_VALUE != m_hFile;
	}

	LPCWSTR GetFileName() const
	{
		return m_szFileName;
	}

protected:
	//派生类需要重载这些方法以响应完成通知或别的通知
	//通常派生类在这些方法中删除缓冲
	virtual void OnCreate(LPCWSTR pszFileName)
	{
	}

	virtual void OnClose(LPCWSTR pszFileName)
	{
	}

	virtual BOOL OnWrite(LPCWSTR pszFileName, void* pData, DWORD dwLen, DWORD dwTransLen, void* pUseData)
	{
		return TRUE;
	}

	virtual BOOL OnRead(LPCWSTR pszFileName, void* pData, DWORD dwLen, DWORD dwTransLen, void* pUseData)
	{
		return TRUE;
	}

	virtual void OnError(LPCWSTR pszFileName, DWORD dwErrCode, UINT nOpType, void* pData, DWORD dwLen, DWORD dwTransLen, void* pUseData)
	{
	}

public:
	BOOL Write(const void* pData, DWORD dwLen, void* pUseData = NULL)
	{
		BOOL bRet = TRUE;
		LPST_LH_FILE_OVERLAPPED pOL = NULL;
		try
		{
			if (!IsOpen())
			{
				throw CLHException(L"文件对象还没有打开，无法进行写入操作！");
			}

			if (m_bUseIOCP)
			{
				pOL = AllocOl();
				pOL->m_dataLen = dwLen;
				pOL->m_OpType = FOP_WRITE;
				pOL->m_pData = const_cast<void*>(pData);
				pOL->m_pThis = this;
				pOL->m_pUserData = pUseData;

				//使用lock-free CAS原子方法控制文件指针
				*((LONGLONG*)&pOL->m_ol.Pointer) = InterlockedCompareExchange64(
					&m_FilePointer.QuadPart, m_FilePointer.QuadPart + dwLen, m_FilePointer.QuadPart);

				bRet = WriteFile(m_hFile, pData, dwLen, &pOL->m_dwRW, (LPOVERLAPPED)pOL);
			}
			else
			{
				bRet = WriteFile(m_hFile, pData, dwLen, &dwLen, NULL);
			}

			if (!bRet && ERROR_IO_PENDING != GetLastError())
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			bRet = FALSE;
			LH_DBGOUT_EXPW(CLHFile::Write, e);
			FreeOl(pOL);
		}

		return bRet;
	}

	BOOL Read(void* pData, DWORD dwBufSize, void* pUseData = NULL)
	{
		BOOL bRet = TRUE;
		LPST_LH_FILE_OVERLAPPED pOL = NULL;
		try
		{
			if (!IsOpen())
			{
				throw CLHException(L"文件还没有打开，无法进行写入操作！");
			}

			if (m_bUseIOCP)
			{
				pOL = AllocOl();
				pOL->m_pThis = this;
				pOL->m_dataLen = dwBufSize;
				pOL->m_pData = pData;
				pOL->m_OpType = FOP_READ;
				pOL->m_pUserData = pUseData;

				*((LONGLONG*)&pOL->m_ol.Pointer) = InterlockedCompareExchange64(&m_FilePointer.QuadPart, m_FilePointer.QuadPart + dwBufSize, m_FilePointer.QuadPart);
				bRet = ReadFile(m_hFile, pData, dwBufSize, &pOL->m_dwRW, (LPOVERLAPPED)pOL);
			}
			else
			{
				bRet = ReadFile(m_hFile, pData, dwBufSize, &dwBufSize, NULL);
			}
			if (!bRet && ERROR_IO_PENDING != GetLastError())
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			bRet = FALSE;
			LH_DBGOUT_EXPW(CLHFile::Read, e);
		}

		return bRet;
	}

protected:
	static unsigned CALLBACK S_FileIOCPThread(void* pParam)
	{
		OVERLAPPED* lpOverlapped = NULL;
		LPST_LH_FILE_OVERLAPPED pOL = NULL;
		CLHFile* pThis = NULL;
		HANDLE hIOCP = (HANDLE)pParam;
		try
		{
			BOOL bRet = FALSE;
			BOOL bLoop = TRUE;
			DWORD dwTrans = 0;
			ULONG_PTR uKey = 0L;

			while (bLoop)
			{
				try
				{
					bRet = GetQueuedCompletionStatus(hIOCP, &dwTrans, (PULONG_PTR)&uKey, &lpOverlapped, INFINITE);
					if (NULL == lpOverlapped)
					{
						throw CLHException(GetLastError());
					}

					LH_ASSERT(NULL != lpOverlapped);
					pOL = CONTAINING_RECORD(lpOverlapped, ST_LH_FILE_OVERLAPPED, m_ol);
					LH_ASSERT(NULL != pOL);

					pThis = pOL->m_pThis;
					if (!bRet)
					{
						LH_DBGOUT(_T("CLHFile:GetQueuedCompletionStatus调用失败,错误码: 0x%08x 内部错误码[0x%08x]\n"), GetLastError(), lpOverlapped->Internal);
						pThis->OnError(pThis->m_szFileName, GetLastError(), pOL->m_OpType, pOL->m_pData, pOL->m_dataLen, dwTrans, pOL->m_pUserData);
						throw(CLHException(GetLastError()));
					}

					LH_ASSERT(pThis != NULL);
					switch (pOL->m_OpType)
					{
					case FOP_WRITE:
						{
							pThis->OnWrite(pThis->m_szFileName, pOL->m_pData, pOL->m_dataLen, dwTrans, pOL->m_pUserData);
						}
						break;
					case FOP_READ:
						{
							pThis->OnRead(pThis->m_szFileName, pOL->m_pData, pOL->m_dataLen, dwTrans, pOL->m_pUserData);
						}
						break;
					case FOP_EXIT:
						{
							LH_DBGOUT(_T("IOCP线程[0x%x]得到退出通知,IOCP线程退出\n"), GetCurrentThreadId());
							bLoop = FALSE;
						}
						break;
					default:
						break;
					}
				}
				catch (CLHException& e)
				{
					LH_DBGOUT_EXPW(CLHFile::S_FileIOCPThread, e);
				}

				pThis->FreeOl(pOL);
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHFile::S_FileIOCPThread, e);
		}

		return 0;
	}
protected:
	wchar_t m_szFileName[MAX_PATH]; //存储文件名
	HANDLE m_hFile; //文件对象句柄
	BOOL m_bUseIOCP; //是否使用完成端口模型
	HANDLE m_hIOCP; //IOCP 句柄
	LARGE_INTEGER m_FilePointer; //文件指针
	HANDLE* m_phIocpThread; //IOCP线程句柄数组
	CLHHeap m_Heap; //存储ST_LH_FILE_OVERLAPPED 结构的堆
};
#endif

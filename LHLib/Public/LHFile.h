//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�������CLHFile�ඨ���ļ� 
//			CLHFile��������й���Windows�ļ��ķ�װ��
//			CLHFile���IOCP��ʽ�����ļ�֧��
//ģ�����ƣ��ļ�����
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-5-22 09:13:29
//�޸���־:
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
		OVERLAPPED m_ol; //overlapped �ṹ
		CLHFile* m_pThis; //ָ���ļ������ָ��
		EM_FILE_OPERATOR m_OpType; //�ļ���������
		DWORD m_dwRW; //��ȡʵ�ʶ�д���ַ���
		LPVOID m_pData; //IO���������ݻ���ָ��
		DWORD m_dataLen; //���ݳ���
		LPVOID m_pUserData; //�����߰���ÿ��IO�����ϵ���������ָ��
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
			//����IOCP����
			m_hIOCP = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)this, si.GetSysInfo()->dwNumberOfProcessors);
			if (NULL == m_hIOCP)
			{
				LH_DBGOUT(_T("CLHFile::BeginIOCP() ���� CreateIoCompletionPort() ʧ��\r\n"));
				throw CLHException(GetLastError());
			}

			// �����߳̾������
			m_phIocpThread = (HANDLE*)m_Heap.Alloc(sizeof(HANDLE) * 2 * si.GetSysInfo()->dwNumberOfProcessors);

			// �����߳�
			for (unsigned int i = 0; i < 2 * si.GetSysInfo()->dwNumberOfProcessors; i++)
			{
				m_phIocpThread[i] = (HANDLE)_beginthreadex(NULL, 0, CLHFile::S_FileIOCPThread, m_hIOCP, CREATE_SUSPENDED, NULL);
				if (NULL == m_phIocpThread[i])
				{
					LH_DBGOUT(_T("CLHFile::BeginIOCP() ���� _beginthreadex() ʧ��\r\n"));
					throw CLHException(GetLastError());
				}

				::ResumeThread(m_phIocpThread[i]);
			}
			// ��IOCP���߳�
			if (NULL == ::CreateIoCompletionPort(m_hFile, m_hIOCP, NULL, 0))
			{
				LH_DBGOUT(_T("CLHFile::BeginIOCP() ���� CreateIoCompletionPort() ���ļ����ʧ��\r\n"));
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
				//1. Ϊÿ��������̷߳���һ���˳�����Ϣ
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

				//2. �ȴ��߳��˳�
				WaitForMultipleObjects(dwActiveThread, m_phIocpThread, TRUE, INFINITE);

				//3. �ر��߳̾��
				for (DWORD i = 0; i < dwActiveThread; i++)
				{
					if (NULL != m_phIocpThread[i])
					{
						CloseHandle(m_phIocpThread[i]);
					}
				}

				//4. �ͷž������
				m_Heap.Free(m_phIocpThread);
				m_phIocpThread = NULL;

				//5. �ر�IOCP���
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
				throw CLHException(L"�ļ���[%s]����ָ���ĳ���[%d]", lpszFileName, MAX_PATH);
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
					throw CLHException(L"����IOCP�̳߳�ʧ��!");
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
					throw CLHException(L"�ر�IOCP�߳�ʧ��!");
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
	//��������Ҫ������Щ��������Ӧ���֪ͨ����֪ͨ
	//ͨ������������Щ������ɾ������
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
				throw CLHException(L"�ļ�����û�д򿪣��޷�����д�������");
			}

			if (m_bUseIOCP)
			{
				pOL = AllocOl();
				pOL->m_dataLen = dwLen;
				pOL->m_OpType = FOP_WRITE;
				pOL->m_pData = const_cast<void*>(pData);
				pOL->m_pThis = this;
				pOL->m_pUserData = pUseData;

				//ʹ��lock-free CASԭ�ӷ��������ļ�ָ��
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
				throw CLHException(L"�ļ���û�д򿪣��޷�����д�������");
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
						LH_DBGOUT(_T("CLHFile:GetQueuedCompletionStatus����ʧ��,������: 0x%08x �ڲ�������[0x%08x]\n"), GetLastError(), lpOverlapped->Internal);
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
							LH_DBGOUT(_T("IOCP�߳�[0x%x]�õ��˳�֪ͨ,IOCP�߳��˳�\n"), GetCurrentThreadId());
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
	wchar_t m_szFileName[MAX_PATH]; //�洢�ļ���
	HANDLE m_hFile; //�ļ�������
	BOOL m_bUseIOCP; //�Ƿ�ʹ����ɶ˿�ģ��
	HANDLE m_hIOCP; //IOCP ���
	LARGE_INTEGER m_FilePointer; //�ļ�ָ��
	HANDLE* m_phIocpThread; //IOCP�߳̾������
	CLHHeap m_Heap; //�洢ST_LH_FILE_OVERLAPPED �ṹ�Ķ�
};
#endif

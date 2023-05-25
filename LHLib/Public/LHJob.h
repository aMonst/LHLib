//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�������CLHJob�ඨ���ļ� CLHJob��������й���Windows��ҵ����ķ�װ��
//			ʵ��ʵ����,����ҵ�������Ϊһ�����̳�
//ģ�����ƣ���ҵ����ģ��
//
//��֯����˾�����ţ����ƣ�aMonst
//���ߣ�aMonst
//�������ڣ�2015-5-22 14:25:45
//�޸���־:
//===================================================================================
#pragma once
#ifndef __LH_JOB_H__
#define __LH_JOB_H__
#define LH_JOBIOCP_EXITMSGID  0xFFFF
#include <atlcoll.h>
#include "LH_Mem.h"
#include "LHThread.h"
#include "LHProcess.h"

class CLHJob : public CLHThread
{
public:
	CLHJob() :
		CLHThread(),
		m_hJob(NULL),
		m_hIOCP(NULL)
	{

	}

	virtual ~CLHJob()
	{

	}
protected:
	BOOL InitIOCP()
	{
		// ������ɶ˿�
		LH_ASSERT(NULL != m_hJob);
		BOOL bRet = FALSE;
		try 
		{
			m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
			if (NULL == m_hIOCP)
			{
				throw CLHException(GetLastError());
			}

			// ����ҵ�����IOCP��
			JOBOBJECT_ASSOCIATE_COMPLETION_PORT jobiocp = {};
			jobiocp.CompletionKey = m_hJob;
			jobiocp.CompletionPort = m_hIOCP;
			if (!SetInformationJobObject(m_hJob, JobObjectAssociateCompletionPortInformation, &jobiocp, sizeof(JOBOBJECT_ASSOCIATE_COMPLETION_PORT)))
			{
				throw CLHException(GetLastError());
			}

			// �����߳�
			CreateThread();
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::InitIOCP, e);
			bRet = FALSE;
			ExitIOCP();
		}

		return bRet;
	}

	BOOL ExitIOCP()
	{
		if (NULL == m_hIOCP)
		{
			return TRUE;
		}

		BOOL bRet = TRUE;
		try
		{
			if (!PostQueuedCompletionStatus(m_hIOCP, LH_JOBIOCP_EXITMSGID, (ULONG_PTR)m_hJob, NULL))
			{
				throw CLHException(GetLastError());
			}
			//�ȴ��߳��Ƴ�
			Wait(INFINITE);
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::ExitIOCP, e);
			bRet = FALSE;
		}
		return bRet;
	}

	void ClearProcInfo()
	{
		DWORD dwProcID = 0;
		PROCESS_INFORMATION* pProcInfo = NULL;
		for (POSITION pos = m_mapID2Proc.GetStartPosition(); pos != NULL;)
		{
			m_mapID2Proc.GetNextAssoc(pos, dwProcID, pProcInfo);
			if (NULL != pProcInfo)
			{
				CloseHandle(pProcInfo->hThread);
				CloseHandle(pProcInfo->hProcess);
				::HeapFree(GetProcessHeap(), 0, pProcInfo);
				pProcInfo = NULL;
			}
		}

		m_mapID2Proc.RemoveAll();
	}

public:
	BOOL CreateJob()
	{
		try
		{
			m_hJob = CreateJobObject(NULL, NULL);
			if (NULL == m_hJob)
			{
				throw CLHException(GetLastError());
			}

			if (!InitIOCP())
			{
				throw CLHException(L"�������ڼ���Job �����¼���IOCP����ʧ��");
			}

			//����Ϊ��Ĭ��ʽ
			SetSilent();
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::CreateJob, e);
			DestroyJob();
		}

		return (NULL != m_hIOCP && NULL != m_hJob);
	}

	BOOL DestroyJob()
	{
		try
		{
			if (NULL != m_hJob)
			{
				CloseHandle(m_hJob);
				m_hJob = NULL;
			}

			ExitIOCP();
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::DestroyJob, e);
		}

		return (NULL == m_hJob && NULL == m_hIOCP);
	}

protected:
	// ��ҵ�����л�߳����ﵽ����
	virtual void OnJobMsgActiveProcessLimit()
	{
#if(defined DEBUG) || (defined _DEBUG)
		LH_DBGOUT(_T("��ҵ�����л�������ﵽ����\n"));
#endif
	}

	// ��ҵ����û�л����
	virtual void OnJobMsgActiveProcessZero()
	{
#if(defined DEBUG) || (defined _DEBUG)
		LH_DBGOUT(_T("��ҵ�����е�ǰû�л����\n"));
#endif
	}
	// ��ҵ����ľ�ָ����ʱ��Ƭ
	virtual void OnJobMsgEndOfJobTime()
	{
#if(defined DEBUG) || (defined _DEBUG)
		LH_DBGOUT(_T("��ҵ����ľ�ָ����ʱ��Ƭ\n"));
#endif
	}

	//��ҵ����ľ�ָ���ڴ�
	virtual void OnJobMsgJobMemoryLimit()
	{
#if(defined DEBUG) || (defined _DEBUG)
		LH_DBGOUT(_T("��ҵ����ľ�ָ�����ڴ�\n"));
#endif
	}

	// �����쳣�˳�
	virtual void OnJobMsgAbnormalExitProcess(LPPROCESS_INFORMATION& pProcInfo)
	{
		DWORD dwExitCode = 0;
		GetExitCodeProcess(pProcInfo->hProcess, &dwExitCode);
		LH_DBGOUT(_T("����[ID:%u]�쳣�˳�,�˳���:%u\n"), pProcInfo->dwProcessId, dwExitCode);
	}

	// ���̵�ʱ��Ƭ�ľ�
	virtual void OnJobMsgEndOfProcessTime(LPPROCESS_INFORMATION& pProcInfo)
	{
#if(defined DEBUG) || (defined _DEBUG)
		LH_DBGOUT(_T("����[ID:%u]�ľ�ʱ��Ƭ\n"), pProcInfo->dwProcessId);
#endif
	}

	// ���������˳�
	virtual void OnJobMsgExitProcess(LPPROCESS_INFORMATION& pProcInfo)
	{
#if(defined DEBUG) || (defined _DEBUG)
		LH_DBGOUT(_T("����[ID:%u]�����˳�\n"), pProcInfo->dwProcessId);
#endif
	}

	// ���̼�����ҵ����
	virtual void OnJobMsgNewProcess(LPPROCESS_INFORMATION& pProcInfo)
	{
#if(defined DEBUG) || (defined _DEBUG)
		LH_DBGOUT(_T("����[ID:%u]������ҵ����[h:0x%08X]\n"), pProcInfo->dwProcessId, m_hJob);
#endif
	}

	// ���������ڴ������ﵽ����
	virtual void OnJobMsgProcessMemoryLimit(LPPROCESS_INFORMATION& pProcInfo)
	{
#if(defined DEBUG) || (defined _DEBUG)
		LH_DBGOUT(_T("����[ID:%u]�����ڴ������ﵽ����\n"), pProcInfo->dwProcessId);
#endif
	}

	virtual UINT __stdcall Run(void* pParam) override
	{
		DWORD dwProcessID = 0;
		DWORD dwEvent = 0;
		BOOL bLoop = TRUE;
		ULONG_PTR uKey = 0;
		while (bLoop)
		{
			try
			{
				if (!GetQueuedCompletionStatus(m_hIOCP, &dwEvent, &uKey, (LPOVERLAPPED*)&dwEvent, INFINITE))
				{
					LH_DBGOUT(_T("IOCPThread: GetQueuedCompletionStatus ����ʧ��,������: 0x%08x \n"), GetLastError());
					continue;
				}

				switch (dwEvent)
				{
				case JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS:
					{
						LPPROCESS_INFORMATION pProcInfo = NULL;
						if (!m_mapID2Proc.Lookup(dwProcessID, pProcInfo) || NULL == pProcInfo)
						{
							throw CLHException(_T("ID[%u]�Ľ��̲�������ҵ������,���ܴ������¼�"), dwProcessID);
						}
						LH_ASSERT(dwProcessID == pProcInfo->dwProcessId);
						OnJobMsgAbnormalExitProcess(pProcInfo);
						m_mapID2Proc.RemoveKey(pProcInfo->dwProcessId);
						CloseHandle(pProcInfo->hThread);
						CloseHandle(pProcInfo->hProcess);
						HeapFree(GetProcessHeap(), 0, pProcInfo);
						pProcInfo = NULL;
					}
					break;
				case JOB_OBJECT_MSG_END_OF_PROCESS_TIME:
					{

						LPPROCESS_INFORMATION pProcInfo = NULL;
						if (!m_mapID2Proc.Lookup(dwProcessID, pProcInfo) || NULL == pProcInfo)
						{
							throw CLHException(_T("ID[%u]�Ľ��̲�������ҵ������,���ܴ������¼�"), dwProcessID);
						}
						LH_ASSERT(dwProcessID == pProcInfo->dwProcessId);
						OnJobMsgEndOfProcessTime(pProcInfo);
					}
					break;
				case JOB_OBJECT_MSG_EXIT_PROCESS:
					{
						LPPROCESS_INFORMATION pProcInfo = NULL;
						if (!m_mapID2Proc.Lookup(dwProcessID, pProcInfo) || NULL == pProcInfo)
						{
							throw CLHException(_T("ID[%u]�Ľ��̲�������ҵ������,���ܴ������¼�"), dwProcessID);
						}
						LH_ASSERT(dwProcessID == pProcInfo->dwProcessId);
						OnJobMsgExitProcess(pProcInfo);
						m_mapID2Proc.RemoveKey(pProcInfo->dwProcessId);
						CloseHandle(pProcInfo->hThread);
						CloseHandle(pProcInfo->hProcess);
						HeapFree(GetProcessHeap(), 0, pProcInfo);
						pProcInfo = NULL;
					}
					break;
				case JOB_OBJECT_MSG_NEW_PROCESS:
					{
						LPPROCESS_INFORMATION pProcInfo = NULL;
						if (!m_mapID2Proc.Lookup(dwProcessID, pProcInfo) || NULL == pProcInfo)
						{
							throw CLHException(_T("ID[%u]�Ľ��̲�������ҵ������,���ܴ������¼�"), dwProcessID);
						}
						LH_ASSERT(dwProcessID == pProcInfo->dwProcessId);
						OnJobMsgNewProcess(pProcInfo);
					}
					break;
				case JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT:
					{

						LPPROCESS_INFORMATION pProcInfo = NULL;
						if (!m_mapID2Proc.Lookup(dwProcessID, pProcInfo) || NULL == pProcInfo)
						{
							throw CLHException(_T("ID[%u]�Ľ��̲�������ҵ������,���ܴ������¼�"), dwProcessID);
						}
						LH_ASSERT(dwProcessID == pProcInfo->dwProcessId);
						OnJobMsgProcessMemoryLimit(pProcInfo);
					}
					break;
				case JOB_OBJECT_MSG_ACTIVE_PROCESS_LIMIT:
					{
						OnJobMsgActiveProcessLimit();
					}
					break;
				case JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO:
					{
						OnJobMsgActiveProcessZero();
					}
					break;
				case JOB_OBJECT_MSG_END_OF_JOB_TIME:
					{
						OnJobMsgEndOfJobTime();
					}
					break;
				case JOB_OBJECT_MSG_JOB_MEMORY_LIMIT:
					{
						OnJobMsgJobMemoryLimit();
					}
					break;
				case LH_JOBIOCP_EXITMSGID:
					{
						LH_DBGOUT(_T("Job Object(class[0x%08x] handle[0x%08x])��IOCP�¼��߳�(ID:0x%x)�ӵ��˳���Ϣ\n"), this, m_hJob, GetCurrentThreadId());
						bLoop = FALSE;
					}
					break;
				default:
					{
						LH_DBGOUT(_T("Job Object(class[0x%08x] handle[0x%08x])��IOCP�¼��߳�(ID:0x%x)�ӵ�һ��δ֪��ϢID:%u,���������Windowsƽ̨�ĵ���������\n")
							, this, m_hJob, GetCurrentThreadId(), dwEvent);
					}
					break;
				}

			}
			catch (CLHException& e)
			{
				LH_DBGOUT_EXPW(CLHJob::Run_While, e);
			}
		}

		return 0;
	}
public:
	// ��ӽ��̵����̳�
	BOOL AddProcess(DWORD dwProcessID)
	{
		BOOL bRet = TRUE;
		PROCESS_INFORMATION* pProcInfo = (PROCESS_INFORMATION*)LH_CALLOC(sizeof(pProcInfo));
		try 
		{
			if (NULL == pProcInfo)
			{
				throw CLHException(GetLastError());
			}

			if ((pProcInfo->hProcess = CLHProcess::GetProcessHanleByPID(dwProcessID)) == NULL)
			{
				throw CLHException(L"���� CLHProcess::GetProcessHandleByPID ��ȡ����idʧ��");
			}
			if (!CLHProcess::GetProcessInformationByID(dwProcessID, *pProcInfo))
			{
				throw CLHException(L"���� CLHProcess::GetProcessInformationByID ��ȡ������Ϣʧ��");
			}

			m_mapID2Proc.SetAt(pProcInfo->dwProcessId, pProcInfo);
			//�����̼��뵽��ҵ������
			if (!AssignProcessToJobObject(m_hJob, pProcInfo->hProcess))
			{
				throw CLHException(GetLastError());
			}

			return TRUE;
		}
		catch (CLHException& e)
		{
			if(NULL != pProcInfo)
			{ 
				m_mapID2Proc.RemoveKey(pProcInfo->dwProcessId);
				::HeapFree(GetProcessHeap(), 0, pProcInfo);
				pProcInfo = NULL;
			}

			bRet = FALSE;
			LH_DBGOUT_EXPW(CLHJob::AddProcess, e);
		}

		return bRet;
	}

	BOOL AddProcess(HANDLE hProcess)
	{
		PROCESS_INFORMATION* pProcInfo = (PROCESS_INFORMATION*)LH_CALLOC(sizeof(pProcInfo));
		BOOL bRet = TRUE;
		try
		{

			if (NULL == pProcInfo)
			{
				throw CLHException(GetLastError());
			}

			pProcInfo->hProcess = hProcess;
			if ((pProcInfo->dwProcessId = GetProcessId(hProcess)) == 0)
			{
				throw CLHException(GetLastError());
			}
			
			if (!CLHProcess::GetProcessInformationByHandle(hProcess, *pProcInfo))
			{
				throw CLHException(L"���� CLHProcess::GetProcessInformationByHandle ��ȡ������Ϣʧ��!");
			}

			m_mapID2Proc.SetAt(pProcInfo->dwProcessId, pProcInfo);
			//�����̼��뵽��ҵ������
			if (!AssignProcessToJobObject(m_hJob, pProcInfo->hProcess))
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			if (NULL != pProcInfo)
			{
				m_mapID2Proc.RemoveKey(pProcInfo->dwProcessId);
				::HeapFree(GetProcessHeap(), 0, pProcInfo);
				pProcInfo = NULL;
			}
			bRet = FALSE;
			LH_DBGOUT_EXPW(CLHJob::AddProcess, e);
		}

		return bRet;
	}

	BOOL AddProcess(LPCTSTR pszExePathName, LPTSTR lpCommandLine = NULL, DWORD dwFlags = 0)
	{
		LH_ASSERT(IsValid());
		PROCESS_INFORMATION* pProcInfo = NULL;
		BOOL bRet = TRUE;
		try
		{
			pProcInfo = (PROCESS_INFORMATION*)LH_CALLOC(sizeof(PROCESS_INFORMATION));
			if (pProcInfo == NULL)
			{
				throw CLHException(GetLastError());
			}
			STARTUPINFO si = { 0 };
			si.cb = sizeof(STARTUPINFO);
			dwFlags |= CREATE_SUSPENDED | CREATE_BREAKAWAY_FROM_JOB;
			if (!CreateProcess(pszExePathName, lpCommandLine, NULL, NULL, FALSE
				, dwFlags, NULL, NULL, &si, pProcInfo))
			{
				throw CLHException(GetLastError());
			}

			m_mapID2Proc.SetAt(pProcInfo->dwProcessId, pProcInfo);

			//�����̼��뵽��ҵ������
			if (!AssignProcessToJobObject(m_hJob, pProcInfo->hProcess))
			{
				throw CLHException(GetLastError());
				bRet = FALSE;
			}
			//�������̵����߳�,ʵ��Ҳ������ʽ�����������
			::ResumeThread(pProcInfo->hThread);
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::AddProcess_3, e);
			if (NULL != pProcInfo)
			{
				if (NULL != pProcInfo->hProcess)
				{
					TerminateProcess(pProcInfo->hProcess, 0);
				}

				m_mapID2Proc.RemoveKey(pProcInfo->dwProcessId);
				HeapFree(GetProcessHeap(), 0, pProcInfo);
				pProcInfo = NULL;
			}

			bRet = FALSE;
		}

		return NULL != pProcInfo;
	}

	BOOL KillAllProcess(UINT nExitCode = -1)
	{
		BOOL bRet = TRUE;
		try
		{
			if (!TerminateJobObject(m_hJob, nExitCode))
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::KillAllProcess, e);
			bRet = FALSE;
		}

		return bRet;
	}

	BOOL IsValid()
	{
		return NULL != m_hJob && NULL != m_hIOCP;
	}

	//����PostThreadMessage��װ��һ������̷�����Ϣ�ķ���,ʵ�ʾ�������̵����̷߳�����Ϣ
	BOOL PostProcessMessage(DWORD dwProcessID, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		BOOL bRet = TRUE;
		try
		{
			PROCESS_INFORMATION* pProcInfo = NULL;
			if (!m_mapID2Proc.Lookup(dwProcessID, pProcInfo) || NULL == pProcInfo)
			{
				throw CLHException(L"����[%d] ������ҵ�����У��޷�������Ϣ!", dwProcessID);
			}

			bRet = ::PostThreadMessage(pProcInfo->dwThreadId, Msg, wParam, lParam);
			if (!bRet)
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::PostProcessMessage, e);
			bRet = FALSE;
		}

		return bRet;
	}

	//������ҵ������ͬʱ��Ľ��̵��������
	BOOL SetMaxActiveProcCnt(DWORD dwCnt)
	{
		LH_ASSERT(IsValid());
		BOOL bRet = TRUE;
		try
		{ 
			JOBOBJECT_BASIC_LIMIT_INFORMATION jbli = { 0 };
			jbli.LimitFlags = JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
			jbli.ActiveProcessLimit = dwCnt;
			if (!SetInformationJobObject(m_hJob, JobObjectBasicLimitInformation, &jbli, sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION)))
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::SetMaxActiveProcCnt, e);
			bRet = FALSE;
		}

		return bRet;
	}

	DWORD GetMaxActiveProcCnt()
	{
		LH_ASSERT(IsValid());
		DWORD dwCnt = 0;
		try
		{
			JOBOBJECT_BASIC_LIMIT_INFORMATION jbli = { 0 };
			jbli.LimitFlags = JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
			DWORD dwRetLen = 0;
			if (!QueryInformationJobObject(m_hJob, JobObjectBasicLimitInformation, &jbli, sizeof(jbli), &dwRetLen))
			{
				throw CLHException(GetLastError());
			}

			dwCnt = jbli.ActiveProcessLimit;
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::GetMaxActiveProcCnt, e);
			dwCnt = 0;
		}

		return dwCnt;
	}

	// ���ƽ������ĵ�ʱ��
	BOOL SetProcessUserTime(LARGE_INTEGER timeLimit)
	{
		LH_ASSERT(IsValid());
		BOOL bRet = TRUE;
		try
		{
			JOBOBJECT_BASIC_LIMIT_INFORMATION jbli = { 0 };
			jbli.LimitFlags = JOB_OBJECT_LIMIT_PROCESS_TIME;
			jbli.PerProcessUserTimeLimit = timeLimit;
			bRet = ::SetInformationJobObject(m_hJob, JobObjectBasicLimitInformation, &jbli, sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION));
			if (!bRet)
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::SetProcessUserTime, e);
			bRet = FALSE;
		}

		return bRet;
	}
	
	BOOL GetProcessUserTime(LARGE_INTEGER& limitTime)
	{
		LH_ASSERT(IsValid());
		try
		{
			DWORD dwLen = 0;
			JOBOBJECT_BASIC_LIMIT_INFORMATION jbli = { 0 };
			jbli.LimitFlags = JOB_OBJECT_LIMIT_PROCESS_TIME;
			BOOL bRet = ::QueryInformationJobObject(m_hJob, JobObjectBasicLimitInformation, &jbli, sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION), &dwLen);
			if (!bRet)
			{
				throw CLHException(GetLastError());
			}

			limitTime = jbli.PerProcessUserTimeLimit;
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::GetProcessUserTime, e);
			limitTime.QuadPart = 0;
			return FALSE;
		}

		return TRUE;
	}
	
	// ����������ҵ�������ĵ�ʱ��
	BOOL SetJobUserTime(LARGE_INTEGER timeLimit)
	{
		LH_ASSERT(IsValid());
		BOOL bRet = TRUE;
		try {
			JOBOBJECT_BASIC_LIMIT_INFORMATION jbli = { 0 };
			jbli.LimitFlags = JOB_OBJECT_LIMIT_JOB_TIME;
			jbli.PerJobUserTimeLimit = timeLimit;
			bRet = ::SetInformationJobObject(m_hJob, JobObjectBasicLimitInformation, &jbli, sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION));
			if (!bRet)
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::SetJobUserTime, e);
			bRet = FALSE;
		}

		return bRet;
	}

	BOOL GetJobUserTime(LARGE_INTEGER& limitTime)
	{
		LH_ASSERT(IsValid());
		BOOL bRet = TRUE;
		try
		{
			JOBOBJECT_BASIC_LIMIT_INFORMATION jbli = { 0 };
			jbli.LimitFlags = JOB_OBJECT_LIMIT_JOB_TIME;
			DWORD dwLen = 0;
			bRet = ::QueryInformationJobObject(m_hJob, JobObjectBasicLimitInformation, &jbli, sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION), &dwLen);
			if (!bRet)
			{
				throw CLHException(GetLastError());
			}
			limitTime = jbli.PerJobUserTimeLimit;
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::GetJobUserTime, e);
			limitTime.QuadPart = 0;
			bRet = FALSE;
		}

		return bRet;
	}

	// ����������ҵ�����д��ڵ���С������
	BOOL SetMinWorkingSetSize(size_t minWorkingSetSize)
	{
		BOOL bRet = TRUE;
		try 
		{
			JOBOBJECT_BASIC_LIMIT_INFORMATION jbli = { 0 };
			jbli.LimitFlags = JOB_OBJECT_LIMIT_WORKINGSET;
			jbli.MinimumWorkingSetSize = minWorkingSetSize;
			bRet = ::SetInformationJobObject(m_hJob, JobObjectBasicLimitInformation, &jbli, sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION));
			if (!bRet)
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::SetMinWorkingSetSize, e);
			bRet = FALSE;
		}
	}

	size_t GetMinWorkingSetSize()
	{
		size_t size = { 0 };
		try
		{
			JOBOBJECT_BASIC_LIMIT_INFORMATION jbli = { 0 };
			jbli.LimitFlags = JOB_OBJECT_LIMIT_WORKINGSET;
			DWORD dwLen = 0;
			if (::QueryInformationJobObject(m_hJob, JobObjectBasicLimitInformation, &jbli, sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION), &dwLen))
			{
				throw CLHException(GetLastError());
			}
			
			size = jbli.MinimumWorkingSetSize;
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::GetMinWorkingSetSize, e);
			size = 0;
		}

		return size;
	}

	BOOL SetMaxWorkingSetSize(SIZE_T maxSize)
	{
		BOOL bRet = TRUE;
		try 
		{
			JOBOBJECT_BASIC_LIMIT_INFORMATION jbli = { 0 };
			jbli.LimitFlags = JOB_OBJECT_LIMIT_WORKINGSET;
			jbli.MaximumWorkingSetSize = maxSize;
			bRet = ::SetInformationJobObject(m_hJob, JobObjectBasicLimitInformation, &jbli, sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION));
			if (!bRet)
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::SetMinWorkingSetSize, e);
			bRet = FALSE;
		}
	}

	size_t GetMaxWorkingSetSize()
	{
		size_t size = { 0 };
		try
		{
			JOBOBJECT_BASIC_LIMIT_INFORMATION jbli = { 0 };
			jbli.LimitFlags = JOB_OBJECT_LIMIT_WORKINGSET;
			DWORD dwLen = 0;
			if (::QueryInformationJobObject(m_hJob, JobObjectBasicLimitInformation, &jbli, sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION), &dwLen))
			{
				throw CLHException(GetLastError());
			}
			
			size = jbli.MaximumWorkingSetSize;
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::GetMinWorkingSetSize, e);
			size = 0;
		}

		return size;
	}
	// ��Job�еĽ�������Ϊ��Ĭ��ʽ, ���������쳣��ֹ�Ի���
	BOOL SetSilent()
	{
		LH_ASSERT(IsValid());
		BOOL bRet = TRUE;
		try {
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
			jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION;
			bRet = ::SetInformationJobObject(m_hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));
			if (!bRet)
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::SetSilent, e);
		}

		return bRet;
	}

	size_t GetProcessCnt()
	{
		return m_mapID2Proc.GetCount();
	}

	POSITION GetFirstProcess()
	{
		return m_mapID2Proc.GetStartPosition();
	}

	PROCESS_INFORMATION* GetNextProcess(POSITION& pos)
	{
		if (NULL == pos)
		{
			return NULL;
		}

		DWORD dwProcessID = 0;
		PROCESS_INFORMATION* pProcInfo = NULL;
		m_mapID2Proc.GetNextAssoc(pos, dwProcessID, pProcInfo);
		return pProcInfo;
	}
protected:
	typedef CAtlMap<DWORD, PROCESS_INFORMATION*> CMapID2Process;
	HANDLE m_hJob; // ��ҵ����ľ��
	HANDLE m_hIOCP; // ������ҵ�¼�֪ͨ��IOCP����
	CMapID2Process m_mapID2Proc; //����ID�Ͷ�Ӧ������������Ϣ�ṹ��ӳ���
};
#endif

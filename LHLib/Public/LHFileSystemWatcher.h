//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�������CLHFileSystemWatcher ��ķ�װ����Ҫ�ṩϵͳ�ļ�Ŀ¼��صĹ��ܣ������������ظ����һЩ֪ͨ�����������ļ�����¼�
//ģ�����ƣ��ļ�ģ��
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2023-5-30 14:34:47
//�޸���־:
//===================================================================================
#pragma once
#include "LH_Mem.h"
#include "LHIOCPBase.h"

class CLHFileSystemWatcher : public CLHIOCPBase
{
protected:
	typedef struct _ST_LH_FILESYSTEMWATCHER_OVERLAPPED 
	{
		OVERLAPPED m_overlapped;
		char notify[1024];
		DWORD dwTransBytes;
	}ST_LH_FILESYSTEMWATCHER_OVERLAPPED, *LPST_LH_FILESYSTEMWATCHER_OVERLAPPED;

public:
	// bWatchTree: �Ƿ�����Ŀ¼
	CLHFileSystemWatcher(LPCTSTR lpcWatchFolder, BOOL bWatchTree) :
		CLHIOCPBase(),
		m_bWatchTree(bWatchTree)
	{
		ZeroMemory(m_szFileName, sizeof(m_szFileName));
		StringCchCopy(m_szFileName, MAX_PATH, lpcWatchFolder);
		// ���Ŀ¼ֻ��һ���̼߳���
		Create(m_szFileName, 1, 1, NULL);
	}

	virtual ~CLHFileSystemWatcher()
	{
		Close(m_pFswOL);
	}

protected:
	bool StartWatcher(LPCTSTR lpFolder)
	{
		bool bRet = TRUE;
		try
		{
			m_hHandle = CreateFile(m_szFileName,
				GENERIC_READ | GENERIC_WRITE | FILE_LIST_DIRECTORY,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
				NULL);

			if (INVALID_HANDLE_VALUE == m_hHandle)
			{
				throw CLHException(GetLastError());
			}

			AddHandle2IOCP(m_hHandle);
			m_pFswOL = (LPST_LH_FILESYSTEMWATCHER_OVERLAPPED)LH_CALLOC(sizeof(ST_LH_FILESYSTEMWATCHER_OVERLAPPED));

			if (!ReadDirectoryChangesW(m_hHandle,
				m_pFswOL->notify,
				sizeof(m_pFswOL->notify),
				m_bWatchTree,
				FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
				FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
				FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS |
				FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY,
				&m_pFswOL->dwTransBytes, &m_pFswOL->m_overlapped, NULL))
			{
				if (INVALID_HANDLE_VALUE != m_hHandle)
				{
					CloseHandle(m_hHandle);
					m_hHandle = NULL;
				}
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHFileSystemWatcher::StartWatcher, e);
			bRet = FALSE;
		}

		return bRet;
	}

	LPCTSTR GetWatchFolder() const
	{
		return m_szFileName;
	}

protected:
	virtual VOID OnCreateIOCP(VOID*pUserData)
    {
		LH_ASSERT(NULL != pUserData);
		StartWatcher((LPCTSTR)pUserData);
    }

	virtual VOID OnCloseIOCP(VOID*pUserData)
    {
		if (m_hHandle != NULL)
		{
			CloseHandle(m_hHandle);
			m_hHandle = NULL;
		}

		LH_FREE(pUserData);
    }
    virtual VOID OnIOComplete(DWORD dwTransBytes,LPOVERLAPPED pOL)
    {
		LPST_LH_FILESYSTEMWATCHER_OVERLAPPED pFswOL = CONTAINING_RECORD(pOL, ST_LH_FILESYSTEMWATCHER_OVERLAPPED, m_overlapped);
		if (NULL != pFswOL)
		{
			FILE_NOTIFY_INFORMATION* pnotify = (FILE_NOTIFY_INFORMATION*)(pFswOL->notify);
			while (true)
			{
				switch (pnotify->Action)
				{
				case FILE_ACTION_ADDED:
					OnFileAdded(pnotify->FileName, pnotify->FileNameLength / sizeof(TCHAR));
					break;
				case FILE_ACTION_REMOVED:
					OnFileRemoved(pnotify->FileName, pnotify->FileNameLength / sizeof(TCHAR));
					break;
				case FILE_ACTION_MODIFIED:
					OnFileModified(pnotify->FileName, pnotify->FileNameLength / sizeof(TCHAR));
					break;
				case FILE_ACTION_RENAMED_OLD_NAME:
					OnFileRenamedOldName(pnotify->FileName, pnotify->FileNameLength / sizeof(TCHAR));
					break;
				case FILE_ACTION_RENAMED_NEW_NAME:
					OnFileRenamedNewName(pnotify->FileName, pnotify->FileNameLength / sizeof(TCHAR));
					break;
				default:
					break;
				}

				if (pnotify->NextEntryOffset)
				{
					pnotify = (FILE_NOTIFY_INFORMATION*)((char*)pnotify + pnotify->NextEntryOffset);
				}else
				{
					break;
				}
			}

			ZeroMemory(pFswOL->notify, sizeof(pFswOL->notify));
			ReadDirectoryChangesW(m_hHandle, 
				pFswOL->notify, 
				sizeof(pFswOL->notify), 
				m_bWatchTree, 
				FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | 
				FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY, 
				&pFswOL->dwTransBytes, 
				&pFswOL->m_overlapped, 
				NULL);
		}
    }

	// �����ļ�
	virtual void OnFileAdded(LPCTSTR lpcFileName, DWORD dwNameLength)
	{
	}

	// �Ƴ��ļ�
	virtual void OnFileRemoved(LPCTSTR lpcFileName, DWORD dwNameLength)
	{
	}

	//�޸��ļ�
	virtual void OnFileModified(LPCTSTR lpcFileName, DWORD dwNameLength)
	{
	}

	// �������ļ�, �µ��ļ���
	virtual void OnFileRenamedNewName(LPCTSTR lpcFileName, DWORD dwNameLength)
	{
	}

	//�������ļ�, ԭ�����ļ���
	virtual void OnFileRenamedOldName(LPCTSTR lpcFileName, DWORD dwNameLength)
	{
	}
protected:
	TCHAR m_szFileName[MAX_PATH];
	HANDLE m_hHandle;
	BOOL m_bWatchTree;
	LPST_LH_FILESYSTEMWATCHER_OVERLAPPED m_pFswOL;
};

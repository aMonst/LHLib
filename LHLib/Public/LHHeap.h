//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�������CLHHeap�ඨ���ļ� CLHHeap����LHLib����й���Windows�ѵķ�װ��
//ģ�����ƣ�����֧���ļ�
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-5-20 17:30:27
//�޸���־:
//===================================================================================

#pragma once
#ifndef __LH_HEAP_H__
#define __LH_HEAP_H__
#include "LH_Debug.h"
#ifdef LH_MEMORY_LEAK
#include <atlcoll.h>
#include <strsafe.h>
#endif

class CLHHeap
{
public:
	// bNoSerialize: �Ƿ�֧�ֶ��߳�
	// bLFH: �Ƿ��ǵ���Ƭ�ѣ�����Ƭ������Ч�����öѿռ�
	CLHHeap(BOOL bNoSerialize = FALSE, BOOL bLFH = TRUE) :
		m_dwOptions(HEAP_GENERATE_EXCEPTIONS),
		m_hHeap(NULL)
	{
		Create(bNoSerialize, bLFH);
	}
	virtual ~CLHHeap()
	{
#ifdef LH_MEMORY_LEAK
		DumpBlock();
#endif
		Destroy();
	}

	BOOL Create(BOOL bNoSerialize = FALSE, BOOL bLFH = TRUE, SIZE_T dwInitialSize = 0, SIZE_T dwMaximumSize = 0)
	{
		LH_ASSERT(m_hHeap == NULL);
		if (NULL != m_hHeap)
		{
			return TRUE;
		}

		if (!bNoSerialize)
		{
			// ����Ҫ�̰߳�ȫ
			m_dwOptions |= HEAP_NO_SERIALIZE;
		}

		m_hHeap = ::HeapCreate(m_dwOptions, dwInitialSize, dwMaximumSize);
		if (NULL == m_hHeap)
		{
			return FALSE;
		}

#ifdef LH_MEMORY_LEAK
		// �����ڴ���¼��Ϣ
		m_hBlockHeap = ::HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0, 0);
		ULONG uFlags = 2;
		::HeapSetInformation(m_hBlockHeap, HeapCompatibilityInformation, &uFlags, sizeof(ULONG));
#endif
		if (bLFH)
		{
			return EnableLFH();
		}

		return m_hHeap != NULL;
	}

	BOOL Destroy()
	{
		if (m_hHeap != NULL)
		{
			if (::HeapDestroy(m_hHeap))
			{
				m_hHeap = NULL;
				m_dwOptions = HEAP_GENERATE_EXCEPTIONS;
			}
		}

#ifdef LH_MEMORY_LEAK
		if (NULL != m_hBlockHeap)
		{
			::HeapDestroy(m_hBlockHeap);
			m_hBlockHeap = NULL;
		}
#endif

		return (m_hHeap == NULL);
	}

	BOOL HeapValidate()
	{
		//����ѵ�������
		LH_ASSERT(m_hHeap != NULL);
		return ::HeapValidate(m_hHeap, 0, NULL);
	}

	void Lock()
	{
		// �����ѣ������Ƕ��̶߳ѵ�ʱ��Ż���ô˷���
		LH_ASSERT(m_hHeap != NULL);
		if (NULL != m_hHeap && IsSerialize())
		{
			::HeapLock(m_hHeap);
		}
	}
	void Unlock()
	{
		// �����ѣ������Ƕ��̶߳�ʱ�Ż���ô˷���
		LH_ASSERT(m_hHeap != NULL);
		if (NULL != m_hHeap && IsSerialize())
		{
			::HeapUnlock(m_hHeap);
		}
	}

	BOOL HeapWalk(LPPROCESS_HEAP_ENTRY lpEntry)
	{
		LH_ASSERT(m_hHeap != NULL);
		return ::HeapWalk(m_hHeap, lpEntry);
	}

	BOOL IsSerialize()
	{
		LH_ASSERT(m_hHeap != NULL);
		return !(m_dwOptions & HEAP_NO_SERIALIZE);
	}

	BOOL EnableLFH()
	{
		//��LFH
		LH_ASSERT(NULL != m_hHeap);
		ULONG  ulHeapFragValue = 2;
		return ::HeapSetInformation(m_hHeap, HeapCompatibilityInformation, &ulHeapFragValue, sizeof(ulHeapFragValue));
	}
	BOOL DisableLFH()
	{
		// �ر�LFH
		LH_ASSERT(NULL != m_hHeap);
		ULONG  ulHeapFragValue = 0;
		return ::HeapSetInformation(m_hHeap, HeapCompatibilityInformation, &ulHeapFragValue, sizeof(ulHeapFragValue));
	}
	BOOL IsLFH()
	{
		LH_ASSERT(NULL != m_hHeap);
		ULONG ulHeapFragValue = 0;
		::HeapQueryInformation(m_hHeap, HeapCompatibilityInformation, &ulHeapFragValue, sizeof(ulHeapFragValue), NULL);
		return (2 == ulHeapFragValue);
	}
	BOOL SetHeapInformation(ULONG ulFlag)
	{
		//���ö���Ϣ,��ʵ����ָ���ѵ�����,ָ��2�ʹ���LFH��ָ��1��ʾ����һ��LAL ��, ָ��0��ʾ����һ����ͨ��
		LH_ASSERT(NULL != m_hHeap);
		return ::HeapSetInformation(m_hHeap, HeapCompatibilityInformation, &ulFlag, sizeof(ulFlag));
	}

	ULONG GetHeapInformation()
	{
		LH_ASSERT(NULL != m_hHeap);
		ULONG ulFlag;
		::HeapQueryInformation(m_hHeap, HeapCompatibilityInformation, &ulFlag, sizeof(ulFlag), NULL);
		return ulFlag;
	}

	LPVOID Alloc(SIZE_T size)
	{
		LH_ASSERT(NULL != m_hHeap);
		return ::HeapAlloc(m_hHeap, 0, size);
	}
	LPVOID CAlloc(SIZE_T size)
	{
		//������ڴ��ͬʱ��ʼ��Ϊ0
		LH_ASSERT(NULL != m_hHeap);
		return ::HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, size);
	}

	// bSamePoint: ԭ�ط����ڴ汣֤����ָ���봫��ָ��һ�£�����ܵ��·������ڴ�ʧ�� 
	LPVOID ReAlloc(LPVOID pMem, SIZE_T szSize, BOOL bSamePoint = FALSE)
	{
		LH_ASSERT(NULL != m_hHeap);
		if (bSamePoint)
		{
			return ::HeapReAlloc(m_hHeap, HEAP_ZERO_MEMORY | HEAP_REALLOC_IN_PLACE_ONLY, pMem, szSize);
		}

		return ::HeapReAlloc(m_hHeap, HEAP_ZERO_MEMORY, pMem, szSize);
	}
	BOOL MValidate(LPVOID pMem)
	{
		// �����ڴ����Ч��
		LH_ASSERT(NULL != m_hHeap);
		return ::HeapValidate(m_hHeap, 0, pMem);
	}

	SIZE_T MSize(LPVOID pMem)
	{
		// ��ȡ�ڴ��ĳߴ�
		LH_ASSERT(NULL != m_hHeap);
		return ::HeapSize(m_hHeap, 0, pMem);
	}
#ifdef LH_MEMORY_LEAK
protected:
	void AddBlockInfo(VOID* pMemBlock, SIZE_T szSize, LPCTSTR pszSourceFile, INT iSourceLine)
	{
		LH_ASSERT(pMemBlock != NULL);
		LH_ASSERT(m_hBlockHeap != NULL && szSize > 0);
		LPST_LH_MBLOCK pBlockInfo = (LPST_LH_MBLOCK)HeapAlloc(m_hBlockHeap, HEAP_ZERO_MEMORY, sizeof(ST_LH_MBLOCK));
		LH_ASSERT(pBlockInfo != NULL);
		StringCchCopy(pBlockInfo->m_pszSouceFile, MAX_PATH, pszSourceFile);
		pBlockInfo->m_iSourceLine = iSourceLine;
		pBlockInfo->m_szAllocSize = szSize;
		m_mapBlockInfo.SetAt(pMemBlock, pBlockInfo);
	}

	void RemoveBlockInfo(VOID* pMemBlock)
	{
		LH_ASSERT(pMemBlock != NULL);
		LH_ASSERT(m_hBlockHeap != NULL);
		LPST_LH_MBLOCK pBlockInfo = NULL;
		m_mapBlockInfo.Lookup(pMemBlock, pBlockInfo);
		if (NULL != pBlockInfo)
		{
			::HeapFree(m_hBlockHeap, 0, pBlockInfo);
			pMemBlock = NULL;
		}

		m_mapBlockInfo.RemoveKey(pMemBlock);
	}

public:
	LPVOID Alloc(SIZE_T szSize, LPCTSTR pszSourceFile, INT iSourceLine)
	{
		LH_ASSERT(m_hHeap != NULL && NULL != m_hBlockHeap);
		LPVOID pMem = ::HeapAlloc(m_hHeap, 0, szSize);
		AddBlockInfo(pMem, szSize, pszSourceFile, iSourceLine);
		return pMem;
	}

	LPVOID CAlloc(SIZE_T szSize, LPCTSTR pszSourceFile, INT iSourceLine)
	{
		LH_ASSERT(m_hHeap != NULL && NULL != m_hBlockHeap);
		LPVOID pMem = ::HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, szSize);
		AddBlockInfo(pMem, szSize, pszSourceFile, iSourceLine);
		return pMem;
	}

	LPVOID ReAlloc(LPVOID pMem, SIZE_T szSize, BOOL bSamePoint, LPCTSTR pszSourceFile, INT iSourceLine)
	{
		RemoveBlockInfo(pMem);
		LPVOID pRet = NULL;
		if (bSamePoint)
		{
			pRet = ::HeapReAlloc(m_hHeap, HEAP_ZERO_MEMORY | HEAP_REALLOC_IN_PLACE_ONLY, pMem, szSize);
		}
		else
		{
			pRet = ::HeapReAlloc(m_hHeap, HEAP_ZERO_MEMORY, pMem, szSize);
		}

		AddBlockInfo(pRet, szSize, pszSourceFile, iSourceLine);

		return pRet;

	}

	//����ڴ�й¶
	void DumpBlock()
	{
		LH_ASSERT(m_hHeap != NULL && m_hBlockHeap != NULL);
		TCHAR szOutputInfo[2 * MAX_PATH] = _T("");
		PROCESS_HEAP_ENTRY phe = { 0 };
		LPST_LH_MBLOCK pBlockInfo = NULL;
		int iLeakCnt = 0;
		StringCchPrintf(szOutputInfo, 2 * MAX_PATH, _T("��ʼ����(0x%08X)�ڴ�й¶���.........\n"), m_hHeap);

		Lock();
		while (HeapWalk(&phe))
		{
			// ��ǰ�ڴ�鱻����δ����, ���ڴ�й¶���
			if (PROCESS_HEAP_ENTRY_BUSY & phe.wFlags)
			{
				// ����ڴ�й¶λ��
				if (m_mapBlockInfo.Lookup(phe.lpData, pBlockInfo) && NULL != pBlockInfo)
				{
					StringCchPrintf(szOutputInfo, 2 * MAX_PATH, _T("%s[%ld]: �ڴ��(Point=0x%08X, Alloc Size=%u, Actual Size=%u)\n"), pBlockInfo->m_pszSouceFile, 
						pBlockInfo->m_iSourceLine,  phe.lpData, pBlockInfo->m_szAllocSize, phe.cbData);
					OutputDebugString(szOutputInfo);
				}
				else
				{
					// δ��¼�����ڴ�й¶
					StringCchPrintf(szOutputInfo, 2 * MAX_PATH, _T("δ��¼�����ڴ��й¶(Point=0x%08X, Actual Size=%u)\n"), phe.lpData, phe.cbData);
					OutputDebugString(szOutputInfo);
				}

				++iLeakCnt;
			}
		}
		Unlock();

		if (iLeakCnt > 0)
		{
			StringCchPrintf(szOutputInfo, 2 * MAX_PATH, _T("��(0x%08X)���ڴ�й¶������,������%d���ڴ�й¶.\n"), m_hHeap, iLeakCnt);
		}
		else
		{
			StringCchPrintf(szOutputInfo, 2 * MAX_PATH, _T("��(0x%08X)���ڴ�й¶������,δ�����ڴ�й¶.\n"), m_hHeap);
		}
		OutputDebugString(szOutputInfo);
	}
#endif
	BOOL Free(LPVOID pMem)
	{
#ifdef LH_MEMORY_LEAK
		RemoveBlockInfo(pMem);
#endif
		return ::HeapFree(m_hHeap, 0, pMem);
	}

protected:
	DWORD m_dwOptions; //�����Ķѵ�һЩ��־����Ϊ��Щ����û���ṩAPI���в�ѯ������������ⶨ��һ���������洴����ʱ�����һЩ����ֵ���Ա���ѯ
	HANDLE m_hHeap; //�ѵľ��

#ifdef LH_MEMORY_LEAK
	typedef struct _ST_LH_MBLOCK
	{
		TCHAR m_pszSouceFile[MAX_PATH]; //������õ�Դ�����ļ�
		int m_iSourceLine; // Դ���������
		SIZE_T m_szAllocSize; //����Ĵ�С
	}ST_LH_MBLOCK, *LPST_LH_MBLOCK;

	typedef CAtlMap<VOID*, LPST_LH_MBLOCK> CMapPtr2Info; //���������ڴ��ַ��block��Ϣ

	HANDLE m_hBlockHeap; //����block��Ϣ�Ķ�
	CMapPtr2Info m_mapBlockInfo; // ����block ������ڴ��ַӳ��

#endif
};
#endif

//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：CLHHeap类定义文件 CLHHeap类是LHLib类库中关于Windows堆的封装类
//模块名称：公共支持文件
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-20 17:30:27
//修改日志:
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
	// bNoSerialize: 是否支持多线程
	// bLFH: 是否是低碎片堆，低碎片堆能有效的利用堆空间
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
			// 不需要线程安全
			m_dwOptions |= HEAP_NO_SERIALIZE;
		}

		m_hHeap = ::HeapCreate(m_dwOptions, dwInitialSize, dwMaximumSize);
		if (NULL == m_hHeap)
		{
			return FALSE;
		}

#ifdef LH_MEMORY_LEAK
		// 创建内存块记录信息
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
		//检验堆的完整性
		LH_ASSERT(m_hHeap != NULL);
		return ::HeapValidate(m_hHeap, 0, NULL);
	}

	void Lock()
	{
		// 锁定堆，当堆是多线程堆的时候才会调用此方法
		LH_ASSERT(m_hHeap != NULL);
		if (NULL != m_hHeap && IsSerialize())
		{
			::HeapLock(m_hHeap);
		}
	}
	void Unlock()
	{
		// 解锁堆，当堆是多线程堆时才会调用此方法
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
		//打开LFH
		LH_ASSERT(NULL != m_hHeap);
		ULONG  ulHeapFragValue = 2;
		return ::HeapSetInformation(m_hHeap, HeapCompatibilityInformation, &ulHeapFragValue, sizeof(ulHeapFragValue));
	}
	BOOL DisableLFH()
	{
		// 关闭LFH
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
		//设置堆信息,其实就是指定堆的类型,指定2就打开了LFH，指定1表示它是一个LAL 堆, 指定0表示它是一个普通堆
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
		//分配堆内存的同时初始化为0
		LH_ASSERT(NULL != m_hHeap);
		return ::HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, size);
	}

	// bSamePoint: 原地分配内存保证返回指针与传入指针一致，这可能导致分配新内存失败 
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
		// 检查堆内存的有效性
		LH_ASSERT(NULL != m_hHeap);
		return ::HeapValidate(m_hHeap, 0, pMem);
	}

	SIZE_T MSize(LPVOID pMem)
	{
		// 获取内存块的尺寸
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

	//检查内存泄露
	void DumpBlock()
	{
		LH_ASSERT(m_hHeap != NULL && m_hBlockHeap != NULL);
		TCHAR szOutputInfo[2 * MAX_PATH] = _T("");
		PROCESS_HEAP_ENTRY phe = { 0 };
		LPST_LH_MBLOCK pBlockInfo = NULL;
		int iLeakCnt = 0;
		StringCchPrintf(szOutputInfo, 2 * MAX_PATH, _T("开始检查堆(0x%08X)内存泄露情况.........\n"), m_hHeap);

		Lock();
		while (HeapWalk(&phe))
		{
			// 当前内存块被分配未回收, 有内存泄露情况
			if (PROCESS_HEAP_ENTRY_BUSY & phe.wFlags)
			{
				// 输出内存泄露位置
				if (m_mapBlockInfo.Lookup(phe.lpData, pBlockInfo) && NULL != pBlockInfo)
				{
					StringCchPrintf(szOutputInfo, 2 * MAX_PATH, _T("%s[%ld]: 内存块(Point=0x%08X, Alloc Size=%u, Actual Size=%u)\n"), pBlockInfo->m_pszSouceFile, 
						pBlockInfo->m_iSourceLine,  phe.lpData, pBlockInfo->m_szAllocSize, phe.cbData);
					OutputDebugString(szOutputInfo);
				}
				else
				{
					// 未记录到的内存泄露
					StringCchPrintf(szOutputInfo, 2 * MAX_PATH, _T("未记录到的内存块泄露(Point=0x%08X, Actual Size=%u)\n"), phe.lpData, phe.cbData);
					OutputDebugString(szOutputInfo);
				}

				++iLeakCnt;
			}
		}
		Unlock();

		if (iLeakCnt > 0)
		{
			StringCchPrintf(szOutputInfo, 2 * MAX_PATH, _T("堆(0x%08X)的内存泄露检查完毕,共发现%d个内存泄露.\n"), m_hHeap, iLeakCnt);
		}
		else
		{
			StringCchPrintf(szOutputInfo, 2 * MAX_PATH, _T("堆(0x%08X)的内存泄露检查完毕,未发现内存泄露.\n"), m_hHeap);
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
	DWORD m_dwOptions; //创建的堆的一些标志，因为有些属性没有提供API进行查询，所以这里额外定义一个变量保存创建堆时传入的一些属性值，以备查询
	HANDLE m_hHeap; //堆的句柄

#ifdef LH_MEMORY_LEAK
	typedef struct _ST_LH_MBLOCK
	{
		TCHAR m_pszSouceFile[MAX_PATH]; //分配调用的源代码文件
		int m_iSourceLine; // 源代码的行数
		SIZE_T m_szAllocSize; //分配的大小
	}ST_LH_MBLOCK, *LPST_LH_MBLOCK;

	typedef CAtlMap<VOID*, LPST_LH_MBLOCK> CMapPtr2Info; //关联分配内存地址与block信息

	HANDLE m_hBlockHeap; //保存block信息的堆
	CMapPtr2Info m_mapBlockInfo; // 保存block 与分配内存地址映射

#endif
};
#endif

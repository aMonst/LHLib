//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：CLHObject 类头文件 CLHObject类是LHLib类库的公共基类
//模块名称：公共支持文件
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-20 17:30:27
//修改日志:
//===================================================================================

#pragma once
#ifndef __LH_OBJECT_H__
#define __LH_OBJECT_H__

#ifndef __LH_WIN_H__
#error 请通过包含LH_Win.h的方法使用CLHObject类
#endif

class CLHObject
{
public:
	CLHObject()
	{

	}

	virtual ~CLHObject()
	{

	}

public:
	// 一般的new和delete
	static void* __stdcall operator new(size_t szSize)
	{
		return ms_Heap.Alloc(szSize);
	}

	static void __stdcall operator delete(void* p)
	{
		ms_Heap.Free(p);
	}

	// 一般数组的new 和delete
	static void* __stdcall operator new[](size_t szSize)
	{
		return ms_Heap.Alloc(szSize);
	}

	static void __stdcall operator delete[](void* p)
	{
		ms_Heap.Free(p);
	}

#ifdef LH_MEMORY_LEAK
	// 带内存泄露检测的new 和 delete，以及 new[] 和 delete[]
	static void* __stdcall operator new(size_t szSize, LPCTSTR pszSourceFile, int iSourceLine)
	{
		return ms_Heap.Alloc(szSize, pszSourceFile, iSourceLine);
	}
	static void* __stdcall operator new[](size_t szSize, LPCTSTR pszSourceFile, int iSourceLine)
	{
		return ms_Heap.Alloc(szSize, pszSourceFile, iSourceLine);
	}
#else
	static void* __stdcall operator new(size_t szSize, LPCTSTR pszSourceFile, int iSourceLine)
	{
		return ms_Heap.Alloc(szSize);
	}
	static void* __stdcall operator new[](size_t szSize, LPCTSTR pszSourceFile, int iSourceLine)
	{
		return ms_Heap.Alloc(szSize);
	}
#endif

	static void __stdcall operator delete(void* p, LPCTSTR pszSourceFile, int iSourceLine)
	{
		ms_Heap.Free(p);
	}
	
	static void __stdcall operator delete[](void* p, LPCTSTR pszSourceFile, int iSourceLine)
	{
		ms_Heap.Free(p);
	}
private:
	//在开启内存泄露检测的情况下，派生自该基类的所有子类都分配到这个堆上，用于后续做内存检测
	static CLHHeap ms_Heap; 
};
#endif

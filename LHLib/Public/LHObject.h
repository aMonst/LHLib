//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�������CLHObject ��ͷ�ļ� CLHObject����LHLib���Ĺ�������
//ģ�����ƣ�����֧���ļ�
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-5-20 17:30:27
//�޸���־:
//===================================================================================

#pragma once
#ifndef __LH_OBJECT_H__
#define __LH_OBJECT_H__

#ifndef __LH_WIN_H__
#error ��ͨ������LH_Win.h�ķ���ʹ��CLHObject��
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
	// һ���new��delete
	static void* __stdcall operator new(size_t szSize)
	{
		return ms_Heap.Alloc(szSize);
	}

	static void __stdcall operator delete(void* p)
	{
		ms_Heap.Free(p);
	}

	// һ�������new ��delete
	static void* __stdcall operator new[](size_t szSize)
	{
		return ms_Heap.Alloc(szSize);
	}

	static void __stdcall operator delete[](void* p)
	{
		ms_Heap.Free(p);
	}

#ifdef LH_MEMORY_LEAK
	// ���ڴ�й¶����new �� delete���Լ� new[] �� delete[]
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
	//�ڿ����ڴ�й¶��������£������Ըû�����������඼���䵽������ϣ����ں������ڴ���
	static CLHHeap ms_Heap; 
};
#endif

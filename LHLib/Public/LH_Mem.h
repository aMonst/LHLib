//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：常用内存操作的宏定义文件
//模块名称：公共支持文件
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-20 17:38:30
//修改日志:
//===================================================================================

#pragma once
#ifndef __LH_MEM_H__
#define __LH_MEM_H__
#define LH_ALLOC(sz) HeapAlloc(GetProcessHeap(), 0, (sz))
#define LH_CALLOC(sz) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (sz))
#define LH_REALLOC(p, sz) HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (p), (sz))
#define LH_FREE(p) \
if((p) != NULL)\
{\
	HeapFree(GetProcessHeap(), 0, (p));\
	(p) = NULL;\
}\

#define LH_MSIZE(p) HeapSize(GetProcessHeap(), 0, p)
#define LH_MVAILD(p) HeapValidate(GetProcessHeap(), 0, p)

//下面这个宏用于打开堆的LFH特性,以提高性能
#define LH_OPEN_HEAP_LFH(h) \
    ULONG  ulLFH = 2;\
    HeapSetInformation((h),HeapCompatibilityInformation,&ulLFH ,sizeof(ULONG) ) ;
#endif

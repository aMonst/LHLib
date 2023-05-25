//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ������������ڴ�����ĺ궨���ļ�
//ģ�����ƣ�����֧���ļ�
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-5-20 17:38:30
//�޸���־:
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

//������������ڴ򿪶ѵ�LFH����,���������
#define LH_OPEN_HEAP_LFH(h) \
    ULONG  ulLFH = 2;\
    HeapSetInformation((h),HeapCompatibilityInformation,&ulLFH ,sizeof(ULONG) ) ;
#endif

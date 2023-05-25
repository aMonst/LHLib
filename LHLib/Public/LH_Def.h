//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�������һЩ��չ�ؼ��� �Լ����ò����ĺ궨���ļ�
//ģ�����ƣ�����֧���ļ�
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-5-20 17:20:18
//�޸���־:
//===================================================================================

#pragma once
#ifndef __LH_DEF_H__
#define __LH_DEF_H__

// �����޷���32λ���ֵ
#ifndef MAXUINT
#define MAXUINT ((UINT)~((UINT)0))
#endif
// �����޷���64λ���ֵ
#ifndef MAXULONGLONG
#define MAXULONGLONG ((ULONGLONG)~((ULONGLONG)0))
#endif

//�������������Ϣʱ�����������͵���Ϣ����ַ����͵�
#define LH_STRINGA2(x) #x
#define LH_STRINGA(x) LH_STRINGA2(x)

#define LH_STRINGW2(x) L#x
#define LH_STRINGW(x) LH_STRINGW2(x)

#ifdef UNICODE
#define LH_STRING(x) LH_STRINGW(x)
#else
#define LH_STRING(x) LH_STRINGA(x)
#endif

//__FILE__Ԥ�����Ŀ��ַ��汾
#define LH_WIDE2(x) L#x
#define LH_WIDE(x) LH_WIDE2(x)
#define __WFILE__ LH_WIDE(__FILE__)

// ȡ����Ԫ�صĸ���
#ifndef COUNTOF
#define COUNTOF(x) sizeof(x) / sizeof(x[0])
#endif

// ����32λ�޷��Ŷ���������ϳ�64λ�޷��Ŷ�������
#ifndef MAKEULONGLONG
#define MAKEULONGLONG(dwHight, dwLow) ((((ULONGLONG)dwHight << 32) | ((ULONGLONG)dwLow & 0xffffffff)))
#endif

#endif
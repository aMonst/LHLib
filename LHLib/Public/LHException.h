//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ��������쳣������ �Լ��쳣ģ��ӿڶ����ļ�
//ģ�����ƣ�C/C++ȫ���쳣֧��ģ��
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-5-21 14:29:15
//�޸���־:
//===================================================================================
#pragma once

#ifndef __LH_EXCEPTION_H__
#define __LH_EXCEPTION_H__
#include "LH_Win.h"
#include "LHString.h"
#include <eh.h>

class CLHException
{
	enum EM_LH_EXCEPTION
	{// �쳣���͵�ö�٣����������쳣����
		ET_Empty = 0x0, //���쳣�����ڳ�ʼ���쳣��
		ET_SE, //ϵͳ��׼�ṹ���쳣
		ET_LastError, //Api ���ñ���
		ET_Customer //�û��Զ����쳣
	};
public:
	CLHException():
		m_EType(ET_Empty),
		m_nSEHCode(0),
		m_EP(NULL),
		m_dwLastError(0),
		m_sErrorMsg()
	{}

	CLHException(UINT nCode, EXCEPTION_POINTERS* ep) :
		m_nSEHCode(nCode),
		m_EP(ep),
		m_EType(ET_Empty),
		m_dwLastError(0),
		m_sErrorMsg()
	{
		switch (m_nSEHCode)
		{
			case 0xC0000029://STATUS_INVALID_UNWIND_TARGET:
				m_sErrorMsg = L"չ��(unwind)������������Ч��չ��λ��.";
				break;
			case EXCEPTION_ACCESS_VIOLATION:
				m_sErrorMsg = L"�̳߳��Զ����ַ��һ����Ч����.";
				break;
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
				m_sErrorMsg = L"�̳߳��Զ���Ӳ������߽����һ��Խ�����.";
				break;
			case EXCEPTION_BREAKPOINT:
				m_sErrorMsg = L"�����ϵ�.";
				break;
			case EXCEPTION_DATATYPE_MISALIGNMENT:
				m_sErrorMsg = L"�̳߳����ڲ�֧�ֶ����Ӳ���Ϸ���δ���������.";
				break;
			case EXCEPTION_FLT_DENORMAL_OPERAND:
				m_sErrorMsg = L"������������̫С.";
				break;
			case EXCEPTION_FLT_DIVIDE_BY_ZERO:
				m_sErrorMsg = L"�����������.";
				break;
			case EXCEPTION_FLT_INEXACT_RESULT:
				m_sErrorMsg = L"����������޷�����ȷ����.";
				break;
			case EXCEPTION_FLT_INVALID_OPERATION:
				m_sErrorMsg = L"δ֪�ĸ������쳣.";
				break;
			case EXCEPTION_FLT_OVERFLOW:
				m_sErrorMsg = L"������������ָ������ָ�����������������.";
				break;
			case EXCEPTION_FLT_STACK_CHECK:
				m_sErrorMsg = L"�����������е��µ�ջ���.";
				break;
			case EXCEPTION_FLT_UNDERFLOW:
				m_sErrorMsg = L"������������ָ������ָ��������Ҫ��������.";
				break;
			case EXCEPTION_GUARD_PAGE:
				m_sErrorMsg = L"�̷߳��ʵ���PAGE_GUARD���η�����ڴ�.";
				break;
			case EXCEPTION_ILLEGAL_INSTRUCTION:
				m_sErrorMsg = L"�߳���ͼִ�в�����ָ��.";
				break;
			case EXCEPTION_IN_PAGE_ERROR:
				m_sErrorMsg = L"�߳���ͼ����һ��ϵͳ��ǰ�޷������ȱʧҳ.";
				break;
			case EXCEPTION_INT_DIVIDE_BY_ZERO:
				m_sErrorMsg = L"���������.";
				break;
			case EXCEPTION_INT_OVERFLOW:
				m_sErrorMsg = L"�������������λ���������Чλ.";
				break;
			case EXCEPTION_INVALID_DISPOSITION:
				m_sErrorMsg = L"�쳣������̷�����һ�������ÿص���쳣���ַ�����.";
				break;
			case EXCEPTION_INVALID_HANDLE:
				m_sErrorMsg = L"�߳�ʹ����һ�������õ��ں˶�����(�þ�������ѹر�).";
				break;
			case EXCEPTION_NONCONTINUABLE_EXCEPTION:
				m_sErrorMsg = L"�߳���ͼ�ڲ��ɼ������쳣֮�����ִ��.";
				break;
			case EXCEPTION_PRIV_INSTRUCTION:
				m_sErrorMsg = L"�߳���ͼͨ��ִ��һ��ָ����һ���������ڵ�ǰ�����ģʽ�Ĳ���.";
				break;
			case EXCEPTION_SINGLE_STEP:
				m_sErrorMsg = L"ĳָ��ִ������ź�, ��������һ�����������������ָ�����.";
				break;
			case EXCEPTION_STACK_OVERFLOW:
				m_sErrorMsg = L"�߳�ջ�ռ�ľ�.";
				break;
			default:
				m_sErrorMsg.Format(L"CLHException��δ֪�쳣,code=0x%08X", m_nSEHCode);
				break;
		}
	}
	CLHException(DWORD dwLastError):
		m_nSEHCode(0),
		m_EType(ET_LastError),
		m_EP(NULL),
		m_dwLastError(dwLastError),
		m_sErrorMsg()
	{
		GetErrorString(dwLastError);
	}

	CLHException(EM_LH_EXCEPTION eType, LPCWSTR lpcFormat, ...) :
		m_nSEHCode(0),
		m_EType(eType),
		m_EP(NULL),
		m_dwLastError(0),
		m_sErrorMsg()
	{
		va_list args;
		va_start(args, lpcFormat);
		m_sErrorMsg.Format(lpcFormat, args);
		va_end(args);
	}

	CLHException(LPCWSTR lpcFormat, ...) :
		m_nSEHCode(0),
		m_EType(ET_Customer),
		m_EP(NULL),
		m_dwLastError(0),
		m_sErrorMsg()
	{
		va_list args;
		va_start(args, lpcFormat);
		m_sErrorMsg.Format(lpcFormat, args);
		va_end(args);
	}

	CLHString& GetErrorString(DWORD dwErrorCode)
	{
		LPWSTR lpcErrorMsg = NULL;
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpcErrorMsg, 0, NULL);
		if (NULL == lpcErrorMsg)
		{
			return m_sErrorMsg;
		}

		m_sErrorMsg = lpcErrorMsg;
		LocalFree(lpcErrorMsg);
		return m_sErrorMsg;
	}

	LPCWSTR GetReason() const
	{
		return m_sErrorMsg.GetString();
	}

	DWORD GetErrorCode() const
	{
		return (m_dwLastError == 0) ? m_nSEHCode : m_dwLastError;
	}

	EM_LH_EXCEPTION GetType() const
	{
		return m_EType;
	}

	LPCWSTR GetTypeString()
	{
		static LPCWSTR pszTypeString[] = {
			L"Empty",
			L"SEH",
			L"Thread Last Error",
		    L"Customer",
			NULL
		};

		if (m_EType < COUNTOF(pszTypeString))
		{
			return pszTypeString[m_EType];
		}
		return NULL;
	}
protected:
	EM_LH_EXCEPTION m_EType; //�쳣����
	UINT m_nSEHCode;
	EXCEPTION_POINTERS* m_EP;
	DWORD m_dwLastError;
	CLHString m_sErrorMsg;
};

void __cdecl LH_SEH_Handle(unsigned int nCode, struct _EXCEPTION_POINTERS* ep)
{
	throw CLHException(nCode, ep);
}

#define LH_SEH_INIT() _set_se_translator(LH_SEH_Handle);
#endif

//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：异常类声明 以及异常模块接口定义文件
//模块名称：C/C++全部异常支持模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-21 14:29:15
//修改日志:
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
	{// 异常类型的枚举，用于区分异常类型
		ET_Empty = 0x0, //空异常，用于初始化异常类
		ET_SE, //系统标准结构化异常
		ET_LastError, //Api 调用报错
		ET_Customer //用户自定义异常
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
				m_sErrorMsg = L"展开(unwind)操作中遭遇无效的展开位置.";
				break;
			case EXCEPTION_ACCESS_VIOLATION:
				m_sErrorMsg = L"线程尝试对虚地址的一次无效访问.";
				break;
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
				m_sErrorMsg = L"线程尝试对有硬件数组边界检查的一次越界访问.";
				break;
			case EXCEPTION_BREAKPOINT:
				m_sErrorMsg = L"遇到断点.";
				break;
			case EXCEPTION_DATATYPE_MISALIGNMENT:
				m_sErrorMsg = L"线程尝试在不支持对齐的硬件上访问未对齐的数据.";
				break;
			case EXCEPTION_FLT_DENORMAL_OPERAND:
				m_sErrorMsg = L"浮点数操作数太小.";
				break;
			case EXCEPTION_FLT_DIVIDE_BY_ZERO:
				m_sErrorMsg = L"浮点数被零除.";
				break;
			case EXCEPTION_FLT_INEXACT_RESULT:
				m_sErrorMsg = L"浮点数结果无法被正确描述.";
				break;
			case EXCEPTION_FLT_INVALID_OPERATION:
				m_sErrorMsg = L"未知的浮点数异常.";
				break;
			case EXCEPTION_FLT_OVERFLOW:
				m_sErrorMsg = L"浮点数操作中指数高于指定类型允许的数量级.";
				break;
			case EXCEPTION_FLT_STACK_CHECK:
				m_sErrorMsg = L"浮点数操作中导致的栈溢出.";
				break;
			case EXCEPTION_FLT_UNDERFLOW:
				m_sErrorMsg = L"浮点数操作中指数低于指定类型需要的数量级.";
				break;
			case EXCEPTION_GUARD_PAGE:
				m_sErrorMsg = L"线程访问到被PAGE_GUARD修饰分配的内存.";
				break;
			case EXCEPTION_ILLEGAL_INSTRUCTION:
				m_sErrorMsg = L"线程试图执行不可用指令.";
				break;
			case EXCEPTION_IN_PAGE_ERROR:
				m_sErrorMsg = L"线程试图访问一个系统当前无法载入的缺失页.";
				break;
			case EXCEPTION_INT_DIVIDE_BY_ZERO:
				m_sErrorMsg = L"整数被零除.";
				break;
			case EXCEPTION_INT_OVERFLOW:
				m_sErrorMsg = L"整数操作结果进位超出最高有效位.";
				break;
			case EXCEPTION_INVALID_DISPOSITION:
				m_sErrorMsg = L"异常处理过程返回了一个不可用控点给异常发分发过程.";
				break;
			case EXCEPTION_INVALID_HANDLE:
				m_sErrorMsg = L"线程使用了一个不可用的内核对象句柄(该句柄可能已关闭).";
				break;
			case EXCEPTION_NONCONTINUABLE_EXCEPTION:
				m_sErrorMsg = L"线程试图在不可继续的异常之后继续执行.";
				break;
			case EXCEPTION_PRIV_INSTRUCTION:
				m_sErrorMsg = L"线程试图通过执行一条指令，完成一个不允许在当前计算机模式的操作.";
				break;
			case EXCEPTION_SINGLE_STEP:
				m_sErrorMsg = L"某指令执行完毕信号, 可能来自一个跟踪陷阱或其他单指令机制.";
				break;
			case EXCEPTION_STACK_OVERFLOW:
				m_sErrorMsg = L"线程栈空间耗尽.";
				break;
			default:
				m_sErrorMsg.Format(L"CLHException类未知异常,code=0x%08X", m_nSEHCode);
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
	EM_LH_EXCEPTION m_EType; //异常类型
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

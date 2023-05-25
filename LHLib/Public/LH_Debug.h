//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：调试输出支持模块封装
//模块名称：调试输出模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-23 17:19:15
//修改日志:
//===================================================================================

#pragma once
#ifndef __LH_DEBUG_H__
#define __LH_DEBUG_H__
#define LH_DEBUG_BUF_LINE 4096
#include "LH_Def.h"
// 定义断言
#ifdef _DEBUG
#ifndef LH_ASSERT
#define LH_ASSERT(x) if(!(x)){::DebugBreak();}
#endif
#else
#define LH_ASSERT(x) 
#endif


inline void __cdecl LHDebugOutputW(LPCWSTR lpszFormat, ...)
{
	va_list arg;
	va_start(arg, lpszFormat);
	wchar_t pBuffer[LH_DEBUG_BUF_LINE] = {0};
	if (S_OK != ::StringCchVPrintfW(pBuffer, COUNTOF(pBuffer), lpszFormat, arg))
	{
		va_end(arg);
		return;
	}

	va_end(arg);
	OutputDebugStringW(pBuffer);
}


inline void __cdecl LHDebugOutputA(LPCSTR lpszFormat, ...)
{
	va_list arg;
	va_start(arg, lpszFormat);
	char pBuffer[LH_DEBUG_BUF_LINE] = {0};
	if (S_OK != ::StringCchVPrintfA(pBuffer, COUNTOF(pBuffer), lpszFormat, arg))
	{
		va_end(arg);
		return;
	}

	va_end(arg);
	OutputDebugStringA(pBuffer);
}

#if (defined DEBUG) | (defined _DEBUG)
#define LH_DBGOUTA(...) LHDebugOutputA(__VA_ARGS__)
#define LH_DBGOUTW(...) LHDebugOutputW(__VA_ARGS__)
#define LH_DBGOUT_LINEA()	LH_DBGOUTA("%s(%d):",__FILE__, __LINE__)
#define LH_DBGOUT_LINEW()	LH_DBGOUTA(L"%s(%d):",__WFILE__, __LINE__)

#ifdef UNICODE
#define LH_DBGOUT(...) LH_DBGOUTW(__VA_ARGS__) 
#define LH_DBGOUT_LINE() LH_DBGOUT_LINEW()
#else
#define LH_DBGOUT(...) LH_DBGOUTA(__VA_ARGS__) 
#define LH_DBGOUT_LINE() LH_DBGOUT_LINEA()
#endif
#else
#define LH_DBGOUTA(...)
#define LH_DBGOUTW(...)
#define LH_DBGOUT_LINEA()
#define LH_DBGOUT_LINEW()

#ifdef UNICODE
#define LH_DBGOUT(...)
#define LH_DBGOUT_LINE()
#else
#define LH_DBGOUT(...) 
#define LH_DBGOUT_LINE()
#endif
#endif

//输出异常CLHException
#ifdef _DEBUG
#define LH_DBGOUT_EXPW(Fun,e) LHDebugOutputW(L"%s(%d):%s函数捕获异常(0x%08X):%s\n",__WFILE__,__LINE__,L#Fun,(e).GetErrorCode(),(e).GetReason())
#else
#define LH_DBGOUT_EXPW(Fun,e)
#endif
#endif
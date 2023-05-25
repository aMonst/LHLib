//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：提升特权函数的封装
//模块名称：安全性模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-23 17:30:15
//修改日志:
//===================================================================================
#pragma once
#ifndef __LH_PRIVILEGE_H__
#define __LH_PRIVILEGE_H__
inline BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege = TRUE)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;
	//查找到特权的LUID值
	if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
	{
		LH_DBGOUT(_T("LookupPrivilegeValue error: %u\n"), GetLastError());
		return FALSE;
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
	{
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	}
	else
	{
		tp.Privileges[0].Attributes = 0;
	}
	//为访问标记（访问字串）分配特权
	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
	{
		LH_DBGOUT(_T("AdjustTokenPrivileges error: %u\n"), GetLastError());
		return FALSE;
	}
	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		LH_DBGOUT(_T("分配指定的特权(%s)失败. \n"), lpszPrivilege);
		return FALSE;
	}
	return TRUE;
}
#endif

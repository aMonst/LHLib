//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�������������Ȩ�����ķ�װ
//ģ�����ƣ���ȫ��ģ��
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-5-23 17:30:15
//�޸���־:
//===================================================================================
#pragma once
#ifndef __LH_PRIVILEGE_H__
#define __LH_PRIVILEGE_H__
inline BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege = TRUE)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;
	//���ҵ���Ȩ��LUIDֵ
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
	//Ϊ���ʱ�ǣ������ִ���������Ȩ
	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
	{
		LH_DBGOUT(_T("AdjustTokenPrivileges error: %u\n"), GetLastError());
		return FALSE;
	}
	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		LH_DBGOUT(_T("����ָ������Ȩ(%s)ʧ��. \n"), lpszPrivilege);
		return FALSE;
	}
	return TRUE;
}
#endif

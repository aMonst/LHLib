//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：CLHProcess类定义文件 CLHProcess类是类库中关于Windows进程的封装类
//模块名称：进程管理模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-23 19:03:11
//修改日志:
//===================================================================================
#pragma once
#ifndef __LH_PROCESS_H__
#define __LH_PROCESS_H__
#include "LH_Win.h"
#include <tlhelp32.h>
#include "LH_Debug.h"
#include "LHObject.h"
#include "LHString.h"
#include "LHException.h"

class CLHProcess : public CLHObject
{
public:
	// 静态方法
	static BOOL CallProcess(LPCTSTR lpszExeFile, LPCTSTR lpszCmdLine, DWORD dwWaitTime, CLHString& strExeOutput, BOOL bShowWindow = FALSE)
	{	
		BOOL bRet = TRUE;
		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		try 
		{
			HANDLE hRead = NULL;
			HANDLE hWrite = NULL;
			size_t dwCmdLength = 0;
			SECURITY_ATTRIBUTES sa = { 0 };
			TCHAR szCmd[MAX_PATH] = _T("");
			StringCchPrintf(szCmd, MAX_PATH, _T("\"%s\" %s"), lpszExeFile, lpszCmdLine);
			if (StringCchLength(szCmd, MAX_PATH, &dwCmdLength) || dwCmdLength == 0)
			{
				throw CLHException(L"未指定进程路径和启动参数");
			}

			sa.bInheritHandle = TRUE;
			sa.lpSecurityDescriptor = NULL;
			sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			if (!CreatePipe(&hRead, &hWrite, &sa, 0))
			{
				throw CLHException(GetLastError());
			}

			STARTUPINFO si = {0};
			si.cb = sizeof(STARTUPINFO);
			GetStartupInfo(&si);
			si.hStdError = hWrite;
			si.hStdOutput = hWrite;
			si.dwFlags =  STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
			si.wShowWindow = bShowWindow ? SW_SHOW : SW_HIDE;

			if (!CreateProcess(NULL, szCmd, NULL, NULL, TRUE, 0, NULL, 0, &si, &pi))
			{
				CloseHandle(hWrite);
				CloseHandle(hRead);
				throw CLHException(GetLastError());
			}

			CloseHandle(hWrite);

			char buffer[4096] = { 0 };
			DWORD bytesRead;
			strExeOutput = L"";
			while (true)
			{
				memset(buffer, 0, 4096);
				if (ReadFile(hRead, buffer, 4096, &bytesRead, NULL) == NULL)
					break;
				strExeOutput += CLHString::AnsiToString(buffer, bytesRead);
			}

			CloseHandle(hRead);
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHProcess::call, e);
			if (NULL != pi.hProcess)
			{
				TerminateProcess(pi.hProcess, 0);
			}

			bRet = FALSE;
		}

		return bRet;

	}

	static BOOL ExecProcess(LPCTSTR lpszExeFile, LPCTSTR lpszCmdLine, BOOL bShowWindow = FALSE)
	{
		BOOL bRet = TRUE;
		PROCESS_INFORMATION pi = { 0 };
		try {
			STARTUPINFO si = { 0 };

			si.cb = sizeof(STARTUPINFO);
			DWORD dwFlags = 0;
			if (!bShowWindow)
			{
				dwFlags = CREATE_NO_WINDOW;
			}
			else
			{
				dwFlags = CREATE_NEW_CONSOLE;
			}

			TCHAR szCmd[1024] = _T("");
			_stprintf(szCmd, _T("\"%s\" %s"), lpszExeFile, lpszCmdLine);

			BOOL bRet = CreateProcess(NULL, szCmd, NULL, NULL, FALSE,
				dwFlags, NULL, NULL, &si, &pi);

			if (!bRet)
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHProcess::exec, e);
			bRet = FALSE;
		}
		return bRet;

	}

	static HANDLE GetProcessHanleByPID(DWORD dwProcessID)
	{
		HANDLE hProcess = NULL;
		try
		{
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);
			if (NULL == hProcess)
			{
				throw CLHException(GetLastError());
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHJob::AddProcess_1, e);
			hProcess = NULL;
		}

		return hProcess;
	}

	static BOOL GetMainThread(HANDLE hProcess, HANDLE& hMainThread, DWORD& dwMainThreadID)
	{
		BOOL bRet = TRUE;
		HANDLE hSnapshot = NULL;
		DWORD dwProcessID = GetProcessId(hProcess);
		try
		{
			hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
			if (INVALID_HANDLE_VALUE == hSnapshot)
			{
				throw CLHException(GetLastError());
			}

			THREADENTRY32 te = { 0 };
			te.dwSize = sizeof(THREADENTRY32);
			if (!Thread32First(hSnapshot, &te))
			{
				throw CLHException(GetLastError());
			}

			ULONGLONG ulCreate = MAXULONGLONG;
			dwMainThreadID = 0;
			do
			{
				if (te.th32OwnerProcessID == dwProcessID)
				{
					HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
					if (hThread == NULL)
					{
						continue;
					}

					FILETIME createTime, exitTime, userTime, kernalTime;
					if (!GetThreadTimes(hThread, &createTime, &exitTime, &kernalTime, &userTime))
					{
						continue;
					}

					if (MAKEULONGLONG(createTime.dwHighDateTime, createTime.dwLowDateTime) < ulCreate)
					{
						dwMainThreadID = te.th32ThreadID;
						ulCreate = MAKEULONGLONG(createTime.dwHighDateTime, createTime.dwLowDateTime);
						hMainThread = hThread;
					}
				}
			} while (Thread32Next(hSnapshot, &te));
		}
		catch (CLHException& e)
		{
			if (NULL != hSnapshot && INVALID_HANDLE_VALUE != hSnapshot)
			{
				CloseHandle(hSnapshot);
			}
			LH_DBGOUT_EXPW(CLHProcess::GetMainThread, e);
		}

		CloseHandle(hSnapshot);
		return bRet;
	}
	static BOOL GetProcessInformationByHandle(HANDLE hProcess, PROCESS_INFORMATION& pi)
	{
		BOOL bRet = TRUE;
		try {
			pi.hProcess = hProcess;
			if((pi.dwProcessId = GetProcessId(hProcess)) == 0)
			{
				throw CLHException(GetLastError());
			}

			return GetMainThread(hProcess, pi.hThread, pi.dwThreadId);
		}
		catch (CLHException& e)
		{
			bRet = FALSE;
			LH_DBGOUT_EXPW(CLHProcess::GetProcessInformationByHandle, e);
		}

		return bRet;
	}

	static BOOL GetProcessInformationByID(DWORD dwProcessID, PROCESS_INFORMATION& pi)
	{
		pi.dwProcessId = dwProcessID;
		if ((pi.hProcess = GetProcessHanleByPID(dwProcessID)) == NULL)
		{
			throw CLHException(GetLastError());
		}

		return GetMainThread(pi.hProcess, pi.hThread, pi.dwThreadId);
	}
};
#endif

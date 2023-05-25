//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：CLHServices类定义文件 CLHServices类是类库中关于Windows服务对象的封装类, 定义了基本的服务控制管理方法
//			后续服务程序可以直接从该类派生
//模块名称：服务模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-27 16:43:52
//修改日志:
//===================================================================================

#pragma once
#ifndef __LH_SERVICES_H__
#define __LH_SERVICES_H__
#include "LHObject.h"
#include "LHString.h"
#include "LHException.h"
#include "LHServices_Def.h"
#include <atlstr.h>

#include <atlcoll.h>
#include <Dbt.h>
#define LH_MAX_SVCNAME 128

class CLHServices: public CLHObject
{
public:
	CLHServices(LPCTSTR lpcName) :
		m_hss(NULL),
		m_hStopEvent(NULL),
		m_dwCurrentStatus(SERVICE_STOPPED)
	{
		ZeroMemory(m_szName, sizeof(TCHAR) * LH_MAX_SVCNAME);
		StringCchCopy(m_szName, LH_MAX_SVCNAME, lpcName);
		ms_ServicesMap.SetAt(lpcName, this);
	}

	virtual ~CLHServices()
	{

	}

protected:
	virtual bool InitService(DWORD dwArgc, LPTSTR* lpszArgv)
	{
		SetStartPending();
		m_hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		SetRunning();
		return true;
	}

	virtual bool Run()
	{
		WaitForSingleObject(m_hStopEvent, INFINITE);
		return TRUE;
	}

	virtual bool ExitService()
	{
		SetStopPending();
		CloseHandle(m_hStopEvent);
		SetStop();
		m_hStopEvent = NULL;
		return true;
	}

protected:
	// 服务命令处理函数
	virtual DWORD OnUserControlHandle(DWORD dwCode)
	{
		return 0U;
	}

	virtual DWORD OnStop()
	{
		if (NULL != m_hStopEvent)
		{
			SetEvent(m_hStopEvent);
		}

		return 0U;
	}

	virtual DWORD OnPause()
	{
		SetPause();
		return 0U;
	}

	virtual DWORD OnContinue()
	{
		SetRunning();
		return 0U;
	}

	virtual DWORD OnInterrogate()
	{
		// 将服务当前状态报告给服务控制管理器
		SetStatus(m_dwCurrentStatus);
		return 0U;
	}

	virtual DWORD OnShutDown()
	{
		if (NULL != m_hStopEvent)
		{
			SetEvent(m_hStopEvent);
		}

		return 0U;
	}

protected://设备变更事件通知处理 SERVICE_CONTROL_DEVICEEVENT
	virtual DWORD OnDeviceArrival(PDEV_BROADCAST_HDR pDbh) { return 0U; }
	virtual DWORD OnDeviceRemoveComplete(PDEV_BROADCAST_HDR pDbh) { return 0U; }
	virtual DWORD OnDeviceQueryRemove(PDEV_BROADCAST_HDR pDbh) { return 0U; }
	virtual DWORD OnDeviceQueryRemoveFailed(PDEV_BROADCAST_HDR pDbh) { return 0U; }
	virtual DWORD OnDeviceRemovePending(PDEV_BROADCAST_HDR pDbh) { return 0U; }
	virtual DWORD OnCustomEvent(PDEV_BROADCAST_HDR pDbh) { return 0U; }
protected://硬件配置文件发生变动 SERVICE_CONTROL_HARDWAREPROFILECHANGE
	virtual DWORD OnConfigChanged() { return 0U; }
	virtual DWORD OnQueryChangeConfig() { return 0U; }
	virtual DWORD OnConfigChangeCanceled() { return 0U; }
protected://设备电源事件 SERVICE_CONTROL_POWEREVENT
	virtual DWORD OnPowerSettingChange(PPOWERBROADCAST_SETTING pPs) { return 0U; }
protected://session 发生变化 SERVICE_CONTROL_SESSIONCHANGE
	virtual DWORD OnWTSConsoleConnect(PWTSSESSION_NOTIFICATION pWn) { return 0U; }
	virtual DWORD OnWTSConsoleDisconnect(PWTSSESSION_NOTIFICATION pWns) { return 0U; }
	virtual DWORD OnWTSRemoteConnect(PWTSSESSION_NOTIFICATION pWns) { return 0U; }
	virtual DWORD OnWTSRemoteDisconnect(PWTSSESSION_NOTIFICATION pWns) { return 0U; }
	virtual DWORD OnWTSSessionLogon(PWTSSESSION_NOTIFICATION pWns) { return 0U; }
	virtual DWORD OnWTSSessionLogoff(PWTSSESSION_NOTIFICATION pWns) { return 0U; }
	virtual DWORD OnWTSSessionLock(PWTSSESSION_NOTIFICATION pWns) { return 0U; }
	virtual DWORD OnWTSSessionUnLock(PWTSSESSION_NOTIFICATION pWns) { return 0U; }
	virtual DWORD OnWTSSessionRemoteControl(PWTSSESSION_NOTIFICATION pWns) { return 0U; }

	
public:
	BOOL RunService(DWORD argc, LPTSTR* argv)
	{
		if (InitService(argc, argv))
		{
			Run();
		}

		return ExitService();
	}

public:
	static DWORD WINAPI ServiceHandlerEx(
		DWORD dwControl,
		DWORD dwEventType,
		LPVOID lpEventData,
		LPVOID lpContext
	)
	{
		DWORD dwRet = ERROR_SUCCESS;

		if (NULL == lpContext)
		{
			return ERROR_INVALID_PARAMETER;
		}

		CLHServices* pThis = reinterpret_cast<CLHServices*>(lpContext);
		if (pThis == NULL)
		{
			return ERROR_INVALID_PARAMETER;
		}

		switch(dwControl)
		{
		case SERVICE_CONTROL_STOP:
			dwRet = pThis->OnStop();
			break;
		case SERVICE_CONTROL_PAUSE:
			dwRet = pThis->OnPause();
			break;
		case SERVICE_CONTROL_CONTINUE:
			dwRet = pThis->OnContinue();
			break;
		case SERVICE_CONTROL_INTERROGATE: //查询
			dwRet = pThis->OnInterrogate();
			break;
		case SERVICE_CONTROL_SHUTDOWN: //系统将要关机，此时做一些清理工作
			dwRet = pThis->OnShutDown();
			break;
		case SERVICE_CONTROL_DEVICEEVENT:
			{
				PDEV_BROADCAST_HDR pDbh = (PDEV_BROADCAST_HDR)lpEventData;
				switch (dwEventType)
				{
				case DBT_DEVICEARRIVAL:
					dwRet = pThis->OnDeviceArrival(pDbh);
					break;
				case DBT_DEVICEREMOVECOMPLETE:
					dwRet = pThis->OnDeviceRemoveComplete(pDbh);
					break;
				case DBT_DEVICEQUERYREMOVE:
					dwRet = pThis->OnDeviceQueryRemove(pDbh);
					break;
				case DBT_DEVICEQUERYREMOVEFAILED:
					dwRet = pThis->OnDeviceQueryRemoveFailed(pDbh);
					break;
				case DBT_DEVICEREMOVEPENDING:
					dwRet = pThis->OnDeviceRemovePending(pDbh);
					break;
				case DBT_CUSTOMEVENT:
					dwRet = pThis->OnCustomEvent(pDbh);
					break;
				default:
					dwRet = ERROR_INVALID_PARAMETER;
					break;
				}
			}
			break;
		case SERVICE_CONTROL_HARDWAREPROFILECHANGE:
			{
				switch (dwEventType)
				{
					case DBT_CONFIGCHANGED:
						dwRet = pThis->OnConfigChanged();
						break;
					case DBT_QUERYCHANGECONFIG:
						dwRet = pThis->OnQueryChangeConfig();
						break;
					case DBT_CONFIGCHANGECANCELED:
						dwRet = pThis->OnConfigChangeCanceled();
						break;
					default:
						dwRet = ERROR_INVALID_PARAMETER;
						break;
				}
			}
			break;
		case SERVICE_CONTROL_SESSIONCHANGE:
			{
				PWTSSESSION_NOTIFICATION pWn = (PWTSSESSION_NOTIFICATION)lpEventData;
				switch (dwEventType)
				{
				case WTS_CONSOLE_CONNECT:
					dwRet = pThis->OnWTSConsoleConnect(pWn);
					break;
				case WTS_CONSOLE_DISCONNECT:
					dwRet = pThis->OnWTSConsoleDisconnect(pWn);
					break;
				case WTS_REMOTE_CONNECT:
					dwRet = pThis->OnWTSRemoteConnect(pWn);
					break;
				case WTS_REMOTE_DISCONNECT:
					dwRet = pThis->OnWTSRemoteDisconnect(pWn);
					break;
				case WTS_SESSION_LOGON:
					dwRet = pThis->OnWTSSessionLogon(pWn);
					break;
				case WTS_SESSION_LOGOFF:
					dwRet = pThis->OnWTSSessionLogoff(pWn);
					break;
				case WTS_SESSION_LOCK:
					dwRet = pThis->OnWTSSessionLock(pWn);
					break;
				case WTS_SESSION_UNLOCK:
					dwRet = pThis->OnWTSSessionUnLock(pWn);
					break;
				case WTS_SESSION_REMOTE_CONTROL:
					dwRet = pThis->OnWTSSessionRemoteControl(pWn);
					break;
				default:
					dwRet = ERROR_INVALID_PARAMETER;
					break;
				}
			}
			break;
		default:		//自定义的操作码(128 to 255)
			if (127 < dwControl && 256 > dwControl)
			{
				dwRet = pThis->OnUserControlHandle(dwControl);
			}
			else
			{
				return ERROR_CALL_NOT_IMPLEMENTED;
			}
			break;
		}

		return dwRet;
	}
public:
	void SetStatusHandle(SERVICE_STATUS_HANDLE hss)
	{
		LH_ASSERT(NULL != hss);
		m_hss = hss;
	}

	BOOL SetServiceStatus(LPSERVICE_STATUS lpSS)
	{
		return ::SetServiceStatus(m_hss, lpSS);
	}

protected:
	//内部的工具方法，设置服务为一个指定的状态
	BOOL SetStatus(DWORD dwStatus, DWORD dwCheckPoint = 0, DWORD dwWaitHint = 0
		, DWORD dwExitCode = 0, DWORD dwAcceptStatus = SERVICE_CONTROL_INTERROGATE)
	{
		LH_ASSERT(m_hss != NULL);
		BOOL bRet = TRUE;
		try
		{
			SERVICE_STATUS ss = { 0 };
			ss.dwServiceType = LH_SVC_TYPE;
			ss.dwCurrentState = dwStatus;
			ss.dwControlsAccepted = SERVICE_CONTROL_INTERROGATE | dwAcceptStatus;
			ss.dwWin32ExitCode = 0;
			ss.dwServiceSpecificExitCode = dwExitCode;
			ss.dwCheckPoint = dwCheckPoint;
			ss.dwWaitHint = dwWaitHint;
			if (!SetServiceStatus(&ss))
			{
				throw CLHException(GetLastError());
			}

			m_dwCurrentStatus = dwStatus;
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHServices::SetStatus, e);
			bRet = FALSE;
		}

		return bRet;
	}

public:
	inline BOOL SetStartPending(DWORD dwCheckPoint = 0, DWORD dwWaitHint = 0)
	{
		return SetStatus(SERVICE_START_PENDING, dwCheckPoint, dwWaitHint);
	}

	inline BOOL SetContinuePending(DWORD dwCheckPoint = 0, DWORD dwWaitHint = 0)
	{
		return SetStatus(SERVICE_CONTINUE_PENDING, dwCheckPoint, dwWaitHint);
	}

	inline BOOL SetPausePending(DWORD dwCheckPoint = 0, DWORD dwWaitHint = 0)
	{
		return SetStatus(SERVICE_PAUSE_PENDING, dwCheckPoint, dwWaitHint);
	}

	inline BOOL SetStopPending(DWORD dwCheckPoint = 0, DWORD dwWaitHint = 0)
	{
		return SetStatus(SERVICE_STOP_PENDING, dwCheckPoint, dwWaitHint);
	}

	inline BOOL SetPause()
	{
		return SetStatus(SERVICE_PAUSED, 0, 0, 0,
			SERVICE_ACCEPT_PAUSE_CONTINUE
			| SERVICE_ACCEPT_SHUTDOWN
			| SERVICE_ACCEPT_STOP);
	}

	inline BOOL SetRunning()
	{
		return SetStatus(SERVICE_RUNNING, 0, 0, 0,
			SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN);
	}

	inline BOOL SetStop(DWORD dwExitCode = 0)
	{
		return SetStatus(SERVICE_STOPPED, 0, 0, dwExitCode);
	}
	typedef CAtlMap<CString, CLHServices*> CLHServicesMap;
protected:
	TCHAR m_szName[LH_MAX_SVCNAME];
	SERVICE_STATUS_HANDLE m_hss;
	DWORD m_dwCurrentStatus;
	HANDLE m_hStopEvent;
public:
	static CLHServicesMap ms_ServicesMap;
};
#endif

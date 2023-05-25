//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�������CLHSvcMgr�ඨ���ļ� CLHSvcMgr��������й���Windows ������ƹ������ķ�װ��
//			����ʹ�ø�����Windows������ƹ��������н����Ա����ע��������ɾ������Ȳ���
//ģ�����ƣ��������ģ��
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-5-27 19:02:14
//�޸���־:
//===================================================================================

#pragma once
#include "LHServices_Def.h"
#include "LH_Debug.h"
#include "LHObject.h"
#include "LHString.h"
#include "LHException.h"
class CLHSvcMgr
{
public:
    CLHSvcMgr(DWORD dwAccess = SC_MANAGER_ALL_ACCESS)
        :m_hSCManager(NULL)
    {
        try
        {
            m_hSCManager = ::OpenSCManager(NULL, NULL, dwAccess);
            if (NULL == m_hSCManager)
            {
                throw CLHException(GetLastError());
            }
        }
        catch (CLHException& e)
        {
            LH_DBGOUT_EXPW(CLHSvcMgr::CLHSvcMgr, e);
            m_hSCManager = NULL;
        }
    }

    virtual ~CLHSvcMgr()
    {
        if (NULL != m_hSCManager)
        {
            ::CloseServiceHandle(m_hSCManager);
            m_hSCManager = NULL;
        }
    }

public:
    BOOL AddService(LPCTSTR pServiceName, // ��������
        LPCTSTR pDispName, //�������ʾ����
        LPCTSTR pDesciption, //������Ϣ
        LPCTSTR pBinaryPathName, //��ִ���ļ�����·��
        DWORD dwDesiredAccess = SERVICE_ALL_ACCESS, //������ܵĿ�����Ϊ
        DWORD dwServiceType = LH_SVC_TYPE, //�����������
        DWORD dwStartType = SERVICE_AUTO_START, //��������״̬
        DWORD dwErrorControl = SERVICE_ERROR_NORMAL // �������Ĵ������
    )
    {
        LH_ASSERT(NULL != m_hSCManager);
        LH_ASSERT(NULL != pServiceName);
        LH_ASSERT(NULL != pDispName)
        LH_ASSERT(NULL != pBinaryPathName);

        BOOL bRet = TRUE;
        SC_HANDLE hService = NULL;
        try
        {
            hService = ::CreateService(m_hSCManager, pServiceName, pDispName, dwDesiredAccess, dwServiceType, dwStartType, dwErrorControl, pBinaryPathName, 
                NULL, NULL, NULL, NULL, NULL);

            if (NULL == hService)
            {
                throw CLHException(GetLastError());
            }

            if (pDesciption != NULL)
            {
                SERVICE_DESCRIPTION sd = {};
                sd.lpDescription = (LPTSTR)pDesciption;

                if (!::ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &sd))
                {
                    throw CLHException(GetLastError());
                }
            }

            CloseServiceHandle(hService);
        }
        catch (CLHException& e)
        {
            bRet = FALSE;
            if (NULL != hService)
            {
                CloseServiceHandle(hService);
            }
            LH_DBGOUT_EXPW(CLHSvcMgr::AddService, e);
        }

        return bRet;
    }

    BOOL DeleteService(LPCTSTR pServiceName)
    {
        LH_ASSERT(m_hSCManager != NULL);
        LH_ASSERT(pServiceName != NULL);

        SC_HANDLE hService = NULL;
        BOOL bRet = TRUE;
        try
        {
            hService = ::OpenService(m_hSCManager, pServiceName, DELETE);
            if (NULL == hService)
            {
                throw CLHException(GetLastError());
            }

            if (!::DeleteService(hService))
            {
                throw CLHException(GetLastError());
            }

            CloseServiceHandle(hService);
        }
        catch (CLHException& e)
        {
            bRet = FALSE;
            if (NULL != hService)
            {
                CloseServiceHandle(hService);
            }

            LH_DBGOUT_EXPW(CLHSvcMgr::DeleteService, e);
        }

        return bRet;
    }

    //�޸ķ����������ʽΪ��ֹ����
    BOOL DisableService(LPCTSTR pszServiceName)
    {
        return SetServiceStartType(pszServiceName, SERVICE_DISABLED);
    }

   //�޸ķ����������ʽΪ�ֶ�����
    BOOL EnableService(LPCTSTR pszServiceName)
    {
        return SetServiceStartType(pszServiceName, SERVICE_DEMAND_START);
    }

    //�޸ķ����������ʽΪ�Զ�����
    BOOL SetAutoService(LPCTSTR pszServiceName)
    {
        return SetServiceStartType(pszServiceName, SERVICE_AUTO_START);
    }

    BOOL SetServiceDesc(LPCTSTR pszServiceName, LPCTSTR pszDescription)
    {
        LH_ASSERT(m_hSCManager != NULL);
        LH_ASSERT(pszServiceName != NULL);

        BOOL bRet = TRUE;
        SC_HANDLE hService = NULL;
        try
        {
            hService = ::OpenService(m_hSCManager, pszServiceName, SERVICE_CHANGE_CONFIG);
            if (NULL == hService)
            {
                throw CLHException(GetLastError());
            }

            SERVICE_DESCRIPTION sd = { 0 };
            sd.lpDescription = (LPTSTR)pszDescription;

            if (!ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &sd))
            {
                throw CLHException(GetLastError());
            }

            CloseServiceHandle(hService);
        }
        catch (CLHException& e)
        {
            bRet = FALSE;
            if (hService != NULL)
            {
                CloseServiceHandle(hService);
            }

            LH_DBGOUT_EXPW(CLHSvcMgr::DisableService, e);
        }

        return bRet;
    }

private:
    BOOL WaitForServiceStatus(SC_HANDLE hService, DWORD dwWaitForStatus, DWORD dwCurrentState, SERVICE_STATUS_PROCESS& ssStatus)
    {
        DWORD dwStartTickCount = GetTickCount();
        DWORD dwOldCheckPoint = ssStatus.dwCheckPoint;
        DWORD dwWaitTime = 0;
        DWORD dwBytesNeeded = 0;
        BOOL bWaitSuccess = TRUE;

        try
        {
            // �ȵȴ�������ֹͣ״̬
            while (ssStatus.dwCurrentState == dwCurrentState)
            {
                dwWaitTime = ssStatus.dwWaitHint / 10;
                if (dwWaitTime < 1000)
                {
                    dwWaitTime = 1000;
                }
                else if (dwWaitTime > 10000)
                {
                    dwWaitTime = 10000;
                }

                WaitForSingleObject(GetCurrentThread(), dwWaitTime);
                if (!QueryServiceStatusEx(
                    hService,
                    SC_STATUS_PROCESS_INFO,
                    (LPBYTE)&ssStatus,
                    sizeof(SERVICE_STATUS_PROCESS),
                    &dwBytesNeeded))
                {
                    throw CLHException(GetLastError());
                }

                if (ssStatus.dwCurrentState == dwWaitForStatus)
                {
                    break;
                }
                if (ssStatus.dwCheckPoint > dwOldCheckPoint)
                {
                    dwStartTickCount = GetTickCount();
                    dwOldCheckPoint = ssStatus.dwCheckPoint;
                }
                else
                {
                    if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
                    {
                        throw CLHException(_T("�ȴ�����״̬��ʱ"));
                    }
                }
            }
        }
        catch (CLHException& e)
        {
            bWaitSuccess = FALSE;
            LH_DBGOUT_EXPW(CLHSvcMgr::WaitForServiceStatus, e);
        }

        return bWaitSuccess;
    }

    BOOL SetServiceStartType(LPCTSTR lpcServiceName, DWORD dwStartType)
    {
       LH_ASSERT(m_hSCManager != NULL);
        LH_ASSERT(lpcServiceName != NULL);

        BOOL bRet = TRUE;
        SC_HANDLE hService = NULL;
        try
        {
            hService = ::OpenService(m_hSCManager, lpcServiceName, SERVICE_CHANGE_CONFIG);
            if (NULL == hService)
            {
                throw CLHException(GetLastError());
            }

            if (!ChangeServiceConfig(hService, SERVICE_NO_CHANGE, dwStartType, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
            {
                throw CLHException(GetLastError());
            }

            CloseServiceHandle(hService);
        }
        catch (CLHException& e)
        {
            bRet = FALSE;
            if (hService != NULL)
            {
                CloseServiceHandle(hService);
            }

            LH_DBGOUT_EXPW(CLHSvcMgr::DisableService, e);
        }

        return bRet;
    }
	//ֹͣһ���������������
    BOOL StopDependentServices(SC_HANDLE hService)
    {
        DWORD i = 0;
        DWORD dwBytesNeeded;
        DWORD dwCount;

        LPENUM_SERVICE_STATUS   lpDependencies = NULL;
        ENUM_SERVICE_STATUS     ess;
        SC_HANDLE               hDepService;
        SERVICE_STATUS_PROCESS  ssp;

        DWORD dwStartTime = GetTickCount();
        DWORD dwTimeout = 30000; // 30-second time-out

        //�����API���ε��÷�ȷ����Ҫ�����С��
        if (EnumDependentServices(hService, SERVICE_ACTIVE,
            lpDependencies, 0, &dwBytesNeeded, &dwCount))
        {
            // ����ڴ治����Ȼ���óɹ�,˵����ǰ����û�������ķ���,����ȥֹͣ
            return TRUE;
        }
        else
        {
            if (GetLastError() != ERROR_MORE_DATA)
            {
                return FALSE;
            }

            // ��������������Ϣ����
            lpDependencies = (LPENUM_SERVICE_STATUS)HeapAlloc(
                GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);

            if (!lpDependencies)
            {
                return FALSE;
            }

            __try
            {
                // ö������������Ϣ
                if (!EnumDependentServices(hService, SERVICE_ACTIVE,
                    lpDependencies, dwBytesNeeded, &dwBytesNeeded,
                    &dwCount))
                {
                    return FALSE;
                }

                for (i = 0; i < dwCount; i++)
                {
                    ess = *(lpDependencies + i);
                    // ����������
                    hDepService = OpenService(m_hSCManager,
                        ess.lpServiceName,
                        SERVICE_STOP | SERVICE_QUERY_STATUS);

                    if (!hDepService)
                    {
                        return FALSE;
                    }

                    __try
                    {
                        // ����ֹͣ����
                        if (!::ControlService(hDepService,
                            SERVICE_CONTROL_STOP,
                            (LPSERVICE_STATUS)&ssp))
                        {
                            return FALSE;
                        }

                        if (!WaitForServiceStatus(hDepService, SERVICE_STOPPED, ssp.dwCurrentState, ssp))
                        {
                            return FALSE;
                        }
                        // �ȴ�����ֹͣ
                    }
                    __finally
                    {
                        // Always release the service handle.
                        CloseServiceHandle(hDepService);
                    }
                }
            }
            __finally
            {
                // Always free the enumeration buffer.
                HeapFree(GetProcessHeap(), 0, lpDependencies);
            }
        }
        return TRUE;
    }

public:
    BOOL StartService(LPCTSTR pszName, LPCTSTR* lpArgc = NULL, DWORD dwArgs = 0)
    {
        BOOL bRet = 0;
        SC_HANDLE hService = NULL;
        try
        {
            SERVICE_STATUS_PROCESS ssp = {};
            DWORD dwOldCheckPoint = 0;
            DWORD dwStartTickCount = 0;
            DWORD dwWaitTime = 0;
            DWORD dwBytesNeeded = 0;

            hService = OpenService(m_hSCManager, pszName, SERVICE_ALL_ACCESS);
            if (NULL == hService)
            {
                throw CLHException(GetLastError());
            }
            if (!QueryServiceStatusEx(
                hService,                     // handle to service 
                SC_STATUS_PROCESS_INFO,         // information level
                (LPBYTE)&ssp,             // address of structure
                sizeof(SERVICE_STATUS_PROCESS), // size of structure
                &dwBytesNeeded))              // size needed if buffer is too small
            {
                throw CLHException(GetLastError());
            }

            if (ssp.dwCurrentState != SERVICE_STOPPED && ssp.dwCurrentState != SERVICE_STOP_PENDING)
            {
                throw CLHException(L"������������[%s],��Ϊ���������������", pszName);
            }

            if (!WaitForServiceStatus(hService, SERVICE_STOPPED, ssp.dwCurrentState, ssp))
            {
                throw CLHException(L"��������[%s]������,�ȴ�������ȷֹͣ��ʱ,�����޷�����", pszName);
            }

            if (!::StartService(hService, 0, NULL))
            {
                throw CLHException(GetLastError());
            }

            if (!QueryServiceStatusEx(
                hService,
                SC_STATUS_PROCESS_INFO,
                (LPBYTE)&ssp,
                sizeof(SERVICE_STATUS_PROCESS),
                &dwBytesNeeded))
            {
                throw CLHException(GetLastError());
            }

            WaitForServiceStatus(hService, SERVICE_RUNNING, ssp.dwCurrentState, ssp);

            if (ssp.dwCurrentState == SERVICE_RUNNING)
            {
                LH_DBGOUT(_T("����[%s]�����ɹ�.\n"), pszName);
            }
            else
            {
                LH_DBGOUT(L"����[%s]����ʧ��. \n", pszName);
                LH_DBGOUT(L"  ��ǰ״̬��: %d\n", ssp.dwCurrentState);
                LH_DBGOUT(L"  �˳�����: %d\n", ssp.dwWin32ExitCode);
                LH_DBGOUT(L"  ����: %d\n", ssp.dwCheckPoint);
                LH_DBGOUT(_T("  ����ȴ�ʱ��: %d\n"), ssp.dwWaitHint);
                throw CLHException(GetLastError());
            }

            CloseServiceHandle(hService);
        }
        catch (CLHException& e)
        {
            bRet = FALSE;
            if (NULL != hService)
            {
                CloseServiceHandle(hService);
            }
            LH_DBGOUT_EXPW(StartService, e);
        }

        return bRet;
    }

    BOOL StopService(LPCTSTR pszName)
    {
        BOOL bRet = TRUE;
        SC_HANDLE hService = NULL;
        try
        {
            SERVICE_STATUS_PROCESS ssp;
            DWORD dwStartTime = GetTickCount();
            DWORD dwBytesNeeded;
            DWORD dwTimeout = 30000; // 30-second time-out

            hService = OpenService(
                m_hSCManager,
                pszName,
                SERVICE_STOP |
                SERVICE_QUERY_STATUS |
                SERVICE_ENUMERATE_DEPENDENTS);

            if (NULL == hService)
            {
                throw CLHException(GetLastError());
            }

            if (!QueryServiceStatusEx(
                hService,
                SC_STATUS_PROCESS_INFO,
                (LPBYTE)&ssp,
                sizeof(SERVICE_STATUS_PROCESS),
                &dwBytesNeeded))
            {
                throw CLHException(GetLastError());
            }

            if (ssp.dwCurrentState == SERVICE_STOPPED)
            {
                throw CLHException(L"����[%s]�Ѿ���ֹͣ״̬.\n", pszName);
            }

            WaitForServiceStatus(hService, SERVICE_STOPPED, ssp.dwCurrentState, ssp);

            //ֹͣ�����ڸ÷������������
            StopDependentServices(hService);

            //ֹͣ����
            if (!::ControlService(
                hService,
                SERVICE_CONTROL_STOP,
                (LPSERVICE_STATUS)&ssp))
            {
                throw CLHException(GetLastError());
            }

            if (!QueryServiceStatusEx(
                hService,
                SC_STATUS_PROCESS_INFO,
                (LPBYTE)&ssp,
                sizeof(SERVICE_STATUS_PROCESS),
                &dwBytesNeeded))
            {
                throw CLHException(GetLastError());
            }

            WaitForServiceStatus(hService, SERVICE_STOPPED, SERVICE_STOP_PENDING, ssp);

            if (SERVICE_STOPPED == ssp.dwCurrentState)
            {
                LH_DBGOUT(L"����[%s]�ѳɹ�ֹͣ\n", pszName);
            }
            CloseServiceHandle(hService);
        }
        catch (CLHException& e)
        {
            bRet = FALSE;
            if (NULL != hService)
            {
                CloseServiceHandle(hService);
            }
            LH_DBGOUT_EXPW(StopService, e);
        }
        return bRet;
    }
    BOOL PauseService(LPCTSTR pszName)
    {
        return ControlService(pszName, SERVICE_CONTROL_PAUSE);
    }

    BOOL ContinueService(LPCTSTR pszName)
    {
        return ControlService(pszName, SERVICE_CONTROL_CONTINUE);
    }

    BOOL ControlService(LPCTSTR pszName, DWORD dwControlCode, LPSERVICE_STATUS lpSS = NULL)
    {
        BOOL bRet = TRUE;
        SC_HANDLE hService = NULL;
        try
        {
            hService = OpenService(m_hSCManager, pszName, SERVICE_ALL_ACCESS);
            if (NULL == hService)
            {
                throw CLHException(GetLastError());
            }
            SERVICE_STATUS ss = {};
            bRet = ::ControlService(hService, dwControlCode, &ss);
        }
        catch (CLHException& e)
        {
            bRet = FALSE;
            if (NULL != hService)
            {
                CloseServiceHandle(hService);
            }
            LH_DBGOUT_EXPW(ControlService, e);
        }
        return bRet;
    }
protected:
    SC_HANDLE m_hSCManager;
};

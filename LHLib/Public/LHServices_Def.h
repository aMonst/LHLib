//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ��������������ģ���õ���һЩ��
//ģ�����ƣ��������ģ��
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-5-27 09:31:25
//�޸���־:
//===================================================================================

#pragma once
#ifndef __LH_SERVICES_DEF_H__
#define __LH_SERVICES_DEF_H__
#include <winsvc.h>
#include <Dbt.h>
#include "LH_Win.h"

#if (defined DEBUG || defined _DEBUG)
//���԰�ʱ�򿪽�������,�������
#define LH_SVC_TYPE SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS
#else
#define LH_SVC_TYPE SERVICE_WIN32_OWN_PROCESS
#endif

//=====================================================================================================================
//���ڶ�������������б�����
#define DECLARE_SVC_ENTRY extern SERVICE_TABLE_ENTRY g_arSvcEntryTable[];

#define LH_SVC_ENTRY_BEGIN() \
    SERVICE_TABLE_ENTRY g_arSvcEntryTable[] = {

#define LH_SVC_ENTRY(svcName) \
    { _T(#svcName),		_SVCMain_##svcName }, 

#define LH_SVC_ENTRY_END() \
    { NULL,                 NULL	}};

//��ΪsvcName�ķ���ServiceMain����ԭ��
#define LH_SVC_MAIN(svcName) VOID WINAPI _SVCMain_##scvName( DWORD dwArgc, LPTSTR* lpszArgv);

// ʵ��һ�������ע�ắ��
#define LH_SVC_MAIN_IMP(svcName,classname) \
    VOID WINAPI _SVCMain_##svcName( DWORD dwArgc, LPTSTR* lpszArgv)\
    {\
    try\
        {\
            CoInitialize(NULL);\
            CLHService* pService = NULL;\
            if(!CLHService::ms_ServicesMap.Lookup(_T(#svcName),pService))\
            {\
                pService = dynamic_cast<CLHService*>( new classname(_T(#svcName)) );\
            }\
            else\
            {\
                return;\
            }\
            SERVICE_STATUS_HANDLE hss = RegisterServiceCtrlHandlerEx(_T(#svcName),CLHService::ServiceHandlerEx, reinterpret_cast<LPVOID>(pService));\
            if(NULL == hss)\
            {\
                throw CLHException(GetLastError());\
            }\
            pService->SetStatusHandle(hss);\
            pService->RunService(dwArgc,lpszArgv);\
            delete dynamic_cast<##classname*>(pService);\
            CoUninitialize();\
        }\
        catch(CLHException&e)\
        {\
            LH_DBGOUT(_T("��%s��������������ػ����쳣(0x%08X):%s\n"),_T(#svcName),e.GetErrCode(),e.GetReason());\
        }\
        catch(...)\
        {\
			LH_DBGOUT(_T("��%s��������������ػ�δ֪�쳣.\n"),_T(#svcName));\
        }\
    }
#endif

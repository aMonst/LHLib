//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：定义服务模块用到的一些宏
//模块名称：服务管理模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-27 09:31:25
//修改日志:
//===================================================================================

#pragma once
#ifndef __LH_SERVICES_DEF_H__
#define __LH_SERVICES_DEF_H__
#include <winsvc.h>
#include <Dbt.h>
#include "LH_Win.h"

#if (defined DEBUG || defined _DEBUG)
//调试版时打开交互特征,方便调试
#define LH_SVC_TYPE SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS
#else
#define LH_SVC_TYPE SERVICE_WIN32_OWN_PROCESS
#endif

//=====================================================================================================================
//用于定义服务主函数列表数组
#define DECLARE_SVC_ENTRY extern SERVICE_TABLE_ENTRY g_arSvcEntryTable[];

#define LH_SVC_ENTRY_BEGIN() \
    SERVICE_TABLE_ENTRY g_arSvcEntryTable[] = {

#define LH_SVC_ENTRY(svcName) \
    { _T(#svcName),		_SVCMain_##svcName }, 

#define LH_SVC_ENTRY_END() \
    { NULL,                 NULL	}};

//名为svcName的服务ServiceMain函数原型
#define LH_SVC_MAIN(svcName) VOID WINAPI _SVCMain_##scvName( DWORD dwArgc, LPTSTR* lpszArgv);

// 实现一个服务的注册函数
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
            LH_DBGOUT(_T("【%s】服务的主函数截获了异常(0x%08X):%s\n"),_T(#svcName),e.GetErrCode(),e.GetReason());\
        }\
        catch(...)\
        {\
			LH_DBGOUT(_T("【%s】服务的主函数截获未知异常.\n"),_T(#svcName));\
        }\
    }
#endif

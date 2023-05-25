//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：CLHSocketStartup类定义文件 CLHSocketStartup类主要用于初始化socket库环境
//模块名称：基础网络模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-31 09:25:11
//修改日志:
//===================================================================================

#pragma once
#include "LHObject.h"
#include "LHString.h"
#include "LHException.h"
#include "LH_Sock.h"

class CLHSocketStartup: public CLHObject
{
public:
	CLHSocketStartup(BYTE bvHigh = 0x2, BYTE bvLow = 0x2)
	{
		ZeroMemory(&m_wsadata, sizeof(WSADATA));
		InitSocket(bvHigh, bvLow);
	}

	virtual ~CLHSocketStartup()
	{
		UnInitSocket();
	}

    BOOL InitSocket(BYTE bvHigh, BYTE bvLow)
    {
        WORD wVer = MAKEWORD(bvHigh, bvLow);
        int err = ::WSAStartup(wVer, &m_wsadata);
        if (0 != err)
        {
            LH_DBGOUT(_T("无法初始化Socket2系统环境，错误码为：%d！\n"), WSAGetLastError());
            return FALSE;
        }
        if (LOBYTE(m_wsadata.wVersion) != bvHigh ||
            HIBYTE(m_wsadata.wVersion) != bvLow)
        {
            LH_DBGOUT(_T("无法初始化%d.%d版本的Socket环境！\n"), bvHigh, bvLow);
            WSACleanup();
            return FALSE;
        }

        LH_DBGOUT(_T("Winsock库初始化成功!\n\t系统中支持最高的Winsock版本为%d.%d\n\t当前应用加载的版本为%d.%d\n")
            , LOBYTE(m_wsadata.wHighVersion), HIBYTE(m_wsadata.wHighVersion)
            , LOBYTE(m_wsadata.wVersion), HIBYTE(m_wsadata.wVersion));

        return TRUE;
    }

    VOID UnInitSocket()
    {
        WSACleanup();
        LH_DBGOUT(_T("Winsock库已被卸载,环境已经被释放!\n\t系统中支持最高的Winsock版本为%d.%d\n\t应用加载的版本为%d.%d\n")
            , LOBYTE(m_wsadata.wHighVersion), HIBYTE(m_wsadata.wHighVersion)
            , LOBYTE(m_wsadata.wVersion), HIBYTE(m_wsadata.wVersion));
    }

    WORD GetRunVersion()
    {
        return m_wsadata.wVersion;
    }

    WORD GetSysHighVersion()
    {
        return m_wsadata.wHighVersion;
    }

    BYTE GetRunMajorVersion()
    {
        return LOBYTE(m_wsadata.wVersion);
    }

    BYTE GetRunMinorVersion()
    {
        return HIBYTE(m_wsadata.wVersion);
    }

    BYTE GetSysHighMajorVersion()
    {
        return LOBYTE(m_wsadata.wHighVersion);
    }

    BYTE GetSysHighMinorVersion()
    {
        return HIBYTE(m_wsadata.wHighVersion);
    }
protected:
	WSADATA m_wsadata;
};

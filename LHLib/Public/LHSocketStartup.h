//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�������CLHSocketStartup�ඨ���ļ� CLHSocketStartup����Ҫ���ڳ�ʼ��socket�⻷��
//ģ�����ƣ���������ģ��
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-5-31 09:25:11
//�޸���־:
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
            LH_DBGOUT(_T("�޷���ʼ��Socket2ϵͳ������������Ϊ��%d��\n"), WSAGetLastError());
            return FALSE;
        }
        if (LOBYTE(m_wsadata.wVersion) != bvHigh ||
            HIBYTE(m_wsadata.wVersion) != bvLow)
        {
            LH_DBGOUT(_T("�޷���ʼ��%d.%d�汾��Socket������\n"), bvHigh, bvLow);
            WSACleanup();
            return FALSE;
        }

        LH_DBGOUT(_T("Winsock���ʼ���ɹ�!\n\tϵͳ��֧����ߵ�Winsock�汾Ϊ%d.%d\n\t��ǰӦ�ü��صİ汾Ϊ%d.%d\n")
            , LOBYTE(m_wsadata.wHighVersion), HIBYTE(m_wsadata.wHighVersion)
            , LOBYTE(m_wsadata.wVersion), HIBYTE(m_wsadata.wVersion));

        return TRUE;
    }

    VOID UnInitSocket()
    {
        WSACleanup();
        LH_DBGOUT(_T("Winsock���ѱ�ж��,�����Ѿ����ͷ�!\n\tϵͳ��֧����ߵ�Winsock�汾Ϊ%d.%d\n\tӦ�ü��صİ汾Ϊ%d.%d\n")
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

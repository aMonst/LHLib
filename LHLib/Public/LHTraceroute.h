//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�������CLHTraceroute ��ķ�װ����Ҫʵ��TraceRoute����Ļ�������
//ģ�����ƣ�����ģ��
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-6-10 17:49:17
//�޸���־:
//===================================================================================

#pragma once
#include "LHPing.h"

class CLHTraceroute : public CLHPing
{
public:
	typedef struct _ST_LH_TRACEROUTE_DATA
    {
        IN_ADDR m_Addr;
        int     m_iTime;
        char m_pszHost[NI_MAXHOST]; //������;����������IP
        char m_pszServ[NI_MAXSERV];
    }
    ST_LH_TRACEROUTE_DATA,*LPST_LH_TRACEROUTE_DATA;

    typedef CSimpleArray<LPST_LH_TRACEROUTE_DATA> CLHTraceRouteDataArray;

public:
    CLHTraceroute() :
        CLHPing(),
        m_aryTraceRouteData()
    {
        ZeroMemory(&m_saPrev, sizeof(SOCKADDR_IN));
    }

    virtual ~CLHTraceroute()
    {
        CLHIOCPBase::Close(NULL);
    }
public:
    VOID TraceRoute(const char* pszDestAddr, HANDLE hEventFinish)
    {
        //��1��ʼ����
        Ping(pszDestAddr, 1, hEventFinish, 32, 1, 6000);
    }

    VOID ClearData()
    {
        ZeroMemory(&m_saPrev, sizeof(SOCKADDR_IN));
        LPST_LH_TRACEROUTE_DATA pTrData = NULL;
        for (int i = 0; i < m_aryTraceRouteData.GetSize(); ++i)
        {
            pTrData = m_aryTraceRouteData[i];
            LH_FREE(pTrData);
        }

        m_aryTraceRouteData.RemoveAll();
    }

    const int GetRouteNodeCnt() const
    {
        // ��ȡ���ֵĽڵ�
        return m_aryTraceRouteData.GetSize();
    }

    const LPST_LH_TRACEROUTE_DATA operator[](int index) const
    {
        return m_aryTraceRouteData[index];
    }

protected:
    virtual VOID OnCloseIOCP(VOID* pUserData) override
    {
        CLHPing::OnCloseIOCP(pUserData);
        ClearData();
    }

    virtual VOID OnRecvFrom(SOCKADDR_IN& saRemote, DWORD dwTransBytes, void* pUserData, LPST_LH_IOCPUDP_OVERLAPPED pUdpOl) override
    {
        LH_ASSERT(pUserData != NULL);
        LPST_LH_PING_DATA pPingData = (LPST_LH_PING_DATA)pUserData;
        pPingData->m_dwRecvTime = GetTickCount();
        if (saRemote.sin_addr.s_addr != m_saPrev.sin_addr.s_addr)
        {
            // �����½ڵ�
            LPST_LH_TRACEROUTE_DATA pTrData = (LPST_LH_TRACEROUTE_DATA)LH_CALLOC(sizeof(ST_LH_TRACEROUTE_DATA));
            m_aryTraceRouteData.Add(pTrData);
            pTrData->m_Addr.s_addr = saRemote.sin_addr.s_addr;
            pTrData->m_iTime = pPingData->m_dwRecvTime - pPingData->m_ICMPHdr.icmp_timestamp;
            GetAddrNameInfo(&saRemote, pTrData->m_pszHost, pTrData->m_pszServ);

            LH_DBGOUTA("CLHTraceroute�����µ�·�ɽڵ�:%s[%s : %s]\n"
                ,inet_ntoa(saRemote.sin_addr),pTrData->m_pszHost,pTrData->m_pszServ);
        }

        if (saRemote.sin_addr.s_addr != ((SOCKADDR_IN*)m_aiDest->ai_addr)->sin_addr.s_addr)
        {
            // ��ת����һ��
            SetTTL(pPingData->m_iTTL + 1);
            LH_DBGOUTA("CLHTraceroute������һ��ping����,������һ���ڵ�,TTL = %d\n" ,pPingData->m_iTTL + 1);
            SendPingCmd(pPingData->m_nsDataSize, pPingData->m_hEventFinish, pPingData->m_iTTL + 1);
        }
        else
        {
            // ׷�ٽ���
            LH_DBGOUTA("CLHTraceroute׷�ٽ���,׷����%d���ڵ�\n" ,m_aryTraceRouteData.GetSize());

            m_pCurrentData = NULL;
            if (NULL != pPingData->m_hEventFinish)
            {
                // ��������֪ͨ
                SetEvent(pPingData->m_hEventFinish);
            }
        }
    }

    virtual VOID OnWaitTimeOut(DWORD dwWatiTime) override
    {
        LH_DBGOUTA("CLHTraceroute�̳߳صȴ���ʱ\n");
        if (NULL != m_pCurrentData)
        {
            SetTTL(m_pCurrentData->m_iTTL + 1);
            LH_DBGOUTA("CLHTraceroute������һ��ping����,������һ���ڵ�,TTL = %d\n", m_pCurrentData->m_iTTL + 1);
            SendPingCmd(m_pCurrentData->m_nsDataSize, m_pCurrentData->m_hEventFinish, m_pCurrentData->m_iTTL + 1);
        }
    }
protected:
    // data
    SOCKADDR_IN m_saPrev;
    CLHTraceRouteDataArray m_aryTraceRouteData;
};

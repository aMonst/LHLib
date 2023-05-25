//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�������CLHPing ��ķ�װ����Ҫʵ��ping����Ļ�������
//ģ�����ƣ�����ģ��
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-6-07 19:49:17
//�޸���־:
//===================================================================================

#pragma once
#include "LHIOCPUDP.h"
#include "LH_TCPIPHeader.h"
#include <ws2tcpip.h>
#define LH_MAX_IPPACKET 0xffff //���IP���Ĵ�С
class CLHPing : public CLHIOCPUdp
{
public:
	typedef struct _ST_LH_PING_DATA
	{
		USHORT			m_nsDataSize; //��ICMPһ���ͳ���ʵ�����ݴ�С
		DWORD			m_dwRecvTime; //���ܵ����ݰ�ʱ��ʱ���
		DWORD			m_dwLastError; //WSASendto/WSARecvFrom�����з����Ĵ�����
		HANDLE			m_hEventFinish;//ÿ��Ping����ʱ��Ҫ����Ϊ���ź�״̬���¼�������,�ɵ������ṩ
                                       //����ΪNULL,��ʾ����Ҫ�ӵ�Ping����֪ͨ
		int				m_iTTL;        //����ICMP��ʱָ����TTLֵ
		union
        {
            ST_ICMP_HDR m_ICMPHdr;        //���͵�Ping����       
            BYTE        m_btSendData[LH_MAX_IPPACKET];       
        };

        union
        {
            ST_IPV4_HDR m_RetPacketHdr;     //���ص�����        
            BYTE        m_btRecvData[LH_MAX_IPPACKET]; 
        };
	}ST_LH_PING_DATA, *LPST_LH_PING_DATA;

    typedef CSimpleArray<LPST_LH_PING_DATA> CLHPingPacketArray;

public:
    CLHPing() :
        CLHIOCPUdp(),
        m_aiDest(NULL),
        m_aiSrc(NULL),
        m_iPingCnt(0),
        m_iCurPingIndex(0),
        m_iRecv(0),
        m_iSend(0),
        m_pCurrentData(NULL),
        m_arPingPacket()
    {
        ZeroMemory(&m_PingOl, sizeof(ST_LH_IOCPUDP_OVERLAPPED));
        ZeroMemory(&m_pszDestHost, sizeof(NI_MAXHOST));
        ZeroMemory(&m_pszDestServ, sizeof(NI_MAXHOST));
    }

    virtual ~CLHPing()
    {
        CLHIOCPBase::Close(NULL);
    }

protected:
    BOOL CreateRawSocketV4()
    {
        BOOL bRet = TRUE;
        LH_ASSERT(NULL == m_skUdp || INVALID_SOCKET == m_skUdp);
        try
        {
            m_skUdp = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, WSA_FLAG_OVERLAPPED);
            if (INVALID_SOCKET == m_skUdp)
            {
                throw CLHException(WSAGetLastError());
            }

            AddHandle2IOCP((HANDLE)m_skUdp);

            if (0 != bind(m_skUdp, m_aiSrc->ai_addr, m_aiSrc->ai_addrlen))
            {
                throw CLHException(WSAGetLastError());
            }


        }
        catch (CLHException& e)
        {
            LH_DBGOUT_EXPW(CLHPing::CreateRawSocketV4, e);
            bRet = FALSE;

            if(INVALID_SOCKET != m_skUdp)
            {
                closesocket(m_skUdp);
                m_skUdp = INVALID_SOCKET;
            }
        }

        return bRet;
    }

    VOID SendPingCmd(USHORT nsDataLen, HANDLE hEventFinish, int iTTL)
    {
        try
        {
            LPST_LH_PING_DATA pPingData = (LPST_LH_PING_DATA)LH_CALLOC(sizeof(ST_LH_PING_DATA));
            m_arPingPacket.Add(pPingData);

            m_pCurrentData = pPingData; //��¼��ǰ���ڴ�������ݿ飬���㳬ʱ����

            m_PingOl.m_lNetworkEvent = FD_READ;
            m_PingOl.m_pUserData = pPingData;
            m_PingOl.m_isaLen = sizeof(SOCKADDR_IN);
            WSABUF wb;
            wb.buf = (char*)pPingData->m_btRecvData;
            wb.len = LH_MAX_IPPACKET;

            pPingData->m_hEventFinish = hEventFinish;
            pPingData->m_iTTL = iTTL;

            if (SOCKET_ERROR == WSARecvFrom(m_skUdp, &wb, 1, &m_PingOl.m_dwTransBytes, &m_PingOl.m_dwFlags, (SOCKADDR*)&m_PingOl.m_saRemote, &m_PingOl.m_isaLen, &m_PingOl.m_ol, NULL))
            {
                DWORD dwLastError = (DWORD)WSAGetLastError();

                if(WSA_IO_PENDING != dwLastError)
                {
                    pPingData->m_dwLastError = dwLastError;
                    throw CLHException(dwLastError);
                }
            }

            WSABUF wbSend = { 0 };
            wbSend.buf = (char*)pPingData->m_btSendData;
            wbSend.len = sizeof(ST_ICMP_HDR) + nsDataLen;

            pPingData->m_ICMPHdr.icmp_type = ICMPV4_ECHO_REQUEST_TYPE;
            pPingData->m_ICMPHdr.icmp_code = ICMPV4_ECHO_REQUEST_CODE;
            pPingData->m_ICMPHdr.icmp_id = (USHORT)GetCurrentProcessId();

            //��ICMP���е�ʣ������ݲ���ͳһ���ó�һ�����ֵ,�����õ���0x32
            memset(pPingData->m_btSendData + sizeof(ST_ICMP_HDR), 0x32, nsDataLen);
            pPingData->m_nsDataSize = nsDataLen;
            pPingData->m_ICMPHdr.icmp_sequence = (USHORT)GetTickCount();
            pPingData->m_ICMPHdr.icmp_timestamp = GetTickCount();
            pPingData->m_ICMPHdr.icmp_checksum = 0;
            pPingData->m_ICMPHdr.icmp_checksum = CheckSum((USHORT*)pPingData->m_btSendData, sizeof(ST_ICMP_HDR) + nsDataLen);

            ++m_iSend;
            ++m_iCurPingIndex;

            DWORD dwSendBytes = 0;
            if (SOCKET_ERROR == WSASendTo(m_skUdp, &wbSend, 1, &dwSendBytes, 0, m_aiDest->ai_addr, m_aiDest->ai_addrlen, NULL, NULL))
            {
                pPingData->m_dwLastError = WSAGetLastError();
                throw CLHException(pPingData->m_dwLastError);
            }
        }
        catch (CLHException& e)
        {
            LH_DBGOUT_EXPW(CLHPing::SendPingCmd, e);
        }
    }

protected:
	// ���߸�������
    //���ݴ����ip�Ͷ˿ڷ��ص�ַ��Ϣ
    addrinfo* GetAddrInfoV4(const char* addr, const char* port)
    {
        addrinfo *res   = NULL;
        try
        {
            addrinfo hins = { 0 };
            int rc = 0;
            hins.ai_flags = ((addr) ? 0 : AI_PASSIVE);
            hins.ai_family = AF_INET;
            hins.ai_socktype = SOCK_RAW;
            hins.ai_protocol = IPPROTO_ICMP;
            rc = getaddrinfo(addr, port, &hins, &res);
            if (rc != 0)
            {
                throw CLHException(WSAGetLastError());
            }

        }
        catch (CLHException& e)
        {
            SafeFreeAddrInfo(res);
            LH_DBGOUT_EXPW(CLHPing::GetAddrInfoV4, e);
        }
		return res;
    }

    //���ݵ�ַ��Ϣ��ȡ����IP��Զ��IP
    void GetAddrNameInfo(SOCKADDR_IN* sa, char* pszHost, char* pszServ)
    {
        try
        {
			int hostlen = NI_MAXHOST;
			int servlen = NI_MAXSERV;
			int iRet = getnameinfo((SOCKADDR*)sa, sizeof(SOCKADDR_IN), pszHost, hostlen, pszServ, servlen,  NI_NUMERICHOST | NI_NUMERICSERV);

            if (iRet != 0)
            {
                throw CLHException(WSAGetLastError());
            }
        }
        catch (CLHException& e)
        {
            LH_DBGOUT_EXPW(CLHPing::GetAddrNameInfo, e);
        }
    }

    void SafeFreeAddrInfo(addrinfo*& pai)
    {
        if(NULL != pai )
        {
            freeaddrinfo(pai);
            pai = NULL;
        }
    }

    USHORT CheckSum(USHORT* buf, int size)
    {
        unsigned long chsum = 0;
        while (size > 1)
        {
            chsum += *buf++;
            size -= sizeof(USHORT);
        }

        if (size)
        {
            chsum += *(UCHAR*)buf;
        }

        chsum = (chsum >> 16) + (chsum & 0xffff);
        chsum += (chsum >> 16);
        return (USHORT)(~chsum);
    }

public:
    // ��Ҫ�����ṩ���ܵķ���
    VOID Ping(const char* pDestAddr, int iPingCnt = 4, HANDLE hEventFinish = NULL, USHORT nsPackSize = 32, int iTTL = 128, DWORD dwWaitTime = 6000)
    {
        // ��һ��ָ����IP��ַ����Ping����
        try
        {
            ClearPingData();
            SafeFreeAddrInfo(m_aiDest);

            //��ȡpingĿ��������ַ��Ϣ
            m_aiDest = GetAddrInfoV4(pDestAddr, "0");
            if (NULL == m_aiDest)
            {
                throw CLHException(L"�޷���ȡĿ���ַ����Ϣ,���ܽ���Ping����!");
            }

            // ��ȡĿ���ַ����������Ϣ
            GetAddrNameInfo((SOCKADDR_IN*)m_aiDest->ai_addr,m_pszDestHost,m_pszDestServ);
            if (!IsValid())
            {
                m_aiSrc = GetAddrInfoV4(NULL, "0");
                if (NULL == m_aiSrc)
                {
                    throw CLHException(L"�޷���ȡ����IPv4�ĵ�ַ��Ϣ,��ʧ��!");
                }

                //�����̳߳�,��ΪPing����һ���߳̾��㹻��
                if (!Create(*(SOCKADDR_IN*)m_aiSrc->ai_addr, 1, 1))
                {
                    throw CLHException(L"IOCP�̳߳ش���ʧ��,�޷����Ping����!");
                }
            }

			SetTTL(iTTL);
			SetWaitTime(dwWaitTime);
			m_iPingCnt = iPingCnt;
			m_iCurPingIndex = 0;
			SendPingCmd(nsPackSize, hEventFinish, iTTL);
        }
        catch (CLHException& e)
        {
            LH_DBGOUT_EXPW(CLHPing::Ping, e);
        }
    }

    VOID ClearPingData()
    {
        LPST_LH_PING_DATA pPingData = NULL;
        for (int i = 0; i < m_arPingPacket.GetSize(); i++)
        {
            pPingData = m_arPingPacket[i];
            LH_FREE(pPingData);
        }

        m_arPingPacket.RemoveAll();
        m_pCurrentData = NULL;

        ZeroMemory(&m_PingOl, sizeof(ST_LH_IOCPUDP_OVERLAPPED));
        ZeroMemory(m_pszDestHost, NI_MAXHOST * sizeof(char));
        ZeroMemory(m_pszDestServ, NI_MAXSERV * sizeof(char));

        m_iCurPingIndex = 0;
        m_iPingCnt = 0;
        m_iSend = 0;
        m_iRecv = 0;
    }

    int GetPingResultCnt() const
    {
        //�õ�Ping����������
        return m_arPingPacket.GetSize();
    }

    const LPST_LH_PING_DATA operator[] (int index) const
    {
        return m_arPingPacket[index];
    }

    const addrinfo* GetDestAddrInfo() const
    {
        return m_aiDest;
    }

    const char* GetDestHostName() const
    {
        return m_pszDestHost;
    }

    const char* GetDestServName() const
    {
        return m_pszDestServ;
    }

    void Stat(int& iAvgPackSize,int& iAvgTime,int& iMaxTime,int& iMinTime, float& fAvgSpeed,int& iSend,int& iRecv,float& fLost)
    {//ͳ��Ping�����
        iAvgPackSize = 0;
        iAvgTime = 0;
        iMaxTime = 0;
        iMinTime = LH_MAX_IPPACKET;
        fAvgSpeed = 0;
        iSend = 0;
        iRecv = 0;
        fLost = 0.0f;
        if (m_arPingPacket.GetSize() < 1)
        {
            return;
        }

        int i = 0;
        int iCnt = m_arPingPacket.GetSize();
        LPST_LH_PING_DATA pPingData = NULL;
        int iTime = 0;
        int iSendBytes = 0;

        for(i = 0; i < iCnt;i ++)
        {
            pPingData = m_arPingPacket[i];
            if (NULL == pPingData)
            {
                return;
            }

            iAvgPackSize += pPingData->m_nsDataSize;
            iTime = (pPingData->m_dwRecvTime - pPingData->m_ICMPHdr.icmp_timestamp);
            iTime = iTime > 0 ? iTime : 0;
            iAvgTime += iTime;
            iMaxTime = max(iTime, iMaxTime);

            if (pPingData->m_dwRecvTime > 0)
            {
                //���ͳɹ������ݰ���ͳ��Ϊ���͵�������
                //�ۼƷ��͵��ֽ���
                iSendBytes += pPingData->m_nsDataSize 
                    + sizeof(ST_ICMP_HDR) + sizeof(ST_IPV4_HDR) 
                    + pPingData->m_RetPacketHdr.ip_totallength;
                //ͳ����С����ʱ��
                iMinTime = min(iTime,iMinTime);
            }
        }

        // ƽ��������������(��Ҫָ��) ʱ��ĵ�λ�Ǻ���,��������λ��Bytes,���ֱ������Ľ����λ������KBytes/S
        fAvgSpeed = ((float)((iSendBytes) / iAvgTime)) * 1000.0f / 1024.0f;
        iSend = m_iSend;
        iRecv = m_iRecv;

        //ƽ������С
        iAvgPackSize /= i;
        //ƽ����ʱ
        iAvgTime /= i;
        //������
        fLost = (float)(iSend - iRecv) / iSend;
    }

    BOOL SetTTL(int iTTL)
    {
        LH_ASSERT(INVALID_SOCKET != m_skUdp);
        BOOL bRet = TRUE;
        try
        {
            int rc = setsockopt(m_skUdp, IPPROTO_IP, IP_TTL, (char*)&iTTL, sizeof(iTTL));
            if (SOCKET_ERROR == rc)
            {
                throw CLHException(WSAGetLastError());
            }
        }
        catch (CLHException& e)
        {
            LH_DBGOUT_EXPW(CLHPing::SetTTL, e);
            bRet = FALSE;
        }

        return bRet;
    }

protected:
    // �¼�������麯��
    virtual VOID OnCreateIOCP(VOID* pUserData) override
    {
        CreateRawSocketV4();
    }

    virtual VOID OnCloseIOCP(VOID* pUserData) override
    {
        CloseUdp();
        SafeFreeAddrInfo(m_aiDest);
        SafeFreeAddrInfo(m_aiSrc);

        ClearPingData();
    }

    virtual VOID OnRecvFrom(SOCKADDR_IN& saRemote, DWORD dwTransBytes, void* pUserData, LPST_LH_IOCPUDP_OVERLAPPED pUdpOL) override
    {
        if (NULL == pUserData)
        {
            LH_ASSERT(FALSE);
            return;
        }

        LPST_LH_PING_DATA pPingData = (LPST_LH_PING_DATA)pUserData;
        pPingData->m_dwRecvTime = GetTickCount(); //��ȡ���յ����ݵ�ʱ��
        ++m_iRecv; //���յ������ݰ�������1

        if( m_iCurPingIndex < m_iPingCnt )
        {//������һ��ping����
            LH_DBGOUTA("������[%d]��ping����\n",m_iCurPingIndex + 1);
            SendPingCmd(pPingData->m_nsDataSize,pPingData->m_hEventFinish,pPingData->m_iTTL);
        }
        else
        {
            LH_DBGOUTA("ping���ý���\n");
            m_pCurrentData = NULL;//��ΪNULL,��ֹ�����̳߳�û�رյ�����³�ʱ������Ping����
            //����Ping����֪ͨ
            if( NULL != pPingData->m_hEventFinish )
            {
                SetEvent(pPingData->m_hEventFinish);
            }
        }
    }

	virtual VOID OnWaitTimeOut(DWORD dwWaitTime) override
	{
		// �ȴ���ʱ
        LH_DBGOUTA("CLHPing �ȴ��̳߳�ʱ\n");
        if (NULL != m_pCurrentData)
        {
            if (m_iCurPingIndex < m_iPingCnt)
            {
                // ������һ��ping����
                LH_DBGOUTA("������[%d]��ping����\n", m_iCurPingIndex + 1);
                SendPingCmd(m_pCurrentData->m_nsDataSize, m_pCurrentData->m_hEventFinish, m_pCurrentData->m_iTTL);
            }
            else
            {
				LH_DBGOUTA("ping���ý���\n");
                if (NULL != m_pCurrentData->m_hEventFinish)
                {
                    SetEvent(m_pCurrentData->m_hEventFinish);
                }
            }
        }
	}
protected:
    addrinfo* m_aiDest; //Ŀ�ĵ�ַ
    addrinfo* m_aiSrc; //Դ��ַ
    ST_LH_PING_DATA* m_pCurrentData;

    //����ͳ�ƶ����ʵı���
    int         m_iSend;
    int         m_iRecv;
    int         m_iPingCnt;
    int         m_iCurPingIndex;

    ST_LH_IOCPUDP_OVERLAPPED m_PingOl;
    char                m_pszDestHost[NI_MAXHOST];
    char                m_pszDestServ[NI_MAXSERV];

    CLHPingPacketArray m_arPingPacket;
};

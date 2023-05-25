//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：CLHPing 类的封装，主要实现ping命令的基本功能
//模块名称：网络模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-6-07 19:49:17
//修改日志:
//===================================================================================

#pragma once
#include "LHIOCPUDP.h"
#include "LH_TCPIPHeader.h"
#include <ws2tcpip.h>
#define LH_MAX_IPPACKET 0xffff //最大IP包的大小
class CLHPing : public CLHIOCPUdp
{
public:
	typedef struct _ST_LH_PING_DATA
	{
		USHORT			m_nsDataSize; //随ICMP一起送出的实际数据大小
		DWORD			m_dwRecvTime; //接受到数据包时的时间戳
		DWORD			m_dwLastError; //WSASendto/WSARecvFrom过程中发生的错误码
		HANDLE			m_hEventFinish;//每个Ping结束时需要设置为有信号状态的事件对象句柄,由调用者提供
                                       //可以为NULL,表示不需要接到Ping结束通知
		int				m_iTTL;        //发出ICMP包时指定的TTL值
		union
        {
            ST_ICMP_HDR m_ICMPHdr;        //发送的Ping数据       
            BYTE        m_btSendData[LH_MAX_IPPACKET];       
        };

        union
        {
            ST_IPV4_HDR m_RetPacketHdr;     //返回的数据        
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

            m_pCurrentData = pPingData; //记录当前正在处理的数据块，方便超时处理

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

            //将ICMP包中的剩余的数据部分统一设置成一个随机值,这里用的是0x32
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
	// 工具辅助方法
    //根据传入的ip和端口返回地址信息
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

    //根据地址信息获取本地IP和远程IP
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
    // 主要对外提供功能的方法
    VOID Ping(const char* pDestAddr, int iPingCnt = 4, HANDLE hEventFinish = NULL, USHORT nsPackSize = 32, int iTTL = 128, DWORD dwWaitTime = 6000)
    {
        // 对一个指定的IP地址发出Ping命令
        try
        {
            ClearPingData();
            SafeFreeAddrInfo(m_aiDest);

            //获取ping目标主机地址信息
            m_aiDest = GetAddrInfoV4(pDestAddr, "0");
            if (NULL == m_aiDest)
            {
                throw CLHException(L"无法获取目标地址的信息,不能进行Ping操作!");
            }

            // 获取目标地址的主机名信息
            GetAddrNameInfo((SOCKADDR_IN*)m_aiDest->ai_addr,m_pszDestHost,m_pszDestServ);
            if (!IsValid())
            {
                m_aiSrc = GetAddrInfoV4(NULL, "0");
                if (NULL == m_aiSrc)
                {
                    throw CLHException(L"无法获取本地IPv4的地址信息,绑定失败!");
                }

                //创建线程池,作为Ping命令一个线程就足够了
                if (!Create(*(SOCKADDR_IN*)m_aiSrc->ai_addr, 1, 1))
                {
                    throw CLHException(L"IOCP线程池创建失败,无法完成Ping命令!");
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
        //得到Ping命令结果个数
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
    {//统计Ping的情况
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
                //发送成功的数据包才统计为发送的数据量
                //累计发送的字节数
                iSendBytes += pPingData->m_nsDataSize 
                    + sizeof(ST_ICMP_HDR) + sizeof(ST_IPV4_HDR) 
                    + pPingData->m_RetPacketHdr.ip_totallength;
                //统计最小往返时间
                iMinTime = min(iTime,iMinTime);
            }
        }

        // 平均往复传输速率(重要指标) 时间的单位是毫秒,数据量单位是Bytes,因此直接相除的结果单位正好是KBytes/S
        fAvgSpeed = ((float)((iSendBytes) / iAvgTime)) * 1000.0f / 1024.0f;
        iSend = m_iSend;
        iRecv = m_iRecv;

        //平均包大小
        iAvgPackSize /= i;
        //平均耗时
        iAvgTime /= i;
        //丢包率
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
    // 事件处理的虚函数
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
        pPingData->m_dwRecvTime = GetTickCount(); //获取接收到数据的时间
        ++m_iRecv; //接收到的数据包总数加1

        if( m_iCurPingIndex < m_iPingCnt )
        {//发出下一个ping调用
            LH_DBGOUTA("发出第[%d]个ping调用\n",m_iCurPingIndex + 1);
            SendPingCmd(pPingData->m_nsDataSize,pPingData->m_hEventFinish,pPingData->m_iTTL);
        }
        else
        {
            LH_DBGOUTA("ping调用结束\n");
            m_pCurrentData = NULL;//置为NULL,防止后面线程池没关闭的情况下超时又引起Ping操作
            //发出Ping结束通知
            if( NULL != pPingData->m_hEventFinish )
            {
                SetEvent(pPingData->m_hEventFinish);
            }
        }
    }

	virtual VOID OnWaitTimeOut(DWORD dwWaitTime) override
	{
		// 等待超时
        LH_DBGOUTA("CLHPing 等待线程超时\n");
        if (NULL != m_pCurrentData)
        {
            if (m_iCurPingIndex < m_iPingCnt)
            {
                // 发送下一个ping调用
                LH_DBGOUTA("发出第[%d]个ping调用\n", m_iCurPingIndex + 1);
                SendPingCmd(m_pCurrentData->m_nsDataSize, m_pCurrentData->m_hEventFinish, m_pCurrentData->m_iTTL);
            }
            else
            {
				LH_DBGOUTA("ping调用结束\n");
                if (NULL != m_pCurrentData->m_hEventFinish)
                {
                    SetEvent(m_pCurrentData->m_hEventFinish);
                }
            }
        }
	}
protected:
    addrinfo* m_aiDest; //目的地址
    addrinfo* m_aiSrc; //源地址
    ST_LH_PING_DATA* m_pCurrentData;

    //用于统计丢包率的变量
    int         m_iSend;
    int         m_iRecv;
    int         m_iPingCnt;
    int         m_iCurPingIndex;

    ST_LH_IOCPUDP_OVERLAPPED m_PingOl;
    char                m_pszDestHost[NI_MAXHOST];
    char                m_pszDestServ[NI_MAXSERV];

    CLHPingPacketArray m_arPingPacket;
};

//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：CLHIOCPUdp 类的封装，该类主要提供UDP 异步模式的基本支撑
//模块名称：网络模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-6-05 10:07:31
//修改日志:
//===================================================================================

#pragma once
#include "LH_Win.h"
#include "LH_Mem.h"
#include "LH_Sock.h"
#include "LHIOCPBase.h"
#include "LHWinSockExApi.h"
#include "LHIOPacket.h"

class CLHIOCPUdp : public CLHIOCPBase
{
protected:
	typedef struct _ST_LH_IOCPUDP_OVERLAPPED
	{
		OVERLAPPED m_ol;
		SOCKADDR_IN m_saRemote; //本次IO操作的远端地址
		int m_isaLen; //SOCKADDR_IN 结构大小
		long m_lNetworkEvent; //本次投递的IO操作类型
		DWORD m_dwTransBytes;
		DWORD m_dwFlags;

		void* m_pUserData; //附带额外的用户数据
	}ST_LH_IOCPUDP_OVERLAPPED, *LPST_LH_IOCPUDP_OVERLAPPED;

public:
	CLHIOCPUdp() :
		m_skUdp(INVALID_SOCKET)
	{
		ZeroMemory(&m_saLocal, sizeof(SOCKADDR_IN));
	}

	virtual ~CLHIOCPUdp()
	{

	}

	BOOL Create(SOCKADDR_IN& saLocal, DWORD dwConcurrent = 0, DWORD dwThreadCnt = 0, LPSECURITY_ATTRIBUTES lpSAThread = NULL)
	{
		CopyMemory(&m_saLocal, &saLocal, sizeof(SOCKADDR_IN));
		return CLHIOCPBase::Create(&saLocal, dwConcurrent, dwThreadCnt, lpSAThread);
	}

protected:
	BOOL CreateUDP(SOCKADDR_IN& saLocal)
	{
		BOOL bRet = TRUE;
		try
		{
			m_skUdp = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
			if (INVALID_SOCKET)
			{
				throw CLHException(WSAGetLastError());
			}

			AddHandle2IOCP((HANDLE)m_skUdp);

			if (!bind(m_skUdp, (SOCKADDR*)&m_saLocal, sizeof(SOCKADDR_IN)))
			{
				throw CLHException(WSAGetLastError());
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPUdp::CreateUDP, e);
			bRet = FALSE;
		}

		return bRet;
	}

	BOOL CloseUdp()
	{
		LH_ASSERT(m_skUdp != INVALID_SOCKET);
		shutdown(m_skUdp, SD_BOTH);
		return 0 == closesocket(m_skUdp);
	}

protected:
	//主要的收发方法，供派生类使用
	BOOL RecvFrom0(void* pUserData)
	{
		BOOL bRet = TRUE;
		try
		{
			LPST_LH_IOCPUDP_OVERLAPPED pUdpOl = (LPST_LH_IOCPUDP_OVERLAPPED)LH_CALLOC(sizeof(ST_LH_IOCPUDP_OVERLAPPED));
			pUdpOl->m_lNetworkEvent = FD_READ;
			pUdpOl->m_pUserData = pUserData;
			WSABUF wb = { 0 };
			if (SOCKET_ERROR == WSARecvFrom(m_skUdp, &wb, 1, &pUdpOl->m_dwTransBytes, &pUdpOl->m_dwFlags, (SOCKADDR*)&pUdpOl->m_saRemote, &pUdpOl->m_isaLen, &pUdpOl->m_ol, NULL))
			{
				int iErrorCode = WSAGetLastError();
				if (iErrorCode != WSA_IO_PENDING)
				{
					throw CLHException(GetLastError());
				}
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPUdp::RecvFrom0, e);
			bRet = FALSE;
		}

		return bRet;
	}

	//以非阻塞的方式接受来自远端的数据，配合RecvFrom0使用，在OnRecv消息中调用
	BOOL NonBlockingRecvFromPK(SOCKADDR_IN* psaRemote, CLHIOPacket& packet, DWORD* dwTransBytes, DWORD* dwFlags)
	{
		BOOL bRet = TRUE;
		LH_ASSERT(packet.GetCount() > 0);
		LH_ASSERT(m_skUdp != INVALID_SOCKET);
		LH_ASSERT(psaRemote != NULL);

		try
		{
			int isaLen = sizeof(SOCKADDR_IN);
			if (SOCKET_ERROR == WSARecvFrom(m_skUdp, packet, packet.GetCount(), dwTransBytes, dwFlags, (SOCKADDR*)psaRemote, &isaLen, NULL, NULL))
			{
				int iErrorCode = WSAGetLastError();
				if (iErrorCode != WSAEWOULDBLOCK)
				{
					throw CLHException(iErrorCode);
				}
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPUdp::NonBlockingRecvFromPK, e);
			bRet = FALSE;
		}

		return bRet;
	}

	BOOL SendToPK(SOCKADDR_IN* pRemoteAddr, CLHIOPacket& packet, void* pUserData, LPST_LH_IOCPUDP_OVERLAPPED pUdpOl)
	{
		LH_ASSERT(m_skUdp != INVALID_SOCKET);
		LH_ASSERT(packet.GetCount() > 0);
		LH_ASSERT(pRemoteAddr != NULL);
		LH_ASSERT(pUdpOl != NULL);

		BOOL bRet = TRUE;
		try
		{
			pUdpOl->m_lNetworkEvent = FD_WRITE;
			pUdpOl->m_pUserData = pUserData;

			if (SOCKET_ERROR == WSASendTo(m_skUdp, packet, packet.GetCount(), &pUdpOl->m_dwTransBytes, 0, (SOCKADDR*)&pUdpOl->m_saRemote, sizeof(SOCKADDR_IN), &pUdpOl->m_ol, NULL))
			{
				int iErrorCode = WSAGetLastError();
				if (iErrorCode != WSA_IO_PENDING)
				{
					throw CLHException(iErrorCode);
				}
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPUdp::SendToPK, e);
		}

		return bRet;
	}

protected:
	// 供派生类重载的完成函数
	virtual VOID OnCreateIOCP(VOID* pUserData) override
	{
		// IOCP Thread Pool被成功创建,派生类在这个方法中完成自己的创建过程
		CreateUDP(m_saLocal);
	}

	virtual VOID OnCloseIOCP(VOID* pUserData) override
	{
		// IOCP Thread Pool 被关闭，派生类在这个方法中完成自己的关闭动作，比如释放某些资源等

	}

	virtual void OnIOComplete(DWORD dwTransBytes, LPOVERLAPPED pOL) override
	{
		// IO完成处理
		LPST_LH_IOCPUDP_OVERLAPPED pUdpOl = CONTAINING_RECORD(pOL, ST_LH_IOCPUDP_OVERLAPPED, m_ol);
		if (NULL == pUdpOl)
		{
			return;
		}
		
		try
		{
			switch (pUdpOl->m_lNetworkEvent)
			{
			case FD_READ:
				{
					OnRecvFrom(pUdpOl->m_saRemote, dwTransBytes, pUdpOl->m_pUserData, pUdpOl);
				}
				break;
			case FD_WRITE:
				{
					OnSendToComplete(pUdpOl->m_saRemote, dwTransBytes, pUdpOl->m_pUserData, pUdpOl);
				}
				break;
			default:
				break;
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPUdp::OnIOComplete, e);
		}
	}

	virtual void OnIOError(DWORD dwErrorCode, DWORD dwTransBytes, LPOVERLAPPED pOL) override
	{

	}

	virtual VOID OnRecvFrom(SOCKADDR_IN& saRemote, DWORD dwTransBytes, void* pUserData, LPST_LH_IOCPUDP_OVERLAPPED pUpdOl)
	{
		//派生类主要在这个方法中调用接收数据的方法来接收数据
	}

	virtual VOID OnSendToComplete(SOCKADDR_IN& saRemote, DWORD dwTransBytes, void* pUserData, LPST_LH_IOCPUDP_OVERLAPPED pUdpOl)
	{ //这个类主要用于派生类清理数据
	}

protected:
	SOCKET m_skUdp; //用于udp传输的socket
	SOCKADDR_IN m_saLocal; //监听的本地地址
};

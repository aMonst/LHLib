//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：CLHIOCPTCP 类的封装，该类主要提供面向连接的基本的服务端和客户端的支持
//模块名称：网络模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-6-04 16:44:49
//修改日志:
//===================================================================================

#pragma once
#include "LH_Win.h"
#include "LH_Mem.h"
#include "LH_Sock.h"
#include "LHIOCPBase.h"
#include "LHWinSockExApi.h"
#include "LHIOPacket.h"

class CLHIOCPTCP : public CLHIOCPBase
{
protected:
	//扩展的overlapped ，可以理解为一个socket的session层
	typedef struct _ST_LH_IOCPTCP_OVERLAPPED
	{
		OVERLAPPED m_ol;
		SOCKET m_socket;
		BYTE m_pAcceptExBuf[2 * (sizeof(SOCKADDR_IN) + 16)];
		SOCKADDR_IN* m_pRemoteAddr; //连接到这个socket的远端地址
		SOCKADDR_IN* m_pLocalAddr; // 连接到这个socket的本地地址
		long m_lNetworkEvents;   //投递的操作类型(FD_READ/FD_WRITE等)
		DWORD m_dwTransBytes;   //为WSASend和WSARecv 准备的参数
		DWORD m_dwFlags;		// 为WSASend和WSARecv准备的dwFlags 参数

		void* m_pUserData;		// 额外的用户自定义数据
	}ST_LH_IOCPTCP_OVERLAPPED, *LPST_LH_IOCPTCP_OVERLAPPED;

// methods
public:
	CLHIOCPTCP():
		CLHIOCPBase(),
		m_WSAPI(),
		m_skListen(INVALID_SOCKET),
		m_SocketList(),
		m_OverlappedList()
	{
		ZeroMemory(&m_saListen, sizeof(SOCKADDR_IN));
	}

	virtual ~CLHIOCPTCP()
	{

	}

	BOOL Create(SOCKADDR_IN& saListen, UINT nAcceptCnt = 5000, DWORD dwConcurrent = 0, DWORD dwThreadCnt = 0, LPSECURITY_ATTRIBUTES lpSAThread = NULL)
	{
		BOOL bRet = TRUE;
		try
		{
			CopyMemory(&m_saListen, &saListen, sizeof(SOCKADDR_IN));
			CLHIOCPBase::Create((LPVOID)&nAcceptCnt, dwConcurrent, dwThreadCnt, lpSAThread);
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPTCP::Create, e);
			bRet = FALSE;
		}

		return bRet;
	}

	BOOL Create(DWORD dwConcurrent = 0, DWORD dwThreadCnt = 0, LPSECURITY_ATTRIBUTES lpSAThread = NULL)
	{
		BOOL bRet = TRUE;
		try
		{
			CLHIOCPBase::Create(NULL, dwConcurrent, dwThreadCnt, lpSAThread);
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPTCP::Create, e);
			bRet = FALSE;
		}

		return bRet;
	}

protected:
	BOOL Listen(SOCKADDR_IN& saListen)
	{
		BOOL bRet = TRUE;
		try
		{
			m_skListen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			if (INVALID_SOCKET == m_skListen)
			{
				throw CLHException(WSAGetLastError());
			}

			AddHandle2IOCP((HANDLE)m_skListen);
			if (0 != bind(m_skListen, (SOCKADDR*)&saListen, sizeof(SOCKADDR_IN)))
			{
				throw CLHException(WSAGetLastError());
			}
			if (0 != listen(m_skListen, SOMAXCONN))
			{
				throw CLHException(WSAGetLastError());
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPTCP::Listen, e);
			bRet = FALSE;
		}

		return bRet;
	}

	BOOL CloseListen()
	{
		BOOL bRet = FALSE;
		//关闭监听
		if (m_skListen != INVALID_SOCKET)
		{
			bRet = (0 == closesocket(m_skListen));
			m_skListen = INVALID_SOCKET;
		}

		return bRet;
	}

	//提前创建连接socket等待客户端连接
	BOOL BatchAcceptEx(UINT nAddCnt)
	{
		BOOL bRet = TRUE;
		try
		{
			SOCKET skAcceptEx = INVALID_SOCKET;
			LPST_LH_IOCPTCP_OVERLAPPED pTCPOl = NULL;
			for (UINT i = 0; i < nAddCnt; i++)
			{
				skAcceptEx = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
				if (skAcceptEx == INVALID_SOCKET)
				{
					throw CLHException(WSAGetLastError());
				}

				AddHandle2IOCP((HANDLE)skAcceptEx);

				//分配需要的OVERLAPPED 结构
				pTCPOl = (LPST_LH_IOCPTCP_OVERLAPPED)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ST_LH_IOCPTCP_OVERLAPPED));
				pTCPOl->m_lNetworkEvents = FD_ACCEPT;
				pTCPOl->m_socket = skAcceptEx;


				//调用AcceptEx
				if (!m_WSAPI.AcceptEx(m_skListen, skAcceptEx, pTCPOl->m_pAcceptExBuf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &pTCPOl->m_dwTransBytes, &pTCPOl->m_ol))
				{
					DWORD dwErr = WSAGetLastError();
					if (dwErr != WSA_IO_PENDING && dwErr != WSAECONNRESET)
					{
						//发生错误
						if (skAcceptEx != INVALID_SOCKET)
						{
							closesocket(skAcceptEx);
							skAcceptEx = INVALID_SOCKET;
						}

						if (NULL != pTCPOl)
						{
							HeapFree(GetProcessHeap(), 0, pTCPOl);
						}

						throw CLHException(dwErr);
					}
				}

				m_SocketList.AddTail(skAcceptEx);
				m_OverlappedList.AddTail(pTCPOl);
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPTCP::BatchAcceptEx, e);
			bRet = FALSE;
		}

		return bRet;
	}


	BOOL CloseAcceptEx()
	{
		BOOL bRet = TRUE;
		try
		{
			for (POSITION pos = m_SocketList.GetHeadPosition(); pos != NULL;)
			{
				SOCKET skTemp = m_SocketList.GetNext(pos);
				if (skTemp != INVALID_SOCKET)
				{
					closesocket(skTemp);
				}
			}

			for (POSITION pos = m_OverlappedList.GetHeadPosition(); pos != NULL;)
			{
				LPST_LH_IOCPTCP_OVERLAPPED pTemp = m_OverlappedList.GetNext(pos);
				LH_FREE(pTemp);
			}

			m_SocketList.RemoveAll();
			m_OverlappedList.RemoveAll();
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPTCP::CloseAcceptEx, e);
			bRet = FALSE;
		}
		catch (CAtlException& atle)
		{
			LH_DBGOUTW(L"%s(%d):%s函数捕获ATL异常(0x%08X)\n"
                ,__WFILE__,__LINE__,L"CLHIOCPTCP::CloseAcceptEx",(HRESULT)atle);
			bRet = FALSE;
		}

		return bRet;
	}

	BOOL ReclaimSocket(SOCKET skClient, VOID* pUserData, LPST_LH_IOCPTCP_OVERLAPPED pTcpOl)
	{
		//关闭连接并回收socket, 这里并不删除连接socket，而是利用disconnectex的特性仅回收而不删除
		BOOL bRet = TRUE;
		LH_ASSERT(skClient != INVALID_SOCKET);
		try
		{
			pTcpOl->m_lNetworkEvents = FD_CLOSE;
			pTcpOl->m_pUserData = pUserData;
			shutdown(skClient, SD_BOTH);
			m_WSAPI.DisConnectEx(skClient, &pTcpOl->m_ol, TF_REUSE_SOCKET, 0);
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPTCP::ReclaimSocket, e);
			bRet = FALSE;
		}

		return bRet;
	}

	void ReAcceptEx(SOCKET skAccept, VOID* pUserData, LPST_LH_IOCPTCP_OVERLAPPED pTcpOl)
	{
		//将连接的socket重新丢回连接池
		try
		{
			pTcpOl->m_lNetworkEvents = FD_ACCEPT;

			if (!m_WSAPI.AcceptEx(m_skListen, skAccept, pTcpOl->m_pAcceptExBuf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &pTcpOl->m_dwTransBytes, &pTcpOl->m_ol))
			{
				DWORD dwErr = WSAGetLastError();
				if (dwErr != WSA_IO_PENDING && dwErr != WSAECONNRESET)
				{
					//发生错误
					throw CLHException(dwErr);
				}
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPTCP::ReAcceptEx, e);
		}
	}

	BOOL ConnectEx(const char* lpcIP, short port)
	{
		BOOL bRet = TRUE;
		try
		{
			SOCKET skConnect = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			if (INVALID_SOCKET == skConnect)
			{
				throw CLHException(WSAGetLastError());
			}

			const int on = 1;
			setsockopt(skConnect, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
			AddHandle2IOCP((HANDLE)skConnect);
			SOCKADDR_IN addr;
			ZeroMemory(&addr, sizeof(SOCKADDR_IN));
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = INADDR_ANY;
			addr.sin_port = 0;

			if (0 != bind(skConnect, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN)))
			{
				throw CLHException(WSAGetLastError());
			}

			SOCKADDR_IN saConnect;
			ZeroMemory(&saConnect, sizeof(saConnect));
			saConnect.sin_family = AF_INET;
			saConnect.sin_addr.s_addr = inet_addr(lpcIP);
			saConnect.sin_port = htons(port);

			LPST_LH_IOCPTCP_OVERLAPPED pTCPOL = (LPST_LH_IOCPTCP_OVERLAPPED)LH_CALLOC(sizeof(ST_LH_IOCPTCP_OVERLAPPED));
			pTCPOL->m_socket = skConnect;
			pTCPOL->m_lNetworkEvents = FD_CONNECT;
			if (!m_WSAPI.ConnectEx(skConnect, (SOCKADDR*)&saConnect, sizeof(SOCKADDR_IN), NULL, 0, &pTCPOL->m_dwTransBytes, &pTCPOL->m_ol))
			{
				int iError = WSAGetLastError();
				if (iError != WSA_IO_PENDING)
				{
					//失败
					if (skConnect != INVALID_SOCKET)
					{
						closesocket(skConnect);
						skConnect = INVALID_SOCKET;
					}

					LH_FREE(pTCPOL);
					throw CLHException(WSAGetLastError());
				}
			}

			m_SocketList.AddTail(skConnect);
			m_OverlappedList.AddTail(pTCPOL);
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPTCP::ConnectEx, e);
			bRet = FALSE;
		}

		return bRet;
	}

	BOOL Recv0(SOCKET skSocket, void* pUserData, LPST_LH_IOCPTCP_OVERLAPPED pTCPOL)
	{
		LH_ASSERT(NULL != pTCPOL);
		LH_ASSERT(NULL != m_OverlappedList.Find(pTCPOL));

		BOOL bRet = TRUE;
		try
		{
			pTCPOL->m_lNetworkEvents = FD_READ;
			pTCPOL->m_pUserData = pUserData;
			WSABUF wb = { 0 };
			if (SOCKET_ERROR == WSARecv(skSocket, &wb, 1, &pTCPOL->m_dwTransBytes, &pTCPOL->m_dwFlags, (LPOVERLAPPED)pTCPOL, NULL))
			{
				int iError = WSAGetLastError();
				if (iError != WSA_IO_PENDING)
				{
					throw CLHException(iError);
				}
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPTCP::Recv0, e);
			bRet = FALSE;
		}

		return bRet;
	}

	BOOL NonBlockingRecvPK(SOCKET skClient, CLHIOPacket& packet, void* pUserData, LPST_LH_IOCPTCP_OVERLAPPED pTCPOL)
	{
		LH_ASSERT(packet.GetCount() > 0);
		LH_ASSERT(pTCPOL != NULL);

		BOOL bRet = TRUE;

		try
		{
			pTCPOL->m_lNetworkEvents = FD_READ;
			pTCPOL->m_pUserData = pUserData;
			if (SOCKET_ERROR == WSARecv(skClient, packet, packet.GetCount(), &pTCPOL->m_dwTransBytes, &pTCPOL->m_dwFlags, NULL, NULL))
			{
				int iError = WSAGetLastError();
				if (iError != WSAEWOULDBLOCK)
				{
					throw CLHException(iError);
				}
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPTCP::NonBlockingRecvPK, e);
			bRet = FALSE;
		}

		return bRet;
	}

	BOOL SendPK(SOCKET skClient, CLHIOPacket& packet, void* pUserData, LPST_LH_IOCPTCP_OVERLAPPED pTCPOL)
	{
		LH_ASSERT(packet.GetCount() > 0);
		LH_ASSERT(pTCPOL != NULL);
		LH_ASSERT(m_OverlappedList.Find(pTCPOL) != NULL);
		
		BOOL bRet = TRUE;
		try
		{
			pTCPOL->m_lNetworkEvents = FD_WRITE;
			pTCPOL->m_pUserData = pUserData;
			LPWSABUF buf = packet;
			if (SOCKET_ERROR == WSASend(skClient, buf, packet.GetCount(), &pTCPOL->m_dwTransBytes, 0, (LPOVERLAPPED)&pTCPOL->m_ol, NULL))
			{
				int iError = WSAGetLastError();
				if (iError != WSA_IO_PENDING)
				{
					throw CLHException(iError);
				}
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPTCP::SendPK, e);
			bRet = FALSE;
		}

		return bRet;
	}

protected:
	// override methods
	virtual VOID OnCreateIOCP(VOID* pUserData) override
	{
		// 监听
		try
		{
			if (!Listen(m_saListen))
			{
				throw CLHException(L"监听失败,无法继续创建对象!\n");
			}

			//投递连接socket
			if (!BatchAcceptEx(*(UINT*)pUserData))
			{
				LH_DBGOUT(_T("投递AcceptEx失败,对象仍将继续工作!\n"));
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPTCP::OnCreateIOCP, e);
		}
	}

	virtual VOID OnCloseIOCP(VOID* pUserData) override
	{
		//关闭
		try
		{
			CloseListen();
			CloseAcceptEx();
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPTCP::OnCloseIOCP, e);
		}
	}

	virtual VOID OnIOComplete(DWORD dwTransBytes, LPOVERLAPPED pOL) override
	{
		// IO 完成，用于填充收发数据缓冲，供外部使用
		LPST_LH_IOCPTCP_OVERLAPPED  pTCPOL = CONTAINING_RECORD(pOL,ST_LH_IOCPTCP_OVERLAPPED,m_ol);
		LH_ASSERT(pTCPOL != NULL);

		try
		{
			switch (pTCPOL->m_lNetworkEvents)
			{
			case FD_ACCEPT:
				{
					int iLocalLen = sizeof(SOCKADDR_IN);
					int iRemoteLen = sizeof(SOCKADDR_IN);

					m_WSAPI.GetAcceptExSockaddrs(pTCPOL->m_pAcceptExBuf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
						(SOCKADDR**)&pTCPOL->m_pLocalAddr, &iLocalLen,
						(SOCKADDR**)&pTCPOL->m_pRemoteAddr, &iRemoteLen);
					if (0 != setsockopt(pTCPOL->m_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_skListen, sizeof(SOCKET)))
					{
						throw CLHException(WSAGetLastError());
					}

					OnAcceptEx(pTCPOL->m_socket, pTCPOL->m_pLocalAddr, pTCPOL->m_pRemoteAddr, pTCPOL);
				}
				break;
			case FD_CONNECT:
				{
					if (0 != setsockopt(pTCPOL->m_socket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0))
					{
						throw CLHException(WSAGetLastError());
					}
					OnConnectEx(pTCPOL->m_socket, pTCPOL->m_dwTransBytes, pTCPOL->m_pUserData, pTCPOL);
				}
				break;
			case FD_CLOSE:
				{
					OnDisconnectExComplete(pTCPOL->m_socket, dwTransBytes, pTCPOL->m_pUserData, pTCPOL);
				}
				break;
			case FD_READ:
				{
					OnRecv(pTCPOL->m_socket, dwTransBytes, pTCPOL->m_pUserData, pTCPOL);
				}
				break;
			case FD_WRITE:
				{
					OnSendComplete(pTCPOL->m_socket,dwTransBytes,pTCPOL->m_pUserData,pTCPOL);
				}
				break;
			default:
				break;
			}
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPTCP::OnIOComplete, e);
		}
	}

	virtual VOID OnIOError(DWORD dwErrorCode, DWORD dwTransBytes, LPOVERLAPPED pOL) override
	{
		LPST_LH_IOCPTCP_OVERLAPPED pTCPOL = CONTAINING_RECORD(pOL, ST_LH_IOCPTCP_OVERLAPPED, m_ol);
		LH_ASSERT(NULL != pTCPOL);
		try
		{
			ReclaimSocket(pTCPOL->m_socket, pTCPOL->m_pUserData, pTCPOL);
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(CLHIOCPTCP::OnIOError, e);
		}
	}

	virtual VOID OnAcceptEx(SOCKET skClient, SOCKADDR_IN* pRemoteAddr, SOCKADDR_IN* pLocalAddr, LPST_LH_IOCPTCP_OVERLAPPED pTCPOL)
	{
		int iBufLen = 0;
		//关闭套接字上的发送缓冲，这样能提高效率
		::setsockopt(skClient, SOL_SOCKET, SO_SNDBUF, (const char*)&iBufLen, sizeof(int));
		//强制发送延时算法关闭,直接发送到网络上去
		BOOL bNODELOY = TRUE;
        ::setsockopt(skClient,IPPROTO_TCP,TCP_NODELAY,(char*)&bNODELOY,sizeof(BOOL));

		Recv0(skClient, pTCPOL->m_pUserData, pTCPOL);
	}

	virtual VOID OnRecv(SOCKET skClient, DWORD dwTransBytes, void* pUserData, LPST_LH_IOCPTCP_OVERLAPPED pTCPOL)
	{
		// 调用NonBlockingRecvPK 获取数据
	}

	virtual VOID OnSendComplete(SOCKET skClient, DWORD dwTransBytes, void* pUserData, LPST_LH_IOCPTCP_OVERLAPPED pTCPOL)
	{
		//主要用于派生类清理数据的缓冲
		LH_FREE(pUserData);
	}

	virtual VOID OnDisconnectExComplete(SOCKET skClient,DWORD dwTransBytes, void* pUserData, LPST_LH_IOCPTCP_OVERLAPPED pTCPOL)
    {
		// 重新连接
        ReAcceptEx(skClient,pUserData, pTCPOL);
    }

	virtual VOID OnConnectEx(SOCKET skClient, DWORD dwTransBytes, void* pUserData, LPST_LH_IOCPTCP_OVERLAPPED pTCPOL)
	{
		//执行连接成功之后的操作，例如客户端连接到服务器之后向服务器发送数据
	}

// members
protected:
    SOCKET      m_skListen;             //监听的SOCKET句柄
    SOCKADDR_IN m_saListen;             //监听的本地地址

    typedef CAtlList<SOCKET> CLHSocketList; //管理远程连接的socket列表
    typedef CAtlList<LPST_LH_IOCPTCP_OVERLAPPED> CLHOLList; //管理远程连接的overlapped结构

    CLHSocketList m_SocketList;
    CLHOLList     m_OverlappedList;
public:
    CLHWinSockExApi m_WSAPI;    //Winsock2 扩展API封装类对象， 方便直接引用WinSock 的扩展函数
};

//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：CLHWinSockExApi 类的封装，该类用于加载常用的WinSock 扩展API
//模块名称：网络模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-6-02 21:14:54
//修改日志:
//===================================================================================

#pragma once
#include "LH_Win.h"
#include "LH_Sock.h"
#include "LHString.h"
#include "LHException.h"

class CLHWinSockExApi
{
public:
	CLHWinSockExApi(SOCKET skTemp = INVALID_SOCKET)
	{
		LoadAllWinSockApi(skTemp);
	}

	CLHWinSockExApi(int af, int type, int protocol)
	{
		LoadAllWinSockApi(af, type, protocol);
	}

	virtual ~CLHWinSockExApi()
	{

	}

protected:
	BOOL LoadWSAFunc(SOCKET& sockTemp, GUID& funGuid, void*& pFun)
	{
		DWORD dwBytes = 0;
		BOOL bRet = TRUE;

		try
		{
			if (sockTemp == INVALID_SOCKET)
			{
				throw CLHException(L"传入了无效的Socket值");
			}

			if (SOCKET_ERROR == ::WSAIoctl(sockTemp, SIO_GET_EXTENSION_FUNCTION_POINTER, &funGuid, sizeof(funGuid), &pFun, sizeof(pFun), &dwBytes, NULL, NULL))
			{
				pFun = NULL;
				throw CLHException(WSAGetLastError());
			}

#ifdef _DEBUG
            {
                GUID Guid = WSAID_ACCEPTEX;
                if (IsEqualGUID(Guid, funGuid))
                {
                    LH_DBGOUT(_T("AcceptEx 加载成功!\n"));
                }
            }

            {
                GUID Guid = WSAID_CONNECTEX;
                if (IsEqualGUID(Guid, funGuid))
                {
                    LH_DBGOUT(_T("ConnectEx 加载成功!\n"));
                }
            }

            {
                GUID Guid = WSAID_DISCONNECTEX;
                if (IsEqualGUID(Guid, funGuid))
                {
                    LH_DBGOUT(_T("DisconnectEx 加载成功!\n"));
                }
            }

            {
                GUID Guid = WSAID_GETACCEPTEXSOCKADDRS;
                if (IsEqualGUID(Guid, funGuid))
                {
                    LH_DBGOUT(_T("GetAcceptExSockaddrs 加载成功!\n"));
                }
            }
            {
                GUID Guid = WSAID_TRANSMITFILE;
                if (IsEqualGUID(Guid, funGuid))
                {
                    LH_DBGOUT(_T("TransmitFile 加载成功!\n"));
                }

            }
            {
                GUID Guid = WSAID_TRANSMITPACKETS;
                if (IsEqualGUID(Guid, funGuid))
                {
                    LH_DBGOUT(_T("TransmitPackets 加载成功!\n"));
                }

            }
            {
                GUID Guid = WSAID_WSARECVMSG;
                if (IsEqualGUID(Guid, funGuid))
                {
                    LH_DBGOUT(_T("WSARecvMsg 加载成功!\n"));
                }
            }

#if(_WIN32_WINNT >= 0x0600)
            {
                GUID Guid = WSAID_WSASENDMSG;
                if (IsEqualGUID(Guid, funGuid))
                {
                    LH_DBGOUT(_T("WSASendMsg 加载成功!\n"));
                }
            }
#endif
#endif
		}
		catch (CLHException& e)
		{
            LH_DBGOUT_EXPW(CLHWinSockExApi::LoadWSAFunc, e);
		}

        return NULL != pFun;
    }

protected:
    BOOL LoadAcceptExFun(SOCKET& skTemp)
    {
        GUID GuidAcceptEx = WSAID_ACCEPTEX;
        return LoadWSAFunc(skTemp, GuidAcceptEx, (void*&)m_pfnAcceptEx);
    }

    BOOL LoadConnectExFun(SOCKET& skTemp)
    {
        GUID GuidConnectEx = WSAID_CONNECTEX;
        return LoadWSAFunc(skTemp, GuidConnectEx, (void*&)m_pfnConnectEx);
    }

    BOOL LoadDisconnectExFun(SOCKET& skTemp)
    {
        GUID GuidDisconnectEx = WSAID_DISCONNECTEX;
        return LoadWSAFunc(skTemp, GuidDisconnectEx, (void*&)m_pfnDisconnectEx);
    }

    BOOL LoadGetAcceptExSockaddrsFun(SOCKET& skTemp)
    {
        GUID GuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
        return LoadWSAFunc(skTemp, GuidGetAcceptExSockaddrs, (void*&)m_pfnGetAcceptExSockaddrs);
    }

    BOOL LoadTransmitFileFun(SOCKET& skTemp)
    {
        GUID GuidTransmitFile = WSAID_TRANSMITFILE;
        return LoadWSAFunc(skTemp, GuidTransmitFile, (void*&)m_pfnTransmitfile);
    }

    BOOL LoadTransmitPacketsFun(SOCKET& skTemp)
    {
        GUID GuidTransmitPackets = WSAID_TRANSMITPACKETS;
        return LoadWSAFunc(skTemp, GuidTransmitPackets, (void*&)m_pfnTransmitPackets);
    }

    BOOL LoadWSARecvMsgFun(SOCKET& skTemp)
    {
        GUID GuidWSARecvMsgFun = WSAID_WSARECVMSG;
        return LoadWSAFunc(skTemp, GuidWSARecvMsgFun, (void*&)m_pfnWSARecvMsg);
    }

#if (_WIN32_WINNT >= 0x0600)
    BOOL LoadWSASendMsgFun(SOCKET& skTemp)
    {
        GUID GuidWSASendMsg = WSAID_WSASENDMSG;
        return LoadWSAFunc(skTemp, GuidWSASendMsg, (void*&)m_pfnWSASendMsg);
    }
#endif

public:
    BOOL LoadAllWinSockApi(SOCKET skTemp)
    {
        BOOL bCreatedSocket = FALSE;
        BOOL bRet = TRUE;

        try
        {
            if (skTemp == INVALID_SOCKET)
            {
                // 如果传入的是无效的socket则默认创建tcp协议的套接字，加载的函数只支持tcp协议
                skTemp = ::WSASocket(AF_INET,
                    SOCK_STREAM,
                    IPPROTO_TCP,
                    NULL,
                    0,
                    WSA_FLAG_OVERLAPPED);

                bCreatedSocket = (skTemp != INVALID_SOCKET);
                if (!bCreatedSocket)
                {
                    throw CLHException((DWORD)WSAGetLastError());
                }
            }

#if (_WIN32_WINNT > 0x0600)
            bRet = (LoadAcceptExFun(skTemp) &&
                LoadGetAcceptExSockaddrsFun(skTemp) &&
                LoadTransmitFileFun(skTemp) &&
                LoadTransmitPacketsFun(skTemp) &&
                LoadDisconnectExFun(skTemp) &&
                LoadConnectExFun(skTemp) &&
                LoadWSARecvMsgFun(skTemp) &&
                LoadWSASendMsgFun(skTemp));
#else
            bRet = (LoadAcceptExFun(skTemp) &&
                LoadGetAcceptExSockaddrsFun(skTemp) &&
                LoadTransmitFileFun(skTemp) &&
                LoadTransmitPacketsFun(skTemp) &&
                LoadDisconnectExFun(skTemp) &&
                LoadConnectExFun(skTemp) &&
                LoadWSARecvMsgFun(skTemp));
#endif
        }
        catch (CLHException& e)
        {
            LH_DBGOUT_EXPW(CLHWinSockExApi::LoadAllWinSockApi, e);
            bRet = FALSE;
        }

        if (bCreatedSocket)
        {
            closesocket(skTemp);
        }

        return bRet;
    }

    BOOL LoadAllWinSockApi(int af, int type, int protocol)
    {
        BOOL bRet = FALSE;
        SOCKET skTemp = INVALID_SOCKET;

        try
        {
            skTemp = ::WSASocket(af, type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
            if (INVALID_SOCKET == skTemp)
            {
                throw CLHException(WSAGetLastError());
            }

            bRet = LoadAllWinSockApi(skTemp);
        }
        catch (CLHException& e)
        {
            bRet = FALSE;
            LH_DBGOUT_EXPW(CLHWinSockExApi, e);
        }

        if (skTemp != INVALID_SOCKET)
        {
            closesocket(skTemp);
        }

        return bRet;
    }

    __forceinline BOOL AcceptEx(
        SOCKET sListenSocket,
        SOCKET sAcceptSocket,
        PVOID lpOutputBuffer,
        DWORD dwReceiveDataLength,
        DWORD dwLocalAddressLength,
        DWORD dwRemoteAddressLength,
        LPDWORD lpdwBytesReceived,
        LPOVERLAPPED lpOverlapped
    )
    {
        LH_ASSERT(m_pfnAcceptEx != NULL);
        return m_pfnAcceptEx(sListenSocket, sAcceptSocket, lpOutputBuffer, dwReceiveDataLength, dwLocalAddressLength, dwRemoteAddressLength, lpdwBytesReceived, lpOverlapped);
    }

    __forceinline BOOL ConnectEx(
        SOCKET s,
        const struct sockaddr FAR* name,
        int namelen,
        PVOID lpSendBuffer,
        DWORD dwSendDataLength,
        LPDWORD lpdwBytesSent,
        LPOVERLAPPED lpOverlapped)
    {
        LH_ASSERT(m_pfnConnectEx != NULL);
        return m_pfnConnectEx(s, name, namelen, lpSendBuffer, dwSendDataLength, lpdwBytesSent, lpOverlapped);
    }

    __forceinline BOOL DisConnectEx(
        SOCKET s,
        LPOVERLAPPED lpOverlapped,
        DWORD  dwFlags,
        DWORD  dwReserved)
    {
        LH_ASSERT(m_pfnDisconnectEx != NULL);
        return m_pfnDisconnectEx(s, lpOverlapped, dwFlags, dwReserved);
    }

    __forceinline VOID GetAcceptExSockaddrs(
        PVOID lpOutputBuffer,
        DWORD dwReceiveDataLength,
        DWORD dwLocalAddressLength,
        DWORD dwRemoteAddressLength,
        sockaddr** LocalSockaddr,
        LPINT LocalSockaddrLength,
        sockaddr** RemoteSockaddr,
        LPINT RemoteSockaddrLength)
    {
        LH_ASSERT(m_pfnGetAcceptExSockaddrs != NULL);
        return m_pfnGetAcceptExSockaddrs(lpOutputBuffer, dwReceiveDataLength, dwLocalAddressLength, dwRemoteAddressLength, LocalSockaddr, LocalSockaddrLength, RemoteSockaddr, RemoteSockaddrLength);
    }

    __forceinline BOOL TransmitFile(
        SOCKET hSocket,
        HANDLE hFile,
        DWORD nNumberOfBytesToWrite,
        DWORD nNumberOfBytesPerSend,
        LPOVERLAPPED lpOverlapped,
        LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
        DWORD dwReserved)
    {
        LH_ASSERT(m_pfnTransmitfile != NULL);
        return m_pfnTransmitfile(hSocket, hFile, nNumberOfBytesToWrite, nNumberOfBytesPerSend, lpOverlapped, lpTransmitBuffers, dwReserved);
    }

    __forceinline BOOL TransmitPackets(
        SOCKET hSocket,
        LPTRANSMIT_PACKETS_ELEMENT lpPacketArray,
        DWORD nElementCount,
        DWORD nSendSize,
        LPOVERLAPPED lpOverlapped,
        DWORD dwFlags)
    {
        LH_ASSERT(m_pfnTransmitPackets != NULL);
        return m_pfnTransmitPackets(hSocket, lpPacketArray, nElementCount, nSendSize, lpOverlapped, dwFlags);
    }

    __forceinline INT WSARecvMsg(
        SOCKET s,
        LPWSAMSG lpMsg,
        LPDWORD lpdwNumberOfBytesRecvd,
        LPWSAOVERLAPPED lpOverlapped,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
    {
        LH_ASSERT(m_pfnWSARecvMsg != NULL);
        return m_pfnWSARecvMsg(s, lpMsg, lpdwNumberOfBytesRecvd, lpOverlapped, lpCompletionRoutine);
    }

#if(_WIN32_WINNT >= 0x0600)
    __forceinline INT WSASendMsg(
        SOCKET s,
        LPWSAMSG lpMsg,
        DWORD dwFlags,
        LPDWORD lpNumberOfBytesSent,
        LPWSAOVERLAPPED lpOverlapped,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
    {
        LH_ASSERT(NULL != m_pfnWSASendMsg);
        return m_pfnWSASendMsg(s, lpMsg, dwFlags, lpNumberOfBytesSent, lpOverlapped, lpCompletionRoutine);
    }
#endif
protected:
    LPFN_ACCEPTEX m_pfnAcceptEx;
    LPFN_CONNECTEX m_pfnConnectEx;
    LPFN_DISCONNECTEX m_pfnDisconnectEx;
    LPFN_GETACCEPTEXSOCKADDRS m_pfnGetAcceptExSockaddrs;
    LPFN_TRANSMITFILE m_pfnTransmitfile;
    LPFN_TRANSMITPACKETS m_pfnTransmitPackets;
    LPFN_WSARECVMSG m_pfnWSARecvMsg;
#if(_WIN32_WINNT >= 0x0600)
    LPFN_WSASENDMSG m_pfnWSASendMsg;
#endif
};

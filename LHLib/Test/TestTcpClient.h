#pragma once
#include "../Public/LHLib.h"

class CLHTestTCPClient : public CLHIOCPTCP
{
protected:
	virtual VOID OnCreateIOCP(VOID* pUserData) override
	{
		ConnectEx("203.208.50.162", 80); //Connect gstatic.com
	}

	virtual VOID OnConnectEx(SOCKET skClient, DWORD dwTransBytes, void* pUserData, LPST_LH_IOCPTCP_OVERLAPPED pTCPOL) override
	{
		//发送一个HTTP请求
		char pszBuf[] = "GET /generate_204 HTTP/1.1\r\n\
Host: www.gstatic.com\r\n\
Pragma: no-cache\r\n\
Cache-Control: no-cache\r\n\
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/112.0.0.0 Safari/537.36\r\n\
Accept-Encoding: gzip, deflate\r\n\
Accept-Language: zh-CN,zh;q=0.9\r\n\
Connection: close\r\n\
\r\n\
";
		size_t nLen = sizeof(pszBuf) / sizeof(char);
		char* buf = (char*)LH_CALLOC(nLen);
		StringCchCopyA(buf, nLen, pszBuf);

		CLHIOPacket packet;
		packet.Add(buf, nLen);
		SendPK(skClient, packet, buf, pTCPOL);
	}

	virtual VOID OnSendComplete(SOCKET skClient, DWORD dwTransBytes, void* pUserData, LPST_LH_IOCPTCP_OVERLAPPED pTCPOL)
	{
		//主要用于派生类清理数据的缓冲
		LH_FREE(pUserData);

		Recv0(skClient, NULL, pTCPOL); //提前发送接收数据的命令，到真实有数据待接收时会调用OnRecv完成函数
	}

	virtual VOID OnRecv(SOCKET skClient, DWORD dwTransBytes, void* pUserData, LPST_LH_IOCPTCP_OVERLAPPED pTCPOL) override
	{
		// 在该函数中正式进行读数据的操作，因为此时网络上已经有数据待读取，所以这里读取的方式是非阻塞的
		char* recvBuf = (char*)LH_CALLOC(1024);
		CLHIOPacket recvPacket;
		recvPacket.Add(recvBuf, 1024);
		NonBlockingRecvPK(skClient, recvPacket, recvBuf, pTCPOL);

		printf("%s", recvBuf);
		LH_FREE(pTCPOL->m_pUserData);

		// 按照HTTP协议，接收数据之后应该断开与服务器的连接
		ReclaimSocket(skClient, pUserData, pTCPOL);
	}

public:

};


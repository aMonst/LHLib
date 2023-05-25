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
		//����һ��HTTP����
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
		//��Ҫ�����������������ݵĻ���
		LH_FREE(pUserData);

		Recv0(skClient, NULL, pTCPOL); //��ǰ���ͽ������ݵ��������ʵ�����ݴ�����ʱ�����OnRecv��ɺ���
	}

	virtual VOID OnRecv(SOCKET skClient, DWORD dwTransBytes, void* pUserData, LPST_LH_IOCPTCP_OVERLAPPED pTCPOL) override
	{
		// �ڸú�������ʽ���ж����ݵĲ�������Ϊ��ʱ�������Ѿ������ݴ���ȡ�����������ȡ�ķ�ʽ�Ƿ�������
		char* recvBuf = (char*)LH_CALLOC(1024);
		CLHIOPacket recvPacket;
		recvPacket.Add(recvBuf, 1024);
		NonBlockingRecvPK(skClient, recvPacket, recvBuf, pTCPOL);

		printf("%s", recvBuf);
		LH_FREE(pTCPOL->m_pUserData);

		// ����HTTPЭ�飬��������֮��Ӧ�öϿ��������������
		ReclaimSocket(skClient, pUserData, pTCPOL);
	}

public:

};


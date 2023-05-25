// LHLib.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "Public/LHLib.h"
#include "Test/TestFile.h"
#include "Test/TestTcpClient.h"
#include "Test/TestThread.h"

#ifdef LH_MEMORY_LEAK
#undef	new
#define new new(__WFILE__,__LINE__)
#endif

void testString()
{
	CLHString str1 = L"Hello, World";
	wchar_t* pStr = str1.GetString();
	CLHString str2 = L"你好世界";
	pStr = str2.GetString();

	CLHString str3 = str1 + str2;
	pStr = str3.GetString();

	char szAnsi[1024] = "";
	size_t iBuffSize = str3.toAnsiString(szAnsi, 1024);
	CLHString str4 = CLHString::AnsiToString(szAnsi, 1024);
	iBuffSize = str3.toUtf8String(szAnsi, 1024);

	CLHString str5 = CLHString::Utf8ToString(szAnsi, 1024);
	str5.Format(L"[%s][%ld], 当前位置进行字符串格式化....", __WFILE__, __LINE__);
}

void testSEH()
{
	LH_SEH_INIT(); //在使用 try catch 的函数中需要调用该宏设置异常处理函数
	try
	{
		//throw CLHException(L"抛出一个自定义异常，异常代码:%d", 111);
		//throw CLHException(GetLastError());
		int i = 0;
		i /= i;
	}
	catch (CLHException& e)
	{
		LH_DBGOUT_EXPW(testSEH, e);
	}
}

void testMemoryLeak()
{
	CLHObject* p = new CLHObject[100];
	delete[] p;
}

void testSystemInfo()
{
	LH_SEH_INIT();
	try {
		CLHSystemInfo& si = CLHSystemInfo::GetSystemInfo();
		CLHString sSysInfo;
		std::wcout << L"Computer Name:" << si.GetComputerName() << std::endl;
		//用户名
		std::wcout << L"\tUser Name:" << si.GetUserName() << std::endl;
		//系统信息
		std::wcout << L"System Info: " << std::endl;
		std::wcout << L"  OEM ID: " << si.GetSysInfo()->dwOemId << std::endl;
		std::wcout << L"  Number of processors: " << si.GetSysInfo()->dwNumberOfProcessors << std::endl;
		std::wcout << L"  Page size: " << si.GetSysInfo()->dwPageSize << std::endl;
		std::wcout << L"  Processor type: " << si.GetSysInfo()->dwProcessorType << std::endl;
		std::wcout << L"  Minimum application address: " << std::hex << si.GetSysInfo()->lpMinimumApplicationAddress << std::endl;
		std::wcout << L"  Maximum application address: " << std::hex << si.GetSysInfo()->lpMaximumApplicationAddress << std::endl;
		std::wcout << L"  Active processor mask: ";
		for (int i = sizeof(si.GetSysInfo()->dwActiveProcessorMask) * 8 - 1; i >= 0; --i)
		{
			std::wcout << ((si.GetSysInfo()->dwActiveProcessorMask >> i) & 1);
		}
		std::wcout << std::endl;
		std::wcout << L"OS Info:" << si.GetOSString() << std::endl;

		std::wcout << L"Windows Directory:" << si.GetWinDir();
		std::wcout << L"System Directory:" << si.GetSysDir() << std::endl;

		std::wcout << L"System Environment Variable:" << std::endl;
		for (DWORD i = 0; i < si.GetEnvValsCnt(); i++)
		{
			std::wcout << L"\t" << si.GetEnvVal(i) << std::endl << std::endl;
		}
	}
	catch(CLHException& e) {
		std::wcout << e.GetReason();
	}

}

void testThread()
{
	CTestThread* pThreadObj = new CTestThread;
	pThreadObj->CreateThread(CREATE_SUSPENDED);
	pThreadObj->SetAffinityCPU(0);
	pThreadObj->ResumeThread();
	pThreadObj->Wait(INFINITE);
	delete pThreadObj;
}

CLHSRWGlobal<int> g_SRWNum;

class CWriteThread :public CLHThread
{
public:
	CWriteThread()
	{

	}
	virtual~CWriteThread()
	{

	}
protected:
	virtual UINT __stdcall Run(void* pParam) override
	{
		try
		{
			int i = 0;
			g_SRWNum.Lock();
			for (; i < 5678; i++)
			{
				g_SRWNum.DirectWrite(i);
				SwitchToThread();
			}
			CLHString csInfo;
			csInfo.Format(L"CWriteThread对象[A:0x%08x]线程[ID:0x%x]写入全局变量值:%d\n"
				, this, m_dwThreadID, g_SRWNum.DirectRead());
			std::wcout << csInfo;
			g_SRWNum.UnLock();
			Sleep(1000);
			g_SRWNum.WakeAll();
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(ThreadProc, e);
		}
		return 0;
	}
};

class CReadThread :public CLHThread
{
public:
	CReadThread()
	{

	}
	virtual~CReadThread()
	{

	}
protected:
	virtual UINT __stdcall Run(void* pParam) override
	{
		try
		{
			int iVal = 0;
			CLHString csInfo;
			csInfo.Format(L"CReadThread对象[A:0x%08x]线程[ID:0x%x]读取全局变量值%s:%d\n"
				, this, m_dwThreadID
				, g_SRWNum.ReadAndWait(iVal, 20) ? _T("成功") : _T("失败"), iVal);
		}
		catch (CLHException& e)
		{
			LH_DBGOUT_EXPW(ThreadProc, e);
		}
		return 0;
	}
};

void testSRWGlobal()
{
	LH_SEH_INIT();
	CWriteThread* pWriteThread = NULL;
	CReadThread* pReadThread = NULL;
	try
	{
		int iReadCnt = 20;
		//注意都使用堆对象来创建线程对象
		pWriteThread = new CWriteThread;
		pReadThread = new CReadThread[iReadCnt];
		CLHSystemInfo& sysInfo = CLHSystemInfo::GetSystemInfo();

		pWriteThread->CreateThread(CREATE_SUSPENDED);
		pWriteThread->SetAffinityCPU(0);
		pWriteThread->ResumeThread();
		for (int i = 0; i < iReadCnt; i++)
		{
			pReadThread[i].CreateThread(CREATE_SUSPENDED);
			pReadThread[i].SetAffinityCPU(i % sysInfo.GetSysInfo()->dwNumberOfProcessors);
			pReadThread[i].ResumeThread();
		}

	}
	catch (CLHException& e)
	{
			LH_DBGOUT_EXPW(testSRWGlobal, e);
	}
	_tsystem(_T("PAUSE"));
	delete[] pReadThread;
	delete pWriteThread;
}


void testFile()
{
	try
	{
		CLHTextFile f(_T("LHLibUseIOCP.txt"));

		UINT  nLen = 256;
		DWORD dwRead = 0;
		WORD* pPrefix = (WORD*)LH_CALLOC(sizeof(WORD));
		*pPrefix = MAKEWORD(0xff, 0xfe);
		f.Write(pPrefix, sizeof(WORD));//写入UNICODE前缀

		std::wcout<< L"请输入第一个文件的内容:\n";
		//重新分配个缓存
		TCHAR* pBuf = (TCHAR*)LH_CALLOC(nLen);
		while (ReadConsole(GetStdHandle(STD_INPUT_HANDLE), pBuf, nLen, &dwRead, NULL))
		{
			if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE
				, pBuf, 4, _T("exit"), 4))
			{
				//退出输入循环
				if (NULL != pBuf)
				{
					HeapFree(GetProcessHeap(), 0, pBuf);
					pBuf = NULL;
				}
				break;
			}
			f.Write(pBuf, dwRead * sizeof(TCHAR));
			pBuf = (TCHAR*)LH_CALLOC(nLen);
		}

		std::wcout<< L"\n请输入第二个文件的内容:\n";

		CLHFile f2(_T("LHLibNotIOCP.txt"), FALSE);
		f2.Write("\xff\xfe", 2);	//写入UNICODE前缀
		TCHAR pBuf2[256] = {};
		while (ReadConsole(GetStdHandle(STD_INPUT_HANDLE), pBuf2, nLen, &dwRead, NULL))
		{
			if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE
				, pBuf2, 4, _T("exit"), 4))
			{
				//退出输入循环
				break;
			}
			f2.Write(pBuf2, dwRead * sizeof(TCHAR));
		}

	}
	catch (CLHException& e)
	{
		LH_DBGOUT(_T("发生异常:Code=0x%08x,原因=%s\n"), e.GetErrorCode(), e.GetReason());
	}
}

void TestTcpClient()
{
	CLHTestTCPClient client;
	client.Create();
	do 
    {
        SHORT nVirtKey = GetAsyncKeyState(VK_RETURN); 
        if (nVirtKey == 1) 
        {//按Enter键退出
            break;
        }
        LH_DBGOUT(_T("主线程进入等待状态......\n"));
    } while (WAIT_TIMEOUT == WaitForSingleObject(GetCurrentThread(),1000));

	client.Close(NULL);
}

void TestPing()
{
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hEvent != NULL)
	{
		CLHPing ping;
		ping.Ping("www.baidu.com", 4, hEvent);
		WaitForSingleObject(hEvent, INFINITE);

		CLHPing::LPST_LH_PING_DATA pPingData = NULL;
        for(int i = 0; i < ping.GetPingResultCnt(); i ++ )
        {
            pPingData = ping[i];
            printf("%d - ",i + 1);
            if( NULL != pPingData )
            {
                if( 0 == pPingData->m_dwRecvTime )
                {//超时
                    printf("Request timed out.\n");
                }
                else
                {
                    DWORD dwTime = pPingData->m_dwRecvTime - pPingData->m_ICMPHdr.icmp_timestamp;
                    if( 0 == dwTime )
                    {
                        printf("Reply from %s : bytes = %d time < 1ms TTL=%d\n",
                            ping.GetDestHostName(),pPingData->m_nsDataSize,pPingData->m_RetPacketHdr.ip_ttl);

                    }
                    else
                    {
                        printf("Reply from %s : bytes=%d time=%ums TTL=%d\n"
                            ,ping.GetDestHostName(),pPingData->m_nsDataSize,dwTime,pPingData->m_RetPacketHdr.ip_ttl);
                    }
                }
            }
        }

		int iAvgPackSize = 0;
        int iAvgTime     = 0;
        int iMaxTime     = 0;
        int iMinTime     = 0; 
        float fAvgSpeed  = 0;
        int iSend        = 0;
        int iRecv        = 0;
        float fLost      = 0;
        ping.Stat(iAvgPackSize,iAvgTime,iMaxTime,iMinTime,fAvgSpeed,iSend,iRecv,fLost);

        printf("统计信息:\n");
        printf("\t平均数据包大小:%d 平均耗时:%d 最大耗时:%d 最小耗时:%d \n\t平均速率:%f KBytes/s 已发送:%d 已接收:%d 丢包率:%f%% \n"
            ,iAvgPackSize,iAvgTime,iMaxTime,iMinTime,fAvgSpeed,iSend,iRecv,fLost * 100);

		CloseHandle(hEvent);

	}
}

void TestTraceRoute()
{
	CLHTraceroute tr;
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hEvent != NULL)
	{
		const char* pDest = "www.sina.com";
		tr.TraceRoute(pDest, hEvent);
		WaitForSingleObject(hEvent, INFINITE);
		printf("TraceRoute %s [%s]\n", pDest, tr.GetDestHostName());
		 
		 CLHTraceroute::LPST_LH_TRACEROUTE_DATA pTrData = NULL;
		 for(int i = 0; i < tr.GetRouteNodeCnt(); i ++)
		 {
			 pTrData = tr[i];
			 printf("%d %dms %s [%s]\n"
				 ,i + 1,pTrData->m_iTime
				 ,inet_ntoa(pTrData->m_Addr)
				 ,pTrData->m_pszHost);
		 }
		 printf("追踪结束,共扫描到%d个路由节点\n",tr.GetRouteNodeCnt());

		 CloseHandle(hEvent);
	}
}

int main()
{
//	//testMemoryLeak();
//	//testString();
//	//testSEH();
//	//testSystemInfo();
//	//testThread();
//	//testSRWGlobal();
//	testFile();

	LH_OPEN_HEAP_LFH(GetProcessHeap());
	CLHSocketStartup initSock;
	//TestTcpClient();
	//TestPing();
	TestTraceRoute();
	return 0;
}

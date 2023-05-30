//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：包含库中的一些公共头文件, 它作为库对外开放的定义入口
//模块名称：公共支持文件
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-20 17:30:27
//修改日志:
//===================================================================================
#pragma once
#ifndef __LH__LIB_H__
#define __LH_LIB_H__

#include "LH_Win.h"
//添加LHLib中的库文件
#include "LH_Def.h"
#include "LH_Mem.h"
#include "LH_Debug.h"
#include "LH_Privilege.h"
#include "LHHeap.h"
#include "LHObject.h"
#include "LHException.h"
#include "LHSystemInfo.h"
#include "LHThread.h"
#include "LHSRWGlobal.h"
#include "LHFile.h"
#include "LHFileSystemWatcher.h"
#include "LHJob.h"

//服务支持部分
#include "LHServices.h"
#include "LHSvcMgr.h"



//WinSock 部分
#include "LH_SOCK.h"
#include "LHSocketStartup.h"

//TCP/IP协议辅助部分
#include "LH_TCPIP.h"

//Winsock2封装部分
#include "LHWinSockExApi.h"
#include "LHIOCPTCP.h"
#include "LHIOCPUDP.h"
#include "LHPing.h"
#include "LHTraceroute.h"
#endif

#pragma once
//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：CLHIOPacket 类的封装，该类主要用来封装Winsock的散播和聚集传输特性
//模块名称：网络模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-6-04 10:11:20
//修改日志:
//===================================================================================

#include "LH_Win.h"
#include "LH_Debug.h"
#include "LHObject.h"
#include "LHString.h"
#include "LHException.h"
#include <atlcoll.h>

class CLHIOPacket : public CLHObject
{
public:
	typedef CAtlArray<WSABUF> CLHWSABufArray;

public:
	CLHIOPacket()
		: m_WsaBuffArray()
	{
	}

	virtual ~CLHIOPacket()
	{

	}

	//用于支持WSASend/WSASendto/WSARecv/WSARecvfrom的方法
	operator LPWSABUF ()
	{
		return m_WsaBuffArray.GetData();
	}

	DWORD GetCount() const
	{
		return m_WsaBuffArray.GetCount();
	}

	//添加待发送数据缓冲的方法
    //要注意的是CLHIOPacket并不对缓冲进行分配和释放的管理操作,这个由调用者负责
	BOOL Add(void* pBuf, size_t len)
	{
		WSABUF wsaBuf = { 0 };
		wsaBuf.buf = (char*)pBuf;
		wsaBuf.len = len;

		return (m_WsaBuffArray.Add(wsaBuf) > 0);
	}

	template<class T>
	BOOL Add(T& data)
	{
		WSABUF wb = { 0 };
		wb.len = sizeof(T);
		wb.buf = (char*)&data;
		return m_WsaBuffArray.Add(wb) > 0;
	}

	template<class T>
	BOOL Insert(T& data, size_t index)
	{
		WSABUF wb = { 0 };
		wb.len = sizeof(T);
		wb.buf = (char*)data;
		return m_WsaBuffArray.InsertAt(index, wb);
	}

	void RemoveHead(void*& pBuf, size_t& len)
	{
		if (m_WsaBuffArray.GetCount() > 0)
		{
			WSABUF wb = m_WsaBuffArray.GetAt(0);
			len = wb.len;
			pBuf = wb.buf;
		}

		m_WsaBuffArray.RemoveAt(0);
	}

	void RemoveAt(void*& pBuf, size_t& len, size_t index)
	{
		if (m_WsaBuffArray.GetCount() > index)
		{
			WSABUF wb = m_WsaBuffArray.GetAt(index);
			len = wb.len;
			pBuf = wb.buf;
		}

		m_WsaBuffArray.RemoveAt(index);
	}

	const WSABUF& operator[](size_t iElement) const
	{
		return m_WsaBuffArray[iElement];
	}

	WSABUF& operator[] (size_t iElement)
	{
		return m_WsaBuffArray[iElement];
	}

protected:
	CLHWSABufArray m_WsaBuffArray; //待接收或者发送的缓冲块
};

#pragma once
//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�������CLHIOPacket ��ķ�װ��������Ҫ������װWinsock��ɢ���;ۼ���������
//ģ�����ƣ�����ģ��
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-6-04 10:11:20
//�޸���־:
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

	//����֧��WSASend/WSASendto/WSARecv/WSARecvfrom�ķ���
	operator LPWSABUF ()
	{
		return m_WsaBuffArray.GetData();
	}

	DWORD GetCount() const
	{
		return m_WsaBuffArray.GetCount();
	}

	//��Ӵ��������ݻ���ķ���
    //Ҫע�����CLHIOPacket�����Ի�����з�����ͷŵĹ������,����ɵ����߸���
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
	CLHWSABufArray m_WsaBuffArray; //�����ջ��߷��͵Ļ����
};

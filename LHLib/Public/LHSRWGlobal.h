//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�����������һЩϵͳ��
//ģ�����ƣ�����֧���ļ�
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-5-21 19:09:23
//�޸���־:
//===================================================================================

#pragma once
#ifndef __LH_SRWGLOBAL_H__
#define __LH_SRWGLOBAL_H__
#include "LH_Switch.h"
#include "LHObject.h"

template <class T>
class CLHSRWGlobal : public CLHObject
{
public:
	CLHSRWGlobal(const T& val) :
		m_data(val)
	{
		// ����ֱ�ӿ�����Ҫ������T�����п�������

		ZeroMemory(&m_sl, sizeof(SRWLOCK));
		ZeroMemory(&m_cv, sizeof(CONDITION_VARIABLE));

		InitializeSRWLock(&m_sl);
		InitializeConditionVariable(&m_cv);
	}
	CLHSRWGlobal() :
		m_data()
	{
		// ����ֱ�ӿ�����Ҫ������T�����п�������

		ZeroMemory(&m_sl, sizeof(SRWLOCK));
		ZeroMemory(&m_cv, sizeof(CONDITION_VARIABLE));

		InitializeSRWLock(&m_sl);
		InitializeConditionVariable(&m_cv);
	}

	virtual ~CLHSRWGlobal() = default;

public:
	// ֱ�Ӷ�дȫ�ֱ����ķ���
	void DirectWrite(const T& v)
	{
		m_data = v;
	}

	T DirectRead() const
	{
		return m_data;
	}

	//��ռ�������� ��Щ���������������� �����õ������������������ͷ� ���⽵��Ч��
	void Lock()
	{
		AcquireSRWLockExclusive(&m_sl);
	}

	void UnLock()
	{
		ReleaseSRWLockExclusive(&m_sl);
	}

	// �ȴ�����ĳһ��
	void WakeSingle()
	{
		WakeConditionVariable(&m_cv);
	}

	void WakeAll()
	{
		WakeAllConditionVariable(&m_cv);
	}

	//��ͨ����������ȫ�ֱ����Ķ�д
	void LockWrite(const T& v)
	{
		__try
		{
			AcquireSRWLockExclusive(&m_sl);
			m_data = v;
		}
		__finally
		{
			ReleaseSRWLockExclusive(&m_sl);
		}
	}

	T ReadLock()
	{
		T ret;
		__try
		{
			AcquireSRWLockShared(&m_sl);
			ret = m_data;
		}
		__finally
		{
			ReleaseSRWLockShared(&m_sl);
		}

		return ret;
	}

	//�����ȴ���ʽ��ȫ�ֱ�����д
	void WriteAndWaitSingle(const T& v)
	{
		__try
		{
			AcquireSRWLockExclusive(&m_sl);
			m_data = v;
		}
		__finally
		{
			ReleaseSRWLockExclusive(&m_sl);
			WakeConditionVariable(&m_cv);
		}
	}

	void WriteAndWaitAll(const T& v)
	{
		__try
		{
			AcquireSRWLockExclusive(&m_sl);
			m_data = v;
		}
		__finally
		{
			ReleaseSRWLockExclusive(&m_sl);
			WakeAllConditionVariable(&m_cv);
		}
	}

	BOOL ReadAndWait(T& v, DWORD dwSleepLen = INFINITE)
	{
		BOOL bRet = FALSE;
		__try
		{
			AcquireSRWLockShared(&m_sl);
			if (SleepConditionVariableSRW(&m_cv, &m_sl, dwSleepLen, CONDITION_VARIABLE_LOCKMODE_SHARED))
			{
				v = m_data;
				bRet = TRUE;
			}
		}
		__finally
		{
			ReleaseSRWLockShared(&m_sl);
		}

		return bRet;
	}
private:
	// ��ֹ�����͸���
	CLHSRWGlobal(const CLHSRWGlobal&) = delete;
	CLHSRWGlobal& operator=(const CLHSRWGlobal&) = delete;
protected:
	T m_data;
	SRWLOCK m_sl;
	CONDITION_VARIABLE m_cv;
};

#endif

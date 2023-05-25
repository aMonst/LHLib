//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：包含一些系统库
//模块名称：公共支持文件
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-21 19:09:23
//修改日志:
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
		// 参数直接拷贝，要求类型T必须有拷贝构造

		ZeroMemory(&m_sl, sizeof(SRWLOCK));
		ZeroMemory(&m_cv, sizeof(CONDITION_VARIABLE));

		InitializeSRWLock(&m_sl);
		InitializeConditionVariable(&m_cv);
	}
	CLHSRWGlobal() :
		m_data()
	{
		// 参数直接拷贝，要求类型T必须有拷贝构造

		ZeroMemory(&m_sl, sizeof(SRWLOCK));
		ZeroMemory(&m_cv, sizeof(CONDITION_VARIABLE));

		InitializeSRWLock(&m_sl);
		InitializeConditionVariable(&m_cv);
	}

	virtual ~CLHSRWGlobal() = default;

public:
	// 直接读写全局变量的方法
	void DirectWrite(const T& v)
	{
		m_data = v;
	}

	T DirectRead() const
	{
		return m_data;
	}

	//独占锁定变量 这些方法用于批量操作 而不用单独锁定单独操作再释放 以免降低效率
	void Lock()
	{
		AcquireSRWLockExclusive(&m_sl);
	}

	void UnLock()
	{
		ReleaseSRWLockExclusive(&m_sl);
	}

	// 等待唤醒某一个
	void WakeSingle()
	{
		WakeConditionVariable(&m_cv);
	}

	void WakeAll()
	{
		WakeAllConditionVariable(&m_cv);
	}

	//普通轻量锁保护全局变量的读写
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

	//条件等待方式的全局变量读写
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
	// 禁止拷贝和复制
	CLHSRWGlobal(const CLHSRWGlobal&) = delete;
	CLHSRWGlobal& operator=(const CLHSRWGlobal&) = delete;
protected:
	T m_data;
	SRWLOCK m_sl;
	CONDITION_VARIABLE m_cv;
};

#endif

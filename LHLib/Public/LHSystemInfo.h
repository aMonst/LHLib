//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：系统信息类声明 以及相关接口定义文件
//模块名称：系统信息模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-22 09:21:19
//修改日志:
//===================================================================================
#pragma once
#ifndef __LH_SYSTEMINFO_H__
#define __LH_SYSTEMINFO_H__
#include "LH_Win.h"
#include "LHException.h"
#include <lmcons.h>
#include <vector>

class CLHSystemInfo : public CLHObject
{
public:
	static CLHSystemInfo& GetSystemInfo()
	{
		static CLHSystemInfo sysInfo;
		return sysInfo;
	}

	virtual ~CLHSystemInfo() = default;
protected:
	CLHSystemInfo()
	{
		Init();
	}
	CLHSystemInfo(const CLHSystemInfo& si) = delete;
	CLHSystemInfo& operator=(const CLHSystemInfo&) = delete;

	void FormatOSString()
	{
		if (VER_PLATFORM_WIN32_NT == m_osVersion.dwPlatformId && m_osVersion.dwMajorVersion > 4)
		{
			m_sVersion = L"Microsoft ";

			if (m_osVersion.dwMajorVersion == 6)
			{
				if (0 == m_osVersion.dwMinorVersion)
				{
					if (m_osVersion.wProductType == VER_NT_WORKSTATION)
					{
						m_sVersion += L"Windows Vista ";
					}
					else
					{
						m_sVersion += L"Windows Server 2008 ";
					}
				}
				else if (1 == m_osVersion.dwMinorVersion)
				{
					if (m_osVersion.wProductType == VER_NT_WORKSTATION)
					{
						m_sVersion += L"Windows 7 ";
					}
					else
					{
						m_sVersion += L"Windows Unknown ";
					}
				}
				else
				{
					m_sVersion += L"Windows Unknown ";
				}

#if _WIN32_WINNT >= 0x0600
				DWORD dwType = 0;
				GetProductInfo(6, 0, 0, 0, &dwType);
				switch (dwType)
				{
				case PRODUCT_ULTIMATE:
					m_sVersion += L"Ultimate Edition";
					break;
				case PRODUCT_HOME_PREMIUM:
					m_sVersion += L"Home Premium Edition";
					break;
				case PRODUCT_HOME_BASIC:
					m_sVersion += L"Home Basic Edition";
					break;
				case PRODUCT_ENTERPRISE:
					m_sVersion += L"Enterprise Edition";
					break;
				case PRODUCT_BUSINESS:
					m_sVersion += L"Business Edition";
					break;
				case PRODUCT_STARTER:
					m_sVersion += L"Starter Edition";
					break;
				case PRODUCT_CLUSTER_SERVER:
					m_sVersion += L"Cluster Server Edition";
					break;
				case PRODUCT_DATACENTER_SERVER:
					m_sVersion += L"Datacenter Edition";
					break;
				case PRODUCT_DATACENTER_SERVER_CORE:
					m_sVersion += L"Datacenter Edition (core installation)";
					break;
				case PRODUCT_ENTERPRISE_SERVER:
					m_sVersion += L"Enterprise Edition";
					break;
				case PRODUCT_ENTERPRISE_SERVER_CORE:
					m_sVersion += L"Enterprise Edition (core installation)";
					break;
				case PRODUCT_ENTERPRISE_SERVER_IA64:
					m_sVersion += L"Enterprise Edition for Itanium-based Systems";
					break;
				case PRODUCT_SMALLBUSINESS_SERVER:
					m_sVersion += L"Small Business Server";
					break;
				case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
					m_sVersion += L"Small Business Server Premium Edition";
					break;
				case PRODUCT_STANDARD_SERVER:
					m_sVersion += L"Standard Edition";
					break;
				case PRODUCT_STANDARD_SERVER_CORE:
					m_sVersion += L"Standard Edition (core installation)";
					break;
				case PRODUCT_WEB_SERVER:
					m_sVersion += L"Web Server Edition";
					break;
				}
#endif
				if (m_si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
				{
					m_sVersion += L", 64-bit";
				}
				else if (m_si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
				{
					m_sVersion += L", 32-bit";
				}
			}

			if (m_osVersion.dwMajorVersion == 5 && m_osVersion.dwMinorVersion == 2)
			{
				if (GetSystemMetrics(SM_SERVERR2))
				{
					m_sVersion += L"Windows Server 2003 R2, ";
				}
				else if (m_osVersion.wSuiteMask == VER_SUITE_STORAGE_SERVER)
				{
					m_sVersion += L"Windows Storage Server 2003";
				}
				else if (m_osVersion.wProductType == VER_NT_WORKSTATION && m_si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
				{
					m_sVersion += L"Windows XP Professional x64 Edition";
				}
				else
				{
					m_sVersion += L"Windows Server 2003, ";
				}

				if (m_osVersion.wProductType != VER_NT_WORKSTATION)
				{
					if (m_si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
					{
						if (m_osVersion.wSuiteMask & VER_SUITE_DATACENTER)
						{
							m_sVersion += L"Datacenter Edition for Itanium-based Systems";
						}
						else if (m_osVersion.wSuiteMask & VER_SUITE_ENTERPRISE)
						{
							m_sVersion += L"Enterprise Edition for Itanium-based Systems";
						}
					}

					else if (m_si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
					{
						if (m_osVersion.wSuiteMask & VER_SUITE_DATACENTER)
						{
							m_sVersion += L"Datacenter x64 Edition";
						}
						else if (m_osVersion.wSuiteMask & VER_SUITE_ENTERPRISE)
						{
							m_sVersion += L"Enterprise x64 Edition";
						}
						else
						{
							m_sVersion += L"Standard x64 Edition";
						}
					}

					else
					{
						if (m_osVersion.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
						{
							m_sVersion += L"Compute Cluster Edition";
						}
						else if (m_osVersion.wSuiteMask & VER_SUITE_DATACENTER)
						{
							m_sVersion += L"Datacenter Edition";
						}
						else if (m_osVersion.wSuiteMask & VER_SUITE_ENTERPRISE)
						{
							m_sVersion += L"Enterprise Edition";
						}
						else if (m_osVersion.wSuiteMask & VER_SUITE_BLADE)
						{
							m_sVersion += L"Web Edition";
						}
						else
						{
							m_sVersion += L"Standard Edition";
						}
					}
				}
			}

			if (m_osVersion.dwMajorVersion == 5 && m_osVersion.dwMinorVersion == 1)
			{
				m_sVersion += L"Windows XP ";
				if (m_osVersion.wSuiteMask & VER_SUITE_PERSONAL)
				{
					m_sVersion += L"Home Edition";
				}
				else
				{
					m_sVersion += L"Professional";
				}
			}

			if (m_osVersion.dwMajorVersion == 5 && m_osVersion.dwMinorVersion == 0)
			{
				m_sVersion += L"Windows 2000 ";

				if (m_osVersion.wProductType == VER_NT_WORKSTATION)
				{
					m_sVersion += L"Professional";
				}
				else
				{
					if (m_osVersion.wSuiteMask & VER_SUITE_DATACENTER)
					{
						m_sVersion += L"Datacenter Server";
					}
					else if (m_osVersion.wSuiteMask & VER_SUITE_ENTERPRISE)
					{
						m_sVersion += L"Advanced Server";
					}
					else
					{
						m_sVersion += L"Server";
					}
				}
			}
			if (_tcslen(m_osVersion.szCSDVersion) > 0)
			{
				m_sVersion += L" ";
				m_sVersion += m_osVersion.szCSDVersion;
			}

			TCHAR buf[80];
			StringCchPrintf(buf, 80, _T(" (build %d)"), m_osVersion.dwBuildNumber);
			m_sVersion += buf;
		}
		else
		{
			m_sVersion += L"未知版本的Windows.\n";
		}
	}

	void Init()
	{
		try {
			ZeroMemory(&m_si, sizeof(SYSTEM_INFO));
			ZeroMemory(&m_osVersion, sizeof(OSVERSIONINFOEXW));
			m_osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
			DWORD dwLen = 0;
			wchar_t pszFileName[MAX_PATH] = L"";
			if (0 == (dwLen = GetModuleFileNameW(NULL, pszFileName, MAX_PATH)))
			{
				throw CLHException(GetLastError());
			}

			for (DWORD i = dwLen; i > 0; --i)
			{
				if (pszFileName[i] == L'\\')
				{
					pszFileName[i] = L'\0';
					break;
				}
			}

			m_sAppDirectory = pszFileName;
			ZeroMemory(pszFileName, sizeof(wchar_t) * MAX_PATH);
			if (!GetSystemDirectoryW(pszFileName, MAX_PATH))
			{
				throw CLHException(GetLastError());
			}

			m_sSystemDirectory = pszFileName;

			ZeroMemory(pszFileName, sizeof(wchar_t) * MAX_PATH);
			if (!GetWindowsDirectoryW(pszFileName, MAX_PATH))
			{
				throw CLHException(GetLastError());
			}
			m_sWindowsDirectory = pszFileName;

			dwLen = MAX_PATH;
			ZeroMemory(pszFileName, sizeof(wchar_t) * MAX_PATH);
			if (!::GetComputerNameW(pszFileName, &dwLen))
			{
				throw CLHException(GetLastError());
			}
			m_sComputerName = pszFileName;

			dwLen = MAX_PATH;
			ZeroMemory(pszFileName, sizeof(wchar_t) * MAX_PATH);
			if (!::GetUserNameW(pszFileName, &dwLen))
			{
				throw CLHException(GetLastError());
			}
			m_sUserName = pszFileName;

			::GetSystemInfo(&m_si);
			if (!::GetVersionExW((OSVERSIONINFOW*)&m_osVersion))
			{
				throw CLHException(GetLastError());
			}
			FormatOSString();

			LPWCH lpEnv = GetEnvironmentStringsW();
			if (NULL != lpEnv)
			{
				LPWSTR lpszVariable = (LPWSTR)lpEnv;
				while (*lpszVariable)
				{
					m_arrEnvVals.push_back(lpszVariable);
					lpszVariable += m_arrEnvVals.back().length() + 1;
				}
			}

			FreeEnvironmentStringsW(lpEnv);
		}
		catch (CLHException& e)
		{
			e;
		}
	}
	public:
		bool IsWin2000() const
		{
			return (5 == m_osVersion.dwMajorVersion && 0 == m_osVersion.dwMinorVersion);
		}

		bool IsWinXP() const
		{
			return (5 == m_osVersion.dwMajorVersion && 1 == m_osVersion.dwMinorVersion);
		}

		bool IsWinVista() const
		{
			return (6 == m_osVersion.dwMajorVersion
				&& 0 == m_osVersion.dwMinorVersion
				&& VER_NT_WORKSTATION == m_osVersion.wProductType);
		}

		BOOL IsWin2003()
		{
			return (5 == m_osVersion.dwMajorVersion && 2 == m_osVersion.dwMinorVersion);
		}
		BOOL IsWin7()
		{
			return (6 == m_osVersion.dwMajorVersion && 1 == m_osVersion.dwMinorVersion
				&& VER_NT_WORKSTATION == m_osVersion.wProductType);
		}
		BOOL IsWin2008()
		{
			return (6 == m_osVersion.dwMajorVersion && m_osVersion.dwMinorVersion <= 1
				&& VER_NT_WORKSTATION != m_osVersion.wProductType);
		}
		BOOL IsWin32()
		{
			return PROCESSOR_ARCHITECTURE_INTEL == m_si.wProcessorArchitecture;
		}
		BOOL IsWin64()
		{
			return (PROCESSOR_ARCHITECTURE_AMD64 == m_si.wProcessorArchitecture
				|| PROCESSOR_ARCHITECTURE_IA64 == m_si.wProcessorArchitecture);
		}
		CLHString GetAppDir() const
		{
			return m_sAppDirectory;
		}

		CLHString GetSysDir() const
		{
			return m_sSystemDirectory;
		}
		
		CLHString GetWinDir() const
		{
			return m_sWindowsDirectory;
		}

		SYSTEM_INFO const* GetSysInfo() const
		{
			return &m_si;
		}

		OSVERSIONINFOEXW const* GetVersionInfo() const
		{
			return &m_osVersion;
		}
		
		CLHString GetOSString() const
		{
			return m_sVersion;
		}

		CLHString GetComputerName() const
		{
			return m_sComputerName;
		}
		CLHString GetUserName() const
		{
			return m_sUserName;
		}

		size_t GetEnvValsCnt() const
		{
			return m_arrEnvVals.size();
		}

		CLHString GetEnvVal(DWORD dwIndex) const
		{
			if (dwIndex >= m_arrEnvVals.size())
			{
				return NULL;
			}
			return m_arrEnvVals[dwIndex];
		}
protected:
	CLHString m_sAppDirectory;
	CLHString m_sSystemDirectory;
	CLHString m_sWindowsDirectory;
	CLHString m_sComputerName;
	CLHString m_sUserName;
	CLHString m_sVersion;
	SYSTEM_INFO m_si;
	OSVERSIONINFOEXW m_osVersion;
	std::vector<CLHString> m_arrEnvVals;
};
#endif

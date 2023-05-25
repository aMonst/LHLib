#include "LHString.h"
CLHString::CLHString()
{
	m_pszBuffer = new wchar_t[1];
	m_pszBuffer[0] = L'\0';
	m_sLength = 0;
}
CLHString::CLHString(LPCWSTR lpcStr)
{
	if (lpcStr != NULL)
	{
		HRESULT hResult = StringCchLengthW(lpcStr, STRSAFE_MAX_CCH, &m_sLength);
		LH_ASSERT(S_OK == hResult);
		m_pszBuffer = new wchar_t[m_sLength + 1];
		ZeroMemory(m_pszBuffer, sizeof(wchar_t) * (m_sLength + 1));

		StringCchCopyW(m_pszBuffer, m_sLength + 1, lpcStr);
	}
	else
	{
		m_pszBuffer = new wchar_t[1];
		m_pszBuffer[0] = L'\0';
		m_sLength = 0;
	}
}

CLHString::CLHString(const CLHString& str):
	CLHString(str.GetString())
{
	
}

CLHString& CLHString::operator=(const CLHString& str)
{
	return operator=(str.GetString());
}

CLHString& CLHString::operator=(LPCWSTR str)
{
	if (str == NULL)
	{
		m_pszBuffer = new wchar_t[1];
		m_pszBuffer[0] = L'\0';
		m_sLength = 0;
		return *this;
	}

	HRESULT hResult = StringCchLengthW(str, STRSAFE_MAX_CCH, &m_sLength);
	LH_ASSERT(S_OK == hResult);
	m_pszBuffer = new wchar_t[m_sLength + 1];
	ZeroMemory(m_pszBuffer, sizeof(wchar_t) * (m_sLength + 1));

	StringCchCopyW(m_pszBuffer, m_sLength + 1, str);

	return *this;
}

CLHString::~CLHString()
{
	if (m_pszBuffer != NULL)
	{
		delete[] m_pszBuffer;
		m_pszBuffer = NULL;
		m_sLength = 0;
	}
}

size_t CLHString::length() const
{
	return m_sLength;
}

size_t CLHString::toAnsiString(char* pszBuffer, size_t iBufferSize) const
{
	return WideCharToMultiByte(CP_ACP, 0, GetString(), -1, pszBuffer, iBufferSize, NULL, NULL);
}

size_t CLHString::toUtf8String(char* pszBuffer, size_t iBufferSize) const
{
	return WideCharToMultiByte(CP_UTF8, 0, GetString(), -1, pszBuffer, iBufferSize, NULL, NULL);
}

CLHString& CLHString::StringCat(LPCWSTR lpcStr)
{
	if (lpcStr == NULL)
		return *this;

	size_t sSize = 0;
	HRESULT hResult = StringCchLength(lpcStr, STRSAFE_MAX_CCH, &sSize);
	LH_ASSERT(S_OK == hResult);
	m_sLength += sSize;

	wchar_t* pszNewBuf = new wchar_t[m_sLength + 1];
	ZeroMemory(pszNewBuf, sizeof(wchar_t) * (m_sLength + 1));
	StringCchCatW(pszNewBuf, m_sLength + 1, lpcStr);
	if (m_pszBuffer != NULL)
	{
		delete[] m_pszBuffer;
	}

	m_pszBuffer = pszNewBuf;

	return *this;
}

CLHString& CLHString::StringCat(const CLHString& str)
{
	m_sLength += str.length();
	wchar_t* pszNewBuff = new wchar_t[m_sLength + 1];
	
	ZeroMemory(pszNewBuff, sizeof(wchar_t) * (m_sLength + 1));
	StringCchCopyW(pszNewBuff, m_sLength + 1, m_pszBuffer);
	StringCchCatW(pszNewBuff, m_sLength + 1, str.GetString());
	if (m_pszBuffer != NULL)
	{
		delete[] m_pszBuffer;
	}

	m_pszBuffer = pszNewBuff;

	return *this;
}

CLHString& CLHString::operator+=(LPCWSTR lpcStr)
{
	return StringCat(lpcStr);
}

CLHString& CLHString::operator+=(const CLHString& lpcStr)
{
	return StringCat(lpcStr);
}

CLHString CLHString::operator+(LPCWSTR lpcStr) const
{
	CLHString strNew = *this;
	strNew.StringCat(lpcStr);
	return strNew;
}

CLHString CLHString::operator+(const CLHString& lpcStr)
{
	CLHString strNew = *this;
	strNew.StringCat(lpcStr);
	return strNew;
}

CLHString& CLHString::Format(LPCWSTR lpcFormat, ...)
{
	if (NULL == lpcFormat)
		return *this;

	va_list ap;
	va_start(ap, lpcFormat);

	Format(lpcFormat, ap);
	va_end(ap);
	return *this;
}

CLHString& CLHString::Format(LPCWSTR lpcFormat, va_list arg)
{
	size_t buffSize = _vscwprintf(lpcFormat, arg);
	m_sLength = buffSize;
	wchar_t* pszNewBuff = new wchar_t[m_sLength + 1];
	ZeroMemory(pszNewBuff, (m_sLength + 1) * sizeof(wchar_t));
	_vsnwprintf_s(pszNewBuff, m_sLength + 1, _TRUNCATE, lpcFormat, arg);
	if (m_pszBuffer != NULL)
	{
		delete[] m_pszBuffer;
	}

	m_pszBuffer = pszNewBuff;
	return *this;
}

wchar_t* CLHString::GetString() const
{
	return m_pszBuffer;
}

CLHString::operator LPCWSTR() const
{
	return GetString();
}

bool CLHString::operator==(LPCWSTR lpcStr)
{
	return _tcscmp(this->m_pszBuffer, lpcStr);
}

bool CLHString::operator==(const CLHString& str)
{
	return operator==(str.GetString());
}

bool CLHString::operator==(LPCSTR lpcStr)
{
	return operator==(CLHString::AnsiToString(lpcStr, strlen(lpcStr)));
}

CLHString CLHString::AnsiToString(LPCSTR lpcAnsi, size_t iAnsiSize)
{
	int nBufSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpcAnsi, iAnsiSize, NULL, 0);
	if (nBufSize != 0)
	{
		wchar_t* pszNewBuf = new wchar_t[nBufSize];
		ZeroMemory(pszNewBuf, sizeof(wchar_t) * nBufSize);
		nBufSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpcAnsi, iAnsiSize, pszNewBuf, nBufSize);
		if (nBufSize != 0)
		{
			CLHString s = pszNewBuf;
			delete[] pszNewBuf;
			return s;
		}
	}

	return CLHString();
}

CLHString CLHString::Utf8ToString(LPCSTR lpcUtf8, size_t iUtf8Size)
{
	int nBufSize = MultiByteToWideChar(CP_UTF8, 0, lpcUtf8, iUtf8Size, NULL, 0);
	if (nBufSize != 0)
	{
		wchar_t* pszNewBuf = new wchar_t[nBufSize];
		ZeroMemory(pszNewBuf, sizeof(wchar_t) * nBufSize);
		nBufSize = MultiByteToWideChar(CP_UTF8, 0, lpcUtf8, iUtf8Size, pszNewBuf, nBufSize);
		if (nBufSize != 0)
		{
			CLHString s = pszNewBuf;
			delete[] pszNewBuf;
			return s;
		}
	}

	return CLHString();
}

std::wostream& operator << (std::wostream& out, const CLHString& str)
{
	out << str.GetString();
	return out;
}

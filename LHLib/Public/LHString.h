//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�������CLHString�ඨ���ļ� CLHString����LHLib����й����ַ�������ķ�װ��
//ģ�����ƣ�����֧���ļ�
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-5-21 14:29:15
//�޸���־:
//===================================================================================

#pragma once
#include "LH_Win.h"
#include "LHHeap.h"
#include "LHObject.h"
#include <iostream>

class CLHString :
    public CLHObject
{
public:
    CLHString();
    CLHString(LPCWSTR lpcStr);
    CLHString(const CLHString& str);
    CLHString& operator=(const CLHString& str);
    CLHString& operator=(LPCWSTR str);
    virtual ~CLHString();

    size_t length() const;
    // ��������buffer�ĳ��ȣ��ⲿʹ��ʱ�����жϴ���ĳ�������ʵ�����Ƿ�һ�����жϵ����Ƿ�ɹ�
    // �����ʵ���ȴ��ڴ���buffer�ĳ��ȣ���Ҫ���·����ڴ�,�������ε���
    size_t toAnsiString(char* pszBuffer, size_t iBufferSize) const;
    size_t toUtf8String(char* pszBuffer, size_t iBufferSize) const;

    CLHString& StringCat(LPCWSTR lpcStr);
    CLHString& StringCat(const CLHString& str);
    CLHString& operator+=(LPCWSTR lpcStr);
    CLHString& operator+=(const CLHString& lpcStr);
    CLHString operator+(LPCWSTR lpcStr) const;
    CLHString operator+(const CLHString& lpcStr);

    CLHString& Format(LPCWSTR lpcFormat, ...);
    CLHString& Format(LPCWSTR lpcFormat, va_list arg);
    wchar_t* GetString() const;
    // TODO: find, split, insert, ....

    operator LPCWSTR() const;
    bool operator==(LPCWSTR);
    bool operator==(const CLHString& str);
    bool operator==(LPCSTR);
    static CLHString AnsiToString(LPCSTR lpcAnsi, size_t iAnsiSize);
    static CLHString Utf8ToString(LPCSTR lpcUtf8, size_t iUtf8Size);
private:
    // windows �ں˲��õ��ǿ��ַ����������ﶨ����ַ�����Ϊ���ַ�
    wchar_t* m_pszBuffer;
    size_t m_sLength;
};

std::wostream& operator << (std::wostream& out, const CLHString& str);

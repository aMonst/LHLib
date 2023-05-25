//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：CLHString类定义文件 CLHString类是LHLib类库中关于字符串处理的封装类
//模块名称：公共支持文件
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-21 14:29:15
//修改日志:
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
    // 返回所需buffer的长度，外部使用时可以判断传入的长度于真实长度是否一致来判断调用是否成功
    // 如果真实长度大于传入buffer的长度，需要重新分配内存,进行两次调用
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
    // windows 内核采用的是宽字符，所以这里定义的字符缓冲为宽字符
    wchar_t* m_pszBuffer;
    size_t m_sLength;
};

std::wostream& operator << (std::wostream& out, const CLHString& str);

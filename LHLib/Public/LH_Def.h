//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：一些扩展关键字 以及常用操作的宏定义文件
//模块名称：公共支持文件
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-20 17:20:18
//修改日志:
//===================================================================================

#pragma once
#ifndef __LH_DEF_H__
#define __LH_DEF_H__

// 定义无符号32位最大值
#ifndef MAXUINT
#define MAXUINT ((UINT)~((UINT)0))
#endif
// 定义无符号64位最大值
#ifndef MAXULONGLONG
#define MAXULONGLONG ((ULONGLONG)~((ULONGLONG)0))
#endif

//用于输出编译信息时，将其他类型的信息变成字符串型的
#define LH_STRINGA2(x) #x
#define LH_STRINGA(x) LH_STRINGA2(x)

#define LH_STRINGW2(x) L#x
#define LH_STRINGW(x) LH_STRINGW2(x)

#ifdef UNICODE
#define LH_STRING(x) LH_STRINGW(x)
#else
#define LH_STRING(x) LH_STRINGA(x)
#endif

//__FILE__预定义宏的宽字符版本
#define LH_WIDE2(x) L#x
#define LH_WIDE(x) LH_WIDE2(x)
#define __WFILE__ LH_WIDE(__FILE__)

// 取数组元素的个数
#ifndef COUNTOF
#define COUNTOF(x) sizeof(x) / sizeof(x[0])
#endif

// 两个32位无符号二进制数组合成64位无符号二进制数
#ifndef MAKEULONGLONG
#define MAKEULONGLONG(dwHight, dwLow) ((((ULONGLONG)dwHight << 32) | ((ULONGLONG)dwLow & 0xffffffff)))
#endif

#endif
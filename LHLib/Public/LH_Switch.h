//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：LHLib 开关宏定义头文件,通过修改这个文件中的一些宏开关定义可以定制该库的特性
//模块名称：公共支持文件
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-5-20 17:38:30
//修改日志:
//===================================================================================

#pragma setlocale("chinese-simplified")
#ifdef _DEBUG
//开启内存泄露检测功能，只要是派生于LHObject 的类都具有内存泄露检测的功能
//使用时需要在CPP文件中定义如下宏
//#ifdef LH_MEMORY_LEAK
//#define new new(__WFILE__,__LINE__)
//#endif

#define LH_MEMORY_LEAK
#endif

#pragma once

//===================================================================================
//项目名称：LH 个人Windows平台代码库 V1.0
//文件描述：定义原始TCP/IP 的IPv4 和IPv6 协议相关的结构
//模块名称：网络模块
//
//组织（公司、集团）名称：None
//作者：aMonst
//创建日期：2015-6-05 11:33:46
//修改日志:
//===================================================================================

#pragma once
#include <pshpack1.h>
#include <WinSock2.h>

//IPv4 头
typedef struct _ST_IPV4_HDR
{
	unsigned char ip_hdrlen : 4; //4 bit的头长度
	unsigned char ip_ver : 4; //4 bit 的协议版本

	// 共 8bit 的服务类型
	unsigned char ip_tosrcv : 1; // 固定值0
	unsigned char ip_tos : 4; // 最新延迟、最大吞吐量、最高可靠性、最小成本，只能4选一
	unsigned char ip_tospri : 3;//优先权位

	unsigned short ip_totallength; // 整个ip包长度
	unsigned short ip_id; //标识符, 用于分割之后识别是否是同一个ip包

	unsigned short ip_offset : 13; //偏移量, 由于网络传输不能保证数据包按照发送的顺序到达，因此后续根据这个偏移量来组成完成的ip包
	
	// 标志字段
	unsigned short ip_FM : 1;          // 是否是最后一个包
	unsigned short ip_DM : 1;          // 是否允许分片
	unsigned short ip_rsv0 : 1;        // 保留字段

	unsigned char  ip_ttl; //生存时间
	unsigned char  ip_protocol; //协议类型，1：ICMP， 2：IGMP， 6：TCP， 17：UDP
	unsigned short ip_checksum; //ip 协议头校验和
	in_addr        ip_srcaddr;       // 请求方地址
    in_addr        ip_destaddr;      // 响应方地址
}ST_IPV4_HDR, *LPST_IPV4_HDR;

//Ip 头选项，最多40个字节 (可选字段)
typedef struct _ST_IPV4_OPTION_HDR
{
	unsigned char opt_code; //选项类型
	unsigned char opt_len; // 选项长度
	unsigned char opt_ptr; // 选项偏移
	unsigned long opt_addr[9]; //IPv4 地址列表
}ST_IPV4_OPTION_HDR, *LPST_IPV4_OPTION_HDR;

//icmp协议头
typedef struct _ST_ICMP_HDR
{
	//前三个字段才是ICMP协议中一般要求具有的字段
	unsigned char icmp_type; //类型
	unsigned char icmp_code; //代码
	unsigned short icmp_checksum; //校验和

	//这两个字段主要为PING命令而设置,不同的类型和代码要求的后续字段是不同的
	unsigned short  icmp_id;
    unsigned short  icmp_sequence;
    //这个纯粹已经算是数据字段了,加在这里的目的是方便计算PING的时间
    unsigned long   icmp_timestamp;
}ST_ICMP_HDR, *LPST_ICMP_HDR;

typedef struct _ST_IPV6_HDR
{
	unsigned long   ipv6_ver : 4; //4 bit 协议版本
	unsigned long   ipv6_tc : 8; //8 bit traffic class
	unsigned long	ipv6_flow : 20;//20 bit flow label
	unsigned short	ipv6_payloadlen; //payload length
	unsigned char	ipv6_nexthdr; //next header protocol value
	unsigned char	ipv6_hoplimit; //TTL
	struct in6_addr ipv6_srcaddr; //source address
	struct in6_addr ipv6_destaddr; //destination address
}ST_IPV6_HDR, *LPST_IPV6_HDR;

typedef struct _ST_IPV6_FRAGMENT_HDR
{
	unsigned char ipv6_frag_nexthdr;
	unsigned char ipv6_frag_reserved;
	unsigned short ipv6_frag_offset;
	unsigned long ipv6_frag_id;
}ST_IPV6_FRAGMENT_HDR, *LPST_IPV6_FRAGMENT_HDR;

typedef struct _ST_ICMPV6_HDR
{
	unsigned char icmp6_type;
	unsigned char icmp6_code;
	unsigned short icmp6_checksum;
}ST_CIMPV6_HDR, *LPST_ICMPV6_HDR;

typedef struct _ST_ICMPV6_ECHO_REQUEST
{
	unsigned short  icmp6_echo_id;
    unsigned short  icmp6_echo_sequence;
}ST_ICMPV6_ECHO_REQUEST, *LPST_ICMPV6_ECHO_REQUEST;

typedef struct _ST_UDP_HDR
{
	unsigned short src_portno;       // Source port no.
    unsigned short dst_portno;       // Dest. port no.
    unsigned short udp_length;       // Udp packet length
    unsigned short udp_checksum;     // Udp checksum (optional)
}ST_UDP_HDR, *LPST_UDP_HDR;

typedef struct _ST_TCP_HDR 
{
    unsigned short src_port     ;
    unsigned short dst_port     ;
    unsigned long  tcp_seq      ;
    unsigned long  tcp_ack_seq  ;  
    unsigned short tcp_res1 : 4 ;
    unsigned short tcp_doff : 4 ;
    unsigned short tcp_fin :  1 ;
    unsigned short tcp_syn :  1 ;
    unsigned short tcp_rst :  1 ;
    unsigned short tcp_psh :  1 ;
    unsigned short tcp_ack :  1 ;
    unsigned short tcp_urg :  1 ;
    unsigned short tcp_res2 : 2 ;
    unsigned short tcp_window ;  
    unsigned short tcp_checksum ;
    unsigned short tcp_urg_ptr ;
}ST_TCP_HDR,*LPST_TCP_HDR;

// IPv4 option for record route
#define IP_RECORD_ROUTE     0x7

// ICMP6 protocol value (used in the socket call and IPv6 header)
#define IPPROTO_ICMP6       58

// ICMP types and codes
#define ICMPV4_ECHO_REQUEST_TYPE   8
#define ICMPV4_ECHO_REQUEST_CODE   0
#define ICMPV4_ECHO_REPLY_TYPE     0
#define ICMPV4_ECHO_REPLY_CODE     0
#define ICMPV4_MINIMUM_HEADER      8

#define ICMPV4_DESTUNREACH    3
#define ICMPV4_SRCQUENCH      4
#define ICMPV4_REDIRECT       5
#define ICMPV4_ECHO           8
#define ICMPV4_TIMEOUT       11
#define ICMPV4_PARMERR       12

// ICPM6 types and codes
#define ICMPV6_ECHO_REQUEST_TYPE   128
#define ICMPV6_ECHO_REQUEST_CODE   0
#define ICMPV6_ECHO_REPLY_TYPE     129
#define ICMPV6_ECHO_REPLY_CODE     0
#define ICMPV6_TIME_EXCEEDED_TYPE  3
#define ICMPV6_TIME_EXCEEDED_CODE  0

// Restore byte alignment back to default
#include <poppack.h>


//===================================================================================
//��Ŀ���ƣ�LH ����Windowsƽ̨����� V1.0
//�ļ�����������ԭʼTCP/IP ��IPv4 ��IPv6 Э����صĽṹ
//ģ�����ƣ�����ģ��
//
//��֯����˾�����ţ����ƣ�None
//���ߣ�aMonst
//�������ڣ�2015-6-05 11:33:46
//�޸���־:
//===================================================================================

#pragma once
#include <pshpack1.h>
#include <WinSock2.h>

//IPv4 ͷ
typedef struct _ST_IPV4_HDR
{
	unsigned char ip_hdrlen : 4; //4 bit��ͷ����
	unsigned char ip_ver : 4; //4 bit ��Э��汾

	// �� 8bit �ķ�������
	unsigned char ip_tosrcv : 1; // �̶�ֵ0
	unsigned char ip_tos : 4; // �����ӳ١��������������߿ɿ��ԡ���С�ɱ���ֻ��4ѡһ
	unsigned char ip_tospri : 3;//����Ȩλ

	unsigned short ip_totallength; // ����ip������
	unsigned short ip_id; //��ʶ��, ���ڷָ�֮��ʶ���Ƿ���ͬһ��ip��

	unsigned short ip_offset : 13; //ƫ����, �������紫�䲻�ܱ�֤���ݰ����շ��͵�˳�򵽴��˺����������ƫ�����������ɵ�ip��
	
	// ��־�ֶ�
	unsigned short ip_FM : 1;          // �Ƿ������һ����
	unsigned short ip_DM : 1;          // �Ƿ������Ƭ
	unsigned short ip_rsv0 : 1;        // �����ֶ�

	unsigned char  ip_ttl; //����ʱ��
	unsigned char  ip_protocol; //Э�����ͣ�1��ICMP�� 2��IGMP�� 6��TCP�� 17��UDP
	unsigned short ip_checksum; //ip Э��ͷУ���
	in_addr        ip_srcaddr;       // ���󷽵�ַ
    in_addr        ip_destaddr;      // ��Ӧ����ַ
}ST_IPV4_HDR, *LPST_IPV4_HDR;

//Ip ͷѡ����40���ֽ� (��ѡ�ֶ�)
typedef struct _ST_IPV4_OPTION_HDR
{
	unsigned char opt_code; //ѡ������
	unsigned char opt_len; // ѡ���
	unsigned char opt_ptr; // ѡ��ƫ��
	unsigned long opt_addr[9]; //IPv4 ��ַ�б�
}ST_IPV4_OPTION_HDR, *LPST_IPV4_OPTION_HDR;

//icmpЭ��ͷ
typedef struct _ST_ICMP_HDR
{
	//ǰ�����ֶβ���ICMPЭ����һ��Ҫ����е��ֶ�
	unsigned char icmp_type; //����
	unsigned char icmp_code; //����
	unsigned short icmp_checksum; //У���

	//�������ֶ���ҪΪPING���������,��ͬ�����ͺʹ���Ҫ��ĺ����ֶ��ǲ�ͬ��
	unsigned short  icmp_id;
    unsigned short  icmp_sequence;
    //��������Ѿ����������ֶ���,���������Ŀ���Ƿ������PING��ʱ��
    unsigned long   icmp_timestamp;
}ST_ICMP_HDR, *LPST_ICMP_HDR;

typedef struct _ST_IPV6_HDR
{
	unsigned long   ipv6_ver : 4; //4 bit Э��汾
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


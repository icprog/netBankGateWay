/* SOCKAPI.H

	一个TCP/IP socket编程接口的简易接口函数组
	       (C)Copyright XWB, 1995-97
	    (SCO UNIX/XENIX, PCTCP for DOS)
	       (C)Copyright JJS, 1996-99
	    (SCO UNIX/XENIX, PCTCP for DOS, Lan WorkPlace for DOS)

   !! 函数定时机制使用ALARM函数, 这将于应用程序的ALARM冲突 !!
   !! 编译条件M_XENIX决定是否为DOS版本.
   !!! 编译条件NOVELL_TCPIP4DOS决定是否为Lan WorkPlace for DOS版本.

   Update History:

     蒋建圣  1998.02.19
	   1) 添加itcp_calln()函数, 增加状态字99
	   2) 添加set_reuseaddr()函数
	   3) 修改itcp_poll()函数, 加上调用set_reuseaddr

     蒋建圣  1998.05.17 影响itcp_send和itcp_recv函数
	   1) 添加宏   NOVELL_TCPIP4DOS  -  Novell Lan WorkPlace for DOS
		 (主要是函数 read -> soread,  write -> sowrite
			     close -> soclose, ioctl -> soioctl,
		  和头文件的变化)
	   2) 添加变量 int  _use_ns = 1;   //是否用网络字节顺序表示包长
		       static char sync_buffer[MAX_SYNCCHAR]; //同步字符缓冲区
		       static int  sync_buflen;               //同步缓冲长度
	   3) 添加函数 void itcp_setsync(int len, char *buffer);
     蒋建圣  1998.08.25 影响itcp_send和itcp_recv函数
	   1) 添加变量 int _nohead = 0;   //没有数据包头(同步字符+包长)
*/

#if !defined (__SOCKAPI_H)
#define __SOCKAPI_H

#include <sys/types.h>

#ifdef NOVELL_TCPIP4DOS
#include "socket.h"
#endif
extern int _use_ns;        /* 数据包长度是否使用网络字节顺序 */
extern int _nohead;        /* 没有数据包头(同步字符+包长) */

#ifndef M_XENIX
/* 安装自定义hook函数.
 * 当执行一个blocking调用时,定义的hook函数将不断地被调用.
 * 格式: int hook( int state );   state=0:polling, 10-reading, 11-writing 99-calling(n)
 *   返回: 0-继续,其它-中断当前被调用的函数,并返回该值.
 */
void itcp_sethook(void *funcptr);
#endif

/* 设置同步字符缓冲区和同步字符缓冲区长度 */
void itcp_setsync(int len, char *buffer);

/* 设置应用程序与接口的连接变量, 应用程序必须首先调用该函数.
 * port ...... 端口号, 应用程序必须使用特定的端口号放可进行通信.
 * sockvar ... 存放socket句柄的变量指针.
 */
void itcp_setenv(int port, int *sockvar);


/* 对指定主机发出连接请求.
 * 注  : 参数为定义在hosts中的主机名或IP地址串.
 * 返回:  0 - 连接成功, 可以进行通信. socket句柄号存放在应用程序给定的变量中;
 *       -1 - 主机名不存在;
 *       -2 - 连接不成功.
 * 连接不成功, 自动关闭申请到的socket.
 */
int itcp_call(char *remotehost);


/* 侦听是否有连接请求. 注: 如果由inetd激发应用程序, 则不能调用本函数.
 * 返回: 0-有一连接请求;
 *	-1-初始化失败. (一般属于严重出错, 如TCP/IP未启动)
 */
int itcp_poll(void);


/* 应答一个连接请求. 这是一个空函数, 实质上, 应答是自动完成的 */
int itcp_accept(void);


/* 发送一包数据. 由于TCP/IP为数据流, 本身无法进行数据分界, 故我们插入4个定界
 * 字符, 以方便应用程序处理.
 * byte0-1=同步字符, byte2-3=本包包长(不包括4个定界字符)
 *	buffer ....... 欲发送的缓冲首指针;
 *	len .......... 缓冲长度;
 *	maxtime ...... 超时值(秒), 0-永远等待,直到发送完毕或socket被关闭.
 * 返回: 0-成功, -1-socket被关闭, -3-超时.
 */
int itcp_send(char *buffer, int len, int maxtime);


/* 接收一包数据. 必须是itcp_send发送的包, 否则数据无法定界.
 *	buffer ....... 欲存放的缓冲首指针;
 *      maxlen ........最大缓冲长度(_nohead == 1时为实际缓冲长度);
 *	maxtime ...... 超时值(秒), 0-永远等待,直到收到完整的包或socket被关闭.
 * 返回: -1-socket被关闭, -3-超时, >0-收到的包的字节数.
 */
int itcp_recv(char *buffer, int maxlen, int maxtime);


/* 即使close一个socket,内核仍然将未送出的信息发送出去.
 * 本函数设置当关闭一个套接字时立即释放缓冲,因此未送出的内容立即丢失.
 * (DOS版本: 当生成socket时,本函数自动调用)
 */
void itcp_setlinger(void);


/* 关闭socket */
void itcp_clear(void);


/* 本函数通常用于子进程中(由poll产生), 用于关闭原来(listen)的socket */
void itcp_closelisten(void);

/* 本函数返回socket的地址(0:本端, 非0:对方) */
u_long itcp_getaddr(int side);

#ifdef M_XENIX
/* 本函数测试指定与指定主机是否连通. 返回: 1-连通, 0-断开, -1-主机名不存在 */
int itcp_testhost(char *host);
#endif

/* 连续对指定主机发出连接请求.
 * 注  : 参数为定义在hosts中的主机名或IP地址串. n为call的次数(<=0为死循环)
 * 返回:  0 - 连接成功, 可以进行通信. socket句柄号存放在应用程序给定的变量中;
 *       -1 - 主机名不存在;
 *       -2 - 连接不成功.
 * 连接不成功, 自动关闭申请到的socket.
 */
int itcp_calln(char *remotehost, int times);

#endif

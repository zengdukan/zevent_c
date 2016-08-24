#ifndef _ZEVENT_CONNECTOR_H_
#define _ZEVENT_CONNECTOR_H_

#include "zbaselib_socket.h"
#include "zevent_event.h"
#include "zevent_typedef.h"

// 检测数据包是否完整，1=是，0-否
typedef int (*zevent_connector_on_check_pack)(zevent* connector);

// 数据解析回调
typedef int (*zevent_connector_on_parse_data)(zevent* connector);

// 上下线回调, conned=1:上线，conned=0:下线
typedef int (*zevent_connector_on_connect)(zevent* connector, int conned);

typedef struct zevent_connector_opt_s
{
	zevent_connector_on_check_pack on_check_pack;
	zevent_connector_on_parse_data on_parse_data;
	zevent_connector_on_connect on_connect;
} zevent_connector_opt;

/*
* add by hyzeng:2013-7-5
* 功能:创建connector
* 结果:NULL=失败
*/
ZLIB_API zevent* zevent_connector_new(int rd_buf_size, int wr_buf_size, zevent_connector_opt opt);

/*
* add by hyzeng:2013-7-5
* 功能:初始化connector
* 结果:
*/
ZLIB_API void zevent_connector_intial(zevent* connector, const struct sockaddr_in* addr,
	void* param1, void* param2);

/*
* add by hyzeng:2013-7-5
* 功能:连接服务器
       函数创建非阻塞socket，并连接服务器，连接成功回调on_connect,
       然后创建收发buffer
* 结果:0=连接成功，-1=失败
*/
ZLIB_API int zevent_connector_connect(zevent* connector);

/*
* add by hyzeng:2013-7-5
* 功能:接收一个已经创建socket，创建收发buffer，成功回调on_connect
       典型应用在服务器accept一个连接
* 结果:0=成功，-1=失败
*/
ZLIB_API int zevent_connector_accept(zevent* connector, int acpt_sock,
	const struct sockaddr_in* addr);

/*
* add by hyzeng:2013-7-5
* 功能:关闭connector
       此函数负责关闭socket，销毁收发buffer，并回调on_connect
* 结果:
*/
ZLIB_API int zevent_connector_close(zevent* connector);

//int zevent_connector_shutdown(zevent* connector, int flags);

ZLIB_API zbaselib_skbuffer* zevent_connector_get_rd_buf(zevent* connector);

ZLIB_API zbaselib_skbuffer* zevent_connector_get_wr_buf(zevent* connector);

// 0=成功, -1=失败，-2=剩余空间不够
ZLIB_API int zevent_connector_send_data(zevent* connector, const char* data, int size);

ZLIB_API int zevent_connector_is_connected(zevent* connector);

ZLIB_API int zevent_connector_get_addr(zevent* connector, struct sockaddr_in* addr);

ZLIB_API void* zevent_connector_get_param1(zevent* connector);

ZLIB_API void* zevent_connector_get_param2(zevent* connector);

ZLIB_API void zevent_connector_set_param1(zevent* connector, void* param);

ZLIB_API void zevent_connector_set_param2(zevent* connector, void* param);

ZLIB_API void zevent_connector_set_rdbuf_size(zevent* connector, int rd_buf_size);

ZLIB_API void zevent_connector_set_wrbuf_size(zevent* connector, int wr_buf_size);

ZLIB_API void zevent_connector_set_opt(zevent* connector, zevent_connector_opt opt);


#endif

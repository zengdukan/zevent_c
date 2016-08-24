#ifndef _ZEVENT_ACCEPTER_H_
#define _ZEVENT_ACCEPTER_H_

#include "zbaselib_socket.h"
//#include "zbaselib_list.h"
#include "zevent_connector.h"
#include "zevent_event.h"

ZBASELIB_BEGIN_DECL

typedef int (*zevent_accepter_on_accept_cb)(zevent* accepter, zevent* connector,
	int clnt_sock, struct sockaddr_in* addr);

/*
* add by hyzeng:2013-7-5
* 功能:创建socket，监听端口
* 结果:NULL=失败
*/
ZLIB_API zevent* zevent_accepter_new(const struct sockaddr_in* addr, 
	zevent_accepter_on_accept_cb on_accept_cb);

ZLIB_API int zevent_accepter_set_param(zevent* accepter, void* param1, void* param2);


ZLIB_API void* zevent_accepter_get_param1(zevent* accepter);


ZLIB_API void* zevent_accepter_get_param2(zevent* accepter);

/*
* add by hyzeng:2013-7-5
* 功能:更新地址，重新创建socket
       要先从looper里移除再重新加入looper
* 结果:0=成功，-1=失败
*/
ZLIB_API int zevent_accepter_update_addr(zevent* accepter, const struct sockaddr_in* addr);

ZBASELIB_END_DECL

#endif/*_ZEVENT_ACCEPTER_H_*/

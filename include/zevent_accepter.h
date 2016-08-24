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
* ����:����socket�������˿�
* ���:NULL=ʧ��
*/
ZLIB_API zevent* zevent_accepter_new(const struct sockaddr_in* addr, 
	zevent_accepter_on_accept_cb on_accept_cb);

ZLIB_API int zevent_accepter_set_param(zevent* accepter, void* param1, void* param2);


ZLIB_API void* zevent_accepter_get_param1(zevent* accepter);


ZLIB_API void* zevent_accepter_get_param2(zevent* accepter);

/*
* add by hyzeng:2013-7-5
* ����:���µ�ַ�����´���socket
       Ҫ�ȴ�looper���Ƴ������¼���looper
* ���:0=�ɹ���-1=ʧ��
*/
ZLIB_API int zevent_accepter_update_addr(zevent* accepter, const struct sockaddr_in* addr);

ZBASELIB_END_DECL

#endif/*_ZEVENT_ACCEPTER_H_*/

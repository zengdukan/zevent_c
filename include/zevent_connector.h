#ifndef _ZEVENT_CONNECTOR_H_
#define _ZEVENT_CONNECTOR_H_

#include "zbaselib_socket.h"
#include "zevent_event.h"
#include "zevent_typedef.h"

// ������ݰ��Ƿ�������1=�ǣ�0-��
typedef int (*zevent_connector_on_check_pack)(zevent* connector);

// ���ݽ����ص�
typedef int (*zevent_connector_on_parse_data)(zevent* connector);

// �����߻ص�, conned=1:���ߣ�conned=0:����
typedef int (*zevent_connector_on_connect)(zevent* connector, int conned);

typedef struct zevent_connector_opt_s
{
	zevent_connector_on_check_pack on_check_pack;
	zevent_connector_on_parse_data on_parse_data;
	zevent_connector_on_connect on_connect;
} zevent_connector_opt;

/*
* add by hyzeng:2013-7-5
* ����:����connector
* ���:NULL=ʧ��
*/
ZLIB_API zevent* zevent_connector_new(int rd_buf_size, int wr_buf_size, zevent_connector_opt opt);

/*
* add by hyzeng:2013-7-5
* ����:��ʼ��connector
* ���:
*/
ZLIB_API void zevent_connector_intial(zevent* connector, const struct sockaddr_in* addr,
	void* param1, void* param2);

/*
* add by hyzeng:2013-7-5
* ����:���ӷ�����
       ��������������socket�������ӷ����������ӳɹ��ص�on_connect,
       Ȼ�󴴽��շ�buffer
* ���:0=���ӳɹ���-1=ʧ��
*/
ZLIB_API int zevent_connector_connect(zevent* connector);

/*
* add by hyzeng:2013-7-5
* ����:����һ���Ѿ�����socket�������շ�buffer���ɹ��ص�on_connect
       ����Ӧ���ڷ�����acceptһ������
* ���:0=�ɹ���-1=ʧ��
*/
ZLIB_API int zevent_connector_accept(zevent* connector, int acpt_sock,
	const struct sockaddr_in* addr);

/*
* add by hyzeng:2013-7-5
* ����:�ر�connector
       �˺�������ر�socket�������շ�buffer�����ص�on_connect
* ���:
*/
ZLIB_API int zevent_connector_close(zevent* connector);

//int zevent_connector_shutdown(zevent* connector, int flags);

ZLIB_API zbaselib_skbuffer* zevent_connector_get_rd_buf(zevent* connector);

ZLIB_API zbaselib_skbuffer* zevent_connector_get_wr_buf(zevent* connector);

// 0=�ɹ�, -1=ʧ�ܣ�-2=ʣ��ռ䲻��
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

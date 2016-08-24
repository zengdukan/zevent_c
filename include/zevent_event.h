#ifndef _ZEVENT_EVENT_H_
#define _ZEVENT_EVENT_H_

#include "zevent_typedef.h"
#include "zbaselib_socket.h"

ZBASELIB_BEGIN_DECL

typedef struct zevent_s zevent;

typedef int (*zevent_read_cb)(zevent* event);
typedef int (*zevent_write_cb)(zevent* event);
typedef void (*zevent_delete_cb)(zevent* event);

// reactor����read/write�������󣬵��ô˺������ṩĬ�Ϻ�������ӡ���err
typedef void (*zevent_error_cb)(zevent* event, int err);

typedef int (*zevent_check_read_cb)(zevent* event);

typedef int (*zevent_check_write_cb)(zevent* event);

struct zevent_s
{
	zbaselib_socket_t fd;
	int type;
	int err;	// �����־��0-�ޣ�����-����
	int isadd;	// �Ƿ���ӵ�looper
	int timeout;
	
	zevent_read_cb read_cb;
	zevent_write_cb write_cb;
	zevent_delete_cb delete_cb;
	
	zevent_error_cb error_cb;	// ÿ��event����ʵ��
	zevent_check_read_cb check_rd_cb;	// �Ƿ������ݿɶ�,����ӵ�select
	zevent_check_write_cb check_wr_cb;	// �Ƿ������ݿ�д
	
	char priv[0];
};

/*
* add by hyzeng:2013-7-4
* ����:������,�¼��������read_cb�����read_cb
       ע��:�˺����ᱻlooper�Զ����ã������ں���
       �������������ں����ڵ��ü�������
* ���:0-�ɹ���-1=ʧ��
*/
ZLIB_API int zevent_read(zevent* event);

/*
* add by hyzeng:2013-7-4
* ����:д�¼�,�¼��������write_cb�����write_cb
       ע��:�˺����ᱻlooper�Զ����ã������ں���
       �������������ں����ڵ��ü�������������������
       ����
* ���:
*/
ZLIB_API int zevent_write(zevent* event);

/*
* add by hyzeng:2013-7-4
* ����:ɾ���ͷ�event,�¼��������delete_cb�����delete_cb
       ע��:���event�����뵽looper��deleteʱ���looperɾ��
       ��zevent_looper_del_event�Ǽ����ģ����Դ˺��������
       ��delete_cb���ܱ������������ã�Ҳ���ܵ��ü�������
       ���������
* ���:
*/
ZLIB_API void zevent_delete(zevent* event);

/*
* add by hyzeng:2013-7-4
* ����:�ص�error_cb, err-�����롣
	   �ص�error_cb���ܵ��ü�������
       ���������
* ���:
*/
ZLIB_API void zevent_error(zevent* event, int err);

/*
* add by hyzeng:2013-7-4
* ����:�����Ƿ������ݶ�
* ���:1=�У�0=��
*/
ZLIB_API int zevent_check_read(zevent* event);

/*
* add by hyzeng:2013-7-4
* ����:����Ƿ�������д
* ���:1=�У�0=��
*/
ZLIB_API int zevent_check_write(zevent* event);

/*
* add by hyzeng:2013-7-4
* ����:����socket
* ���:
*/
ZLIB_API zbaselib_socket_t zevent_get_fd(zevent* event);

ZLIB_API int zevent_get_type(zevent* event);

ZLIB_API int zevent_get_timeout(zevent* event);

ZLIB_API int zevent_get_error(zevent* event);

/*
* add by hyzeng:2013-7-4
* ����:��ʼ��event�������ԣ�ͨ����event�����init��������
* ���:
*/
ZLIB_API void zevent_base_init(zevent* event, int timeout, int type);

/*
* add by hyzeng:2013-7-4
* ����:����event�������ԣ���zevent_delete����
* ���:
*/
ZLIB_API void zevent_base_uninit(zevent* event);

/*
* add by hyzeng:2013-7-4
* ����:����error_cb�ص�
* ���:
*/
ZLIB_API void zevent_set_error_cb(zevent* event, zevent_error_cb cb);

ZBASELIB_END_DECL

#endif
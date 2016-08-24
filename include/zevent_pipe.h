#ifndef _ZEVENT_PIPE_H_
#define _ZEVENT_PIPE_H_

#include "zevent_event.h"

ZBASELIB_BEGIN_DECL

/*
* add by hyzeng:2013-7-4
* ����:�ܵ���������ע��:���ڲ��ܵ��ü���ϵͳ������
* ���:0=�ɹ���-1=ʧ��
*/
typedef int (*zevent_pipe_process_msg_cb)(void* data, int data_size, int msg_type,
	void* param1, void* param2);

/*
* add by hyzeng:2013-7-4
* ����:�����ܵ�
* ���:NULL=ʧ��
*/
ZLIB_API zevent* zevent_pipe_new(zevent_pipe_process_msg_cb process_msg);

/*
* add by hyzeng:2013-7-4
* ����:��ʼ��pipe
* ���:
*/
ZLIB_API void zevent_pipe_intial(zevent* pipe, void* param1, void* param2);

/*
* add by hyzeng:2013-7-4
* ����:����pipe����
* ���:0=�ɹ���-1=ʧ��
*/
ZLIB_API int zevent_pipe_send_msg(zevent* pipe, void* data, int data_size, int msg_type);

ZBASELIB_END_DECL

#endif/*_ZEVENT_PIPE_H_*/
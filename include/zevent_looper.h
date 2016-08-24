#ifndef _ZEVENT_LOOPER_H_
#define _ZEVENT_LOOPER_H_

#include "zevent_event.h"
#include "zevent_reactor.h"
#include "zbaselib_list.h"
#include "zbaselib_thread.h"

ZBASELIB_BEGIN_DECL

typedef struct zevent_looper_s
{
	zbaselib_list* ev_lists;		// ��ʱ���С�����ź���
	zevent_reactor* reactor;
	int is_run;				// ֹͣ��־
	zbaselib_thread_t th_id;
	zbaselib_mutex_t mutex;
} zevent_looper;

/*
* add by hyzeng:2013-7-5
* ����:����looper
* ���:NULL=ʧ��
*/
ZLIB_API zevent_looper* zevent_looper_new();

/*
* add by hyzeng:2013-7-5
* ����:����looper
* ���:
*/
ZLIB_API void zevent_looper_delete(zevent_looper* looper);

/*
* add by hyzeng:2013-7-5
* ����:����¼����˺����������ע������
* ���:0=�ɹ���-1=ʧ��
*/
ZLIB_API int zevent_looper_add_event(zevent_looper* looper, zevent* ev);

ZLIB_API int zevent_looper_add_event_unlock(zevent_looper* looper, zevent* ev);

/*
* add by hyzeng:2013-7-5
* ����:�Ƴ��¼����˺����������ע������
* ���:0=�ɹ���-1=ʧ��
*/
ZLIB_API int zevent_looper_del_event(zevent_looper* looper, zevent* ev);

ZLIB_API int zevent_looper_del_event_unlock(zevent_looper* looper, zevent* ev);

/*
* add by hyzeng:2013-7-5
* ����:����looper
* ���:0=�ɹ���-1=ʧ��
*/
ZLIB_API int zevent_looper_start(zevent_looper* looper);

/*
* add by hyzeng:2013-7-5
* ����:ֹͣlooper
* ���:0=�ɹ���-1=ʧ��
*/
ZLIB_API int zevent_looper_stop(zevent_looper* looper);

ZBASELIB_END_DECL

#endif
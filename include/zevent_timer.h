#ifndef _ZEVENT_TIMER_H_
#define _ZEVENT_TIMER_H_

#include "zevent_event.h"
//#include "zevent_looper.h"

ZBASELIB_BEGIN_DECL

#define ZEVENT_TIMER_CONTINUE	1
#define ZEVENT_TIMER_OK			0	// timer_cb�ص�����OK����ʾ��ʱ�����

/*
* add by hyzeng:2013-7-4
* ����:��ʱ���ص���������looper�߳��ڵ��ã��˻ص��ڲ��ܼ�������������
* ���:ZEVENT_TIMER_CONTINUE/ZEVENT_TIMER_OK
*/
typedef int (*zevent_timer_cb)(void* param1, void* param2);

/*
* add by hyzeng:2013-7-4
* ����:������ʱ��
       interval=��ʱ�������unit:ms
       timer_cb=��ʱ�ص�����
* ���:NULL=ʧ��
*/
ZLIB_API zevent* zevent_timer_new(int interval, zevent_timer_cb timer_cb);

/*
* add by hyzeng:2013-7-4
* ����:��ʼ����ʱ��
* ���:
*/
ZLIB_API void zevent_timer_intial(zevent* timer, void* param1, void* param2);

/*
* add by hyzeng:2013-7-4
* ����:�޸�ʱ��������λms
* ���:0=�ɹ���-1=ʧ��
*/
ZLIB_API int zevent_timer_modify(zevent* timer, int interval);

/*
* add by hyzeng:2013-7-4
* ����:��λ���¼�ʱ
* ���:0=�ɹ���-1=ʧ��
*/
ZLIB_API int zevent_timer_reset(zevent* timer);

/*
* add by hyzeng:2013-7-4
* ����:������һ����Чʱ��
* ���:������һ����Чʱ��
*/
ZLIB_API llong zevent_timer_get_next_time(zevent* timer);

/*
* add by hyzeng:2013-7-4
* ����:������һ����Чʱ��
* ���:
*/
ZLIB_API void zevent_timer_calc_next_time(zevent* timer);

ZBASELIB_END_DECL

#endif

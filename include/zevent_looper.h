#ifndef _ZEVENT_LOOPER_H_
#define _ZEVENT_LOOPER_H_

#include "zevent_event.h"
#include "zevent_reactor.h"
#include "zbaselib_list.h"
#include "zbaselib_thread.h"

ZBASELIB_BEGIN_DECL

typedef struct zevent_looper_s
{
	zbaselib_list* ev_lists;		// 按时间从小到大排好序
	zevent_reactor* reactor;
	int is_run;				// 停止标志
	zbaselib_thread_t th_id;
	zbaselib_mutex_t mutex;
} zevent_looper;

/*
* add by hyzeng:2013-7-5
* 功能:创建looper
* 结果:NULL=失败
*/
ZLIB_API zevent_looper* zevent_looper_new();

/*
* add by hyzeng:2013-7-5
* 功能:销毁looper
* 结果:
*/
ZLIB_API void zevent_looper_delete(zevent_looper* looper);

/*
* add by hyzeng:2013-7-5
* 功能:添加事件，此函数会加锁，注意死锁
* 结果:0=成功，-1=失败
*/
ZLIB_API int zevent_looper_add_event(zevent_looper* looper, zevent* ev);

ZLIB_API int zevent_looper_add_event_unlock(zevent_looper* looper, zevent* ev);

/*
* add by hyzeng:2013-7-5
* 功能:移除事件，此函数会加锁，注意死锁
* 结果:0=成功，-1=失败
*/
ZLIB_API int zevent_looper_del_event(zevent_looper* looper, zevent* ev);

ZLIB_API int zevent_looper_del_event_unlock(zevent_looper* looper, zevent* ev);

/*
* add by hyzeng:2013-7-5
* 功能:启动looper
* 结果:0=成功，-1=失败
*/
ZLIB_API int zevent_looper_start(zevent_looper* looper);

/*
* add by hyzeng:2013-7-5
* 功能:停止looper
* 结果:0=成功，-1=失败
*/
ZLIB_API int zevent_looper_stop(zevent_looper* looper);

ZBASELIB_END_DECL

#endif
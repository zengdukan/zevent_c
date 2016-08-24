#ifndef _ZEVENT_TIMER_H_
#define _ZEVENT_TIMER_H_

#include "zevent_event.h"
//#include "zevent_looper.h"

ZBASELIB_BEGIN_DECL

#define ZEVENT_TIMER_CONTINUE	1
#define ZEVENT_TIMER_OK			0	// timer_cb回调返回OK，表示定时器完成

/*
* add by hyzeng:2013-7-4
* 功能:定时器回调函数，在looper线程内调用，此回调内不能加锁，否则死锁
* 结果:ZEVENT_TIMER_CONTINUE/ZEVENT_TIMER_OK
*/
typedef int (*zevent_timer_cb)(void* param1, void* param2);

/*
* add by hyzeng:2013-7-4
* 功能:创建定时器
       interval=定时器间隔，unit:ms
       timer_cb=定时回调函数
* 结果:NULL=失败
*/
ZLIB_API zevent* zevent_timer_new(int interval, zevent_timer_cb timer_cb);

/*
* add by hyzeng:2013-7-4
* 功能:初始化定时器
* 结果:
*/
ZLIB_API void zevent_timer_intial(zevent* timer, void* param1, void* param2);

/*
* add by hyzeng:2013-7-4
* 功能:修改时间间隔，单位ms
* 结果:0=成功，-1=失败
*/
ZLIB_API int zevent_timer_modify(zevent* timer, int interval);

/*
* add by hyzeng:2013-7-4
* 功能:复位重新计时
* 结果:0=成功，-1=失败
*/
ZLIB_API int zevent_timer_reset(zevent* timer);

/*
* add by hyzeng:2013-7-4
* 功能:返回下一次有效时间
* 结果:返回下一次有效时间
*/
ZLIB_API llong zevent_timer_get_next_time(zevent* timer);

/*
* add by hyzeng:2013-7-4
* 功能:计算下一次有效时间
* 结果:
*/
ZLIB_API void zevent_timer_calc_next_time(zevent* timer);

ZBASELIB_END_DECL

#endif

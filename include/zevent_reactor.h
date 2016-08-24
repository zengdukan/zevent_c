#ifndef _ZEVENT_REACTOR_H_
#define _ZEVENT_REACTOR_H_

#include "zevent_event.h"

ZBASELIB_BEGIN_DECL

typedef struct zevent_reactor_s zevent_reactor;


typedef void (*zevent_reactor_deinit_cb)(zevent_reactor* reactor);
typedef int (*zevent_reactor_add_event_cb)(zevent_reactor* reactor, zevent* ev);
typedef int (*zevent_reactor_del_event_cb)(zevent_reactor* reacotr, zevent* ev);

// @timeout:毫秒ms                
// 定时事件-ontimer返回continue 更新时间，继续添加它；返回ok/error:从队列中移除它
typedef int (*zevent_reactor_do_event_cb)(zevent_reactor* reactor, int timeout);

struct zevent_reactor_s
{
	zevent_reactor_deinit_cb deinit_cb;
	zevent_reactor_add_event_cb add_event_cb;
	zevent_reactor_del_event_cb del_event_cb;
	zevent_reactor_do_event_cb do_event_cb;
	char priv[0];
};

static void zevent_reactor_deinit(zevent_reactor* reactor)
{
	(*reactor->deinit_cb)(reactor);
}

static int zevent_reactor_add_event(zevent_reactor* reactor, zevent* ev)
{
	return (*reactor->add_event_cb)(reactor, ev);
}

static int zevent_reactor_del_event(zevent_reactor* reactor, zevent* ev)
{
	return(*reactor->del_event_cb)(reactor, ev);
}

static int zevent_reactor_do_event(zevent_reactor* reactor, int timeout)
{
	return (*reactor->do_event_cb)(reactor, timeout);
}

ZBASELIB_END_DECL
#endif
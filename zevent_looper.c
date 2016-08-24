#include "zevent_looper.h"
#include "zevent.h"
#include "zbaselib_array.h"
#ifdef WIN32
#include "zevent_reactor_winselect.h"
#else
#include "zevent_reactor_select.h"
#endif

#define ZEVENT_MAX_TIME	100		// 100ms

extern llong zevent_current_time;


static int zbaselib_list_addbymintime(zbaselib_list* thiz, zevent* ev);

static zbaselib_thread_result ZBASELIB_THREAD_PROCSPEC zevent_looper_work_thrd(zbaselib_thread_param param);


zevent_looper* zevent_looper_new()
{
	zevent_looper* looper = (zevent_looper*) calloc(1, sizeof(zevent_looper));
	if(looper != NULL)
	{
		looper->ev_lists = zbaselib_list_create(NULL);
#ifdef WIN32
		looper->reactor = zevent_reactor_winselect_init();
#else
		looper->reactor = zevent_reactor_select_init();
#endif
		looper->is_run = 0;
		looper->mutex = zbaselib_mutex_create();

		ZEVENT_PRINT_INFO("looper create ok.\n");
	}
	else
		ZEVENT_PRINT_INFO("looper create fail.\n");

	return looper;
}

void zevent_looper_delete(zevent_looper* looper)
{
	if(looper != NULL)
	{
		if(looper->is_run == 1)
			zevent_looper_stop(looper);

		if(looper->ev_lists != NULL)
		{
			zbaselib_list_destroy(&looper->ev_lists);
			looper->ev_lists = NULL;
		}

		if(looper->reactor != NULL)
		{
			zevent_reactor_deinit(looper->reactor);
			looper->reactor = NULL;
		}

		zbaselib_mutex_destroy(looper->mutex);

		free(looper);
	}
}

int zevent_looper_start(zevent_looper* looper)
{
	if(looper == NULL || looper->is_run == 1)
		return -1;

	looper->th_id = zbaselib_thread_create(zevent_looper_work_thrd, 
		(zbaselib_thread_param)looper);

	return looper->th_id == ZBASELIB_INVALID_THREAD ? -1 : 0;
}

int zevent_looper_stop(zevent_looper* looper)
{
	if(looper == NULL || looper->is_run == 0)
		return -1;

	looper->is_run = 0;
	zbaselib_thread_join(looper->th_id);
	return 0;
}

static int zbaselib_list_addbymintime(zbaselib_list* thiz, zevent* ev)
{
	zevent* pevent = NULL;
	int i = 0;
	zbaselib_list_iterater* zi = NULL;
	
	if(thiz == NULL || ev == pevent)
		return -1;
	
	zi = zbaselib_list_iterater_create(thiz);
	
	for(pevent = (zevent*)zbaselib_list_iterater_first(zi); 
		pevent != NULL && !zbaselib_list_iterater_isend(zi); 
		pevent = (zevent*)zbaselib_list_iterater_next(zi), i++)
	{
		if(zevent_get_timeout(pevent) >= zevent_get_timeout(ev))
			break;
	}

	zbaselib_list_add(thiz, (void*)ev, i);
	zbaselib_list_iterater_destroy(&zi);

	return 0;
}

static int zevent_do_timer(void* data, void* ctx)
{
	zevent* ev = (zevent*)data;
	//zevent_looper* looper = (zevent_looper*)ctx;
	if(zevent_get_type(ev) == ZEVENT_TYPE_TIMER)
	{
		llong next_time = zevent_timer_get_next_time(ev);
		if(next_time <= 0)
		{
			ev->isadd = 0;
			return ZEVENT_TIMER_OK;
		}
		else if(next_time <= zevent_current_time)
		{
			int ret = (zevent_write(ev) == ZEVENT_TIMER_OK);
			if(ret)
				ev->isadd = 0;
			return ret;
		}
	}

	return 0;
}

static zbaselib_thread_result ZBASELIB_THREAD_PROCSPEC zevent_looper_work_thrd(zbaselib_thread_param param)
{
	int min_timer = 0;
	llong interval = 0;
	zevent* pev = NULL;
	llong last_time = 0;
	
	zevent_looper* looper = (zevent_looper*)param;
	looper->is_run = 1;
	
	while(looper->is_run)
	{
		zbaselib_mutex_lock(looper->mutex);
		
		if(zbaselib_list_empty(looper->ev_lists))
		{
			zevent_update_current_time();			
			zbaselib_mutex_unlock(looper->mutex);
			zbaselib_sleep_msec(100);
			continue;
		}

		pev = (zevent*)zbaselib_list_gethead(looper->ev_lists);
		min_timer = zevent_get_timeout(pev);
		min_timer  = (min_timer < ZEVENT_MAX_TIME) ? min_timer : ZEVENT_MAX_TIME;

		last_time = zevent_current_time;

		// 执行reactor的precessEvent函数
		zevent_reactor_do_event(looper->reactor, min_timer);
		
		// 更新时间
		zevent_update_current_time();

		interval = zevent_current_time - last_time;
		if(interval > 0)
		{
			// 搜索列表中定时器时间超时的事件执行它们
			int del_num = zbaselib_list_del_use_cond(looper->ev_lists, zevent_do_timer, (void*)looper);
			if(del_num > 0)
			{
				ZEVENT_PRINT_INFO("zevent_looper del %d events; total events=%d.\n", del_num, 
					zbaselib_list_size(looper->ev_lists));
			}
		}

		zbaselib_mutex_unlock(looper->mutex);

		// 这里需要休眠，不然其他位置可能因为此处循环太快内核来不及通知释放锁导致竞争不到锁
		// 可能会发生add_event函数得不到锁阻塞
		zbaselib_sleep_msec(1);
	}

	return 0;
}

int zevent_looper_add_event(zevent_looper* looper, zevent* ev)
{
	int ret = 0;

	if(ev->isadd)
	{
		ZEVENT_PRINT_INFO("event(%p) had been added to looper.\n", ev);
		return -1;
	}

	zbaselib_mutex_lock(looper->mutex);
	
	if(zevent_get_type(ev) != ZEVENT_TYPE_TIMER)
		ret = zevent_reactor_add_event(looper->reactor, ev);
	
	if(ret == 0)
		ret = zbaselib_list_addbymintime(looper->ev_lists, ev);
	
	if(ret < 0)
	{
		ZEVENT_PRINT_INFO("zevent_looper add event(%p) fail.\n", ev);
		zbaselib_mutex_unlock(looper->mutex);
		return -1;
	}

	zbaselib_mutex_unlock(looper->mutex);
	ev->isadd = 1;
	ZEVENT_PRINT_INFO("zevent_looper add a event(%p) ok, type=%d; total events=%d.\n", ev, zevent_get_type(ev), zbaselib_list_size(looper->ev_lists));
	return ret;
}

int zevent_looper_add_event_unlock(zevent_looper* looper, zevent* ev)
{
	int ret = 0;

	if(ev->isadd)
	{
		ZEVENT_PRINT_INFO("event(%p) had been added to looper.\n", ev);
		return -1;
	}

	if(zevent_get_type(ev) != ZEVENT_TYPE_TIMER)
		ret = zevent_reactor_add_event(looper->reactor, ev);
	
	if(ret == 0)
		ret = zbaselib_list_addbymintime(looper->ev_lists, ev);
	
	if(ret < 0)
	{
		ZEVENT_PRINT_INFO("zevent_looper add event(%p) fail.\n", ev);
		return -1;
	}

	ev->isadd = 1;
	ZEVENT_PRINT_INFO("zevent_looper add a event(%p) ok, type=%d; total events=%d.\n", ev, zevent_get_type(ev), zbaselib_list_size(looper->ev_lists));
	return ret;
}

static int zevent_cmp(void *data, void *ctx)
{
	return (data == ctx);
}

int zevent_looper_del_event(zevent_looper* looper, zevent* ev)
{
	int index = 0;

	if(ev->isadd == 0)
	{
		ZEVENT_PRINT_INFO("event(%p) not in looper.\n", ev);
		return -1;
	}

	zbaselib_mutex_lock(looper->mutex);
	
	if(zevent_get_type(ev) != ZEVENT_TYPE_TIMER)
	{
		if(zevent_reactor_del_event(looper->reactor, ev) == -1)
		{
			ZEVENT_PRINT_INFO("zevent_looper del event(%p) fail\n", ev);
			zbaselib_mutex_unlock(looper->mutex);
			return -1;
		}
	}

//	if(zevent_get_type(ev) == ZEVENT_TYPE_TIMER)
	{
		index = zbaselib_list_del_use_cond(looper->ev_lists, zevent_cmp, ev);
		if(index <= 0)
			ZEVENT_PRINT_INFO("zevent_looper no find event(%p)\n", ev);
	}

	ZEVENT_PRINT_INFO("zevent_looper del a event(%p); total events=%d.\n", ev, 
		zbaselib_list_size(looper->ev_lists));
	
	ev->isadd = 0;
	zbaselib_mutex_unlock(looper->mutex);
	return 0;
}

int zevent_looper_del_event_unlock(zevent_looper* looper, zevent* ev)
{
	int index = 0;

	if(ev->isadd == 0)
	{
		ZEVENT_PRINT_INFO("event(%p) not in looper.\n", ev);
		return -1;
	}
	/*
	if(zevent_get_type(ev) != ZEVENT_TYPE_TIMER)
	{
		if(zevent_reactor_del_event(looper->reactor, ev) == -1)
		{
			ZEVENT_PRINT_INFO("zevent_looper del event(%p) fail\n", ev);
			return -1;
		}
	}
	*/
	//if(zevent_get_type(ev) == ZEVENT_TYPE_TIMER)
	{
		index = zbaselib_list_del_use_cond(looper->ev_lists, zevent_cmp, ev);
		if(index < 0)
			ZEVENT_PRINT_INFO("zevent_looper no find event(%p)\n", ev);
	}

	ZEVENT_PRINT_INFO("zevent_looper del a event(%p); total events=%d.\n", ev, 
		zbaselib_list_size(looper->ev_lists));
	ev->isadd = 0;

	return 0;
}


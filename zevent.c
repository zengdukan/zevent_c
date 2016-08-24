#include "zevent.h"
#include "zbaselib.h"

llong zevent_current_time = 0;		// 全局的系统时间,单位ms

zevent_looper* default_looper = NULL;

void zevent_update_current_time()
{
	struct timeval tv = {0};
	zbaselib_gettimeofday(&tv);
	zevent_current_time = zbaselib_tv_to_msec(&tv);
}

llong zevent_get_current_time()
{
	return zevent_current_time;
}

int zevent_init()
{
	zbaselib_socket_init();
	zevent_update_current_time();

	default_looper = zevent_looper_new();
	if(default_looper == NULL)
		return -1;

//	if(zevent_looper_start(default_looper) != 0)
//		return -1;

	return 0;
}

void zevent_deinit()
{
	if(default_looper != NULL)
	{
		zevent_looper_stop(default_looper);
		zevent_looper_delete(default_looper);	

		default_looper = NULL;
	}
	
	zbaselib_socket_deinit();
}

zevent_looper* zevent_get_default_looper()
{
	return default_looper;
}
#include "zevent.h"
#include <assert.h>

int timer_cb(void* param1, void* param2)
{
	printf("timeout\n");

	return ZEVENT_TIMER_OK;
}


int main(int argc, char* argv[])
{
	char c;
	zevent* timer = NULL;
	
	assert(zevent_init() == 0);
	assert(zevent_looper_start(zevent_get_default_looper()) == 0);

	timer = zevent_timer_new(5*1000, timer_cb);

	while((c = getchar()) != 'q')
	{
		switch(c)
		{
		case '1':
			zevent_timer_reset(timer);
			assert(0 == zevent_looper_add_event(zevent_get_default_looper(), timer));
			break;
		case '2':
			assert(0 == zevent_looper_del_event(zevent_get_default_looper(), timer));
			break;
		}
	}

	zevent_delete(timer);
	
	zevent_looper_stop(zevent_get_default_looper());
	zevent_deinit();

	return 0;
}


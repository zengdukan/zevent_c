#include "zevent_timer.h"
//#include "zevent_looper.h"
#include "zevent_typedef.h"

/************************************************************************/
/* struct define                                                        */
/************************************************************************/
typedef struct priv_info_s
{
	zevent_timer_cb timer_cb;
	void* param1;
	void* param2;
	llong next_time;
} priv_info;

extern llong zevent_current_time;

/************************************************************************/
/* static function prev or var define                                   */
/************************************************************************/
static int zevent_timer_write(zevent* timer);

static void zevent_timer_delete(zevent* timer);

zevent* zevent_timer_new(int interval, zevent_timer_cb timer_cb)
{
	zevent* event = NULL;

	if(interval < 0)
	{
		ZEVENT_PRINT_DEBUG("timer new fail!\n");
		return NULL;
	}

	event = (zevent*) calloc(1, sizeof(zevent) + sizeof(priv_info));
	if(event != NULL)
	{
		DECL_PRIV(event, priv);

		zevent_base_init(event, interval, ZEVENT_TYPE_TIMER);
		event->read_cb = NULL;
		event->write_cb = zevent_timer_write;
		event->delete_cb = zevent_timer_delete;
		event->check_rd_cb = NULL;
		event->check_wr_cb = NULL;

		priv->timer_cb = timer_cb;
		zevent_timer_calc_next_time(event);
	}

	return event;
}

void zevent_timer_intial(zevent* timer, void* param1, void* param2)
{
	DECL_PRIV(timer, priv);
	priv->param1 = param1;
	priv->param2 = param2;
}


static int zevent_timer_write(zevent* timer)
{
	int ret = ZEVENT_TIMER_OK;
	DECL_PRIV(timer, priv);

	if(timer != NULL && priv->timer_cb != NULL)
		ret = (*priv->timer_cb)(priv->param1, priv->param2);

	zevent_timer_calc_next_time(timer);

	return ret;
}

static void zevent_timer_delete(zevent* timer)
{
	ZEVENT_PRINT_DEBUG("destroy timer %p.\n", timer);
	//FREE_PTR_OBJ(timer);
}

int zevent_timer_modify(zevent* timer, int interval)
{
	DECL_PRIV(timer, priv);

	if(timer == NULL || priv == NULL)
		return -1;

	timer->timeout = interval;
	zevent_timer_calc_next_time(timer);

	return 0;
}

int zevent_timer_reset(zevent* timer)
{
	DECL_PRIV(timer, priv);

	if(timer == NULL || priv == NULL)
		return -1;

	zevent_timer_calc_next_time(timer);

	return 0;
}

llong zevent_timer_get_next_time(zevent* timer)
{
	DECL_PRIV(timer, priv);

	if(timer == NULL || priv == NULL)
		return -1;

	return priv->next_time;
}

void zevent_timer_calc_next_time(zevent* timer)
{
	DECL_PRIV(timer, priv);

	if(timer == NULL || priv == NULL)
		return;
	
	priv->next_time = zevent_current_time + timer->timeout;
}



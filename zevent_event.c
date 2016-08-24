#include "zevent_event.h"
#include "zevent_looper.h"

static void zevent_default_error(zevent* event, int err)
{
	printf("err code = %d\n", event->err);
}

void zevent_base_init(zevent* event, int timeout, int type)
{
	event->fd = INVALID_SOCKET;
	event->type = type;
	event->timeout = timeout;
	event->err = 0;
	event->error_cb = zevent_default_error;
	//zevent_calc_next_time(event);
}

void zevent_base_uninit(zevent* event)
{
	if(event->fd != INVALID_SOCKET)
		zbaselib_socket_close(event->fd);
}

zbaselib_socket_t zevent_get_fd(zevent* event)
{
	return event->fd;
}

int zevent_get_type(zevent* event)
{
	return event->type;
}

int zevent_get_timeout(zevent* event)
{
	return event->timeout;
}

int zevent_read(zevent* event)
{
	return event->read_cb == NULL ? 0 : (event->read_cb(event));
}

int zevent_write(zevent* event)
{
	return event->write_cb == NULL ? 0 : (event->write_cb(event));
}

extern zevent_looper* default_looper;

void zevent_delete(zevent* event)
{
	if(event == NULL)
		return;
	
	if(event->isadd)
	{
		zevent_looper_del_event(default_looper, event);
		event->isadd = 0;
	}
	
	zevent_base_uninit(event);
	if(event->delete_cb != NULL)
		event->delete_cb(event);

	free(event);
	ZEVENT_PRINT_INFO("delete zevent %p\n", event);
}

void zevent_error(zevent* event, int err)
{
	event->err = err;
	if(event->error_cb != NULL)
		event->error_cb(event, err);
}

int zevent_check_read(zevent* event)
{
	if(event->check_rd_cb != NULL)
		return event->check_rd_cb(event);
	else
		return 1;
}

int zevent_check_write(zevent* event)
{
	if(event->check_wr_cb != NULL)
		return event->check_wr_cb(event);
	else
		return 1;
}

int zevent_get_error(zevent* event)
{
	return event->err;
}

void zevent_set_error_cb(zevent* event, zevent_error_cb cb)
{
	event->error_cb = cb;
}



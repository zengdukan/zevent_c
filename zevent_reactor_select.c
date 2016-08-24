#include "zevent_reactor_select.h"
#include "zevent_event.h"
#include "zbaselib_list.h"
#include "zevent_typedef.h"
#include "zevent.h"
#include <sys/select.h>

typedef struct priv_info_s
{
	zbaselib_list* evlist;		// 事件数组

	fd_set fs_read;
	fd_set fs_write;

	int nready1;
	int nready2;
	
	int max_fd;
} priv_info;


static void zevent_reactor_select_deinit(zevent_reactor* reactor);

static int zevent_reactor_select_add_event(zevent_reactor* reactor, zevent* ev);

static int zevent_reactor_select_del_event(zevent_reactor* reactor, zevent* ev);

static int zevent_reactor_select_do_event(zevent_reactor* reactor, int timeout);

static void zevent_reactor_select_repair_fd_set(zevent_reactor* reactor);

zevent_reactor* zevent_reactor_select_init()
{
	zevent_reactor* reactor = (zevent_reactor*) calloc(1, sizeof(zevent_reactor) +
		sizeof(priv_info));

	if(reactor != NULL)
	{
		DECL_PRIV(reactor, priv);

		reactor->deinit_cb = zevent_reactor_select_deinit;
		reactor->add_event_cb = zevent_reactor_select_add_event;
		reactor->del_event_cb = zevent_reactor_select_del_event;
		reactor->do_event_cb = zevent_reactor_select_do_event;

		priv->evlist = zbaselib_list_create(NULL);
		priv->max_fd = -1;
		FD_ZERO(&priv->fs_read);
		FD_ZERO(&priv->fs_write);
	}

	return reactor;
}

static void zevent_reactor_select_deinit(zevent_reactor* reactor)
{
	DECL_PRIV(reactor, priv);

	if(reactor == NULL || priv == NULL)
		return ;

	if(priv->evlist != NULL)
		zbaselib_list_destroy(&priv->evlist);

	free(reactor);
}

static int zevent_reactor_select_add_event(zevent_reactor* reactor, zevent* ev)
{
	DECL_PRIV(reactor, priv);
	zbaselib_socket_t ev_fd = -1;

	if(ev == NULL || reactor == NULL || priv == NULL)
		return -1;

	ev_fd = zevent_get_fd(ev);
	if(priv->max_fd != -1 && priv->max_fd < ev_fd)
		priv->max_fd = ev_fd;

	zbaselib_list_addtail(priv->evlist, (void*)ev);
	ZEVENT_PRINT_INFO("ZEventReactor_Select add event in list(%p), list size=%d\n",
		ev, zbaselib_list_size(priv->evlist));

	return 0;
}

static int zevent_ptr_cmp(void* ctx, void* data)
{
	return (ctx == data);
}

static int zevent_reactor_select_del_event(zevent_reactor* reactor, zevent* ev)
{
	DECL_PRIV(reactor, priv);
	zbaselib_socket_t ev_fd = -1;
	
	if(reactor == NULL || priv == NULL || ev == NULL)
		return -1;

	ev_fd = zevent_get_fd(ev);
	if(ev_fd == priv->max_fd)
		priv->max_fd = -1;

	if(zbaselib_list_del_use_cond(priv->evlist, zevent_ptr_cmp, (void*)ev) > 0)
	{
		ZEVENT_PRINT_INFO("ZEventReactor_Select del event(%p) from list; event list size=%d\n",
				ev, zbaselib_list_size(priv->evlist));
	}
	else
	{
		ZEVENT_PRINT_INFO("ZEventReactor_Select del event(%p) not find in list; event list size=%d\n",
				ev, zbaselib_list_size(priv->evlist));
	}

	return 0;
}

static void zevent_list_get_max(void *data, void *ctx)
{
	int* max_fd = (int*)ctx;
	zevent* zev = (zevent*)data;
	if(*max_fd < zevent_get_fd(zev))
				*max_fd = zevent_get_fd(zev);
}

static void zevent_select_add_fdset(void* data, void* ctx)
{
	priv_info* priv = (priv_info*)ctx;
	zevent* zev = (zevent*)data;
	int type = zevent_get_type(zev);
	
	if((type & ZEVENT_TYPE_READ)
		&& zevent_check_read(zev))
	{
		zbaselib_socket_t fd = zevent_get_fd(zev);
		if(zbaselib_socket_isvalid(fd))
			FD_SET(fd, &priv->fs_read);
	}

	if((type & ZEVENT_TYPE_WRITE)
		&& zevent_check_write(zev))
	{
		zbaselib_socket_t fd = zevent_get_fd(zev);
		if(zbaselib_socket_isvalid(fd))
			FD_SET(fd, &priv->fs_write);
	}
}

extern int zevent_looper_del_event_unlock(zevent_looper* looper, zevent* ev);

static int zevent_do_event(void* data, void* ctx)
{
	zevent* zev = (zevent*)data;
	priv_info* priv = (priv_info*)ctx;
	int ret = 0;
	
	if(zevent_get_type(zev) & ZEVENT_TYPE_READ)
	{
		zbaselib_socket_t fd = zevent_get_fd(zev);
		if(zbaselib_socket_isvalid(fd))
		{
			if(FD_ISSET(fd, &priv->fs_read))
			{
				priv->nready2++;
				ret = zevent_read(zev);
			}
		}
		else
			ret = -1;
	}

	if(ret == 0 && (zevent_get_type(zev) & ZEVENT_TYPE_WRITE))
	{
		zbaselib_socket_t fd = zevent_get_fd(zev);
		if(zbaselib_socket_isvalid(fd))
		{
			if(FD_ISSET(fd, &priv->fs_write))
			{
				priv->nready2++;
				ret = zevent_write(zev);
			}
		}
		else
			ret = -1;
	}

	if(ret != 0)
	{
		zevent_looper_del_event_unlock(zevent_get_default_looper(), zev);
		zevent_error(zev, zbaselib_socket_geterror());
	}

	return (ret != 0);
}

static int zevent_reactor_select_do_event(zevent_reactor* reactor, int timeout)
{
	static struct timeval tv;

	DECL_PRIV(reactor, priv);

	if(reactor == NULL || priv == NULL)
		return -1;

	tv.tv_sec = (long) (timeout / 1000);
	tv.tv_usec = (long) ((timeout % 1000) * 1000);

	if(zbaselib_list_empty(priv->evlist))
	{
		zbaselib_sleep_msec(timeout);
		return 0;
	}

	if(priv->max_fd == -1)
	{
		zbaselib_list_foreach(priv->evlist, zevent_list_get_max, (void*)&priv->max_fd);
	}
	ZEVENT_PRINT_DEBUG("select max fd = %d\n", priv->max_fd);

	FD_ZERO(&priv->fs_read);
	FD_ZERO(&priv->fs_write);
	priv->nready1 = 0;
	priv->nready2 = 0;

	// 添加读写fd到fd_set
	zbaselib_list_foreach(priv->evlist, zevent_select_add_fdset, (void*)priv);	

	priv->nready1 = select(priv->max_fd + 1, &priv->fs_read, &priv->fs_write, NULL, &tv);	
	// 错误
	if(priv->nready1 < 0)
	{
		if (errno == EBADF)
		{
			zevent_reactor_select_repair_fd_set(reactor);
			ZEVENT_PRINT_INFO("bad fd\n");
		}
		return -1;
	}
	else if(priv->nready1 == 0)
	{
		//ZEVENT_PRINT_INFO("timeout\n");
		return 0;
	}

	zbaselib_list_del_use_cond(priv->evlist, zevent_do_event, (void*)priv);

	if(priv->nready1 != priv->nready2)
	{
		ZEVENT_PRINT_INFO("ZEventReactor_Select:events size no match. %d:%d\n", 
			priv->nready1, priv->nready2);
		zevent_reactor_select_repair_fd_set(reactor);
	}

	return 0;
}


static void zevent_reactor_select_repair_fd_set(zevent_reactor* reactor)
{
	int           n;
	socklen_t     len;
	zbaselib_socket_t  s;
	
	DECL_PRIV(reactor, priv);
	if(reactor == NULL || priv == NULL)
		return;

	for (s = 0; s <= priv->max_fd; s++)
	{
		if (FD_ISSET(s, &priv->fs_read) == 0) 
			continue;

		len = sizeof(int);

		if (getsockopt(s, SOL_SOCKET, SO_TYPE, &n, &len) == -1) 
		{
			ZEVENT_PRINT_INFO("invalid descriptor %d in read fd_set\n", s);
			FD_CLR(s, &priv->fs_read);
		}
	}

	for (s = 0; s <= priv->max_fd; s++) 
	{
		if (FD_ISSET(s, &priv->fs_write) == 0) 
			continue;

		len = sizeof(int);

		if (getsockopt(s, SOL_SOCKET, SO_TYPE, &n, &len) == -1) 
		{
			ZEVENT_PRINT_INFO("invalid descriptor %d in write fd_set\n", s);
			FD_CLR(s, &priv->fs_write);
		}
	}

	priv->max_fd = -1;
}


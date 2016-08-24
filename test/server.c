#include "zevent.h"

extern llong zevent_current_time;
static zevent* clnt;


typedef struct svr_data_s
{
	int bytes;
	int old_bytes;
	llong last_time;
} svr_data_t;

static svr_data_t svr_data;
zbaselib_list* clnt_list = NULL;

//static int accept_cb(zevent* accepter, int clnt_sock, 
//	struct sockaddr_in* addr);

static int check_pack(zevent* connector)
{
	return zbaselib_skbuffer_datasize(zevent_connector_get_rd_buf(connector)) > 0;
}

static int timer_cb(void* param1, void* param2)
{
	zevent_update_current_time();
	printf("speed = %d(byte/s)\n", (svr_data.bytes - svr_data.old_bytes)/
		((zevent_current_time - svr_data.last_time)/1000));

	svr_data.old_bytes = svr_data.bytes;
	svr_data.last_time = zevent_current_time;

	return ZEVENT_TIMER_CONTINUE;
}

static int parse_data(zevent* connector)
{
	svr_data.bytes += zbaselib_skbuffer_datasize(
		zevent_connector_get_rd_buf(connector));
	zbaselib_skbuffer_clear(zevent_connector_get_rd_buf(connector));

	return 0;
}

zevent_connector_opt opt = {
		check_pack,
		parse_data,
		NULL,
};

int EventCmpFunc(void *data, void *ctx)
{
	return (data == ctx);
}


void clnt_error_cb(zevent* event, int err)
{
	//event->delete_cb(event);
	//free(event);
	zevent_delete(event);

	if(clnt_list != NULL)
	{
		zbaselib_list_del_use_cond(clnt_list, EventCmpFunc, (void*)event);
	}
}

int accepter_on_accept_cb(zevent* accepter, zevent* connector,
	int clnt_sock, struct sockaddr_in* addr)
{
	zevent_connector_set_opt(connector, opt);
	zevent_set_error_cb(connector, clnt_error_cb);

	if(clnt_list != NULL)
		zbaselib_list_addtail(clnt_list, (void*)connector);
	
	return 0;
}


int main(int argc, char* argv[])
{
	zevent* server;
	struct sockaddr_in addr;
	zevent* timer;

	zbaselib_list_iterater* zi = NULL;
	zevent* ev;
	
	zbaselib_socket_setaddr(&addr, htonl(INADDR_ANY), 9000);
	memset(&svr_data, 0, sizeof(svr_data));

	clnt_list = zbaselib_list_create(NULL);
	
	zevent_init();
	zevent_looper_start(zevent_get_default_looper());

	svr_data.last_time = zevent_current_time;

	server = zevent_accepter_new(&addr, accepter_on_accept_cb);
	timer = zevent_timer_new(1000, timer_cb);

	zevent_looper_add_event(zevent_get_default_looper(), server);
	zevent_looper_add_event(zevent_get_default_looper(), timer);
	
	getchar();

	zevent_looper_stop(zevent_get_default_looper());

	zevent_delete(server);
	zevent_delete(timer);

	zi = zbaselib_list_iterater_create(clnt_list);
    for(ev = zbaselib_list_iterater_first(zi); !zbaselib_list_iterater_isend(zi); ev = zbaselib_list_iterater_next(zi))
        zevent_delete(ev);

	zbaselib_list_iterater_destroy(&zi);
	zbaselib_list_destroy(&clnt_list);

	zevent_deinit();

	return 0;
}
/*
static int accept_cb(zevent* accepter, int clnt_sock, 
	struct sockaddr_in* addr)
{
	zevent_connector_opt opt = {
		check_pack,
		parse_data,
		NULL,
	};
	clnt = zevent_connector_new(1024*1024, 32, opt);
	zevent_connector_accept(clnt, clnt_sock, addr);
	zevent_looper_add_event(zevent_get_default_looper(), clnt);

	return 0;
}
*/

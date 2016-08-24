#include "zevent.h"

extern llong zevent_current_time;
zbaselib_list* clnt_list = NULL;
int total = 0;
int ok = 0;


static int check_pack(zevent* connector)
{
	return zbaselib_skbuffer_datasize(zevent_connector_get_rd_buf(connector)) >= 4;
}

static int parse_data(zevent* connector)
{
	int data = 0;
	int size = zbaselib_skbuffer_read(zevent_connector_get_rd_buf(connector), (char*)&data, 4);
	if(4 == size)
	{
		if(data == (int)connector)
			ok++;
			
	}

	// ·µ»Ø-1£¬reactorÅĞ¶Ï¶Á³ö´í£¬¹Ø±Õ²¢ÒÆ³ısocket
	return -1;
}

int connector_on_connect(zevent* connector, int conned)
{
	if(conned)
	{
		int data = (int)connector;
		int size = zevent_connector_send_data(connector, &data, 4);
	}

	return 0;
}


zevent_connector_opt opt = {
		check_pack,
		parse_data,
		connector_on_connect,
};

int EventCmpFunc(void *data, void *ctx)
{
	return (data == ctx);
}


void clnt_error_cb(zevent* event, int err)
{
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
	total++;
	
	if(clnt_list != NULL)
		zbaselib_list_addtail(clnt_list, (void*)connector);
	
	return 0;
}

int my_getchar()
{
	fd_set rfds;
   struct timeval tv;
   int retval;
	int ret = -1;
	char c;
   /* Watch stdin (fd 0) to see when it has input. */
   FD_ZERO(&rfds);
   FD_SET(0, &rfds);

   /* Wait up to five seconds. */
   tv.tv_sec = 0;
   tv.tv_usec = 500000;

   retval = select(1, &rfds, NULL, NULL, &tv);
   /* Don?¡¥t rely on the value of tv now! */

   if(retval > 0)
   {
   
	 ret = read(0, &c, 1);
	 if(ret > 0)
	 	ret = c;
   }

   return ret;
}


int main(int argc, char* argv[])
{
	zevent* server;
	struct sockaddr_in addr;
	zevent* timer;

	zbaselib_list_iterater* zi = NULL;
	zevent* ev;
	char c = 0;
	
	zbaselib_socket_setaddr(&addr, htonl(INADDR_ANY), 9000);

	clnt_list = zbaselib_list_create(NULL);
	
	zevent_init();
	zevent_looper_start(zevent_get_default_looper());

	server = zevent_accepter_new(&addr, accepter_on_accept_cb);

	zevent_looper_add_event(zevent_get_default_looper(), server);
	
	while((c = getchar()) != 'q')
	{
		printf("total = %d, ok = %d, fail = %.2f%\n", total, ok, total == 0 ? 0 : (ok + 0.0)/total);
	}

	zevent_looper_stop(zevent_get_default_looper());

	zevent_delete(server);

	zi = zbaselib_list_iterater_create(clnt_list);
    for(ev = zbaselib_list_iterater_first(zi); !zbaselib_list_iterater_isend(zi); ev = zbaselib_list_iterater_next(zi))
        zevent_delete(ev);

	zbaselib_list_iterater_destroy(&zi);
	zbaselib_list_destroy(&clnt_list);

	zevent_deinit();

	return 0;
}


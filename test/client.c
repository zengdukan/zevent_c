#include "zevent.h"
#include <assert.h>

#define PORT  9000


void connector_on_error(zevent* connector, int error_code)
{
	printf("err:%s\n", strerror(error_code));
}
#if 0
int connector_on_check_pack(zevent* connector, void* param1, void* param2)
{
	zbaselib_skbuffer* skbuf = zevent_connector_get_rd_buf(connector);
	return (zbaselib_skbuffer_datasize(skbuf) > 0);
}

int connector_on_parse_data(zevent* connector, void* param1, void* param2)
{
	zbaselib_skbuffer* skbuf = zevent_connector_get_rd_buf(connector);
	int size = zbaselib_skbuffer_datasize(skbuf);
	if(size > 0)
	{
		//char* data = 
	}

	return 0;
}
#endif
int connector_on_connect(zevent* connector, int conned)
{
	if(conned)
		printf("connectd to svr\n");
	else
		printf("disconnected from svr\n");

	return 0;
}


int main(int argc, char* argv[])
{
	zevent* clnt;
	struct sockaddr_in addr;
	int size = 10240;//atoi(argv[1]);
	char* data = (char*) calloc(1, size);
	zevent_connector_opt conn_opt = {NULL, NULL, connector_on_connect};
	int conned;
	int i = 0;
	
	assert(zevent_init() == 0);
	zevent_looper_start(zevent_get_default_looper());
	
	zbaselib_socket_setaddr(&addr, inet_addr("127.0.0.1"), PORT);
	clnt = zevent_connector_new(32, 1024*1024, conn_opt);
	zevent_set_error_cb(clnt, connector_on_error);
	zevent_connector_intial(clnt, &addr, NULL, NULL);
CONN:
	if(i > 10)
		goto QUIT;
	conned = zevent_connector_connect(clnt);
	if(conned == 0)
	{
		conned = 1;
		assert(zevent_connector_is_connected(clnt) == 1);
		zevent_looper_add_event(zevent_get_default_looper(), clnt);
		i++;
		while(zevent_connector_is_connected(clnt))
		{
#if 1
			if(zevent_connector_send_data(clnt, data,size) < 0)
			{
				printf("error:%d\n", zbaselib_socket_geterror());
				zevent_connector_close(clnt);
			//	zevent_delete(clnt);
				break;
			}

#endif
			zbaselib_sleep_msec(100);
		}

		zevent_looper_del_event(zevent_get_default_looper(), clnt);
		goto CONN;
	}
	else
	{
		printf("error:%d\n", zbaselib_socket_geterror());
		zbaselib_sleep_msec(1000);
		goto CONN;
	}

QUIT:
	zevent_looper_stop(zevent_get_default_looper());

	zevent_delete(clnt);
	free(data);

	zevent_deinit();

	return 0;
}
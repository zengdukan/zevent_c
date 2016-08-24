#include "zevent.h"
#include <assert.h>

#define PORT  9000


// 检测数据包是否完整，1=是，0-否
int connector_on_check_pack(zevent* connector)
{
	return (zbaselib_skbuffer_datasize(zevent_connector_get_rd_buf(connector)) >= 4);
}

// 数据解析回调
int connector_on_parse_data(zevent* connector)
{
	char data[128];
	int size  = 0;
	size = zbaselib_skbuffer_read(zevent_connector_get_rd_buf(connector), data, 128);

	if(size > 0)
	{
		zevent_connector_send_data(connector, data, size);
	}

	return 0;
}


int main(int argc, char* argv[])
{
	zevent* clnt;
	struct sockaddr_in addr;
	zevent_connector_opt conn_opt = {connector_on_check_pack, connector_on_parse_data, NULL};
	int n = 0;

	assert(zevent_init() == 0);
	zevent_looper_start(zevent_get_default_looper());
	
	zbaselib_socket_setaddr(&addr, inet_addr("192.168.101.1"), PORT);
	clnt = zevent_connector_new(32, 128, conn_opt);
	zevent_connector_intial(clnt, &addr, NULL, NULL);

	while(n < 100)
	{
		if(zevent_connector_connect(clnt) == 0)
		{
			n++;
			zevent_looper_add_event(zevent_get_default_looper(), clnt);
			while(zevent_connector_is_connected(clnt))
			{
			//	zevent_connector_send_data(clnt, (char*)clnt, 4);
				zbaselib_sleep_msec(100);
			}
			
			zevent_looper_del_event(zevent_get_default_looper(), clnt);
			zevent_connector_close(clnt);
		}	
		else
		{
			printf("error:%d\n", zbaselib_socket_geterror());
			zbaselib_sleep_msec(1000);
		}
	}

	zevent_looper_stop(zevent_get_default_looper());

	zevent_delete(clnt);

	zevent_deinit();

	return 0;
}


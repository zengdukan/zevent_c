#include "zevent_accepter.h"
#include "zevent_looper.h"

// socket读写等待时间，单位ms
#define ZEVENT_ACCEPTOR_RW_TIMEOUT	100

typedef struct priv_info_s
{
	zevent_accepter_on_accept_cb on_accept;
	struct sockaddr_in addr;
	zevent_connector_opt opt;

	void* param1;
	void* param2;
} priv_info;

static int zevent_accepter_read(zevent* accepter);

static void zevent_accepter_delete(zevent* accepter);

static int zevent_accepter_listen(zevent* accepter);

zevent* zevent_accepter_new(const struct sockaddr_in* addr, 
	zevent_accepter_on_accept_cb on_accept_cb)
{
	zevent* accepter = NULL;
	if(addr == NULL)
		return NULL;

	accepter = (zevent*) calloc(1, sizeof(zevent) + sizeof(priv_info));
	if(accepter != NULL)
	{
		DECL_PRIV(accepter, priv);
		
		zevent_base_init(accepter, ZEVENT_ACCEPTOR_RW_TIMEOUT, ZEVENT_TYPE_READ);
		accepter->read_cb = zevent_accepter_read;
		accepter->write_cb = NULL;
		accepter->delete_cb = NULL;

		priv->on_accept = on_accept_cb;
		priv->addr = *addr;

		if(zevent_accepter_listen(accepter) == -1)
		{
			zevent_delete(accepter);
			accepter = NULL;
		}
	}

	return accepter;
}

int zevent_accepter_set_param(zevent* accepter, void* param1, void* param2)
{
	DECL_PRIV(accepter, priv);
	
	if(accepter == NULL || priv == NULL)
		return -1;

	priv->param1 = param1;
	priv->param2 = param2;

	return 0;
}

void* zevent_accepter_get_param1(zevent* accepter)
{
	DECL_PRIV(accepter, priv);
	
	if(accepter == NULL || priv == NULL)
		return NULL;

	return priv->param1;
}

void* zevent_accepter_get_param2(zevent* accepter)
{
	DECL_PRIV(accepter, priv);
	
	if(accepter == NULL || priv == NULL)
		return NULL;

	return priv->param2;
}


int zevent_accepter_update_addr(zevent* accepter, const struct sockaddr_in* addr)
{
	DECL_PRIV(accepter, priv);
	zbaselib_socket_t svr;
	
	if(accepter == NULL || priv == NULL || addr == NULL)
		return -1;

	svr = zbaselib_socket_create_tcpserver(addr);
	if(!zbaselib_socket_isvalid(svr))
	{
		MESSAGE("[error] create tcp svr fail, code = %d\n", zbaselib_socket_geterror());
		return -1;
	}

	zbaselib_socket_close(accepter->fd);
	accepter->fd = svr;
	priv->addr = *addr;

	return 0;
}

extern int zevent_looper_add_event_unlock(zevent_looper* looper, zevent* ev);
extern zevent_looper* default_looper;


static int zevent_accepter_read(zevent* accepter)
{
	zbaselib_socket_t acpt_sock;
	struct sockaddr_in addr = {0};
	socklen_t addr_size = sizeof(addr);
	zevent* clnt;

	// 这里要一直读直到block
	do
	{
		memset(&addr, 0, sizeof(addr));
		acpt_sock = accept(accepter->fd, (struct sockaddr*)&addr, &addr_size);
		if(zbaselib_socket_isvalid(acpt_sock))
		{
			DECL_PRIV(accepter, priv);
			
			clnt = zevent_connector_new(1024, 1024, priv->opt);
			
			if(priv->on_accept != NULL)
				priv->on_accept(accepter, clnt, acpt_sock, &addr);
			
			zevent_connector_accept(clnt, acpt_sock, &addr);
			zevent_looper_add_event_unlock(default_looper, clnt);

			ZEVENT_PRINT_INFO("accept a clnt %d.\n", acpt_sock);
		}
		else if(!zbaselib_socket_accept_wouldblock())
		{
			zbaselib_socket_close(acpt_sock);
			return -1;
		}
	} while(!zbaselib_socket_accept_wouldblock());

	return 0;
}
/*
static void zevent_accepter_delete(zevent* accepter)
{

}
*/
static int zevent_accepter_listen(zevent* accepter)
{
	DECL_PRIV(accepter, priv);
	zbaselib_socket_t svr = zbaselib_socket_create_tcpserver(&priv->addr);
	if(!zbaselib_socket_isvalid(svr))
	{
		MESSAGE("[error] create tcp svr fail, code = %d\n", zbaselib_socket_geterror());
		return -1;
	}

	accepter->fd = svr;

	return 0;
}

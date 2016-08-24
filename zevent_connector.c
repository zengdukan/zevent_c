#include "zevent_connector.h"

// 接收buffer大小
#define ZEVENT_CONNECTOR_BUFFER_SIZE	256

// 等待连接成功时间，单位ms
#define ZEVENT_CONNECTOR_WAIT_CONNECT_TIME	2000

// socket读写等待时间，单位ms
#define ZEVENT_CONNECTOR_RW_TIMEOUT	100

typedef struct //priv_info_s
{
	void* param1;
	void* param2;

	int rd_buf_size;
	int wr_buf_size;
	zbaselib_skbuffer* rd_buf;
	zbaselib_skbuffer* wr_buf;

	int connected;		// 0:未连接， 1-连接
	struct sockaddr_in addr;

	zevent_connector_opt opt;
} priv_info;

static int zevent_connector_read(zevent* connector);

static int zevent_connector_write(zevent* connector);

static void zevent_connector_delete(zevent* connector);

static int connector_check_wr(zevent* connector);

zevent* zevent_connector_new(int rd_buf_size, int wr_buf_size, zevent_connector_opt opt)
{
	zevent* connector = (zevent*) calloc(1, sizeof(zevent) + sizeof(priv_info));

	if(connector != NULL)
	{
		DECL_PRIV(connector, priv);

		zevent_base_init(connector, ZEVENT_CONNECTOR_RW_TIMEOUT,
			ZEVENT_TYPE_READ|ZEVENT_TYPE_WRITE);
		
		connector->read_cb = zevent_connector_read;
		connector->write_cb = zevent_connector_write;
		connector->delete_cb = zevent_connector_delete;
		connector->check_rd_cb = NULL;
		connector->check_wr_cb = connector_check_wr;
		
//		priv->param1 = param1;
//		priv->param2 = param2;
		priv->rd_buf_size = rd_buf_size > ZEVENT_CONNECTOR_BUFFER_SIZE ? 
			rd_buf_size : ZEVENT_CONNECTOR_BUFFER_SIZE;
		priv->wr_buf_size = wr_buf_size > ZEVENT_CONNECTOR_BUFFER_SIZE ?
			wr_buf_size : ZEVENT_CONNECTOR_BUFFER_SIZE;
	
		priv->opt = opt;
		priv->connected = 0;
	}
	 
	return connector;
}

void zevent_connector_intial(zevent* connector, const struct sockaddr_in* addr,
	void* param1, void* param2)
{
	DECL_PRIV(connector, priv);

	if(addr != NULL)
		priv->addr = *addr;
	priv->param1 = param1;
	priv->param2 = param2;
}


int zevent_connector_connect(zevent* connector)
{
	zbaselib_socket_t clnt;
	DECL_PRIV(connector, priv);
	if(connector == NULL || priv == NULL)
		return -1;

	clnt = zbaselib_socket_create_tcpclient(&priv->addr);
	if(zbaselib_socket_isvalid(clnt) && 
		zbaselib_socket_waitforconnect(clnt, ZEVENT_CONNECTOR_WAIT_CONNECT_TIME) == 0)
	{
		ZEVENT_PRINT_INFO("connect to %s ok.\n", inet_ntoa(priv->addr.sin_addr));
		connector->fd = clnt;
		priv->rd_buf = zbaselib_skbuffer_create(priv->rd_buf_size, (priv->rd_buf_size<<3), 1, clnt);
		priv->wr_buf = zbaselib_skbuffer_create(priv->wr_buf_size, (priv->wr_buf_size<<3), 1, clnt);
		//priv->addr = *addr;
		priv->connected = 1;
		if(priv->opt.on_connect != NULL)
			priv->opt.on_connect(connector, 1);

		return 0;
	}
	else
	{
		zbaselib_socket_close(clnt);
		ZEVENT_PRINT_INFO("connect to %s fail.\n", inet_ntoa(priv->addr.sin_addr));
		return -1;
	}
}

int zevent_connector_accept(zevent* connector, int acpt_sock,
	const struct sockaddr_in* addr)
{
	DECL_PRIV(connector, priv);

	if(connector == NULL || priv == NULL)
		return -1;

	if(!zbaselib_socket_isvalid(acpt_sock))
		return -1;

	connector->fd = acpt_sock;
	priv->addr = *addr;
	priv->connected = 1;
	priv->rd_buf = zbaselib_skbuffer_create(priv->rd_buf_size, (priv->rd_buf_size<<3), 1, acpt_sock);
	priv->wr_buf = zbaselib_skbuffer_create(priv->wr_buf_size, (priv->wr_buf_size<<3), 1, acpt_sock);
	if(priv->opt.on_connect != NULL)
		priv->opt.on_connect(connector, 1);
	
	return 0;
}

int zevent_connector_close(zevent* connector)
{
	DECL_PRIV(connector, priv);

	if(connector == NULL || priv == NULL)
		return -1;

	ZEVENT_PRINT_INFO("connector close.\n");
	zbaselib_socket_close(connector->fd);

	connector->fd = INVALID_SOCKET;
	priv->connected = 0;

	if(priv->wr_buf != NULL)
	{
		zbaselib_skbuffer_destroy(priv->wr_buf);
		priv->wr_buf = NULL;
	}

	if(priv->rd_buf != NULL)
	{
		zbaselib_skbuffer_destroy(priv->rd_buf);
		priv->rd_buf = NULL;
	}

	if(priv->opt.on_connect != NULL)
		priv->opt.on_connect(connector, 0);

	return 0;
}

int zevent_connector_shutdown(zevent* connector, int flags)
{
	int how = 0;
	DECL_PRIV(connector, priv);

	if(connector == NULL || priv == NULL)
		return -1;

	ZEVENT_PRINT_INFO("connector shutdown flags = %d.\n", flags);
	if(flags & ZEVENT_CLOSEFLAG_READ)
		how |= ZBASELIB_SOCKET_SHUTDOWN_RD;
	if(flags & ZEVENT_CLOSEFLAG_WRITE)
		how |= ZBASELIB_SOCKET_SHUTDOWN_WR;
	if(flags == ZEVENT_CLOSEFLAG_BOTH)
		how = ZBASELIB_SOCKET_SHUTDOWN_BOTH;
	priv->connected = 0;

	zbaselib_socket_shutdown(connector->fd, how);

	if(priv->wr_buf != NULL)
	{
		zbaselib_skbuffer_destroy(priv->wr_buf);
		priv->wr_buf = NULL;
	}

	if(priv->rd_buf != NULL)
	{
		zbaselib_skbuffer_destroy(priv->rd_buf);
		priv->rd_buf = NULL;
	}

	if(priv->opt.on_connect != NULL)
		priv->opt.on_connect(connector, 0);

	return 0;
}

zbaselib_skbuffer* zevent_connector_get_rd_buf(zevent* connector)
{
	DECL_PRIV(connector, priv);

	if(connector == NULL || priv == NULL)
		return NULL;

	return priv->rd_buf;
}

zbaselib_skbuffer* zevent_connector_get_wr_buf(zevent* connector)
{
	DECL_PRIV(connector, priv);

	if(connector == NULL || priv == NULL)
		return NULL;

	return priv->wr_buf;
}

int zevent_connector_send_data(zevent* connector, const char* data, int size)
{
	int ret = 0;
	int count = 0;
	
	DECL_PRIV(connector, priv);

	if(connector == NULL || priv == NULL || data == NULL || size <= 0)
		return -1;

	do
	{
		ret = zbaselib_skbuffer_write(priv->wr_buf, data, size);
		if(ret == -2)
		{
			zbaselib_sleep_msec(20);
			count++;
		}
	} while(ret == -2 && count < 100);
	
	return ret;
}

int zevent_connector_is_connected(zevent* connector)
{
	DECL_PRIV(connector, priv);

	if(connector == NULL || priv == NULL)
		return 0;

	return priv->connected;
}

int zevent_connector_get_addr(zevent* connector, struct sockaddr_in* addr)
{
	DECL_PRIV(connector, priv);

	if(connector == NULL || priv == NULL || addr == NULL)
		return -1;

	*addr = priv->addr;

	return 0;
}

static int zevent_connector_read(zevent* connector)
{
	DECL_PRIV(connector, priv);
	int ret = 0;
	if(connector == NULL || priv == NULL)
		return -1;

	ret = zbaselib_skbuffer_recv(priv->rd_buf);
	if(ret == 0 || ret == -3)
	{
		if(((priv->opt.on_check_pack != NULL) && 
			priv->opt.on_check_pack(connector)) ||
			priv->opt.on_check_pack == NULL)
		{
			if(priv->opt.on_parse_data != NULL)
				ret = priv->opt.on_parse_data(connector);
		}
	}

	if(ret == -1)
	{
		zevent_connector_close(connector);
		priv->connected = 0;
		return -1;
	}

	return 0;
}

static int zevent_connector_write(zevent* connector)
{
	DECL_PRIV(connector, priv);
	int ret = 0;

	if(connector == NULL || priv == NULL)
		return -1;

	ret = zbaselib_skbuffer_send(priv->wr_buf);
	if(ret == -1)
	{
		zevent_connector_close(connector);
		priv->connected = 0;
		return -1;
	}

	return 0;
}

static void zevent_connector_delete(zevent* connector)
{
	DECL_PRIV(connector, priv);

	if(connector == NULL || priv == NULL)
		return;

	if(priv->wr_buf != NULL)
	{
		zbaselib_skbuffer_destroy(priv->wr_buf);
		priv->wr_buf = NULL;
	}

	if(priv->rd_buf != NULL)
	{
		zbaselib_skbuffer_destroy(priv->rd_buf);
		priv->rd_buf = NULL;
	}
}

static int connector_check_wr(zevent* connector)
{
	DECL_PRIV(connector, priv);

	return (!zbaselib_skbuffer_empty(priv->wr_buf));
}

void* zevent_connector_get_param1(zevent* connector)
{
	DECL_PRIV(connector, priv);
	return priv == NULL ? NULL : priv->param1;
}

void* zevent_connector_get_param2(zevent* connector)
{
	DECL_PRIV(connector, priv);
	return priv == NULL ? NULL : priv->param2;
}

void zevent_connector_set_param1(zevent* connector, void* param)
{
	DECL_PRIV(connector, priv);

	if(priv != NULL)
		priv->param1 = param;
}

void zevent_connector_set_param2(zevent* connector, void* param)
{
	DECL_PRIV(connector, priv);

	if(priv != NULL)
		priv->param2 = param;
}	

void zevent_connector_set_rdbuf_size(zevent* connector, int rd_buf_size)
{
	DECL_PRIV(connector, priv);

	if(priv != NULL)
		priv->rd_buf_size = rd_buf_size > ZEVENT_CONNECTOR_BUFFER_SIZE ? 
			rd_buf_size : ZEVENT_CONNECTOR_BUFFER_SIZE;
		
}

void zevent_connector_set_wrbuf_size(zevent* connector, int wr_buf_size)
{
	DECL_PRIV(connector, priv);

	if(priv != NULL)
		priv->wr_buf_size = wr_buf_size > ZEVENT_CONNECTOR_BUFFER_SIZE ?
			wr_buf_size : ZEVENT_CONNECTOR_BUFFER_SIZE;
}

void zevent_connector_set_opt(zevent* connector, zevent_connector_opt opt)
{
	DECL_PRIV(connector, priv);

	if(priv != NULL)
		priv->opt = opt;
}



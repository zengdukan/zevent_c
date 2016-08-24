#include "zevent_pipe.h"
#include "zevent_typedef.h"
#include "zbaselib_socket.h"
#include "zbaselib_thread.h"

/************************************************************************/
/* struct define                                                        */
/************************************************************************/
#define ZEVENT_PIPE_TIMEOUT	100			// 100ms
#define ZEVENT_PIPE_RDFD	pair[0]
#define ZEVENT_PIPE_WRFD	pair[1]
#define ZEVENT_PIPE_ERCV_BUFFER_SIZE	120

typedef struct priv_info_s
{
	zbaselib_socket_t pair[2];	// 0-read fd, 1-write fd
	zbaselib_mutex_t mutex;
	zbaselib_skbuffer* rd_buf;
	void* param1;
	void* param2;
	zevent_pipe_process_msg_cb process_msg_cb;
} priv_info;

typedef struct zevent_msg_s
{
	int data_size;
	int type;
	char data[0];
} zevent_msg_t;

/************************************************************************/
/* static function prev or var define                                   */
/************************************************************************/
static int zevent_pipe_read(zevent* pipe);

static void zevent_pipe_delete(zevent* pipe);

zevent* zevent_pipe_new(zevent_pipe_process_msg_cb process_msg)
{
	zevent* event = NULL;

	if(process_msg == NULL)
		return NULL;

	event = (zevent*) calloc(1, sizeof(zevent) + sizeof(priv_info));
	if(event != NULL)
	{
		DECL_PRIV(event, priv);
		
		zevent_base_init(event, ZEVENT_PIPE_TIMEOUT, ZEVENT_TYPE_READ);
		event->read_cb = zevent_pipe_read;
		event->write_cb = NULL;
		event->delete_cb = zevent_pipe_delete;
		event->check_rd_cb = NULL;
		event->check_wr_cb = NULL;
		
		priv->process_msg_cb = process_msg;

		if(zbaselib_socket_pair(priv->pair) == 0)
		{
			event->fd = priv->ZEVENT_PIPE_RDFD;
			zbaselib_socket_nonblocking(event->fd);
			priv->mutex = zbaselib_mutex_create();
			priv->rd_buf = zbaselib_skbuffer_create(ZEVENT_PIPE_ERCV_BUFFER_SIZE, (ZEVENT_PIPE_ERCV_BUFFER_SIZE<<3), 0, 
				priv->ZEVENT_PIPE_RDFD);
		}
		else
		{
			ZEVENT_PRINT_DEBUG("make pair fail.\n");
			zevent_delete(event);
			event = NULL;
		}
	}

	return event;
}

void zevent_pipe_intial(zevent* pipe, void* param1, void* param2)
{
	DECL_PRIV(pipe, priv);
	priv->param1 = param1;
	priv->param2 = param2;
}

int zevent_pipe_send_msg(zevent* pipe, void* data, int data_size, int msg_type)
{
	zevent_msg_t* msg = NULL;

	if(pipe != NULL)
	{
		DECL_PRIV(pipe, priv);

		msg = (zevent_msg_t*) calloc(1, sizeof(zevent_msg_t) + data_size);
		msg->type = msg_type;
		msg->data_size = data_size;
		if(data != NULL)
			memcpy(msg->data, data, data_size);

		zbaselib_mutex_lock(priv->mutex);
		zbaselib_socket_sendn(priv->ZEVENT_PIPE_WRFD, (char*)msg, sizeof(zevent_msg_t) + data_size);
		ZEVENT_PRINT_DEBUG("[%p]send message(%d, %p, %d)\n", pipe, msg->type, msg->data_size);
		zbaselib_mutex_unlock(priv->mutex);

		free(msg);
		return 0;
	}

	return -1;
}

static int zevent_pipe_read(zevent* pipe)
{
	int msg_size = sizeof(zevent_msg_t);
	int ret = 0;
	int size;
	zevent_msg_t* msg;

	DECL_PRIV(pipe, priv);
	ret = zbaselib_skbuffer_recv(priv->rd_buf);
	if(ret == -1 || ret == -2)
	{
		ZEVENT_PRINT_DEBUG("recv fail.\n");
		return -1;
	}

	while(zbaselib_skbuffer_datasize(priv->rd_buf) >= msg_size)
	{
		size = 0;
		zbaselib_skbuffer_get(priv->rd_buf, (char*)&size, 4);
		size += msg_size;

		if(zbaselib_skbuffer_datasize(priv->rd_buf) >= size)
		{
			msg = (zevent_msg_t*) calloc(1, size);
			zbaselib_skbuffer_read(priv->rd_buf, (char*)msg, size);
			priv->process_msg_cb(msg->data, msg->data_size, msg->type, priv->param1, priv->param2);
			free(msg);
		}
		else
			break;
	}
	
	return 0;
}

static void zevent_pipe_delete(zevent* pipe)
{
	if(pipe != NULL) 
	{
		DECL_PRIV(pipe, priv);
		zbaselib_socket_close(priv->ZEVENT_PIPE_WRFD);

		zbaselib_mutex_destroy(priv->mutex);
		zbaselib_skbuffer_destroy(priv->rd_buf);
	}
}


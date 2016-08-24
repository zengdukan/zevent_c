#ifndef _ZEVENT_PIPE_H_
#define _ZEVENT_PIPE_H_

#include "zevent_event.h"

ZBASELIB_BEGIN_DECL

/*
* add by hyzeng:2013-7-4
* 功能:管道处理函数，注意:其内不能调用加了系统锁函数
* 结果:0=成功，-1=失败
*/
typedef int (*zevent_pipe_process_msg_cb)(void* data, int data_size, int msg_type,
	void* param1, void* param2);

/*
* add by hyzeng:2013-7-4
* 功能:创建管道
* 结果:NULL=失败
*/
ZLIB_API zevent* zevent_pipe_new(zevent_pipe_process_msg_cb process_msg);

/*
* add by hyzeng:2013-7-4
* 功能:初始化pipe
* 结果:
*/
ZLIB_API void zevent_pipe_intial(zevent* pipe, void* param1, void* param2);

/*
* add by hyzeng:2013-7-4
* 功能:发送pipe数据
* 结果:0=成功，-1=失败
*/
ZLIB_API int zevent_pipe_send_msg(zevent* pipe, void* data, int data_size, int msg_type);

ZBASELIB_END_DECL

#endif/*_ZEVENT_PIPE_H_*/
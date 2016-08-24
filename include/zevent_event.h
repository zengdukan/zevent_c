#ifndef _ZEVENT_EVENT_H_
#define _ZEVENT_EVENT_H_

#include "zevent_typedef.h"
#include "zbaselib_socket.h"

ZBASELIB_BEGIN_DECL

typedef struct zevent_s zevent;

typedef int (*zevent_read_cb)(zevent* event);
typedef int (*zevent_write_cb)(zevent* event);
typedef void (*zevent_delete_cb)(zevent* event);

// reactor调用read/write发生错误，调用此函数，提供默认函数，打印输出err
typedef void (*zevent_error_cb)(zevent* event, int err);

typedef int (*zevent_check_read_cb)(zevent* event);

typedef int (*zevent_check_write_cb)(zevent* event);

struct zevent_s
{
	zbaselib_socket_t fd;
	int type;
	int err;	// 错误标志，0-无，其他-错误
	int isadd;	// 是否添加到looper
	int timeout;
	
	zevent_read_cb read_cb;
	zevent_write_cb write_cb;
	zevent_delete_cb delete_cb;
	
	zevent_error_cb error_cb;	// 每个event具体实现
	zevent_check_read_cb check_rd_cb;	// 是否有数据可读,有添加到select
	zevent_check_write_cb check_wr_cb;	// 是否有数据可写
	
	char priv[0];
};

/*
* add by hyzeng:2013-7-4
* 功能:读数据,事件如果设置read_cb则调用read_cb
       注意:此函数会被looper自动调用，不能在函数
       内阻塞，不能在函数内调用加锁函数
* 结果:0-成功，-1=失败
*/
ZLIB_API int zevent_read(zevent* event);

/*
* add by hyzeng:2013-7-4
* 功能:写事件,事件如果设置write_cb则调用write_cb
       注意:此函数会被looper自动调用，不能在函数
       内阻塞，不能在函数内调用加锁函数及被加锁函数
       调用
* 结果:
*/
ZLIB_API int zevent_write(zevent* event);

/*
* add by hyzeng:2013-7-4
* 功能:删除释放event,事件如果设置delete_cb会调用delete_cb
       注意:如果event被加入到looper，delete时会从looper删除
       ，zevent_looper_del_event是加锁的，所以此函数及其回
       调delete_cb不能被加锁函数调用，也不能调用加锁函数
       否则会死锁
* 结果:
*/
ZLIB_API void zevent_delete(zevent* event);

/*
* add by hyzeng:2013-7-4
* 功能:回调error_cb, err-错误码。
	   回调error_cb不能调用加锁函数
       否则会死锁
* 结果:
*/
ZLIB_API void zevent_error(zevent* event, int err);

/*
* add by hyzeng:2013-7-4
* 功能:检验是否有数据读
* 结果:1=有，0=无
*/
ZLIB_API int zevent_check_read(zevent* event);

/*
* add by hyzeng:2013-7-4
* 功能:检测是否有数据写
* 结果:1=有，0=无
*/
ZLIB_API int zevent_check_write(zevent* event);

/*
* add by hyzeng:2013-7-4
* 功能:返回socket
* 结果:
*/
ZLIB_API zbaselib_socket_t zevent_get_fd(zevent* event);

ZLIB_API int zevent_get_type(zevent* event);

ZLIB_API int zevent_get_timeout(zevent* event);

ZLIB_API int zevent_get_error(zevent* event);

/*
* add by hyzeng:2013-7-4
* 功能:初始化event基本属性，通常被event子类的init函数调用
* 结果:
*/
ZLIB_API void zevent_base_init(zevent* event, int timeout, int type);

/*
* add by hyzeng:2013-7-4
* 功能:销毁event基本属性，被zevent_delete调用
* 结果:
*/
ZLIB_API void zevent_base_uninit(zevent* event);

/*
* add by hyzeng:2013-7-4
* 功能:设置error_cb回调
* 结果:
*/
ZLIB_API void zevent_set_error_cb(zevent* event, zevent_error_cb cb);

ZBASELIB_END_DECL

#endif
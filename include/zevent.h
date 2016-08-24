/************************************************************************/
// 1.处理事件何时移除和删除释放空间
// 2.标志位的控制
// 3.错误发生时怎么删除事件
// 4.connecter里的关闭操作要注意同步，可能要加锁
/************************************************************************/
#ifndef _ZEVENT_H_
#define _ZEVENT_H_


#include "zevent_connector.h"
#include "zevent_accepter.h"
#include "zevent_event.h"
#include "zevent_looper.h"
#include "zevent_pipe.h"
#include "zevent_reactor.h"
#include "zevent_timer.h"
#include "zevent_typedef.h"
#include "zbaselib_typedef.h"

ZBASELIB_BEGIN_DECL

// 更新时间
ZLIB_API void zevent_update_current_time();

//获取当前系统时间毫秒值
ZLIB_API llong zevent_get_current_time();

/*
初始化模块:
1.初始化socket
2.创建default_looper
*/
ZLIB_API int zevent_init();

/*
注销模块:
1.注销socket
2.销毁default_looper
*/
ZLIB_API void zevent_deinit();

// 获取default_looper
ZLIB_API zevent_looper* zevent_get_default_looper();

ZBASELIB_END_DECL

#endif

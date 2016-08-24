#ifndef _ZEVENT_TYPEDEF_H_
#define _ZEVENT_TYPEDEF_H_

#include "zbaselib_typedef.h"

typedef enum zevent_type
{
	ZEVENT_TYPE_READ = 0x01,		// 可读事件
	ZEVENT_TYPE_WRITE = 0x02,		// 可写事件
	ZEVENT_TYPE_TIMER = 0x04,		// 定时事件，与读、写互斥
} zevent_type;

typedef enum zevent_closeflag
{
	ZEVENT_CLOSEFLAG_READ = 0x01,
	ZEVENT_CLOSEFLAG_WRITE = 0x02,
	ZEVENT_CLOSEFLAG_BOTH = 0x03,
} zevent_closeflag;

#define FREE_PTR_OBJ(ptrobj)	if(ptrobj != NULL) {free(ptrobj); ptrobj = NULL;}

#define DECL_PRIV(thiz, priv)  priv_info* priv = thiz != NULL ? (priv_info*)thiz->priv : NULL

#define ZEVENT_INFO		1
#define ZEVENT_DEBUG	2

#define ZEVENT_LEVEL	0

#if (ZEVENT_LEVEL >= ZEVENT_DEBUG)
#define ZEVENT_PRINT_INFO(fmt, ...)		printf("[INFO (%s:%d)]: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ZEVENT_PRINT_DEBUG(fmt, ...)	printf("[DEBUG (%s:%d)]: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#elif (ZEVENT_LEVEL >= ZEVENT_INFO)
#define ZEVENT_PRINT_INFO(fmt, ...)		printf("[INFO (%s:%d)]: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ZEVENT_PRINT_DEBUG(fmt, ...)
#else
#define ZEVENT_PRINT_INFO(fmt, ...)
#define ZEVENT_PRINT_DEBUG(fmt, ...)
#endif


#endif
